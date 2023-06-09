#pragma once
#include "sphere.h"
#include "poly.h"
#include "quad.h"
#include "voxel.h"
vector<poly> load_mesh(const char* filename, vec4 off = 0, float scale = 1.f, bool flip_face = 0);
vector<poly> load_MSH(path name, vec4 off = 0, float scale = 1.f, bool flip_face = 0);
bool OBJ_to_MSH(path name);
template <class primitive>
class mesh {
public:
	mesh() {}
	mesh(const primitive& _prim, uint _mat) : prim(new primitive[1]{ _prim }), mat(_mat), size(1) {
		fit();
	}
	mesh(const vector<primitive>& _prim, uint _mat) : prim(new primitive[_prim.size()]), mat(_mat), size(_prim.size()) {
		copy(_prim.data());
		fit();
	}
	mesh(const mesh& cpy) : prim(new primitive[cpy.size]) {
		bbox = cpy.bbox, P = cpy.P, A = cpy.A, mat = cpy.mat, size = cpy.size;
		copy(cpy.prim.get());
	}
	mesh(mesh&& cpy)noexcept :mesh() {
		swap(*this, cpy);
	}
	mesh& operator=(mesh cpy) {
		swap(*this, cpy);
		return *this;
	}
	__forceinline bool hit(const ray& r, hitrec& rec) const {
		uchar any_hit = false;
		for (uint i = 0; i < size; i++)
			any_hit |= prim[i].move(P).hit(r, rec);
		return any_hit;
	}
	__forceinline bool hit(const ray& r, hitrec& rec, uint prim_id) const {
		bool any_hit = prim[prim_id].move(P).hit(r, rec);
		return any_hit;
	}
	__forceinline float pdf(const ray& r, uint prim_id)const {
		return prim[prim_id].move(P).pdf(r) / size;
	}
	__forceinline float pdf(const ray& r)const {
		if (size == 1)return prim[0].move(P).pdf(r);
		float y = 0.f;
		float lw = 1.f / size;
		for (uint i = 0; i < size; i++)
			y += lw * prim[i].move(P).pdf(r);
		return y;
	}
	__forceinline vec4 rand_to(vec4 O) const {
		uint id = raint(size - 1);
		return prim[id].move(P).rand_to(O);
	}
	__forceinline ray rand_from() const {
		uint id = raint(size - 1);
		return prim[id].move(P).rand_from();
	}

	inline aabb get_box()const { return bbox; }
	inline aabb get_box(uint prim_id)const { return prim[prim_id].move(P).get_box(); }
	inline vector<primitive> get_data()const {
		return vector<primitive>(prim, prim + size);
	}
	inline primitive* get_ptr(uint i) const {
		return &(prim[i]);
	}
	inline primitive get_data(uint i) const {
		return prim[i].move(P);
	}
	inline primitive get_raw(uint i) const {
		return prim[i];
	}
	inline uint get_mat()const {
		return mat;
	}
	inline uint get_size()const {
		return size;
	}
	inline mat4 get_trans()const {
		return mat4(P, A);
	}
	inline mesh& set_trans(const mat4& T) {
		P = T.P();
		A = T.A();
		fit();
		return *this;
	}
	inline mesh& transform(const mat4& T, bool deform = 1) {
		mat4 dT(0, T.A() - A);
		P = T.P();
		A = T.A();
		if (deform) {
			for (uint i = 0; i < size; i++)
				prim[i] = prim[i].trans(dT);
		}
		fit();
		return *this;
	}
	inline void set_mat(uint _mat) {
		mat = _mat;
	}
	friend void swap(mesh& m1, mesh& m2) {
		std::swap(m1.bbox, m2.bbox);
		std::swap(m1.P, m2.P);
		std::swap(m1.A, m2.A);
		std::swap(m1.prim, m2.prim);
		std::swap(m1.mat, m2.mat);
		std::swap(m1.size, m2.size);
	}
private:
	inline void fit() {
		bbox = aabb();
		for (uint i = 0; i < size; i++)
			bbox.join(get_box(i));
	}
	inline void copy(const primitive* cpy) {
		for (uint i = 0; i < size; i++)
			prim[i] = cpy[i];
	}
	aabb bbox;
	vec4 P, A;
	std::unique_ptr<primitive[]> prim;
	uint mat = -1;
	uint size = 0;
};

using pmesh = mesh<poly>;
using qmesh = mesh<quad>;
using smesh = mesh<sphere>;
using vmesh = mesh<voxel>;
/*
SSS -> SAVING SOURCE SPACE
Brought to you by laziness 101
*/
#define SEL_MSH(id,lhs,rhs,def)\
switch(id){ \
    case o_pol: lhs p.rhs; break; \
    case o_qua: lhs q.rhs; break; \
    case o_sph: lhs s.rhs; break; \
    case o_vox: lhs v.rhs; break; \
    default: lhs def; break;\
} 

