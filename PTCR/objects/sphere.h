#pragma once
#include "obj.h"

//sphere vec4(x,y,z,r) 16B
class sphere {
public:
	sphere() {}
	sphere(vec4 Q, float r) :Qr(Q, r) {}
	sphere(vec4 Qr) :Qr(Qr) {}

	inline bool hit(const ray& r, hitrec& rec) const {
		//Quadratic equation, own solution
		vec4 OQ = Qr - r.O;
		float b = dot(r.D, OQ);
		float c = dot(OQ, OQ) - Qr.w() * Qr.w();
		float d2 = b * b - c;
		float d = sqrtf(d2);
		float t1 = b - d;
		float t2 = b + d;
		bool face = c > 0;
		float t = face ? t1 : t2;
		if (inside(t, eps2, rec.t)) {
			vec4 P = r.at(t);
			vec4 N = (P - Qr) / Qr.w();
			rec.N = face ? N : -N;
			rec.P = P;
			rec.t = t;
			rec.u = (fatan2(-N.z(), N.x()) + pi) * ipi2;
			rec.v = facos(N.y()) * ipi;
			rec.face = face;
			return true;
		}
		return false;
	}
	inline aabb get_box()const {
		return aabb(Qr - Qr.w(), Qr + Qr.w());
	}

	inline sphere trans(const mat4& T) const {
		return sphere(T.pnt(Qr));
	}
	inline sphere move(vec4 P) const {
		return sphere(vec4(Qr + P, Qr.w() * P.w()));
	}
	inline float pdf(const ray& r)const {
		hitrec rec;
		if (!hit(r, rec))return 0;
		if (!rec.face) {
			float S = pi4 * Qr.w() * Qr.w();
			float NoL = absdot(rec.N, r.D);
			return rec.t * rec.t / (S * NoL);
		}
		else {
			//propability according to sampled cone, from:
			//https://raytracing.github.io/books/RayTracingTheRestOfYourLife.html#importancesamplingmaterials/randomhemispheresampling
			float theta = sqrtf(1.f - Qr.w() * Qr.w() / (Qr - r.O).len2());
			return  1.f / (pi2 * (1.f - theta));
		}
		return 0;
	}

	inline vec4 rand_to(vec4 O) const {
		vec4 OQ = Qr - O;
		float d2 = OQ.len2();
		float R2 = Qr.w() * Qr.w();
		//if inside, pick uniform coordinate
		if (d2 <= R2) {
			vec4 N = sa_sph();
			vec4 P = Qr + N * Qr.w();
			vec4 L = P - O;
			return norm(L);
		}
		//sample cone in direction of sphere, from:
		//https://raytracing.github.io/books/RayTracingTheRestOfYourLife.html#importancesamplingmaterials/randomhemispheresampling
		float r[2]; rafl_tuple(r);
		float z = 1.f + r[1] * (sqrtf(1.f - R2 / d2) - 1.f);
		float phi = pi2 * r[0];
		vec4 xy = sqrtf(1.f - z * z) * cossin(phi);
		return onb(norm(OQ)).world(xy + vec4(0, 0, z));
	}
	inline ray rand_from() const {
		vec4 N = sa_sph();
		vec4 O = Qr + N * Qr.w();
		vec4 L = onb(rafl() < 0.5f ? N : -N).world(sa_cos());
		return ray(O, L);
	}
	inline float area()const {
		return pi4 * Qr.w() * Qr.w();
	}
	inline vec4 Q() { return Qr; }
	vec4 Qr;
};

