#pragma once
#include "vec3.h"
#include "ray.h"
#include "obj.h"
#include "albedo.h"
#include "samplers.h"
#include "shading.h"

struct matrec {
	vec3 N;	//Normal from normal map
	vec3 P; //Adjusted hitpoint
	vec3 L; //Ray dir: diffuse, specular
	vec3 aten, emis; //Color
	uchar sd = 0; // specular / diff
};

enum mat_enum {
	mat_mix, mat_ggx, mat_vnd, mat_lig, mat_las
};

namespace material {
	constexpr int mat_cnt = 5;
	__forceinline void mix(const ray& r, const hitrec& rec, const albedo& tex, matrec& mat) {
		//simple mix of lambertian and mirror reflection/transmission + emission
		vec3 rgb = tex.rgb(rec.u, rec.v);
		vec3 mer = tex.mer(rec.u, rec.v);
		vec3 nor = tex.nor(rec.u, rec.v);
		float mu = mer.x();
		float em = mer.y();
		float ro = mer.z();
		float a = ro * ro;
		float ir = tex.ir();
		ir = rec.face ? 1.f / ir : ir;
		vec3 N = normal_map(rec.N, nor);
		onb n(N);
		//100% lambertian (reduced variance, since no rough reflections are allowed)
		if (mu <= eps && ro >= 1.f - eps && rgb.w() >= 1.f - eps) {
			mat.L = n.world(sa_cos());
			mat.P = rec.P + rec.N * eps;
			mat.sd = 2;
			mat.N = N;
			mat.aten = rgb;
			mat.emis = em * rgb;
			mat.sd *= not0(mat.aten);
			return;
		}
		vec3 H = n.world(sa_ggx(a));
		float HoV = absdot(H, r.D);
		float Fr = fres_refl(HoV, ir);
		bool metal = rafl() < mu;
		bool opaque = rafl() < rgb.w();
		bool mirror = ir * sqrtf(1.f - HoV * HoV) > 1.f || Fr > rafl();
		if ((opaque && metal) || mirror) {
			mat.P = rec.P + rec.N * eps;
			mat.L = reflect(r.D, H);
			mat.sd = 1;
		}
		else if (!opaque && !mirror) {
			mat.P = rec.P - rec.N * eps;
			mat.L = refract(r.D, H, ir);
			mat.sd = 1;
		}
		else
		{
			mat.L = n.world(sa_cos());
			mat.P = rec.P + rec.N * eps;
			mat.sd = 2;
		}
		mat.N = N;
		mat.aten = rgb;
		mat.emis = em * rgb;
		mat.sd *= not0(mat.aten);
	}
	__forceinline void ggx(const ray& r, const hitrec& rec, const albedo& tex, matrec& mat) {
		vec3 rgb = tex.rgb(rec.u, rec.v);
		vec3 mer = tex.mer(rec.u, rec.v);
		vec3 nor = tex.nor(rec.u, rec.v);
		float mu = mer.x();
		float em = mer.y();
		float ro = mer.z();
		float a = ro * ro;
		vec3 F0 = mix(0.04f, vec3(rgb, 1), mu);
		vec3 N = normal_map(rec.N, nor);
		onb n = onb(N);
		vec3 V = n.local(-r.D);
		vec3 H = sa_ggx(a);
		vec3 L = reflect(-V, H);
		float NoV = V.z();
		float NoL = L.z();
		float NoH = H.z();
		float HoV = absdot(H, V);
		vec3 F = fres_spec(HoV, F0);
		float f = F.w();
		bool spec = rafl() < f;
		if (spec) {
			if (NoV <= 0 || NoL <= 0 || NoH <= 0) return;
			float G = GGX(NoL, NoV, a);
			float W = HoV / (NoV * NoH);
			mat.aten = F * G * W / f;
			mat.L = n.world(L);
			mat.sd = 1;
		}
		else {
			mat.aten = (1.f - mu) * (1.f - F) * rgb / (1.f - f);
			mat.L = n.world(sa_cos());
			mat.sd = 2;
		}
		mat.N = N;
		mat.P = rec.P + rec.N * eps;
		mat.emis = rgb * em;
		mat.sd *= not0(mat.aten);
	}
	__forceinline void vndf(const ray& r, const hitrec& rec, const albedo& tex, matrec& mat) {
		vec3 rgb = tex.rgb(rec.u, rec.v);
		vec3 mer = tex.mer(rec.u, rec.v);
		vec3 nor = tex.nor(rec.u, rec.v);
		float mu = mer.x();
		float em = mer.y();
		float ro = mer.z();
		float a = ro * ro;
		vec3 F0 = mix(0.04f, vec3(rgb, 1), mu);
		vec3 N = normal_map(rec.N, nor);
		onb n = onb(N);
		vec3 V = n.local(-r.D);
		vec3 H = sa_vndf(V, a);
		vec3 L = reflect(-V, H);
		float NoV = V.z();
		float NoL = L.z();
		float HoV = absdot(H, V);
		vec3 F = fres_spec(HoV, F0);
		float f = F.w();
		bool spec = rafl() < f;
		if (spec) {
			if (NoV <= 0 || NoL <= 0) return;
			mat.aten = F * VNDF_GGX(NoL, NoV, a) / f;
			mat.L = n.world(L);
			mat.sd = 1;
		}
		else {
			mat.aten = (1.f - F) * (1.f - mu) * rgb / (1.f - f);
			mat.L = n.world(sa_cos());
			mat.sd = 2;
		}
		mat.N = N;
		mat.P = rec.P + rec.N * eps;
		mat.emis = rgb * em;
		mat.sd *= not0(mat.aten);
	}

	__forceinline void light(const ray& r, const hitrec& rec, const albedo& tex, matrec& mat) {
		vec3 rgb = tex.rgb(rec.u, rec.v);
		vec3 mer = tex.mer(rec.u, rec.v);
		mat.N = rec.N;
		float em = mer.y();
		mat.emis = em * rgb;
	}
	__forceinline void laser(const ray& r, const hitrec& rec, const albedo& tex, matrec& mat) {
		vec3 rgb = tex.rgb(rec.u, rec.v);
		vec3 mer = tex.mer(rec.u, rec.v);
		mat.N = rec.N;
		float em = mer.y();
		mat.emis = absdot(rec.N, r.D) * em * rgb;
	}
}
struct mat_var {
	mat_var();
	mat_var(const albedo& tex, mat_enum type) :tex(tex), type(type) {}

	__forceinline void sample(const ray& r, const hitrec& rec, matrec& mat)const {
		switch (type) {
		case mat_mix: return material::mix(r, rec, tex, mat);
		case mat_ggx: return material::ggx(r, rec, tex, mat);
		case mat_vnd: return material::vndf(r, rec, tex, mat);
		case mat_lig: return material::light(r, rec, tex, mat);
		case mat_las: return material::laser(r, rec, tex, mat);
		default: return;
		};
	}
	albedo tex;
	mat_enum type;
};
