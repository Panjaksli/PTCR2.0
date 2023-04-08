#pragma once

#include "pdf.h"
#include "obj_list.h"
#include "camera.h"


struct scene_opt {
	scene_opt() {}
	vec3 sun_noon = vec3(20, 18, 9);
	vec3 sky_noon = vec3(0.5, 0.6, 1);
	vec3 sun_dawn = vec3(9, 2, 0);
	vec3 sky_dawn = vec3(0.5, 0.05, 0.1);
	vec3 fog_col = 1;
	uint selected = -1;
	int max_spp = infp;
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
#if DEBUG
	bool dbg_f = 0;
	bool dbg_e = 0;
	bool dbg_n = 0;
	bool dbg_uv = 0;
	bool dbg_t = 0;
	bool dbg_light = 0;
	bool dbg_direct = 1;
#endif
	bool paused = 0;
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
	uint get_id(const ray& r, hitrec& rec) const;
	uint get_id(float py, float px);
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
	void Reproject(const projection& proj, uint* disp, uint pitch);
	void Screenshot(bool reproject = 0)const;
	__forceinline mat_var& material_at(uint idx) {
		return world.materials[idx];
	}
	__forceinline const mat_var& material_at(uint idx)const {
		return world.materials[idx];
	}
	__forceinline mesh_var& object_at(uint idx) {
		return world.objects[idx];
	}
	__forceinline const mesh_var& object_at(uint idx)const {
		return world.objects[idx];
	}
private:
	
