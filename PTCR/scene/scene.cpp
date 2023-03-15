#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <omp.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>
#include "scene.h"
// a=f/m
// v+=a*t
// x+=v*t
vec3 scene::cam_collision(vec3 v, float dt) const {
	if (!cam.collision)return 0;
	hitrec rec;
	for (int i = 0; i < 5; i++)
	{
		vec3 d = v + eps * ravec();
		ray r = ray(cam.T.P(), d, true);
		if (world.hit(r, rec) && rec.t - d.len() * dt <= eps) {
			return rec.N * (d.len() - rec.t * dt) * dot(rec.N, -r.D) / dot(r.D, norm(v));
		}
	}
	return 0;
}
vec3 scene::cam_free(vec3 d) const {
	vec3 s = cam.T * vec3(1, 0, 0);
	vec3 fw = cam.T * vec3(0, 0, -1), si(s.x(), 0, s.z()), up = vec3(0, 1, 0);
	fw = norm(fw);
	si = norm(si);
	return  (d.x() * fw + d.y() * up + d.z() * si) * cam.speed;
}
vec3 scene::cam_fps(vec3 d) const {
	vec3 f = cam.T * vec3(0, 0, -1);
	vec3 s = cam.T * vec3(1, 0, 0);
	vec3 fw(f.x(), 0, f.z()), si(s.x(), 0, s.z()), up = vec3(0, 1, 0);
	fw = norm(fw);
	si = norm(si);
	return (d.x() * fw + d.y() * up + d.z() * si) * cam.speed;
}
void scene::cam_move(vec3 dir, float dt) {
	static vec3 V = 0;
	vec3 G = vec3(0, -9.81, 0);
	vec3 F0 = cam.free ? cam_free(dir) : cam_fps(dir);
	vec3 F = F0;// + G;
	V += F * dt;
	V += cam_collision(V, dt);
	V += cam_collision(V, dt);
	V += cam_collision(V, dt);
	if (eq0(cam_collision(V, dt))) cam.move(V * dt);
	V -= fminf(10 * dt, 1) * V;
}
void scene::cam_autofocus() {
	if (cam.autofocus) {
		ray r(cam.focus_ray());
		float new_t = closest_t(r);
		cam.foc_t = 0.5 * cam.foc_t + 0.5f * new_t;
	}
}
void scene::cam_manufocus(float py, float px) {
	if (!cam.autofocus) {
		ray r(cam.focus_ray(py * cam.ih, px * cam.iw));
		cam.foc_t = fminf(closest_t(r), 1e6);
		cam.moving = 1;
	}
}
int scene::get_id(const ray& r, hitrec& rec) const
{
	return world.get_id(r, rec);
}

int scene::get_id(float py, float px)
{
	ray r(cam.focus_ray(py * cam.ih, px * cam.iw));
	hitrec rec;
	int id = get_id(r, rec);
	opt.selected = id;
	return id;
}
obj_flags scene::get_flag() const
{
	if (opt.selected < 0)return obj_flags(o_bla, 0, 0);
	else return world.get_flag(opt.selected);
}

obj_flags scene::get_trans(mat4& T) const {
	if (opt.selected == -1)
	{
		//if nothing hit, get sky
		T = sun_pos;
	}
	else
	{
		world.get_trans(opt.selected, T);
	}
	return world.get_flag(opt.selected);
}

