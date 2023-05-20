#pragma once
#include "ray.h"
#include "mat4.h"
#include "aabb.h"
#include "onb.h"

extern bool en_bvh;
enum obj_enum {
	o_pol, o_qua, o_sph, o_vox, o_bla
};

inline const char* obj_enum_str(int val, bool mesh = 0) {
	switch (val) {
	case o_pol: return "Polygon";
	case o_qua: return "Quad";
	case o_sph: return "Sphere";
	case o_vox: return "Voxel";
	default: return mesh ? "Mesh" : "Blank";
	};
}

struct obj_flags {
	obj_flags() {}
	obj_flags(obj_enum type, bool bvh, bool lig, bool fog = 0) : flag((type & 0x0F) | (bvh << 7) | (lig << 6) | (fog << 5)) {}
	operator obj_enum() const { return obj_enum(flag & 0x0F); }
	void set_bvh(bool x) {
		flag &= ~(1 << 7);
		flag |= x << 7;
	}
	void set_lig(bool x) {
		flag &= ~(1 << 6);
		flag |= x << 6;
	}
	void set_fog(bool x) {
		flag &= ~(1 << 5);
		flag |= x << 5;
	}
	void set_type(obj_enum x) {
		flag &= 0xF0;
		flag |= x & 0x0F;
	}
	bool bvh() const {
		return flag & 0x80;
	}
	bool lig() const {
		return flag & 0x40;
	}
	bool fog() const {
		return flag & 0x20;
	}
	obj_enum type() const {
		return obj_enum(flag & 0x0F);
	}
	uchar flag = 0;
};

struct bvh_node {
	bvh_node() {}
	bvh_node(aabb bbox, uint n1, uint n2, bool parent = 0) :bbox(bbox), n1(n1), n2(n2), parent(parent) {}
	aabb bbox;
	uint n1, n2;
	bool parent;
};

struct hitrec {
	vec4 N, P;
	float t = infp;
	float u, v;
	uint idx = -1;
	bool face;
};

#include "material.h"