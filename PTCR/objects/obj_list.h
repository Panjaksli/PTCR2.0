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
	float lw_tot = 0;
	bool en_bvh = 1;
	bool bvh_lin = 1;
	bool march = 0;
	//secondary rays dont need to be shifted, as the probably originate inside the bbox of scene

	template <bool secondary = false>
	__forceinline bool hit(const ray& sr, hitrec& rec) const {
		ray r = sr;
		float dt = 0;
		if (secondary ? bbox.hit(r) : bbox.shift(r, dt)){
		if (en_bvh) {
			for (const auto& obj : nonbvh)
				rec.idx = objects[obj].hit(r, rec) ? obj : rec.idx;
			if (bvh[0].bbox.hit(r)) traverse_bvh(r, rec, 0);
		}
		else {
			for (uint obj = 0; obj < objects.size(); obj++) {
				rec.idx = objects[obj].hit(r, rec) ? obj : rec.idx;
			}
		}
		}
		if (!secondary) rec.t += dt;
		return rec.idx != -1;
	}
	__forceinline float pdf(const ray& r)const {
		float y = 0.f;
		if (en_bvh) {
			for (const auto& light : lights)
				if (!objects[light].bvh())y += objects[light].pdf(r);
			return (y + bvh_pdf(r)) * lw_tot;
		}
		else {
			for (const auto& light : lights) {
				y += objects[light].pdf(r);
			}
			return y * lw_tot;
		}


	}
	__forceinline bool visible(const ray& r, uint idx)const {
		hitrec rec;
		return hit(r, rec) && rec.idx == idx;
	}
	__forceinline vec3 rand_to(vec3 O) const {
		uint id = raint(lights.size() - 1);
		const uint light = lights[id];
		return rand_idx(O, light);
	}
	__forceinline vec3 rand_to(vec3 O, uint& idx) const {
		uint id = raint(lights.size() - 1);
		idx = lights[id];
		return rand_idx(O, idx);
	}
	__forceinline vec3 rand_idx(vec3 O, uint idx) const {
		return objects[idx].rand_to(O);
	}
	__forceinline ray rand_from(uint& light) const {
		uint id = raint(lights.size() - 1);
		light = lights[id];
		return objects[light].rand_from();
	}

	void fit();
	void clear();
	void update_lights();
	void update_nonbvh();
	void update_lists();
	void update_all(uint node_size = 2);
	void build_bvh(bool print = 1, uint node_size = 2);
	void rebuild_bvh(bool print = 0, uint node_size = 2);
	void update_bvh(bool print = 0, uint node_size = 2);
	void bvh_builder(bool print, uint node_size);
	uint get_id(const ray& r, hitrec& rec) const;
	void get_trans(uint id, mat4& T) const;
	obj_flags get_flag(uint id) const;
	void set_trans(uint id, const mat4& T, uint node_size = 2);

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
	void remove_mat(uint id) {
		if (id < materials.size()) {
			materials.erase(materials.begin() + id);
			for (auto& obj : objects) {
				if (obj.get_mat() >= id && obj.get_mat() != -1)obj.set_mat(obj.get_mat() - 1);
			}
		}
	}
	void duplicate_mat(uint id) {
		if (id < materials.size())
			materials.push_back(materials[id]);
	}
	void remove_mesh(uint id) {
		if (id < objects.size()) {
			objects.erase(objects.begin() + id);
			update_all();
		}
	}
	void duplicate_mesh(uint id) {
		if (id < objects.size()) {
			objects.push_back(objects[id]);
			update_all();
		}
	}
private:
	void obj_create();
	void obj_update();
	uint sort(uint be, uint en, uchar axis, float plane);
	float split_cost(uint be, uint en, uint& split, aabb bbox);
	void split_bvh(uint parent, uint node_size);
	void split_bvh(uint be, uint en, uint node_size);
	aabb box_from(uint begin, uint end);

	template <typename T>
	void add_obj(mesh<T> object, vec3 offset, bool skip_bvh = 0, bool is_light = 0, bool has_fog = 0)
	{
		if (object.size > 0)
			add_obj(object.set_trans(offset), skip_bvh, is_light, has_fog);
	}
	template <typename T>
	void add_obj(const mesh<T>& object, bool skip_bvh = 0, bool is_light = 0, bool has_fog = 0)
	{
		if (skip_bvh)nonbvh.emplace_back(objects.size());
		if (is_light)lights.emplace_back(objects.size());
		objects.emplace_back(object, !skip_bvh, is_light, has_fog);
		bbox.join(object.get_box());
		lw_tot = 1.f / lights.size();
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
	//ordered bvh traversal
	__forceinline uchar traverse_bvh(const ray& r, hitrec& rec, uint n0 = 0) const {
		const bvh_node& node = bvh[n0];
		if (node.parent)
		{
			float t1 = rec.t;
			float t2 = rec.t;
			bool h1 = bvh[node.n1].bbox.hit(r, t1);
			bool h2 = bvh[node.n2].bbox.hit(r, t2);
			if (h1 && h2) {
				bool swap = t2 < t1;
				uint n1 = swap ? node.n2 : node.n1;
				uint n2 = swap ? node.n1 : node.n2;
				float t = swap ? t1 : t2;
				uchar first = traverse_bvh(r, rec, n1);
				return rec.t < t ? first : traverse_bvh(r, rec, n2) | first;
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
};

