#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <omp.h>
#include "Scene.h"
#if DEBUG
//eeewww global variable
bool use_normal_maps = 1;
#endif
// a=f/m
// v+=a*t
// x+=v*t
vec4 Scene::cam_collision(vec4 v, float dt) const {
	if (!cam.collision)return 0;
	hitrec rec;
	for (int i = 0; i < 5; i++)
	{
		vec4 d = v + eps * ravec();
		ray r = ray(cam.T.P(), d, true);
		if (world.hit(r, rec) && rec.t - d.len() * dt <= eps) {
			vec4 V = rec.N * (d.len() - rec.t * dt) * dot(rec.N, -r.D) / dot(r.D, norm(v));
			return clamp(V, -abs(v), abs(v));
		}
	}
	return 0;
}
vec4 Scene::cam_free(vec4 d) const {
	vec4 s = cam.T.vec(vec4(1, 0, 0));
	vec4 fw = cam.T.vec(vec4(0, 0, -1)), si(s.x(), 0, s.z()), up = vec4(0, 1, 0);
	fw = norm(fw);
	si = norm(si);
	return  (d.x() * fw + d.y() * up + d.z() * si) * cam.speed;
}
vec4 Scene::cam_fps(vec4 d) const {
	vec4 f = cam.T.vec(vec4(0, 0, -1));
	vec4 s = cam.T.vec(vec4(1, 0, 0));
	vec4 fw(f.x(), 0, f.z()), si(s.x(), 0, s.z()), up = vec4(0, 1, 0);
	fw = norm(fw);
	si = norm(si);
	return (d.x() * fw + d.y() * up + d.z() * si) * cam.speed;
}
void Scene::cam_move(vec4 dir, float dt) {
	vec4& V = cam.V;
	vec4 G = vec4(0, -9.81, 0);
	vec4 F0 = cam.free ? cam_free(dir) : cam_fps(dir);
	vec4 F = F0;
	V += F * dt;
	V += cam_collision(V, dt);
	V += cam_collision(V, dt);
	V += cam_collision(V, dt);
	if (eq0(cam_collision(V, dt))) cam.move(V * dt);
	V -= fminf(10 * dt, 1) * V;
	V = min(max(V, infn), infp);
}
void Scene::cam_rotate(vec4 dir, float dt) {
	cam.rotate(dir * dt * cam.sens);
}
void Scene::cam_autofocus() {
	if (cam.autofocus) {
		ray r(cam.focus_ray());
		float new_t = closest_t(r);
		cam.foc_t = 0.5 * cam.foc_t + 0.5f * new_t;
	}
}
void Scene::cam_manufocus(float py, float px) {
	if (!cam.autofocus) {
		ray r(cam.pinhole_ray(vec4(px, py)));
		cam.foc_t = fminf(closest_t(r), 1e6);
		cam.moving = 1;
	}
}
uint Scene::get_id(const ray& r, hitrec& rec) const
{
	return world.get_id(r, rec);
}
vec4 Scene::get_point(float py, float px, float max_t) const
{
	ray r(cam.pinhole_ray(vec4(px, py)));
	hitrec rec;
	if (world.hit(r, rec)) {
		return rec.P;
	}
	else return r.at(max_t);
}
float Scene::get_dist(float py, float px) const
{
	ray r(cam.pinhole_ray(vec4(px, py)));
	hitrec rec;
	world.hit(r, rec);
	return rec.t;
}
float Scene::get_dist_box(float py, float px) const
{
	ray r(cam.pinhole_ray(vec4(px, py)));
	float t = infp;
	selected().get_box().hit(r,t);
	return t;
}
uint Scene::get_id(float py, float px)
{
	ray r(cam.pinhole_ray(vec4(px, py)));
	hitrec rec;
	uint id = get_id(r, rec);
	opt.selected = id;
	return id;
}
obj_flags Scene::get_flag() const
{
	if (opt.selected < world.objects.size()) return world.get_flag(opt.selected);
	else return obj_flags(o_bla, 0, 0);
}
const char* Scene::get_name() const
{
	if (opt.selected < world.objects.size()) return world.objects[opt.selected].get_name();
	else return "Empty";
}
obj_flags Scene::get_trans(mat4& T) const {
	if (opt.selected < world.objects.size())
	{
		world.get_trans(opt.selected, T);
		return world.get_flag(opt.selected);

	}
	else
	{
		//if nothing hit, get sky
		T = sun_pos;
		return obj_flags(o_bla, 0, 0);
	}
}

void Scene::set_trans(const mat4& T) {
	if (opt.selected < world.objects.size())
	{
		world.set_trans(opt.selected, T, opt.node_size);
	}
	else
	{
		sun_pos = T;
		sun_pos.set_P(vec4());

	}
}

void Scene::set_skybox(const char* name)
{
	skybox = name;
	opt.skybox = true;
}

