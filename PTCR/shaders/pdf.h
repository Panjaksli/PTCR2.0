#pragma once
#include "vec4.h"
#include "obj_list.h"

class ggx_pdf {
public:
	ggx_pdf() {}
	ggx_pdf(vec4 _N, vec4 _V, vec4 _L, float _a) : N(_N), V(_V), L(_L), a(_a) {}
	inline float value(vec4 _L) const {
		//this breaks down under steep angles...
		vec4 H = norm(V + _L);
		float NoH = dot(N, H);
		return  DGGX(NoH, a) * NoH / (4.f * absdot(H, V));
	}
	inline vec4 generate() const {
		return L;
	}
	vec4 N, V, L;
	float a;
};
class cos_pdf {
public:
	cos_pdf() {}
	cos_pdf(vec4 _N, vec4 _L) : N(_N), L(_L) {}

	inline float value(vec4 V) const {
		return dot(N, V) * ipi;
	}
	inline vec4 generate() const {
		return L;
	}
	vec4 N, L;
};
class sph_pdf {
public:
	sph_pdf() {}
	inline float value(vec4 V) const {
		return 1.f / (4.f * pi);
	}
	inline vec4 generate() const {
		return sa_sph();
	}
};
class lig_pdf {
public:
	lig_pdf(const obj_list& world, vec4 _O) : world(world), O(_O) {}
	inline float value(vec4 V) const {
		return world.pdf(ray(O, V));
	}
	inline vec4 generate() const {
		return world.rand_to(O);
	}
	inline vec4 generate(uint &idx) const {
		return world.rand_to(O,idx);
	}
	const obj_list& world;
	vec4 O;
};

constexpr float sun_angle = 11.889f;
const float sun_maxdp = sun_angle / sqrtf(2.f + sun_angle * sun_angle);
class sun_pdf {
public:
	sun_pdf(const mat4& _T, vec4 _O) : T(_T), O(_O) {}
	inline float value(vec4 V) const {
		//a bit of geometry
		float dp = dot(V, T.vec(vec4(0, 1, 0)));
		constexpr float pdf = sun_angle * sun_angle * ipi;
		if (dp < sun_maxdp)return 0;
		return pdf / dp;
	}
	inline vec4 generate() const {
		//simplify sun projection as simple cone sampling
		vec4 r = sa_disk();
		vec4 V = T.vec(norm(vec4(r[0], sun_angle, r[1])));
		return V;
	}
	const mat4& T;
	vec4 O;
};
template <class P1, class P2>
class mix_pdf {
public:
	mix_pdf(const P1& _p1, const P2& _p2) :p1(_p1), p2(_p2) {}
	inline float value(vec4 V) const {
		return  0.5f * (p1.value(V) + p2.value(V));
	}
	inline vec4 generate() const {
		if (rafl() < 0.5f)
			return p1.generate();
		else
			return p2.generate();
	}
	inline vec4 generate(bool& dir) const {
		dir = 0;
		if (rafl() < 0.5f)
			return p1.generate();
		dir = 1;
		return p2.generate();
	}
	const P1& p1;
	const P2& p2;
};

template <class P1, class P2>
class bias_pdf {
public:
	bias_pdf(const P1& _p1, const P2& _p2, float b) :p1(_p1), p2(_p2), b(b) {}
	inline float value(vec4 V) const {
		return lerpf(p1.value(V), p2.value(V), b);
	}
	inline vec4 generate() const {
		if (rafl() > b)
			return p1.generate();
		else
			return p2.generate();
	}

	const P1& p1;
	const P2& p2;
	float b = 0.5f;
};