void scene::set_trans(const mat4& T) {
	if (opt.selected == -1)
	{
		//if nothing hit, transform sky
		sun_pos = T;
		sun_pos.set_P(vec3());
	}
	else
	{
		world.set_trans(opt.selected, T, opt.node_size);
	}
}
void scene::set_skybox(const albedo& bg)
{
	skybox = bg;
	opt.skybox = true;
}
void scene::Render(uint* disp, uint pitch) {
	cam_autofocus();
	cam.CCD.dt(opt.samples);
	cam.CCD.set_disp(disp, pitch);
	if (cam.moving)cam.CCD.spp = 0.f;
	static bool odd = 0;
	bool old_lisa = opt.li_sa;
	bool old_sunsa = opt.sun_sa;
	float blink_t = (hpi * clock()) / CLOCKS_PER_SEC;
	world.en_bvh = opt.en_bvh && world.bvh.size() > 0;
	opt.li_sa = opt.li_sa && world.lights.size() > 0;
	world.lw = 1.f / world.lights.size();
	opt.sun_sa = opt.sun_sa && opt.sky && !opt.skybox;
	opt.inv_sa = 1.f / fmaxf(opt.samples, 1);
#if DEBUG
	opt.inv_sa = opt.dbg_at || opt.dbg_n || opt.dbg_uv || opt.dbg_t ? 1.f : opt.inv_sa;
#endif
#pragma omp parallel for collapse(2) schedule(dynamic, 100)
	for (int i = 0; i < cam.h; i++) {
		for (int j = opt.p_mode ? (i + odd) % 2 : 0; j < cam.w; j += opt.p_mode ? 2 : 1) {
			float t = 0;
			vec3 xy(i, j), col(0);
			if (cam.moving) cam.CCD.clear(i, j);
			else xy += 0.5f * ravec();
			ray r = cam.optical_ray(xy.x(), xy.y());
#if DEBUG
			if (opt.dbg_at)		 col = raycol_at(r);
			else if (opt.dbg_n && opt.dbg_uv)  col = raycol_face(r);
			else if (opt.dbg_n)  col = raycol_n(r);
			else if (opt.dbg_uv) col = raycol_uv(r);
			else if (opt.dbg_t)  col = raycol_t(r);
			else col = raycol(r, t);
#else
			col = raycol(r, t);
#endif
			cam.add(i, j, col);
		}
	}
	if (opt.p_mode) {
#pragma omp parallel for collapse(2) schedule(dynamic, 100)
		for (int i = 0; i < cam.h; i++) {
			for (int j = (i + !odd) % 2; j < cam.w; j += 2) {
				if (cam.moving) cam.CCD.clear(i, j);
				vec3 rgb[4];
				rgb[0] = cam.CCD.get(i, fmax_int(j - 1, 0));
				rgb[1] = cam.CCD.get(i, fmin_int(j + 1, cam.w - 1));
				rgb[2] = cam.CCD.get(fmax_int(i - 1, 0), j);
				rgb[3] = cam.CCD.get(fmin_int(i + 1, cam.h - 1), j);
				vec3 col = med4(rgb);
				cam.add(i, j, col / col.w() / cam.exposure);
			}
		}
	}
	float blink = 0.05f * (1 - fabsf(sinf(blink_t)));
#pragma omp parallel for collapse(2) schedule(dynamic, 100)
	for (int i = 0; i < cam.h; i++) {
		for (int j = 0; j < cam.w; j++) {
			uint off = i * pitch + j;
			vec3 rgb = cam.get_med(i, j, opt.med_thr);
			if (opt.selected != -1) {
				aabb aura = world.objects[opt.selected].get_box();
				ray r = cam.optical_ray(i, j);
				if (aura.hit(r))bgr(rgb.fact() + blink, cam.CCD.disp[off]);
				else bgr(rgb, cam.CCD.disp[off]);
			}
			else bgr(rgb, cam.CCD.disp[off]);
		}
	}
	odd = !odd && opt.re_sam;
	opt.li_sa = old_lisa;
	opt.sun_sa = old_sunsa;
	cam.moving = 0;
}

void scene::Screenshot() const {
	int spp = cam.CCD.spp;
	uint wh = cam.CCD.n;
	uint w = cam.CCD.w;
	uint h = cam.CCD.h;
	char name[40] = {};
	std::tm tm; time_t now = time(0);
	localtime_s(&tm, &now);
	strftime(name, sizeof(name), "%Y%m%d_%H%M%S", &tm);
	string file;
	string name_spp = string(name) + "_" + std::to_string(spp) + "SPP_";
	vector<uint> buff(wh);
	file = "screenshots\\" + name_spp + ".png";
	for (uint i = 0; i < wh; i++)
		rgb(median2d3(cam.CCD.data.data(), i / w, i % w, h, w, opt.med_thr), buff[i]);
	stbi_write_png(file.c_str(), w, h, 4, buff.data(), 4 * w);
	cout << "Saved file in: " << file << "\n";
}

void scene::save(const char* name) const {
	std::ofstream out;
	out.open(std::string(name) + ".scn");
	out.write((char*)this, sizeof(scene));
	out.close();
}
void scene::load(const char* name) {
	std::ifstream out;
	out.open(std::string(name) + ".scn");
	out.read((char*)this, sizeof(scene));
	out.close();
}
