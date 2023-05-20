#pragma once
#include "obj.h"
#if SMOOTH_SHADING
class poly {
public:
	poly() {}
	poly(vec4 A, vec4 B, vec4 C) : Q(A), U(B - A), V(C - A), n0(poly_ns(U, V)), n1(n0), n2(n0) {}
	poly(vec4 Q, vec4 U, vec4 V, bool param) :Q(Q), U(U), V(V), n0(poly_ns(U, V)), n1(n0), n2(n0) {}
	poly(vec4 Q, vec4 U, vec4 V, vec4 n0, vec4 n1, vec4 n2) :Q(Q), U(U), V(V), n0(n0), n1(n1), n2(n2) {}
	inline bool hit(const ray& r, hitrec& rec) const {
		//Moller-Trumbore algorithm, based on:
		//https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
		vec4 pV = cross(r.D, V);
		float D = dot(U, pV);
		float iD = 1.f / D;
		vec4 tV = r.O - Q;
		vec4 qV = cross(tV, U);
		float u = dot(tV, pV) * iD;
		float v = dot(r.D, qV) * iD;
		float t = dot(V, qV) * iD;
		if (within(u, 0.f, 1.f) && within(v, 0.f, 1.f - u) && inside(t, eps2, rec.t)) {
			rec.face = D > 0;
			vec4 N = normal(u, v);
			rec.N = rec.face ? N : -N;
			rec.P = Q + u * U + v * V;
			rec.u = u; rec.v = v; rec.t = t;
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
	inline poly move(vec4 P) const {
		float S = n0.w() * P.w() * P.w();
		return poly(Q * P.w() + P, U * P.w(), V * P.w(), vec4(n0, S), vec4(n1, S), vec4(n2, S));
	}
	inline float pdf(const ray& r)const {
		hitrec rec;
		if (!hit(r, rec))return 0;
		vec4 N = normal(rec.u, rec.v);
		float S = N.w();
		float NoL = absdot(N, r.D);
		return (rec.t * rec.t) / (S * NoL);
	}
	inline vec4 rand_to(vec4 O) const {
		float r[2]; rafl_tuple(r);
		if (r[0] + r[1] > 1.f) {
			r[0] = 1.f - r[0];
			r[1] = 1.f - r[1];
		}
		vec4 P = Q + r[0] * U + r[1] * V;
		vec4 L = norm(P - O);
		return L;
	}
	inline void set_quv(vec4 a, vec4 b, vec4 c) {
		Q = a;
		U = b - a;
		V = c - a;
		n0.xyz[3] = 0.5f * cross(U, V).len();
	}
	inline void set_nor(vec4 N0, vec4 N1, vec4 N2) {
		float S = n0.w();
		n0 = vec4(norm(N0), S);
		n1 = vec4(norm(N1), S);
		n2 = vec4(norm(N2), S);
	}
	inline ray rand_from() const {
		float r[2]; rafl_tuple(r);
		if (r[0] + r[1] > 1.f) {
			r[0] = 1.f - r[0];
			r[1] = 1.f - r[1];
		}
		vec4 O = Q + r[0] * U + r[1] * V;
		vec4 N = normal(r[0], r[1]);
		vec4 L = onb(rafl() < 0.5f ? N : -N).world(sa_cos());
		return ray(O, L);
	}
	inline float area()const {
		return 0.5f * cross(U, V).len();
	}
	inline vec4 A() const { return Q; }
	inline vec4 B() const { return Q + U; }
	inline vec4 C() const { return Q + V; }
	vec4 Q, U, V;
	vec4 n0, n1, n2;
private:
	inline vec4 normal(float u, float v)const {
		return norm((1.f - u - v) * n0 + u * n1 + v * n2);
	}
};
#else
class poly {
public:
	poly() {}
	poly(vec4 A, vec4 B, vec4 C) :Q(A), U(B - A), V(C - A), N(poly_ns(U, V)) {}
	poly(vec4 _Q, vec4 _U, vec4 _V, bool param) :Q(_Q), U(_U), V(_V), N(poly_ns(U, V)) {}
	poly(vec4 _Q, vec4 _U, vec4 _V, vec4 _N) :Q(_Q), U(_U), V(_V), N(_N) {}
	inline bool hit(const ray& r, hitrec& rec) const {
		//Moller-Trumbore algorithm, based on:
		//https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
		vec4 pV = cross(r.D, V);
		float D = dot(U, pV);
		float iD = 1.f / D;
		vec4 tV = r.O - Q;
		vec4 qV = cross(tV, U);
		float u = dot(tV, pV) * iD;
		float v = dot(r.D, qV) * iD;
		float t = dot(V, qV) * iD;
		if (within(u, 0.f, 1.f) && within(v, 0.f, 1.f - u) && inside(t, eps2, rec.t)) {
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
	inline float SDF(vec4 P) const {
		vec4 v1 = Q, v2 = Q + U, v3 = Q + V;
		vec4 v21 = v2 - v1; vec4 p1 = P - v1;
		vec4 v32 = v3 - v2; vec4 p2 = P - v2;
		vec4 v13 = v1 - v3; vec4 p3 = P - v3;
		vec4 nor = cross(v21, v13);
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
		return poly(T * vec4(Q, 1), T * U, T * V, true);
	}
	inline poly move(vec4 P) const {
		return poly(Q * P.w() + P, U * P.w(), V * P.w(), vec4(N, N.w() * P.w()));
	}
	inline float pdf(const ray& r)const {
		hitrec rec;
		if (!hit(r, rec))return 0;
		float iS = N.w();
		float NoL = absdot(N, r.D);
		return rec.t * rec.t * iS / NoL;
	}
	inline vec4 rand_to(vec4 O) const {
		float r[2]; rafl_tuple(r);
		if (r[0] + r[1] > 1.f) {
			r[0] = 1.f - r[0];
			r[1] = 1.f - r[1];
		}
		vec4 P = Q + r[0] * U + r[1] * V;
		vec4 L = norm(P - O);
		return L;
	}
	inline ray rand_from() const {
		float r[2]; rafl_tuple(r);
		if (r[0] + r[1] > 1.f) {
			r[0] = 1.f - r[0];
			r[1] = 1.f - r[1];
		}
		vec4 O = Q + r[0] * U + r[1] * V;
		vec4 L = onb(rafl() < 0.5f ? N : -N).world(sa_cos());
		return ray(O, L);
	}
	inline float area()const {
		return 0.5f * cross(U, V).len();
	}
	inline vec4 A() { return Q; }
	inline vec4 B() { return Q + U; }
	inline vec4 C() { return Q + V; }
	vec4 Q, U, V, N;
};

#endif