	__forceinline void sample_material(const ray& r, const hitrec& rec, matrec& mat) const {
		uint mat_id = object_at(rec.idx).get_mat();
		if (mat_id < world.materials.size())
			material_at(mat_id).sample(r, rec, mat);
		else {
			mat.emis = vec3(1, 0, 1);
		}
	}
	__forceinline ray sa_fog(const vec3& P, float ft, float& p1, float& p2) const
	{
		ray R;
		bool lisa = opt.li_sa, sunsa = opt.sun_sa;
		if (lisa && sunsa) {
			sph_pdf fog;
			sun_pdf sun(sun_pos, P);
			lig_pdf lights(world, P);
			mix_pdf lig(sun, lights);
			mix_pdf mix(lig, fog);
			bool dir = 0;
			R = ray(P, rafl() < 0.5f ? fog.generate() : lig.generate(dir));
			p1 = fog.value(R.D);
			p2 = mix.value(R.D);
			if (dir && lights.value(R.D) <= 0.f) {
				R = ray(P, sa_sph());
				p1 = p2 = 1;
			}
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
			//Base code, causes 'holograms' in the distance
			//Holograms seem to be caused by biasing ray direction towards lights, whilst some object is blocking it (pdf doesn't check for that)
			//That results in oversampling of certain objects with incorrect pdf, problem is more prominent with lower fog densities (larger distances to lights)
			sph_pdf fog;
			lig_pdf lights(world, P);
			mix_pdf mix(fog, lights);
			bool dir = 0;
			R = ray(P, mix.generate(dir));
			p1 = fog.value(R.D);
			p2 = mix.value(R.D);
			//Fix for 'holograms', credit goes to 15 hours of tinkering
			//No idea how this fixes it, it just does so whilst not destroying image quality...
			if (dir && lights.value(R.D) <= 0.f) {
				R = ray(P, sa_sph());
				p1 = p2 = 1;
			}
		}
		else {
			R = ray(P, sa_sph());
			p1 = p2 = 1;
		}
		return R;
	}
	//Diffuse importance sampling
	__forceinline ray sa_diff(const matrec& mat, float& p1, float& p2) const
	{
		if (opt.li_sa && opt.sun_sa) return sa_li_sun(mat, p1, p2, cos_pdf(mat.N, mat.L));
		else if (opt.sun_sa) return sa_sun(mat, p1, p2, cos_pdf(mat.N, mat.L));
		else if (opt.li_sa) return sa_li(mat, p1, p2, cos_pdf(mat.N, mat.L));
		else return sa_none(mat, p1, p2);
	}
	__forceinline ray sa_spec(const matrec& mat, const vec3& rD, float& p1, float& p2) const
	{
		if (mat.a < 0.001f) return sa_none(mat, p1, p2);
		else if (opt.li_sa && opt.sun_sa) return sa_li_sun(mat, p1, p2, ggx_pdf(mat.N, rD, mat.L, mat.a));
		else if (opt.sun_sa) return sa_sun(mat, p1, p2, ggx_pdf(mat.N, rD, mat.L, mat.a));
		else if (opt.li_sa) return sa_li(mat, p1, p2, ggx_pdf(mat.N, rD, mat.L, mat.a));
		else return sa_none(mat, p1, p2);
	}
	//Fog importance sampling
	template<typename pdf>
	__forceinline ray sa_li_sun(const matrec& mat, float& p1, float& p2, const pdf& base) const
	{
		lig_pdf lights(world, mat.P);
		sun_pdf sun(sun_pos, mat.P);
		mix_pdf<sun_pdf, lig_pdf> ill(sun, lights);
		mix_pdf<pdf, mix_pdf<sun_pdf, lig_pdf>> mix(base, ill);
		ray R(mat.P, mix.generate());
		p1 = base.value(R.D);
		p2 = mix.value(R.D);
		return R;
	}
	template<typename pdf>
	__forceinline ray sa_li(const matrec& mat, float& p1, float& p2, const pdf& base) const
	{
		lig_pdf lights(world, mat.P);
		mix_pdf<pdf, lig_pdf> mix(base, lights);
		ray R(mat.P, mix.generate());
		p1 = base.value(R.D);
		p2 = mix.value(R.D);
		return R;
	}
	template<typename pdf>
	__forceinline ray sa_sun(const matrec& mat, float& p1, float& p2, const pdf& base) const
	{
		sun_pdf sun(sun_pos, mat.P);
		mix_pdf<pdf, sun_pdf> mix(base, sun);
		ray R(mat.P, mix.generate());
		p1 = base.value(R.D);
		p2 = mix.value(R.D);
		return R;
	}
	__forceinline ray sa_none(const matrec& mat, float& p1, float& p2) const
	{
		ray R(mat.P, mat.L);
		p1 = p2 = 1.f;
		return R;
	}
	//Color compute
	__forceinline vec3 raycol(const ray& r)const {
		vec3 col;  hitrec rec;
		bool hit = world.hit<1>(r, rec);
		if (opt.en_fog) {
			for (int i = 0; i < opt.samples; i++)
				col += volumetric_pt<true>(r, opt.bounces, rec, hit) * opt.inv_sa;
			rec.t = col[3];
		}
		else {
			if (!hit) col = sky(r.D);
			else
				for (int i = 0; i < opt.samples; i++)
					col += iterative_pt(r, rec, opt.bounces) * opt.inv_sa;
		}
		return vec3(col,rec.t);
	}
	//Volumetric path tracing (main volumetrics logic)
	template <bool first = false>
	__forceinline vec3 volumetric_pt(const ray& r, int depth, hitrec frec = hitrec(), bool fhit = 0)const {
		if (depth <= -1)return 0;
		hitrec rec = first ? frec : hitrec();
		bool hit = first ? fhit : world.hit<1>(r, rec);
		float ft = opt.ninv_fog * flogf(rafl());
		if (ft < rec.t && (hit ? rec.face || object_at(rec.idx).fog() : 1)) {
			float p1 = 0, p2 = 1;
			ray sr = sa_fog(r.at(ft), ft, p1, p2);
			hitrec srec;
			bool hit = world.hit<1>(sr, srec);
			if(first){
			if (!hit)return vec3(p1 / p2 * opt.fog_col * sky(sr.D),ft);
			else return vec3(p1 / p2 * opt.fog_col * recur_pt(sr, srec, depth),ft);
			}
			else {
				if (!hit)return p1 / p2 * opt.fog_col * sky(sr.D);
				else return p1 / p2 * opt.fog_col * recur_pt(sr, srec, depth);
			}
		}
		else {
			if (first) {
				if (!hit) return vec3(sky(r.D),rec.t);
				if (rafl() >= opt.p_life)return vec3(0,0,0,rec.t);
				else return vec3(opt.i_life * recur_pt(r, rec, depth),rec.t);
			}
			else {
				if (!hit) return sky(r.D);
				if (rafl() >= opt.p_life)return 0;
				else return opt.i_life * recur_pt(r, rec, depth);
			}
		}
	}
	//Volumetrics (sample materials)
	__forceinline  vec3 recur_pt(const ray& r, const hitrec& rec, int depth) const {
		matrec mat; vec3 aten;
		sample_material(r, rec, mat);
		if (mat.refl) {
			ray R;
			float p1, p2;
			if (mat.refl == refl_diff || mat.refl == refl_spec)
			{
				R = mat.refl == refl_diff ? sa_diff(mat, p1, p2) : sa_spec(mat, -r.D, p1, p2);
				if (p1 > 0) aten += (p1 / p2) * mat.aten * volumetric_pt(R, depth - 1);
			}
			else aten += mat.aten * volumetric_pt(ray(mat.P, mat.L, true), depth - 1);
			return aten + mat.emis;
		}
		else return mat.emis;
	}
	//NO Volumetrics, iterative PT algorithm
	__forceinline  vec3 iterative_pt(const ray& sr, const hitrec& srec, int depth) const {
		vec3 col(0), aten(1.f); ray r = sr; float ir = 1.f;
		for (int i = 0; i < depth + 1; i++)
		{
			hitrec rec; matrec mat;
			if (i == 0) rec = srec;
			else if (!world.hit<1>(r, rec)) return col += aten * sky(r.D);
			else if (rafl() >= opt.p_life) break;
			else aten *= opt.i_life;
			mat.ir = ir;
			sample_material(r, rec, mat);
			col += mat.emis * aten;
			if (mat.refl)
			{
				float p1, p2;
				if (mat.refl == refl_diff || mat.refl == refl_spec)
				{
					r = mat.refl == refl_diff ? sa_diff(mat, p1, p2) : sa_spec(mat, -r.D, p1, p2);
					if (p1 > 0)aten *= (p1 / p2);
					else break;
				}
				else {
					ir = mat.ir;
					r = ray(mat.P, mat.L, true);
				}
				aten *= mat.aten;
			}
			else break;
		}
		return col;
	}
	//Separates lighting into direct and indirect
	__forceinline void separate_pt(vec3& direct, vec3& indirect, const ray& sr, const hitrec& srec, int depth) const {
		vec3 aten(1.f); ray r = sr;
		for (int i = 0; i < depth + 1; i++)
		{
			hitrec rec; matrec mat;
			if (i == 0) rec = srec;
			else if (!world.hit<1>(r, rec)) {
				if (i < 2)direct += aten * sky(r.D);
				else indirect += aten * sky(r.D);
				break;
			}
			else if (rafl() >= opt.p_life) break;
			else aten *= opt.i_life;
			sample_material(r, rec, mat);
			if (i < 2)direct += mat.emis * aten;
			else indirect += mat.emis * aten;;
			if (mat.refl)
			{
				float p1, p2;
				if (mat.refl == refl_diff || mat.refl == refl_spec)
				{
					r = mat.refl == refl_diff ? sa_diff(mat, p1, p2) : sa_spec(mat, -r.D, p1, p2);
					if (p1 > 0)aten *= (p1 / p2);
					else break;
				}
				else r = ray(mat.P, mat.L, true);
				aten *= mat.aten;
			}
			else break;
		}
		return;
	}
	//Compute basic sky color
	__forceinline vec3 sky(vec3 V) const
	{
		if (!opt.sky)return 0;
		else if (opt.skybox) {
			float u = (fatan2(V.z(), -V.x()) + pi) * ipi2;
			float v = facos(V.y()) * ipi;
			return skybox.rgb(u, v) * skybox.mer(u, v).y();
		}
		else
		{
			vec3 sun_noon = GAMMA2 ? opt.sun_noon * opt.sun_noon : opt.sun_noon;
			vec3 sun_dawn = GAMMA2 ? opt.sun_dawn * opt.sun_dawn : opt.sun_dawn;
			vec3 sky_noon = GAMMA2 ? opt.sky_noon * opt.sky_noon : opt.sky_noon;
			vec3 sky_dawn = GAMMA2 ? opt.sky_dawn * opt.sky_dawn : opt.sky_dawn;
			vec3 A = sun_pos * vec3(0, 1, 0);
			float dp = posdot(V, A);
			float dp2 = 0.5f * (1.f + dot(V, A));
			float ip = 1.f - fabsf(A.y());
			float mp = 0.5f * (A.y() + 1.f);
			vec3 skycol = (pow2n(mp, 2) + 0.001f) * mix(sky_noon, sky_dawn, ip * ip);
			vec3 suncol = mix(sun_noon, sun_dawn, ip * ip);
			skycol = mix(skycol * 0.5f, 2.f * skycol, dp2);
			if (dp > 0.985f)
				return mix(skycol, suncol, pow2n(dp, 10));
			else
				return skycol;
		}
	}

