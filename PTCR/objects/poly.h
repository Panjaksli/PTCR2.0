#pragma once
#include "obj.h"
#if SMOOTH_SHADING
class poly {
public:
	poly() {}
	poly(vec3 _A, vec3 _B, vec3 _C) :Q(_A), U(_B - _A), V(_C - _A), n0(poly_nis(U, V)), n1(n0), n2(n0) {}
	poly(vec3 _Q, vec3 _U, vec3 _V, bool param) :Q(_Q), U(_U), V(_V), n0(poly_nis(U, V)), n1(n0), n2(n0) {}
	poly(vec3 _Q, vec3 _U, vec3 _V, vec3 n0, vec3 n1, vec3 n2) :Q(_Q), U(_U), V(_V), n0(n0), n1(n1), n2(n2) {}
	inline bool hit(const ray& r, hitrec& rec) const
	{
		//if (!get_box().hit(r))return false;
		//Moller-Trumbore algorithm, based on:
		//https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
		vec3 pV = cross(r.D, V);
		float D = dot(U, pV);
		float iD = 1.f / D;
		vec3 tV = r.O - Q;
		vec3 qV = cross(tV, U);
		float u = dot(tV, pV) * iD;
		float v = dot(r.D, qV) * iD;
		float t = dot(V, qV) * iD;
		if (within(u, 0.f, 1.f) && within(v, 0.f, 1.f - u) && inside(t, eps2, rec.t))
		{
			bool face = D > 0;
			vec3 N = normal(u, v);
			rec.N = face ? N : -N;
			rec.P = Q + u * U + v * V;
			rec.t = t;
			rec.u = u;
			rec.v = v;
			rec.face = face;
			return true;
		}
		return false;
	}

