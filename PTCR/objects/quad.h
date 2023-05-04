#pragma once
#include "obj.h"
//quad vec4(Q) vec4(U) vec4(V) vec4(N), 4*4*4 = 64B
class quad {
public:
	quad() {}
	quad(vec4 A, vec4 B, vec4 C) :Q(A), U(B - A), V(C - A), N(quad_ns(U, V)) {}
	quad(vec4 _Q, vec4 _U, vec4 _V, bool param) :Q(_Q), U(_U), V(_V), N(quad_ns(U, V)) {}
	quad(vec4 _Q, vec4 _U, vec4 _V, vec4 _N) :Q(_Q), U(_U), V(_V), N(_N) {}
	inline bool hit(const ray& r, hitrec& rec) const
	{
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
		if (within(u, 0.f, 1.f) && within(v, 0.f, 1.f) && inside(t, eps2, rec.t))
		{
			rec.face = true;
			rec.N = D > 0 ? N : -N;
			rec.P = Q + u * U + v * V;
			rec.u = u; rec.v = v; rec.t = t;
			return true;
		}
		return false;
	}
	inline aabb get_box()const {
		return aabb(Q, Q + U, Q + V, Q + U + V).padded();
	}
	inline quad trans(const mat4& T) const {
		return quad(T.pnt(Q), T.vec(U), T.vec(V), true);
	}
	inline quad move(vec4 P) const {
		return quad(Q * P.w() + P, U * P.w(), V * P.w(), vec4(N, N.w() * P.w() * P.w()));
	}
	inline float pdf(const ray& r)const {
		hitrec rec;
		if (!hit(r, rec))return 0;
		float S = N.w();
		float NoL = absdot(N, r.D);
		return (rec.t * rec.t) / (S * NoL);
	}
	inline vec4 rand_to(vec4 O) const {
		float r[2]; rafl_tuple(r);
		//pick point on quad using UV coordinates and make direction
		vec4 P = Q + r[0] * U + r[1] * V;
		vec4 L = norm(P - O);
		return L;
	}
	inline ray rand_from() const {
		float r[2]; rafl_tuple(r);
		vec4 O = Q + r[0] * U + r[1] * V;
		vec4 L = onb(rafl() < 0.5f ? N : -N).world(sa_cos());
		return ray(O, L);
	}
	inline float area()const {
		return cross(U, V).len();
	}
	inline vec4 A() { return Q; }
	inline vec4 B() { return Q + U; }
	inline vec4 C() { return Q + V; }
	vec4 Q, U, V, N;
};