void Scene::Render(uint* disp, uint pitch) {
	cam.CCD.set_disp(disp, pitch);
	cam_autofocus();
	static bool odd = 0;
	float blink_t = (hpi * clock()) / CLOCKS_PER_SEC;
	float blink = 0.02f * (1 + sinf(2*blink_t));
	opt.selected = clamp_int(opt.selected, -1, world.objects.size() - 1);
	if (cam.moving)cam.CCD.reset();
	bool paused = !cam.moving && opt.paused;
	if (cam.CCD.spp < opt.max_spp && !paused && opt.samples > 0) {
		world.en_bvh = opt.en_bvh && !world.bvh.empty() && !world.objects.empty() && !world.obj_bvh.empty();
		li_sa = opt.li_sa && !world.lights.empty();
		sun_sa = opt.sun_sa && opt.sky && !opt.skybox;
		inv_sa = 1.0 / opt.samples;
		cam.CCD.dt(opt.samples);
#pragma omp parallel for collapse(2) schedule(dynamic, 100)
		for (int i = 0; i < cam.h; i++) {
			for (int j = opt.p_mode ? (i + odd) % 2 : 0; j < cam.w; j += opt.p_mode ? 2 : 1) {
				if (cam.moving) cam.CCD.clear(i, j);
				vec4 xy = vec4(j, i) + 0.5f * ravec();
				ray r = cam.optical_ray(xy);
#if DEBUG
				if (opt.dbg_at)		 cam.add(i, j, dbg_at(r));
				else if (opt.dbg_n)   cam.add(i, j, dbg_n(r));
				else if (opt.dbg_uv)  cam.add(i, j, dbg_uv(r));
				else if (opt.dbg_f)  cam.add(i, j, dbg_f(r));
				else if (opt.dbg_bvh)  cam.add(i, j, dbg_bvh(r));
				else if (opt.dbg_e)  cam.add(i, j, dbg_e(r));
				else if (opt.dbg_t)  cam.add(i, j, dbg_t(r));
				else if (opt.dbg_light)  cam.add(i, j, dbg_ill(r));
				else cam.add(i, j, raycol(r));
#else
				cam.add(i, j, raycol(r));
#endif
			}
		}
		if (opt.p_mode) {
#pragma omp parallel for collapse(2) schedule(dynamic, 100)
			for (int i = 0; i < cam.h; i++) {
				for (int j = (i + !odd) % 2; j < cam.w; j += 2) {
					if (cam.moving) cam.CCD.clear(i, j);
					vec4 rgb[4];
					rgb[0] = cam.CCD.get(i, fmax_int(j - 1, 0));
					rgb[1] = cam.CCD.get(i, fmin_int(j + 1, cam.w - 1));
					rgb[2] = cam.CCD.get(fmax_int(i - 1, 0), j);
					rgb[3] = cam.CCD.get(fmin_int(i + 1, cam.h - 1), j);
					vec4 col = med4(rgb);
					cam.add(i, j, col);
				}
			}
		}
	}
#pragma omp parallel for collapse(2) schedule(dynamic, 100)
	for (int i = 0; i < cam.h; i++) {
		for (int j = 0; j < cam.w; j++) {
			vec4 rgb = cam.get_med(i, j, opt.med_thr);
			if (i % 2 && j % 2 && opt.selected < world.objects.size() && !opt.framegen && opt.outline) {
				hitrec rec;
				ray r = cam.pinhole_ray(vec4(j, i));
				aabb aura = object_at(opt.selected).get_box();
				if (aura.hit(r)) {
					cam.display(i, j, rgb + blink);
				}
				else cam.display(i, j, rgb);
			}
			else cam.display(i, j, rgb);
		}
	}
	odd = !odd && opt.re_sam;
	cam.moving = 0;
}
void Scene::Gen_projection(const projection& proj) {
	vec4* buff = cam.CCD.buff.data();
	vec4* data = cam.CCD.data.data();
	mat4 iT = cam.T.inverse();
	for (int i = 0; i < cam.CCD.n; i++)
		buff[i] = vec4(0, 0, 0, infp*infp);
#pragma omp parallel for collapse(2) schedule(dynamic, 100)
	for (int i = 0; i < cam.h; i++) {
		for (int j = 0; j < cam.w; j++) {
			uint off = i * cam.w + j;
			float dist = data[off].w();
			vec4 xy = cam.SS(vec4(j, i), proj);
			vec4 pt = proj.T.P() + proj.T.vec(norm(xy)) * dist;
			vec4 spt = iT.pnt(pt);
			if (spt.z() < 0) [[likely]] {
				vec4 uv = spt / fabsf(spt.z());
				uv = cam.inv_SS(uv);
				int x = uv[0], y = uv[1];
				if (x < cam.w && y < cam.h && dist <= buff[y * cam.w + x].w())
					buff[y * cam.w + x] = vec4(data[off], dist);
			}
		}
	}
}
void Scene::Reproject(const projection& proj, uint* disp, uint pitch) {
	cam.CCD.set_disp(disp, pitch);
	if (cam.moving || opt.framegen)
	Gen_projection(proj);
	vec4* buff = cam.CCD.buff.data();
#pragma omp parallel for collapse(2) schedule(dynamic, 100)
	for (int i = 0; i < cam.h; i++) {
		for (int j = 0; j < cam.w; j++) {
			cam.display(i, j, median_3x3(buff, i, j, cam.h, cam.w, opt.med_thr));
		}
	}
	if(!opt.framegen)cam.moving = false;
}
void Scene::Screenshot(bool reproject) const {
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
	if (reproject) {
		for (uint i = 0; i < wh; i++)
			buff[i] = vec2rgb(median_3x3(cam.CCD.buff.data(), i / w, i % w, h, w, opt.med_thr)) | (255 << 24);
	}
	else {
		for (uint i = 0; i < wh; i++)
			buff[i] = vec2rgb(median_3x3(cam.CCD.data.data(), i / w, i % w, h, w, opt.med_thr)) | (255 << 24);
	}
	stbi_write_png(file.c_str(), w, h, 4, buff.data(), 4 * w);
	cout << "Saved file in: " << file << "\n";
}