	inline aabb get_box()const {
		return aabb(Q, Q + U, Q + V).padded();
	}
	inline poly trans(const mat4& T) const {
		return poly(T.pnt(Q), T.vec(U), T.vec(V), T.vec(n0), T.vec(n1), T.vec(n2));
	}
	inline poly move(vec3 P) const {
		float S = n0.w() * P.w() * P.w();
		return poly(Q * P.w() + P, U * P.w(), V * P.w(), vec3(n0, S), vec3(n1, S), vec3(n2, S));
	}
	inline float pdf(const ray& r)const {
		hitrec rec;
		if (!hit(r, rec))return 0;
		vec3 N = normal(rec.u, rec.v);
		float S = N.w();
		float NoL = absdot(N, r.D);
		return (rec.t * rec.t) / (S * NoL);
	}
	inline vec3 rand_to(vec3 O) const {
		float r[2]; rafl_tuple(r);
		if (r[0] + r[1] > 1.f) {
			r[0] = 1.f - r[0];
			r[1] = 1.f - r[1];
		}
		vec3 P = Q + r[0] * U + r[1] * V;
		vec3 L = norm(P - O);
		return L;
	}
	inline void set_quv(vec3 a, vec3 b, vec3 c) {
		Q = a;
		U = b - a;
		V = c - a;
		n0.xyz[3] = 0.5f * cross(U, V).len();
	}
	inline void set_nor(vec3 N0, vec3 N1, vec3 N2) {
		float S = n0.w();
		n0 = vec3(norm(N0), S);
		n1 = vec3(norm(N1), S);
		n2 = vec3(norm(N2), S);
	}
	inline ray rand_from() const {
		float r[2]; rafl_tuple(r);
		if (r[0] + r[1] > 1.f) {
			r[0] = 1.f - r[0];
			r[1] = 1.f - r[1];
		}
		vec3 O = Q + r[0] * U + r[1] * V;
		vec3 N = normal(r[0], r[1]);
		vec3 L = onb(rafl() < 0.5f ? N : -N).world(sa_cos());
		return ray(O, L);
	}
	inline float area()const {
		return 0.5f * cross(U, V).len();
	}
	vec3 Q, U, V;
	vec3 n0, n1, n2;
private:
	inline vec3 normal(float u, float v)const {
		return norm((1.f - u - v) * n0 + u * n1 + v * n2);
	}
};
#else
class poly {
public:
	poly() {}
	poly(vec3 _A, vec3 _B, vec3 _C) :Q(_A), U(_B - _A), V(_C - _A), N(poly_nis(U, V)) {}
	poly(vec3 _Q, vec3 _U, vec3 _V, bool param) :Q(_Q), U(_U), V(_V), N(poly_nis(U, V)) {}
	poly(vec3 _Q, vec3 _U, vec3 _V, vec3 _N) :Q(_Q), U(_U), V(_V), N(_N) {}
	inline bool hit(const ray& r, hitrec& rec) const
	{
		//Moller-Trumbore algorithm, based on:
		//https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
		vec3 pV = cross(r.D, V);
		float D = dot(U, pV);
		float iD = 1.f / D;
		vec3 tV = r.O - Q;
		vec3 qV = cross(tV, U);
		float u = dot(tV, pV) * iD;
		float v = dot(r.D, qV) * iD;
		float t = dot(V, qV) * iD;
		if (within(u, 0.f, 1.f) && within(v, 0.f, 1.f - u) && inside(t, eps2, rec.t))
		{
			bool face = D > 0;
			rec.N = face ? N : -N;
			rec.P = Q + u * U + v * V;
			rec.t = t;
			rec.u = u;
			rec.v = v;
			rec.face = face;
			return true;
		}
		return false;
	}
	inline float SDF(vec3 P) const {
		vec3 v1 = Q, v2 = Q + U, v3 = Q + V;
		vec3 v21 = v2 - v1; vec3 p1 = P - v1;
		vec3 v32 = v3 - v2; vec3 p2 = P - v2;
		vec3 v13 = v1 - v3; vec3 p3 = P - v3;
		vec3 nor = cross(v21, v13);
		bool cond = signf(dot(cross(v21, nor), p1)) + signf(dot(cross(v32, nor), p2)) + signf(dot(cross(v13, nor), p3)) < 2.0f;
		float x1 = (v21 * clamp(dot(v21, p1) / v21.len2(), 0.0, 1.0) - p1).len2();
		float x2 = (v32 * clamp(dot(v32, p2) / v32.len2(), 0.0, 1.0) - p2).len2();
		float x3 = (v13 * clamp(dot(v13, p3) / v13.len2(), 0.0, 1.0) - p3).len2();
		float x4 = dot(nor, p1) * dot(nor, p1) / nor.len2();
		return sqrtf(cond ? fminf(fminf(x1, x2), x3) : x4);
	}
	inline aabb get_box()const {
		return aabb(Q, Q + U, Q + V).padded();
	}
	inline poly trans(const mat4& T) const {
		return poly(T * vec3(Q, 1), T * U, T * V, true);
	}
	inline poly move(vec3 P) const {
		return poly(Q * P.w() + P, U * P.w(), V * P.w(), vec3(N, N.w() * P.w()));
	}
	inline float pdf(const ray& r)const {
		hitrec rec;
		if (!hit(r, rec))return 0;
		float iS = N.w();
		float NoL = absdot(N, r.D);
		return rec.t * rec.t * iS / NoL;
	}
	inline vec3 rand_to(vec3 O) const {
		float r[2]; rafl_tuple(r);
		if (r[0] + r[1] > 1.f) {
			r[0] = 1.f - r[0];
			r[1] = 1.f - r[1];
		}
		vec3 P = Q + r[0] * U + r[1] * V;
		vec3 L = norm(P - O);
		return L;
	}
	inline ray rand_from() const {
		float r[2]; rafl_tuple(r);
		if (r[0] + r[1] > 1.f) {
			r[0] = 1.f - r[0];
			r[1] = 1.f - r[1];
		}
		vec3 O = Q + r[0] * U + r[1] * V;
		vec3 L = onb(rafl() < 0.5f ? N : -N).world(sa_cos());
		return ray(O, L);
	}
	inline float area()const {
		return 0.5f * cross(U, V).len();
	}
	vec3 Q, U, V, N;
};

#endif