struct mesh_var {
	mesh_var() {}
	mesh_var(const char* name, mat4 T, uint mat, bool bvh = 1, bool lig = 0, bool fog = 0) : p(load_mesh(name), mat), name(name), flag(o_pol, bvh, lig, fog) { p.transform(T); }
	mesh_var(const mesh<poly>& m, bool bvh = 0, bool lig = 0, bool fog = 0, const char* name = nullptr) :p(m), name(name), flag(o_pol, bvh, lig, fog) {}
	mesh_var(const mesh<quad>& m, bool bvh = 0, bool lig = 0, bool fog = 0, const char* name = nullptr) :q(m), name(name), flag(o_qua, bvh, lig, fog) {}
	mesh_var(const mesh<sphere>& m, bool bvh = 0, bool lig = 0, bool fog = 0, const char* name = nullptr) :s(m), name(name), flag(o_sph, bvh, lig, fog) {}
	mesh_var(const mesh<voxel>& m, bool bvh = 0, bool lig = 0, bool fog = 0, const char* name = nullptr) :v(m), name(name), flag(o_vox, bvh, lig, fog) {}
	mesh_var(const mesh_var& cpy) : name(cpy.name), flag(cpy.flag) {
		switch (type()) { //Assignment operator doesn't work, since union is not initialized...
		case o_pol: new(&p) auto(cpy.p); break;
		case o_qua: new(&q) auto(cpy.q); break;
		case o_sph: new(&s) auto(cpy.s); break;
		case o_vox: new(&v) auto(cpy.v); break;
		default: break;
		}
	}
	mesh_var& operator=(mesh_var cpy) {
		swap(*this, cpy);
		return *this;
	}
	mesh_var(mesh_var&& cpy)noexcept : mesh_var() {
		swap(*this, cpy);
	}
	~mesh_var() {
		//Destructor DOES matter here (each type holds a different pointer, thus correct delete[] method shall be called !)
		//Also handles blank objects.
		//Doesn't leak memory, for now :)
		SEL_MSH(type(), , ~mesh(), );
	}

	__forceinline bool hit(const ray& r, hitrec& rec) const {
		if (!p.get_box().hit(r))return false;
		SEL_MSH(type(), return, hit(r, rec), false);
	}

	__forceinline bool hit(const ray& r, hitrec& rec, uint prim_id) const {
		SEL_MSH(type(), return, hit(r, rec, prim_id), false);
	}

	__forceinline float pdf(const ray& r, uint prim_id) const {
		SEL_MSH(type(), return, pdf(r, prim_id), 0);
	}

	mat4 get_trans()const {
		SEL_MSH(type(), return, get_trans(), mat4());
	}
	const mesh_var& transform(const mat4& T) {
		SEL_MSH(type(), , transform(T), );
		return *this;
	}
	//Does not matter which variable is used, they all have the same memory layout ?
	void set_mat(uint mat) {
		return p.set_mat(mat);
	}
	uint get_size()const {
		return p.get_size();
	}
	uint get_mat()const {
		return p.get_mat();
	}
	aabb get_box()const {
		return p.get_box();
	}
	aabb get_box(uint prim_id)const {
		SEL_MSH(type(), return, get_box(prim_id), aabb());
	}
	__forceinline float pdf(const ray& r)const {
		if (!p.get_box().hit(r))return 0;
		SEL_MSH(type(), return, pdf(r), 0);
	}
	__forceinline vec4 rand_to(vec4 O) const {
		SEL_MSH(type(), return, rand_to(O), 0);
	}
	__forceinline ray rand_from() const {
		SEL_MSH(type(), return, rand_from(), ray());
	}
	const char* get_name() const {
		if (!name.empty())return name;
		else return obj_enum_str(type());
	}
	bool light() const {
		return flag.lig();
	}
	bool bvh() const {
		return flag.bvh();
	}
	bool fog() const {
		return flag.fog();
	}
	obj_enum type() const {
		return flag.type();
	}
	//one way swap m1 gets type of m2. Thus m2 can become blank
	friend void swap(mesh_var& m1, mesh_var& m2) {
		switch (m2.type()) {
		case o_pol: swap(m1.p, m2.p); break;
		case o_qua: swap(m1.q, m2.q); break;
		case o_sph: swap(m1.s, m2.s); break;
		case o_vox: swap(m1.v, m2.v); break;
		default: break;
		}
		swap(m1.name, m2.name);
		std::swap(m1.flag, m2.flag);
	}
	union {
		mesh<poly> p;
		mesh<quad> q;
		mesh<sphere> s;
		mesh<voxel> v;
	};
	c_str name;
	obj_flags flag;
};

struct mesh_raw {
	mesh_raw(aabb bbox, uint obje_id, uint prim_id) : obje_id(obje_id), prim_id(prim_id) {}
	__forceinline bool hit(const mesh_var* obj, const ray& r, hitrec& rec) const {
		if (obj[obje_id].hit(r, rec, prim_id)) {
			rec.idx = obje_id;
			return true;
		}
		return false;
	}
	__forceinline float pdf(const mesh_var* obj, const ray& r) const {
		return obj[obje_id].pdf(r, prim_id);
	}
	inline aabb get_box(const mesh_var* obj)const {
		return obj[obje_id].get_box(prim_id);
	}
	uint obje_id;
	uint prim_id;
};



