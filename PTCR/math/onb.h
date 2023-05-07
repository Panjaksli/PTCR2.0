#pragma once
#include "vec4.h"
#if DEBUG
extern bool use_normal_maps;
#else
constexpr bool use_normal_maps = 1;
#endif
class onb
{
public:
	onb() {}
	onb(const vec4& n) { build(n); }
	inline vec4 operator[](int i) const { return uvw[i]; }
	inline vec4& operator[](int i) { return uvw[i]; }

	__forceinline vec4 world(const vec4& a) const {
		return a.x() * u + a.y() * v + a.z() * w;
	}
#if USE_SSE
	__forceinline vec4 local(const vec4& a)const {
		__m128 x = u.xyz, y = v.xyz, z = w.xyz, w{};
		_MM_TRANSPOSE4_PS(x,y,z,w);
		return a[0] * x + a[1] * y + a[2] * z;
	}
#else
	__forceinline vec4 local(const vec4& a)const {
		return vec4(dot(a, u), dot(a, v), dot(a, w));
	}
#endif
	/*
	branchlessONB
	https://graphics.pixar.com/library/OrthonormalB/paper.pdf
	*/
	__forceinline void build(const vec4& n) {
		float sign = signf(n.z());
		float a = 1.f / (sign + n.z());
		vec4 s = vec4(a, a, 1.f, 1.f);
		vec4 xy = vec4(n.x(), n.y(), 1.f, 1.f);
		vec4 sxy = s * xy;
		u = vec4(1, 0, 0, 0) - sign * n.x() * sxy;
		v = vec4(0, sign, 0, 0) - n.y() * sxy;
		w = n;
	}
	void print() {
		u.print();
		v.print();
		w.print();
	}
	union {
		struct {
			vec4 u, v, w;
		};
		vec4 uvw[3];
	};
};
//map is texture value (0 to 1)
__forceinline vec4 normal_map(vec4 N, vec4 map)
{
	if (!use_normal_maps || eq(map, vec4(0.5f, 0.5f, 1.f)))return N;
	onb uvw(N);
	map = 2.f * map - 1.f;
	return norm(uvw.world(map));
}
