#pragma once
#include "obj.h"
#include "mesh.h"

class obj_list {
public:
	obj_list() {}
	~obj_list() { clear(); }
	aabb bbox;
	vector<mesh_var> objects;
	vector<mesh_raw> obj_bvh;
	vector<bvh_node> bvh;
	vector<mat_var> materials;
	vector<uint> lights;
	vector<uint> nonbvh;
	float lw = 0;
	bool en_bvh = 1;
	bool bvh_lin = 1;
	bool march = 0;
	template <bool secondary = false>
	__forceinline bool hit(const ray& sr, hitrec& rec) const {
		ray r = sr;
		if (secondary ? !bbox.hit(r) :!bbox.shift(r)) return false;
		uchar any_hit = false;
		if (en_bvh) {
			for (const auto& obj : nonbvh)
				any_hit |= objects[obj].hit(r, rec);
			if (bvh[0].bbox.hit(r))
				any_hit |= traverse_bvh(r, rec, 0);
		}
		else {
			for (auto& obj : objects)
				any_hit |= obj.hit(r, rec);
		}
		if (!secondary && any_hit)rec.t += (r.O - sr.O).len();
		return any_hit;
	}
	__forceinline float pdf(const ray& r)const {
			float y = 0.f;
			for (const auto& light : lights)
				y += objects[light].pdf(r);
			return y * lw;
	}
	__forceinline vec3 rand_to(vec3 O) const {
		uint id = raint(lights.size() - 1);
		const uint light = lights[id];
		return objects[light].rand_to(O);
	}
	__forceinline ray rand_from(uint& light) const {
		uint id = raint(lights.size() - 1);
		light = lights[id];
		return objects[light].rand_from();
	}

	__forceinline float pdf(const ray& r, uint light)const {
		return objects[light].pdf(r);
	}

	__forceinline float bvh_pdf(const ray& r, uint id = 0) const {
		const bvh_node& node = bvh[id];
		float l = 0;
		if (node.parent)
		{
			if (bvh[node.n1].bbox.hit(r)) l += bvh_pdf(r, node.n1);
			if (bvh[node.n2].bbox.hit(r)) l += bvh_pdf(r, node.n2);
		}
		else {
			for (uint i = node.n1; i < node.n2; i++)
			{
				if (objects[obj_bvh[i].obje_id].light())
				{
					l += obj_bvh[i].pdf(objects.data(), r);
				}
			}
		}
		return l;
	}

	__forceinline uchar traverse_bvh(const ray& r, hitrec& rec, uint id = 0) const {
		const bvh_node& node = bvh[id];
		if (node.parent)
		{
			float t1 = rec.t;
			float t2 = rec.t;
			bool h1 = bvh[node.n1].bbox.hit(r, t1);
			bool h2 = bvh[node.n2].bbox.hit(r, t2);
			if (h1 && h2) {
#if 1
				bool swap = t2 < t1;
				uint n1 = swap ? node.n2 : node.n1;
				uint n2 = swap ? node.n1 : node.n2;
				float t = swap ? t1 : t2;
				uchar first = traverse_bvh(r, rec, n1);
				return rec.t < t ? first : traverse_bvh(r, rec, n2) | first;
#else
				if (t1 <= t2) return traverse_bvh(r, rec, node.n1) | traverse_bvh(r, rec, node.n2);
				else return traverse_bvh(r, rec, node.n2) | traverse_bvh(r, rec, node.n1);
#endif
			}
			else if (h1)return traverse_bvh(r, rec, node.n1);
			else if (h2)return traverse_bvh(r, rec, node.n2);
			else return false;
		}
		else {
			uchar any_hit = 0;
			for (uint i = node.n1; i < node.n2; i++)
				any_hit |= obj_bvh[i].hit(objects.data(), r, rec);
			return any_hit;
		}
	}
	//Declarations
	void clear();
	int get_id(const ray& r, hitrec& rec) const;
	void get_trans(const int id, mat4& T) const;
	obj_flags get_flag(int id) const;
	void set_trans(const int id, const mat4& T, uint node_size = 2);
	void fit();
	void update_lights();
	void update_nonbvh(uint node_size);
	void update_lists();
	void update_all();
	void update_bvh(bool print = 0, uint node_size = 2);
	void rebuild_bvh(bool print = 0, uint node_size = 2);
	void bvh_builder(bool print, uint node_size);
	void obj_create();
	void obj_update();
	void build_bvh(bool print = 1, uint node_size = 2);
	uint sort(uint be, uint en, uchar axis, float plane);
	float split_cost(uint be, uint en, uint& split, aabb bbox);
	void split_bvh(uint parent,uint node_size);
	void split_bvh(uint be, uint en, uint node_size);
	//void split_bvh2(aabb bbox, uint be, uint en, uint node_size, uint parent);
	aabb box_from(uint begin, uint end);
	//uint nodes = 1;

	void add_mat(const albedo& tex, const mat_enum type) {
		add_mat(mat_var(tex, type));
	}
	void add_mat(const mat_var& mat) {
		materials.emplace_back(mat);
	}
	template <typename T>
	void add_mesh(const T& object, vec3 pos = 0, float scl = 1, uint mat = 0, bool skip_bvh = 0, bool is_light = 0, bool has_fog = 0)
	{
		add_obj(mesh<T>(object, mat), vec3(pos, scl), skip_bvh, is_light, has_fog);
	}
	template <typename T>
	void add_mesh(const vector<T>& object, vec3 pos = 0, float scl = 1, uint mat = 0, bool skip_bvh = 0, bool is_light = 0, bool has_fog = 0)
	{
		add_obj(mesh<T>(object, mat), vec3(pos, scl), skip_bvh, is_light, has_fog);
	}
	void remove_mesh(int id) {
		if (id >= 0&&id<objects.size()) {
			objects.erase(objects.begin()+id);
			update_all();
		}
	}
private:
	template <typename T>
	void add_obj(mesh<T> object, vec3 offset, bool skip_bvh = 0, bool is_light = 0, bool has_fog = 0)
	{
		add_obj(object.set_trans(offset), skip_bvh, is_light, has_fog);
	}
	template <typename T>
	void add_obj(const mesh<T>& object, bool skip_bvh = 0, bool is_light = 0, bool has_fog = 0)
	{
		if (skip_bvh)nonbvh.emplace_back(objects.size());
		if (is_light)lights.emplace_back(objects.size());
		objects.emplace_back(object, !skip_bvh, is_light, has_fog);
		bbox.join(object.get_box());
		lw = 1.f / lights.size();
	}
};

