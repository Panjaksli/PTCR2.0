#pragma once
#include "vec3.h"
#include "ray.h"
#include "obj.h"
#include "albedo.h"
#include "samplers.h"
#include "shading.h"
enum refl_type {
	refl_none, refl_diff, refl_spec, refl_tran
};

struct matrec {
	vec3 N;	//Normal from normal map
	vec3 P; //Adjusted hitpoint
	vec3 L; //Ray dir: diffuse, specular
	vec3 aten, emis; //Color
	float a = 0; // Rougness
	float ir = 1.f;
	uchar refl = refl_none;
};

constexpr int mat_cnt = 5;
enum mat_enum {
	mat_mix, mat_ggx, mat_vnd, mat_lig, mat_las
};

inline const char* mat_enum_str(int val) {
	switch (val) {
	case mat_mix: return "Mix";
	case mat_ggx: return "GGX";
	case mat_vnd: return "VNDF";
	case mat_lig: return "Light";
	default: return "Dir. light";
	};
}

namespace material {
	template <bool use_vndf = 0>
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
		//sample microfacet normal according to rougness coefficient
		vec3 H = use_vndf ? sa_vndf(V, a) : sa_ggx(a);
		vec3 L = reflect(-V, H);
		float NoV = fabsf(V.z());
		float NoL = L.z();
		float NoH = H.z();
		float HoV = dot(H, V);
		vec3 F = fres_spec(HoV, F0);
		vec3 Fs = fres_spec(HoV, tex.specular());
		bool spec = rafl() < F.w();
		if (spec) {
			if (NoL <= 0 || HoV <= 0) return;
			mat.aten = mix(F.fact(), Fs, tex.spec.w()) * (use_vndf ? VNDF_GGX(NoL, NoV, a) : GGX(NoL, NoV, a) * HoV / (NoV * NoH));
			mat.L = n.world(L);
			mat.refl = refl_spec;
		}
		else {
			mat.aten = rgb;
			mat.L = n.world(sa_cos());
			mat.refl = refl_diff;
		}
		mat.a = a;
		mat.N = N;
		mat.P = rec.P + rec.N * eps;
		mat.emis = rgb * em;
		mat.refl *= not0(mat.aten);
	}
	__forceinline void mixed(const ray& r, const hitrec& rec, const albedo& tex, matrec& mat) {
		//simple mix of lambertian and mirror reflection/transmission + emission
		vec3 rgb = tex.rgb(rec.u, rec.v);
		vec3 mer = tex.mer(rec.u, rec.v);
		vec3 nor = tex.nor(rec.u, rec.v);
		float mu = mer.x();
		float em = mer.y();
		float ro = mer.z();
		float a = ro * ro;
		float n1 = rec.face ? mat.ir : mat.ir == 1.f ? tex.ir : mat.ir;
		float n2 = rec.face ? tex.ir : tex.ir == n1 ? 1.f : tex.ir;
		bool opaque = rafl() < rgb.w();
		vec3 N = normal_map(rec.N, nor);
		onb n(N);
		//perfect diffuse && solid
		if (mu < eps && ro > 1 - eps && opaque) {
			mat.aten = rgb;
			mat.L = n.world(sa_cos());
			mat.N = N;
			mat.P = rec.P + rec.N * eps;
			mat.emis = rgb * em;
			mat.refl = refl_diff * not0(mat.aten);
			return;
		}
		else if (opaque)return ggx(r, rec, tex, mat);
		vec3 H = n.world(sa_ggx(a));
		float HoV = absdot(H, r.D);
		float Fr = fresnel(HoV, n1, n2, mu);
		bool refl = Fr > rafl();
		if (refl) {
			mat.L = reflect(r.D, H);
			float NoV = absdot(-r.D,N);
			float NoL = dot(N,mat.L);
			float NoH = dot(N, H);
			if (NoL <= 0)return;
			vec3 F0 = mix(0.04f, vec3(rgb, 1), mu);
			vec3 F = fres_spec(HoV, F0).fact();
			vec3 Fs = fres_spec(HoV, tex.specular());
			mat.aten = rec.face ? mix(F, Fs, tex.spec.w()) * GGX(NoL, NoV, a) * HoV / (NoV * NoH) : rgb;
			mat.P = rec.P + rec.N * eps;
			mat.refl = rec.face ? refl_spec : refl_tran;
		}
		else {
			mat.aten = rgb;
			mat.P = rec.P - rec.N * eps;
			mat.L = refract(r.D, H, n1 / n2);
			mat.refl = refl_tran;
			mat.ir = n2;
		}
		//if (!rec.face)mat.aten *= saturate(expf(-20.f * rec.t * (1.f - rgb)));
		mat.a = a;
		mat.N = N;
		mat.emis = em * rgb;
		mat.refl *= not0(mat.aten);
	}
	__forceinline void vndf(const ray& r, const hitrec& rec, const albedo& tex, matrec& mat) {
		return ggx<1>(r, rec, tex, mat);
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
	mat_var() {};
	mat_var(const albedo& tex, mat_enum type) :tex(tex), type(type) {}

	__forceinline void sample(const ray& r, const hitrec& rec, matrec& mat)const {
		switch (type) {
		case mat_mix: return material::mixed(r, rec, tex, mat);
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