	//Debug info
#if DEBUG
	inline vec3 dbg_ill(const ray& r) {
		hitrec rec;
		if (!world.hit(r, rec)) return opt.dbg_direct ? vec3(sky(r.D),rec.t) : vec3(0,0,0,rec.t);
		vec3 direct, indirect;
		separate_pt(direct, indirect, r, rec, opt.dbg_direct ? 1 : opt.bounces);
		return vec3(opt.dbg_direct ? direct : indirect,rec.t);
	}
	inline vec3 dbg_f(const ray& r)const {
		hitrec rec;
		if (!world.hit(r, rec)) {
			float res = GAMMA2 ? 0.01 : 0.1;
			return vec3(res, res, res, rec.t);
		}
		return vec3(rec.face,rec.face,rec.face,rec.t);
	}
	inline vec3 dbg_at(const ray& r)const {
		hitrec rec; matrec mat;
		if (!world.hit(r, rec)) {
			float res = GAMMA2 ? 0.01 : 0.1;
			return vec3(res, res, res, rec.t);
		}
		sample_material(r, rec, mat);
		return vec3(mat.aten + mat.emis,rec.t);
	}
	inline vec3 dbg_n(const ray& r)const {
		hitrec rec; matrec mat;
		if (!world.hit(r, rec)) {
			float res = GAMMA2 ? 0.01 : 0.1;
			return vec3(res, res, res, rec.t);
		}
		sample_material(r, rec, mat);
		vec3 col = vec3((mat.N + 1.f) * 0.5f,rec.t);
		return GAMMA2 ? col * vec3(col,1) : col;
	}
	inline vec3 dbg_uv(const ray& r)const {
		hitrec rec;
		if (!world.hit(r, rec)) {
			float res = GAMMA2 ? 0.01 : 0.1;
			return vec3(res, res, res, rec.t);
		}
		return GAMMA2 ? vec3(rec.u, 0, rec.v,rec.t) * vec3(rec.u, 0, rec.v,1) : vec3(rec.u, 0, rec.v,rec.t);
	}
	inline vec3 dbg_e(const ray& sr)const {
		ray r = sr;
		hitrec rec;
		uint i = 0;
		float t = infp;
		while (world.hit(r, rec) && i++ < (opt.dbg_t ? 1 : 10)) {
			if (i == 1) t = rec.t;
			switch (object_at(rec.idx).type()) {
			case o_pol: if (!(inside(rec.u, 0.01f, 0.99f) && inside(rec.v, 0.01f, 0.99f) && inside(1 - rec.u - rec.v, 0.01f, 0.99f))) return vec3(1,1,1,t); break;
			case o_qua: if (!(inside(rec.u, 0.01f, 0.99f) && inside(rec.v, 0.01f, 0.99f))) return vec3(1, 1, 1, t); break;
			case o_sph: if (absdot(rec.N, r.D) < 0.15f) return vec3(1, 1, 1, t); break;
			case o_vox: if (!(inside(rec.u, 0.01f, 0.99f) && inside(rec.v, 0.01f, 0.99f)))  return vec3(1, 1, 1, t);  break;
			default: return vec3(0,0,0,infp);
			}
			rec.t = infp;
			r.O = rec.P + r.D * eps;
		}
		float res = GAMMA2 ? 0.01 : 0.1;
		return vec3(res,res,res,t);
	}
	inline vec3 dbg_t(const ray& r)const {
		float t = closest_t(r);
		return min(vec3(t * 0.1f, t * 0.01f, t * 0.001f), t);
	}
#endif
	inline float closest_t(const ray& r) const {
		hitrec rec;
		world.hit(r, rec);
		return rec.t;
	}
};


