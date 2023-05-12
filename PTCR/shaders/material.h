#pragma once
#include "vec4.h"
#include "ray.h"
#include "obj.h"
#include "albedo.h"
#include "samplers.h"
#include "shading.h"
enum refl_type {
	refl_none, refl_diff, refl_spec, refl_tran
};
struct matrec {
	vec4 N, P, L;	//Normal, Hitpoint, Direction
	vec4 aten, emis; //Color, Emission
	float a = 0, ir = 1.f; //Rougness, refractive idx
	uint refl = refl_none; //Reflection type
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
		vec4 rgb = tex.rgb(rec.u, rec.v);
		vec4 mer = tex.mer(rec.u, rec.v);
		vec4 nor = tex.nor(rec.u, rec.v);
		float mu = mer.x();
		float em = mer.y();
		float ro = mer.z();
		float a = ro * ro;
		vec4 N = normal_map(rec.N, nor);
		onb n = onb(N);
		vec4 V = n.local(-r.D);
		vec4 H = use_vndf ? sa_vndf(V, a) : sa_ggx(a);
		vec4 L = reflect(-V, H);
		float NoV = fabsf(V.z());
		float NoL = L.z();
		float NoH = H.z();
		float HoV = absdot(H, V);
		vec4 F0 = mix(0.04f, vec4(rgb, 0.04f), mu);
		vec4 F = fres_spec(HoV, F0);
		bool metal = rafl() < mu;
		bool spec = rafl() < F.w();
		bool backface = NoL <= 0;
		if (metal) {
			if (backface)return;
			F = mix(F, tex.tinted(), (1 - HoV) * tex.tint.w());
			mat.aten = F * (use_vndf ? VNDF_GGX(NoL, NoV, a) : GGX(NoL, NoV, a) * HoV / (NoV * NoH));
			mat.L = n.world(L);
			mat.refl = refl_spec;
		}
		else if (spec && !backface) {
			F = mix(1, tex.tinted(), (1 - HoV) * tex.tint.w());
			mat.aten = F * (use_vndf ? VNDF_GGX(NoL, NoV, a) : GGX(NoL, NoV, a) * HoV / (NoV * NoH));
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
		vec4 rgb = tex.rgb(rec.u, rec.v);
		vec4 mer = tex.mer(rec.u, rec.v);
		vec4 nor = tex.nor(rec.u, rec.v);
		float mu = mer.x();
		float em = mer.y();
		float ro = mer.z();
		float a = ro * ro;
		float n1 = rec.face ? mat.ir : mat.ir == 1.f ? tex.ir : mat.ir;
		float n2 = rec.face ? tex.ir : tex.ir == n1 ? 1.f : tex.ir;
		bool opaque = rafl() < rgb.w();
		vec4 N = normal_map(rec.N, nor);
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
		else if (tex.alpha) {
			mat.aten = rgb;
			mat.N = rec.N;
			mat.P = rec.P - rec.N * eps;
			mat.L = r.D;
			mat.refl = refl_tran * not0(mat.aten);
			mat.ir = 1;
			return;
		}
		vec4 H = n.world(sa_ggx(a));
		float HoV = absdot(H, r.D);
		float Fr = fresnel(HoV, n1, n2, mu);
		bool reflective = Fr > rafl();
		mat.L = reflective ? reflect(r.D, H) : refract(r.D, H, n1 / n2);
		float NoL = dot(N, mat.L);
		bool spec = NoL > 0;
		if (spec) {
			if (rec.face) {
				float NoV = absdot(-r.D, N);
				float NoH = dot(N, H);
				vec4 F0 = mix(0.04f, vec4(rgb, 0.04f), mu);
				vec4 F = fres_spec(HoV, F0);
				F = mix(mix(1, F, mu), tex.tinted(), (1 - HoV) * tex.tint.w());
				mat.aten = F * GGX(NoL, NoV, a) * HoV / (NoV * NoH);
				mat.refl = refl_spec;
			}
			else {
				mat.aten = mix(rgb, tex.tinted(), (1 - HoV) * tex.tint.w());
				mat.refl = refl_tran;
			}
			mat.P = rec.P + rec.N * eps;
		}
		else {
			mat.P = rec.P - rec.N * eps;
			mat.aten = mix(rgb, tex.tinted(), (1 - HoV) * tex.tint.w());
			mat.refl = refl_tran;
			mat.ir = n2;
		}
		mat.a = a;
		mat.N = N;
		mat.emis = em * rgb;
		mat.refl *= not0(mat.aten);
	}
	__forceinline void vndf(const ray& r, const hitrec& rec, const albedo& tex, matrec& mat) {
		return ggx<1>(r, rec, tex, mat);
	}
	__forceinline void light(const ray& r, const hitrec& rec, const albedo& tex, matrec& mat) {
		vec4 rgb = tex.rgb(rec.u, rec.v);
		vec4 mer = tex.mer(rec.u, rec.v);
		float em = mer.y();
		float nov = absdot(rec.N, r.D);
		mat.emis = em * mix(rgb, tex.tinted(),(1-nov)* tex.tint.w());
		mat.N = rec.N;
	}
	__forceinline void laser(const ray& r, const hitrec& rec, const albedo& tex, matrec& mat) {
		vec4 rgb = tex.rgb(rec.u, rec.v);
		vec4 mer = tex.mer(rec.u, rec.v);
		float em = mer.y();
		float nov = absdot(rec.N, r.D);
		mat.emis = nov * em * mix(rgb, tex.tinted(), (1 - nov) * tex.tint.w());
		mat.N = rec.N;
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
