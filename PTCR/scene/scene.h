#pragma once

#include "pdf.h"
#include "obj_list.h"
#include "camera.h"

struct scene_opt {
	scene_opt() {}
	vec3 sun_noon = vec3(300, 210, 90);
	vec3 sky_noon = vec3(0.123, 0.226, 0.490);
	vec3 sun_dawn = vec3(80, 4, 0);
	vec3 sky_dawn = vec3(0.152, 0.0384, 0.05);
	vec3 fog_col = 1.f;
	int selected = -1;
	float res_scale = 1.f;
	float res_rate = 1.f;
	float p_life = 0.9f;
	float i_life = 1.f / 0.9f;
	float ninv_fog = -10.f;
	float med_thr = 0.5f;
	float inv_sa = 0.5f;
	int bounces = 10;
	int node_size = 2;
	int samples = 2;
	bool en_fog = 1;
	bool en_bvh = 1;
	bool sky = 1;
	bool skybox = 0;
	bool sun_sa = 1;
	bool li_sa = 1;
	bool dbg_at = 0;
	bool re_sam = 1;
	bool dbg_n = 0;
	bool dbg_uv = 0;
	bool dbg_t = 0;
	bool recur = 0;
	bool p_mode = 1;
};

class scene {
public:
	camera cam;
	obj_list world;
	mat4 sun_pos;
	scene_opt opt;
	albedo skybox;
	scene() {}
	scene(camera cam, obj_list world = {}) :cam(cam), world(world) {}
	scene(uint w, uint h, float fov, obj_list world = {}) :cam(w, h, fov), world(world) {}
	void set_skybox(const albedo& bg);
	void save(const char* name) const;
	void load(const char* name);
	int get_id(const ray& r, hitrec& rec) const;
	int get_id(float py, float px);
	obj_flags get_flag() const;
	void cam_autofocus();
	void cam_manufocus(float py = 0, float px = 0);
	vec3 cam_collision(vec3 d, float dt) const;
	void cam_move(vec3 dir, float dt);
	vec3 cam_free(vec3 d) const;
	vec3 cam_fps(vec3 d) const;
	void set_trans(const mat4& T);
	obj_flags get_trans(mat4& T) const;
	void Render(uint* disp, uint pitch);
	void Screenshot()const;
private:
	//Debug info
	inline vec3 raycol_face(const ray& r)const {
		hitrec rec;
		if (!world.hit(r, rec)) return 0.5;
		return rec.face;
	}
	inline vec3 raycol_at(const ray& r)const {
		hitrec rec; matrec mat;
		if (!world.hit(r, rec)) return 0;
		world.materials[rec.mat].sample(r, rec, mat);
		return mat.aten + mat.emis;
	}
	inline vec3 raycol_n(const ray& r)const {
		hitrec rec; matrec mat;
		if (!world.hit(r, rec)) return 0;
		world.materials[rec.mat].sample(r, rec, mat);
		vec3 col = (mat.N + 1.f) * 0.5f;
		return col * col;
	}
	inline vec3 raycol_uv(const ray& r)const {
		hitrec rec;
		if (!world.hit(r, rec)) return 0;
		return vec3(rec.u, 0, rec.v) * vec3(rec.u, 0, rec.v);
	}
	inline vec3 raycol_t(const ray& r)const {
		float t = closest_t(r);
		return min(vec3(t * 0.1f, t * 0.01f, t * 0.001f), 1);
	}
	inline float closest_t(const ray& r) const {
		hitrec rec;
		world.hit(r, rec);
		return rec.t;
	}
	//Diffuse importance sampling
	__forceinline ray sa_diff(const matrec& mat, const vec3& P, float& p1, float& p2) const
	{
		if (opt.li_sa && opt.sun_sa) return sa_li_sun(mat, P, p1, p2);
		else if (opt.sun_sa) return sa_sun(mat, P, p1, p2);
		else if (opt.li_sa) return sa_li(mat, P, p1, p2);
		else return sa_none(mat, P, p1, p2);
	}
	//Fog importance sampling
	template <bool first = false>
	__forceinline ray sa_fog(const vec3& P, float ft, float& p1, float& p2) const
	{
		ray R;
		bool lisa = opt.li_sa, sunsa = opt.sun_sa;
		if (first && ft * ft > infp) lisa = 0;
		if (lisa && sunsa) {
			sph_pdf fog;
			lig_pdf lights(world, P);
			sun_pdf sun(sun_pos, P);
			mix_pdf<sun_pdf, lig_pdf> ill(sun, lights);
			mix_pdf<sph_pdf, mix_pdf<sun_pdf, lig_pdf>> mix(fog, ill);
			R = ray(P, mix.generate());
			p1 = fog.value(R.D);
			p2 = mix.value(R.D);
		}
		else if (sunsa) {
			sph_pdf fog;
			sun_pdf sun(sun_pos, P);
			mix_pdf<sun_pdf, sph_pdf> mix(sun, fog);
			R = ray(P, mix.generate());
			p1 = fog.value(R.D);
			p2 = mix.value(R.D);
		}
		else if (lisa) {
			sph_pdf fog;
			lig_pdf lights(world, P);
			mix_pdf<sph_pdf, lig_pdf> mix(fog, lights);
			R = ray(P, mix.generate());
			p1 = fog.value(R.D);
			p2 = mix.value(R.D);
		}
		else {
			R = ray(P, sa_sph());
			p1 = p2 = 1;
		}
		return R;
	}
	__forceinline ray sa_li_sun(const matrec& mat, const vec3& P, float& p1, float& p2) const
	{
		cos_pdf cosine(mat.N, mat.L);
		lig_pdf lights(world, P);
		sun_pdf sun(sun_pos, P);
		mix_pdf<sun_pdf, lig_pdf> ill(sun, lights);
		mix_pdf<cos_pdf, mix_pdf<sun_pdf, lig_pdf>> mix(cosine, ill);
		ray R(P, mix.generate());
		p1 = cosine.value(R.D);
		p2 = mix.value(R.D);
		return R;
	}
	__forceinline ray sa_li(const matrec& mat, const vec3& P, float& p1, float& p2) const
	{
		cos_pdf cosine(mat.N, mat.L);
		lig_pdf lights(world, P);
		mix_pdf<cos_pdf, lig_pdf> mix(cosine, lights);
		ray R(P, mix.generate());
		p1 = cosine.value(R.D);
		p2 = mix.value(R.D);
		return R;
	}
	__forceinline ray sa_sun(const matrec& mat, const vec3& P, float& p1, float& p2) const
	{
		cos_pdf cosine(mat.N, mat.L);
		sun_pdf sun(sun_pos, P);
		mix_pdf<cos_pdf, sun_pdf> mix(cosine, sun);
		ray R(P, mix.generate());
		p1 = cosine.value(R.D);
		p2 = mix.value(R.D);
		return R;
	}
	__forceinline ray sa_none(const matrec& mat, const vec3& P, float& p1, float& p2) const
	{
		ray R(P, mat.L);
		p1 = p2 = 1.f;
		return R;
	}
	//Color compute
	__forceinline vec3 raycol(const ray& r, float &t)const {
		hitrec rec; vec3 col;
		bool hit = world.hit(r, rec);
		t = rec.t;
		if (opt.en_fog) {
			for (int i = 0; i < opt.samples; i++)
				col += volumetric_pt<true>(r, opt.bounces, rec, hit);
		}
		else {
			if (!hit) return sky(r.D);
			for (int i = 0; i < opt.samples; i++)
				col += iterative_pt(r, rec, opt.bounces);
		}
		return col * opt.inv_sa;
	}
	//Volumetric path tracing
	template <bool first = false>
	__forceinline vec3 volumetric_pt(const ray& r, int depth, hitrec frec = hitrec(),bool fhit = 0)const {
		hitrec rec = frec;
		if (depth <= -1)return 0;
		bool hit = first ? fhit : world.hit(r, rec);
		float ft = opt.ninv_fog * flogf(rafl());
		if (ft < rec.t && (hit ? rec.fog : 1)) {
			float p1, p2;
			ray sr = sa_fog<first>(r.at(ft), ft, p1, p2);
			hitrec srec;
			bool hit = world.hit<1>(sr, srec);
			if (!hit)return p1 / p2 * opt.fog_col * sky(sr.D);
			else return p1 / p2 * opt.fog_col * recur_pt(sr, srec, depth);
		}
		else {
			if (!hit) return sky(r.D);
			if (rafl() >= opt.p_life)return 0;
			else return opt.i_life * recur_pt(r, rec, depth);
		}
	}
	//Volumetrics, recursive approach
	__forceinline  vec3 recur_pt(const ray& r, const hitrec& rec, int depth) const {
		matrec mat; vec3 aten;
		world.materials[rec.mat].sample(r, rec, mat);
		if (mat.sd) {
			if (mat.sd == 1)
				aten += mat.aten * volumetric_pt(ray(mat.P, mat.L, true), depth - 1);
			else
			{
				ray R; float p1, p2;
				R = sa_diff(mat, mat.P, p1, p2);
				if (p1 > 0) aten += (p1 / p2) * mat.aten * volumetric_pt(R, depth - 1);
			}
			return aten + mat.emis;
		}
		else return mat.emis;
	}
	//NO Volumetrics, iterative PT algorithm
	__forceinline  vec3 iterative_pt(const ray& sr, const hitrec& srec, int depth) const {
		vec3 col(0), aten(1.f); ray r = sr;
		for (int i = 0; i < depth + 1; i++)
		{
			hitrec rec; matrec mat;
			if (i == 0) rec = srec;
			else if (!world.hit(r, rec)) return col += aten * sky(r.D);
			else if (rafl() >= opt.p_life) break;
			else aten *= opt.i_life;
			world.materials[rec.mat].sample(r, rec, mat);
			col += mat.emis * aten;
			if (mat.sd)
			{
				if (mat.sd == 1)
					r = ray(mat.P, mat.L, true);
				else
				{
					float p1, p2;
					r = sa_diff(mat, mat.P, p1, p2);
					if (p1 > 0)aten *= (p1 / p2);
					else break;
				}
				aten *= mat.aten;
			}
			else break;
		}
		return col;
	}
	//Compute basic sky color
	__forceinline vec3 sky(vec3 V) const
	{
		if (!opt.sky)return 0;
		else if (opt.skybox) {
			float u = (fatan2(V.z(), -V.x()) + pi) * ipi2;
			float v = facos(V.y()) * ipi;
			return skybox.rgb(u, v) * skybox.mer().y();
		}
		else
		{
		vec3 A = sun_pos * vec3(0, 1, 0);
		float dp = posdot(V, A);
		float dp2 = 0.5f * (1.f + dot(V, A));
		float ip = 1.f - fabsf(A.y());
		float mp = 0.5f * (A.y() + 1.f);
		vec3 skycol = (pow2n(mp, 2) + 0.001f) * mix(opt.sky_noon, opt.sky_dawn, ip * ip);
		vec3 suncol = mix(opt.sun_noon, opt.sun_dawn, ip * ip);
		skycol = mix(skycol * 0.5f, 2.f * skycol, dp2);
		if (dp > 0.985f)
			return mix(skycol, suncol, pow2n(dp, 10));
		else
			return skycol;
		}
	}

};


