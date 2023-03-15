#pragma once
#include "vec3.h"
#if DEBUG
extern bool use_normal_maps;
#else
constexpr bool use_normal_maps = 1;
#endif
class onb
{
public:
	onb() {}
	onb(const vec3& n) { build(n); }
	inline vec3 operator[](int i) const { return uvw[i]; }
	inline vec3& operator[](int i) { return uvw[i]; }

	__forceinline vec3 world(const vec3& a) const {
		return a.x() * u + a.y() * v + a.z() * w;
	}
	__forceinline vec3 local(const vec3& a)const {
		return vec3(dot(a, u), dot(a, v), dot(a, w));
	}
	/*
	branchlessONB
	https://graphics.pixar.com/library/OrthonormalB/paper.pdf
	*/
	__forceinline void build(const vec3& n) {
		float sign = signf(n.z());
		float a = 1.f / (sign + n.z());
		vec3 s = vec3(a, a, 1.f, 1.f);
		vec3 xy = vec3(n.x(), n.y(), 1.f, 1.f);
		vec3 sxy = s * xy;
		u = vec3(1, 0, 0, 0) - sign * n.x() * sxy;
		v = vec3(0, sign, 0, 0) - n.y() * sxy;
		w = n;
	}
	void print() {
		u.print();
		v.print();
		w.print();
	}
	union {
		struct {
			vec3 u, v, w;
		};
		vec3 uvw[3];
	};
};
//map is texture value (0 to 1)
__forceinline vec3 normal_map(vec3 N, vec3 map)
{
	if (!use_normal_maps || eq(map, vec3(0.5f, 0.5f, 1.f)))return N;
	onb uvw(N);
	map = 2.f * map - 1.f;
	return norm(uvw.world(map));
}
