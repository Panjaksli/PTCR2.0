#include "Scenes.h"
struct ln {
	ln() {}
	ln(const char* text) { strcpy_s(x, 255, text); }
	operator char* () { return x; }
	char x[256] = {};
};
//Simple text format
bool scn_save(Scene& scn, const char* filename) {
	path name(u8path(filename));
	if (name.empty())return false;
	if (!name.has_extension())
		name.replace_extension(".scn");
	if (!name.has_parent_path())
		name = "scenes" / name;
	std::ofstream file(name);
	if (!file.is_open()) {
		cout << "Failed to write to file: " << name << "\n";
		return false;
	}
	double time = timer();
	ln line; vector<ln> lines;
	lines.reserve(256);
	lines.push_back("*Objects");
	for (const auto& obj : scn.world.objects) {
		mat4 T = obj.get_trans();
		vec4 P = T.P(), A = T.A();
		if (obj.get_size() > 0) {
			if (obj.name.empty()) {
				if (obj.type() == o_sph) {
					if (obj.get_size() == 1) {
						vec4 q = obj.s.get_raw(0).Qr;
						sprintf(line, "sphere P=%g,%g,%g,%g A=%g,%g,%g mat=%d bvh=%d lig=%d fog=%d {%g,%g,%g,%g}",
							P[0], P[1], P[2], P[3], A[0], A[1], A[2], obj.get_mat(), obj.bvh(), obj.light(), obj.fog(), q[0], q[1], q[2], q[3]);
						lines.push_back(line);
					}
					else if (obj.get_size() > 1) {
						sprintf(line, "sphere P=%g,%g,%g,%g A=%g,%g,%g mat=%d bvh=%d lig=%d fog=%d {",
							P[0], P[1], P[2], P[3], A[0], A[1], A[2], obj.get_mat(), obj.bvh(), obj.light(), obj.fog());
						lines.push_back(line);
						for (int i = 0; i < obj.get_size(); i++) {
							vec4 q = obj.s.get_raw(i).Qr;
							sprintf(line, "%g,%g,%g,%g", q[0], q[1], q[2], q[3]); lines.push_back(line);
						}
						lines.push_back("}");
					}
				}
				else if (obj.type() == o_vox) {
					if (obj.get_size() == 1) {
						vec4 q = obj.v.get_raw(0).Qa;
						sprintf(line, "voxel P=%g,%g,%g,%g A=%g,%g,%g mat=%d bvh=%d lig=%d fog=%d {%g,%g,%g,%g}",
							P[0], P[1], P[2], P[3], A[0], A[1], A[2], obj.get_mat(), obj.bvh(), obj.light(), obj.fog(), q[0], q[1], q[2], q[3]);
						lines.push_back(line);
					}
					else if (obj.get_size() > 1) {
						sprintf(line, "voxel P=%g,%g,%g,%g A=%g,%g,%g mat=%d bvh=%d lig=%d fog=%d {",
							P[0], P[1], P[2], P[3], A[0], A[1], A[2], obj.get_mat(), obj.bvh(), obj.light(), obj.fog());
						lines.push_back(line);
						for (int i = 0; i < obj.get_size(); i++) {
							vec4 q = obj.v.get_raw(0).Qa;
							sprintf(line, "%g,%g,%g,%g", q[0], q[1], q[2], q[3]); lines.push_back(line);
						}
						lines.push_back("}");
					}
				}
				else if (obj.type() == o_qua) {
					if (obj.get_size() == 1) {
						char data[256] = {};
						vec4 a = obj.p.get_raw(0).A();
						vec4 b = obj.p.get_raw(0).B();
						vec4 c = obj.p.get_raw(0).C();
						sprintf(data, "%g,%g,%g,%g,%g,%g,%g,%g,%g", a[0], a[1], a[2], b[0], b[1], b[2], c[0], c[1], c[2]);
						sprintf(line, "quad P=%g,%g,%g,%g A=%g,%g,%g mat=%d bvh=%d lig=%d fog=%d {%s}",
							P[0], P[1], P[2], P[3], A[0], A[1], A[2], obj.get_mat(), obj.bvh(), obj.light(), obj.fog(), data);
						lines.push_back(line);
					}
					else if (obj.get_size() > 1) {
						sprintf(line, "quad P=%g,%g,%g,%g A=%g,%g,%g mat=%d bvh=%d lig=%d fog=%d {",
							P[0], P[1], P[2], P[3], A[0], A[1], A[2], obj.get_mat(), obj.bvh(), obj.light(), obj.fog());
						lines.push_back(line);
						for (int i = 0; i < obj.get_size(); i++) {
							vec4 a = obj.q.get_raw(i).A();
							vec4 b = obj.q.get_raw(i).B();
							vec4 c = obj.q.get_raw(i).C();
							sprintf(line, "%g,%g,%g,%g,%g,%g,%g,%g,%g", a[0], a[1], a[2], b[0], b[1], b[2], c[0], c[1], c[2]); lines.push_back(line);
						}
						lines.push_back("}");
					}
				}
				else if (obj.type() == o_pol) {
					if (obj.get_size() == 1) {
						ln data;
						vec4 a = obj.p.get_raw(0).A();
						vec4 b = obj.p.get_raw(0).B();
						vec4 c = obj.p.get_raw(0).C();
						sprintf(data, "%g,%g,%g,%g,%g,%g,%g,%g,%g", a[0], a[1], a[2], b[0], b[1], b[2], c[0], c[1], c[2]);
						sprintf(line, "poly P=%g,%g,%g,%g A=%g,%g,%g mat=%d bvh=%d lig=%d fog=%d {%s}",
							P[0], P[1], P[2], P[3], A[0], A[1], A[2], obj.get_mat(), obj.bvh(), obj.light(), obj.fog(), data.x);
						lines.push_back(line);
					}
					else if (obj.get_size() > 1) {
						sprintf(line, "poly P=%g,%g,%g,%g A=%g,%g,%g mat=%d bvh=%d lig=%d fog=%d {",
							P[0], P[1], P[2], P[3], A[0], A[1], A[2], obj.get_mat(), obj.bvh(), obj.light(), obj.fog());
						lines.push_back(line);
						for (int i = 0; i < obj.get_size(); i++) {
							vec4 a = obj.p.get_raw(i).A();
							vec4 b = obj.p.get_raw(i).B();
							vec4 c = obj.p.get_raw(i).C();
							sprintf(line, "%g,%g,%g,%g,%g,%g,%g,%g,%g", a[0], a[1], a[2], b[0], b[1], b[2], c[0], c[1], c[2]); lines.push_back(line);
						}
						lines.push_back("}");
					}
				}
			}
			else {
				sprintf(line, "mesh P=%g,%g,%g,%g A=%g,%g,%g mat=%d bvh=%d lig=%d fog=%d {%s}",
					P[0], P[1], P[2], P[3], A[0], A[1], A[2], obj.get_mat(), obj.bvh(), obj.light(), obj.fog(), obj.name.text());
				lines.push_back(line);
			}
		}
	}
	lines.push_back("*Materials");
	for (const auto& mat : scn.world.materials) {
		vec4 srgb = mat.tex._rgb.rgb;
		vec4 smer = mat.tex._mer.rgb;
		vec4 snor = mat.tex._nor.rgb;
		vec4 tint = mat.tex.tint;
		int s1 = mat.tex._rgb.solid();
		int s2 = mat.tex._mer.solid();
		int s3 = mat.tex._nor.solid();
		char rgb[256], mer[256], nor[256];
		sprintf(rgb, "%s,%g,%g,%g,%g,%d", mat.tex._rgb.name.text(), srgb[0], srgb[1], srgb[2], srgb[3], s1);
		sprintf(mer, "%s,%g,%g,%g,%d", mat.tex._mer.name.text(), smer[0], smer[1], smer[2], s2);
		sprintf(nor, "%s,%g,%g,%g,%d", mat.tex._nor.name.text(), snor[0], snor[1], snor[2], s3);
		sprintf(line, "albedo type=%d rgb=%s mer=%s nor=%s scl=%g ir=%g tint=%g,%g,%g,%g alpha=%d check=%d",
			mat.type, rgb, mer, nor, mat.tex.rep, mat.tex.ir, tint[0], tint[1], tint[2], tint[3], mat.tex.alpha(), mat.tex.checker());
		lines.push_back(line);
	}
	lines.push_back("*Params");
	sprintf(line, "skybox=%s", scn.skybox.name.text()); lines.push_back(line);
	sprintf(line, "ambient=%g,%g,%g", scn.opt.ambient[0], scn.opt.ambient[1], scn.opt.ambient[2]); lines.push_back(line);
	sprintf(line, "sky_noon=%g,%g,%g", scn.opt.sky_noon[0], scn.opt.sky_noon[1], scn.opt.sky_noon[2]); lines.push_back(line);
	sprintf(line, "sky_dawn=%g,%g,%g", scn.opt.sky_dawn[0], scn.opt.sky_dawn[1], scn.opt.sky_dawn[2]); lines.push_back(line);
	sprintf(line, "sun_noon=%g,%g,%g", scn.opt.sun_noon[0], scn.opt.sun_noon[1], scn.opt.sun_noon[2]); lines.push_back(line);
	sprintf(line, "sun_dawn=%g,%g,%g", scn.opt.sun_dawn[0], scn.opt.sun_dawn[1], scn.opt.sun_dawn[2]); lines.push_back(line);
	sprintf(line, "fog_col=%g,%g,%g", scn.opt.fog_col[0], scn.opt.fog_col[1], scn.opt.fog_col[2]); lines.push_back(line);
	sprintf(line, "sun_pos=%g,%g,%g", scn.sun_pos.A()[0], scn.sun_pos.A()[1], scn.sun_pos.A()[2]); lines.push_back(line);
	sprintf(line, "bounces=%d", scn.opt.bounces); lines.push_back(line);
	sprintf(line, "samples=%d", scn.opt.samples); lines.push_back(line);
	sprintf(line, "en_fog=%d", scn.opt.en_fog); lines.push_back(line);
	sprintf(line, "en_bvh=%d", scn.opt.en_bvh); lines.push_back(line);
	sprintf(line, "en_sky=%d", scn.opt.sky); lines.push_back(line);
	sprintf(line, "en_box=%d", scn.opt.skybox); lines.push_back(line);
	sprintf(line, "select=%d", scn.opt.selected); lines.push_back(line);
	sprintf(line, "outline=%d", scn.opt.outline); lines.push_back(line);
	sprintf(line, "fog_dens=%g", scn.opt.ninv_fog); lines.push_back(line);
	sprintf(line, "cam_collision=%d", scn.cam.collision); lines.push_back(line);
	sprintf(line, "cam_blur=%d", scn.cam.bokeh); lines.push_back(line);
	sprintf(line, "cam_auto=%d", scn.cam.autofocus); lines.push_back(line);
	sprintf(line, "cam_fov=%g", scn.cam.fov); lines.push_back(line);
	sprintf(line, "cam_fstop=%g", scn.cam.fstop); lines.push_back(line);
	sprintf(line, "cam_exp=%g", scn.cam.exposure); lines.push_back(line);
	sprintf(line, "cam_foc_t=%g", scn.cam.foc_t); lines.push_back(line);
	sprintf(line, "cam_speed=%g", scn.cam.speed); lines.push_back(line);
	sprintf(line, "cam_pos=%g,%g,%g", scn.cam.P[0], scn.cam.P[1], scn.cam.P[2]); lines.push_back(line);
	sprintf(line, "cam_rot=%g,%g,%g", scn.cam.T.A()[0], scn.cam.T.A()[1], scn.cam.T.A()[2]); lines.push_back(line);
	for (const auto& line : lines)
		file << line.x << std::endl;
	cout << "Saved scene: " << name << " took: " << timer(time) << "\n";
	return true;
}

bool scn_load(Scene& scn, const char* filename, bool update_only) {
	double time = timer();
	path name(u8path(filename));
	if (name.empty())return false;
	if (!name.has_extension())
		name.replace_extension(".scn");
	if (!name.has_parent_path())
		name = "scenes" / name;
	std::ifstream file(name);
	if (!file.is_open()) {
		cout << "File not found: " << name << "\n";
		return false;
	}
	if (!update_only)scn.skybox.clear();
	scn.world.clear();
	constexpr int BUFF = 256;
	string line; line.reserve(BUFF);
	int state = -1;
	while (std::getline(file, line)) {
		vec4 V, P, A;
		vec4 q, a, b, c;
		char buff1[BUFF], buff2[BUFF], buff3[BUFF];
		int mat = 0, bvh = 1, lig = 0, fog = 0, ival = 0;
		int alpha = 0, check = 0;
		float scl = 1, ir = 1, fval = 0;
		if (line[0] == '*') { state++; }
		else switch (state) {
		case 0:
			if (sscanf_s(line.c_str(), "%s P=%g,%g,%g,%g A=%g,%g,%g mat=%d bvh=%d lig=%d fog=%d {%s",
				buff1, BUFF, &P[0], &P[1], &P[2], &P[3], &A[0], &A[1], &A[2], &mat, &bvh, &lig, &fog, buff2, BUFF) >= 4) {
				if (strcmp(buff1, "poly") == 0) {
					vector<poly> data; data.reserve(100);
					if (sscanf_s(buff2, "%g,%g,%g,%g,%g,%g,%g,%g,%g", &a[0], &a[1], &a[2], &b[0], &b[1], &b[2], &c[0], &c[1], &c[2]) > 1)
						data.emplace_back(a, b, c);
					else {
						while (std::getline(file, line)) {
							if (line[0] == '}') break;
							else if (sscanf_s(line.c_str(), "%g,%g,%g,%g,%g,%g,%g,%g,%g", &a[0], &a[1], &a[2], &b[0], &b[1], &b[2], &c[0], &c[1], &c[2]) > 1)
								data.emplace_back(a, b, c);
						}
					}
					scn.world.add_mesh(data, mat4(P, A), mat, bvh, lig, fog, 0);
				}
				else if (strcmp(buff1, "quad") == 0) {
					vector<quad> data; data.reserve(100);
					if (sscanf_s(buff2, "%g,%g,%g,%g,%g,%g,%g,%g,%g", &a[0], &a[1], &a[2], &b[0], &b[1], &b[2], &c[0], &c[1], &c[2]) > 1)
						data.emplace_back(a, b, c);
					else {
						while (std::getline(file, line)) {
							if (line[0] == '}') break;
							else if (sscanf_s(line.c_str(), "%g,%g,%g,%g,%g,%g,%g,%g,%g", &a[0], &a[1], &a[2], &b[0], &b[1], &b[2], &c[0], &c[1], &c[2]) > 1)
								data.emplace_back(a, b, c);
						}
					}
					scn.world.add_mesh(data, mat4(P, A), mat, bvh, lig, fog, 0);
				}
				else if (strcmp(buff1, "sphere") == 0) {
					vector<sphere> data; data.reserve(100);
					if (sscanf_s(buff2, "%g,%g,%g,%g", &q[0], &q[1], &q[2], &q[3]) > 1)
						data.emplace_back(q);
					else {
						while (std::getline(file, line)) {
							if (line[0] == '}') break;
							else if (sscanf_s(line.c_str(), "%g,%g,%g,%g", &q[0], &q[1], &q[2], &q[3]) > 1)
								data.emplace_back(q);
						}
					}
					scn.world.add_mesh(data, mat4(P, A), mat, bvh, lig, fog, 0);
				}
				else if (strcmp(buff1, "voxel") == 0) {
					vector<voxel> data; data.reserve(100);
					if (sscanf_s(buff2, "%g,%g,%g,%g", &q[0], &q[1], &q[2], &q[3]) > 1)
						data.emplace_back(q);
					else {
						while (std::getline(file, line)) {
							if (line[0] == '}') break;
							else if (sscanf_s(line.c_str(), "%g,%g,%g,%g", &q[0], &q[1], &q[2], &q[3]) > 1)
								data.emplace_back(q);
						}
					}
					scn.world.add_mesh(data, mat4(P, A), mat, bvh, lig, fog, 0);
				}
				else if (strcmp(buff1, "mesh") == 0) {
					sscanf_s(buff2, "%[^}]s", buff1, BUFF);
					scn.world.load_mesh(buff1, mat4(P, A), mat, bvh, lig, fog);
				}
			}
			break;
		case 1:
			if (sscanf_s(line.c_str(), "albedo type=%d rgb=%s mer=%s nor=%s scl=%g ir=%g tint=%g,%g,%g,%g alpha=%d check=%d",
				&mat, buff1, BUFF, buff2, BUFF, buff3, BUFF, &scl, &ir, &V[0], &V[1], &V[2], &V[3], &alpha, &check) > 0) {
				int s1 = -1, s2 = -1, s3 = -1;
				vec4 srgb, smer, snor;
				char rgb[BUFF], mer[BUFF], nor[BUFF];
				c_str::replace(buff1, ',', ' ', BUFF);
				c_str::replace(buff2, ',', ' ', BUFF);
				c_str::replace(buff3, ',', ' ', BUFF);
				sscanf_s(buff1, "%s %g %g %g %g %d", rgb, BUFF, &srgb[0], &srgb[1], &srgb[2], &srgb[3], &s1);
				sscanf_s(buff2, "%s %g %g %g %d", mer, BUFF, &smer[0], &smer[1], &smer[2], &s2);
				sscanf_s(buff3, "%s %g %g %g %d", nor, BUFF, &snor[0], &snor[1], &snor[2], &s3);
				scn.world.add_mat(
					albedo(
						texture(srgb, rgb, s1),
						texture(smer, mer, s2),
						texture(snor, nor, s3),
						scl, ir, V, alpha, check
					),
					(mat_enum)mat
				);
			}
			break;
		case 2:
			if (!update_only) {
				if (sscanf_s(line.c_str(), "skybox=%s", buff1, BUFF) > 0) { scn.set_skybox(buff1); }
				else if (sscanf_s(line.c_str(), "ambient=%g,%g,%g", &V[0], &V[1], &V[2]) > 0) { scn.opt.ambient = V; }
				else if (sscanf_s(line.c_str(), "sky_noon=%g,%g,%g", &V[0], &V[1], &V[2]) > 0) { scn.opt.sky_noon = V; }
				else if (sscanf_s(line.c_str(), "sky_dawn=%g,%g,%g", &V[0], &V[1], &V[2]) > 0) { scn.opt.sky_dawn = V; }
				else if (sscanf_s(line.c_str(), "sun_noon=%g,%g,%g", &V[0], &V[1], &V[2]) > 0) { scn.opt.sun_noon = V; }
				else if (sscanf_s(line.c_str(), "sun_dawn=%g,%g,%g", &V[0], &V[1], &V[2]) > 0) { scn.opt.sun_dawn = V; }
				else if (sscanf_s(line.c_str(), "fog_col=%g,%g,%g", &V[0], &V[1], &V[2]) > 0) { scn.opt.fog_col = V; }
				else if (sscanf_s(line.c_str(), "sun_pos=%g,%g,%g", &V[0], &V[1], &V[2]) > 0) { scn.sun_pos.set_A(V); }
				else if (sscanf_s(line.c_str(), "bounces=%d", &ival) > 0) scn.opt.bounces = ival;
				else if (sscanf_s(line.c_str(), "samples=%d", &ival) > 0) scn.opt.samples = ival;
				else if (sscanf_s(line.c_str(), "en_fog=%d", &ival) > 0) scn.opt.en_fog = ival;
				else if (sscanf_s(line.c_str(), "en_bvh=%d", &ival) > 0) scn.opt.en_bvh = ival;
				else if (sscanf_s(line.c_str(), "en_sky=%d", &ival) > 0) scn.opt.sky = ival;
				else if (sscanf_s(line.c_str(), "en_box=%d", &ival) > 0) scn.opt.skybox = ival;
				else if (sscanf_s(line.c_str(), "select=%d", &ival) > 0) scn.opt.selected = ival;
				else if (sscanf_s(line.c_str(), "outline=%d", &ival) > 0) scn.opt.outline = ival;
				else if (sscanf_s(line.c_str(), "fog_dens=%g", &fval) > 0) scn.opt.ninv_fog = fval;
				else if (sscanf_s(line.c_str(), "cam_collision=%d", &ival) > 0) scn.cam.collision = ival;
				else if (sscanf_s(line.c_str(), "cam_blur=%d", &ival) > 0) scn.cam.bokeh = ival;
				else if (sscanf_s(line.c_str(), "cam_auto=%d", &ival) > 0) scn.cam.autofocus = ival;
				else if (sscanf_s(line.c_str(), "cam_fov=%g", &fval) > 0) scn.cam.set_fov(fval);
				else if (sscanf_s(line.c_str(), "cam_fstop=%g", &fval) > 0) scn.cam.fstop = fval;
				else if (sscanf_s(line.c_str(), "cam_exp=%g", &fval) > 0) scn.cam.exposure = fval;
				else if (sscanf_s(line.c_str(), "cam_foc_t=%g", &fval) > 0) scn.cam.foc_t = fval;
				else if (sscanf_s(line.c_str(), "cam_speed=%g", &fval) > 0) scn.cam.speed = fval;
				else if (sscanf_s(line.c_str(), "cam_pos=%g,%g,%g", &P[0], &P[1], &P[2]) > 0) scn.cam.set_P(P);
				else if (sscanf_s(line.c_str(), "cam_rot=%g,%g,%g", &A[0], &A[1], &A[2]) > 0) scn.cam.set_A(A);
			}
			break;
		default: break;
		}
	}
	scn.cam.V = 0;
	scn.cam.moving = 1;
	scn.world.build_bvh(1, scn.opt.node_size);
	scn.world.update_lists();
	cout << "Loaded scene: " << name << " took: " << timer(time) << "\n";
	return true;
}
void scn_load(Scene& scn, int n) {
	scn.skybox.clear();
	scn.world.clear();
	scn.opt = scene_opt();
	scn.cam.reset_opt();
	switch (n) {
	case 1: scn1(scn); break;
	case 2: scn2(scn); break;
	case 3: scn3(scn); break;
	case 4: scn4(scn); break;
	case 5: scn5(scn); break;
	case 6: scn6(scn); break;
	case 7: scn7(scn); break;
	case 8: scn8(scn); break;
	default: scn1(scn); break;
	}
	scn.cam.moving = 1;
#if TEST
	scn.opt.med_thr = 0;
	scn.opt.p_mode = 0;
	//scn.opt.en_fog = 0;
	scn.cam.bokeh = 0;
#endif
	scn.cam.V = 0;
	scn.world.update_lists();
	scn.world.build_bvh(1, scn.opt.node_size);
}
void scn1(Scene& scn) {
	albedo gre(vec4(0.7, 0.9, 0.7, 0), 0, vec4(0.5, 0.5, 1), 1, 1.2);
	albedo carpet(vec4(0.8, 0.2, 0.2, 1), vec4(0, 0, 1), "snow_normal", 10);
	albedo blu(vec4(0.1, 0.28, 0.8, 1), vec4(0.5, 0, 0.1));
	scn.set_skybox("hotel");
	scn.world.add_mat(carpet, mat_mix);
	scn.world.add_mat(gre, mat_mix);
	scn.world.add_mat(blu, mat_ggx);
	scn.world.add_mesh(quad(vec4(-10, 0, -10), vec4(-10, 0, 10), vec4(10, eps, -10)), vec4(0, 1, 0, 1), 0, 0);
	scn.world.load_mesh("bunny", vec4(-0.44, 0.8, -1, 6), 1);
	scn.world.load_mesh("teapot", vec4(0.7, 1, -1, 0.25), 2);
	scn.cam.autofocus = 0;
	scn.cam.foc_t = 1.556;
	scn.cam.exposure = 1.f;
	scn.sun_pos = rotat_mat4(vec4(1, 0, 0.32));
	scn.opt.bounces = 10;
	scn.opt.ninv_fog = -5;
	scn.opt.p_life = 0.9f;
	scn.opt.i_life = 1.f / 0.9f;
	scn.cam.setup(mat4(vec4(0.1, 1.4, 0.8), 0), 70, 1.f);
	scn.opt.en_fog = false;
	scn.opt.en_bvh = 1;
}

void scn2(Scene& scn) {
	scn.opt.li_sa = 1;
	albedo iron(vec4(0.7, 0.7, 0.9), vec4(1, 0, 0));
	albedo white(vec4(0.8, 0.8, 0.8), vec4(0, 2.2, 0));
	scn.world.add_mat(albedo(vec4(0.8, 0.8, 0.8, 1), vec4(0, 0, 1)), mat_mix);
	scn.world.add_mat(iron, mat_ggx);
	scn.world.add_mat(white, mat_lig);
	vector<quad> room(6);
	room.emplace_back(quad(vec4(-1, 0, -1), vec4(3, 0, -1), vec4(-1, 0, 1)));
	room.emplace_back(quad(vec4(-1, 0, -1), vec4(-1, 2, -1), vec4(-1, 0, 1)));
	room.emplace_back(quad(vec4(3, 0, -1), vec4(3, 1.5, -1), vec4(3, 0, 1)));
	room.emplace_back(quad(vec4(-1, 2, -1), vec4(3, 2, -1), vec4(-1, 2, 1)));
	room.emplace_back(quad(vec4(-1, 0, -1), vec4(3, 0, -1), vec4(-1, 2, -1)));
	room.emplace_back(quad(vec4(-1, 0, 1), vec4(3, 0, 1), vec4(-1, 2, 1)));
	scn.opt.ninv_fog = -100;
	scn.world.add_mesh(room, vec4(0, 0, 0, 1), 0);
	scn.world.add_mesh(quad(0, vec4(1, 0, 0), vec4(0, 1, 0.999)), vec4(0, 0, 0, 1), 1, 0); //0.24 0 0.67
	scn.world.add_mesh(voxel(vec4(0, 1.9, 0, 0.1)), vec4(0, 0, 0, 1), 2, 0, 1);
	scn.sun_pos = rotat_mat4(vec4(-1, 0, 0));
	scn.cam.setup(mat4(vec4(2, 1, 0), vec4(0, hpi, 0)), 90, 10);
	scn.opt.en_bvh = 0;
}
void scn3(Scene& scn) {
	for (int i = 0; i <= 10; i++) {
		for (int j = 0; j <= 10; j++) {
			albedo pbrcol(vec4(0.8f, 0.1f, 0.1f), vec4(0.1 * i, 0, 0.1 * j), vec4(0.5, 0.5, 1), 10);
			scn.world.add_mat(pbrcol, mat_ggx);
			scn.world.add_mesh(sphere(vec4(0, 0, 0, 0.5)), vec4(6 - i, 6 - j, -3, 1), i * 11 + j);
		}
	}
	scn.opt.en_fog = 0;
	scn.sun_pos = rotat_mat4(vec4(1, 0, 1));
	scn.cam.setup(mat4(vec4(1, 1, 10), 0), 47, 10);
	scn.opt.en_bvh = 1;
}
void scn4(Scene& scn) {
	albedo gre(vec4(0.7, 0.9, 0.7, 0), 0, vec4(0.5, 0.5, 1), 1, 1.2);
	albedo yel(vec4(0.5, 0.3, 0.0, 1), vec4(0, 0, 1), "snow_normal", 10);
	albedo wat("water", 0, "water_normal", 20, 1.333);
	albedo blu(vec4(0.1, 0.28, 0.8, 1), vec4(0.5, 0, 0.1));
	scn.world.add_mat(yel, mat_mix);
	scn.world.add_mat(gre, mat_mix);
	scn.world.add_mat(blu, mat_ggx);
	scn.world.add_mat(wat, mat_mix);
	scn.world.add_mesh(quad(vec4(-10, 0, -10), vec4(-10, 0, 10), vec4(10, eps, -10)), vec4(0, 1, 0, 1), 0);
	scn.world.load_mesh("bunny", vec4(-0.44, 0.8, -1, 6), 1);
	scn.world.load_mesh("teapot", vec4(0.7, 1, -1, 0.25), 2);
	scn.world.add_mesh(voxel(0, 50), vec4(0, -48.8, 0, 1), 3, 1, 0, 1);
	scn.cam.autofocus = 0;
	scn.cam.foc_t = 1.556;
	scn.cam.exposure = 1.f;
	scn.sun_pos = rotat_mat4(vec4(0, 0, 0.32));
	scn.opt.bounces = 10;
	scn.opt.ninv_fog = -100;
	scn.opt.p_life = 0.9f;
	scn.opt.i_life = 1.f / 0.9f;
	scn.cam.setup(mat4(vec4(0.1, 1.4, 0.8), 0), 70, 1.f);
	scn.opt.en_bvh = 1;
	scn.opt.en_fog = 1;
}
void scn5(Scene& scn) {
	albedo white(vec4(0.73, 0.73, 0.73, 1), vec4(0, 0, 1));
	albedo green(vec4(0.12, 0.45, 0.12, 1), vec4(0, 0, 1));
	albedo red(vec4(0.45, 0.12, 0.12, 1), vec4(0, 0, 1));
	albedo blue(vec4(0.12, 0.12, 0.45, 1), vec4(0, 0, 1));
	albedo light(vec4(1), vec4(0, 10, 0));
	albedo clear(vec4(0.73, 0.73, 0.73, 0), vec4(0, 0, 0.1), vec4(0.5, 0.5, 1), 1, 1.5);
	albedo mirror(vec4(0.73, 0.73, 0.73, 1.f), vec4(1, 0, 0.1));
	scn.world.add_mat(white, mat_mix);
	scn.world.add_mat(red, mat_mix);
	scn.world.add_mat(green, mat_mix);
	scn.world.add_mat(blue, mat_mix);
	scn.world.add_mat(clear, mat_mix);
	scn.world.add_mat(mirror, mat_ggx);
	scn.world.add_mat(light, mat_lig);
	constexpr float l = 0.555f;
	scn.world.add_mesh(quad(0, vec4(l, 0, 0), vec4(0, 0, -l)), vec4(0, 0, 0, 1), 0);
	scn.world.add_mesh(quad(vec4(0, l, 0), vec4(l, l, 0), vec4(0, l, -l)), vec4(0, 0, 0, 1), 0);
	scn.world.add_mesh(quad(vec4(0, 0, -l), vec4(l, 0, -l), vec4(0, l, -l)), vec4(0, 0, 0, 1), 3);
	scn.world.add_mesh(quad(0, vec4(0, 0, -l), vec4(0, l, 0)), vec4(0, 0, 0, 1), 1);
	scn.world.add_mesh(quad(vec4(l, 0, 0), vec4(l, 0, -l), vec4(l, l, 0)), vec4(0, 0, 0, 1), 2);
	scn.world.add_mesh(quad(0, vec4(0.13, 0, 0), vec4(0, 0, -0.105)), vec4(0.213, l - eps, -0.227, 1), 6, 0, 1);
	scn.world.add_mesh(sphere(vec4(l / 4, 0.1, -l / 4, 0.1)), vec4(0, 0, 0, 1), 0);
	scn.world.add_mesh(sphere(vec4(l - l / 4, 0.1, -l / 4, 0.1)), vec4(0, 0, 0, 1), 4);
	scn.world.add_mesh(sphere(vec4(l / 2, 0.1, -l + l / 4, 0.1)), vec4(0, 0, 0, 1), 5);
	scn.opt.ninv_fog = -2;
	scn.opt.sky = false;
	scn.cam.exposure = 1.f;
	scn.opt.p_life = 0.9f;
	scn.opt.i_life = 1.f / 0.9f;
	scn.cam.setup(mat4(vec4(l / 2, 0.2f, l), vec4(0)), 40, 16.f);
	scn.opt.en_bvh = 0;
}

void scn6(Scene& scn) {
	albedo gnd(vec4(0.8, 0.4, 0.3, 1), 0);
	albedo trans(vec4(0.7, 0.7, 0.99, 0), vec4(0, 0, 0.2), vec4(0.5, 0.5, 1), 1, 1.2);
	albedo trans2(vec4(0.99, 0.7, 0.7, 0), vec4(0, 0, 0.2), vec4(0.5, 0.5, 1), 1, 1.2);
	albedo red(vec4(0.8, 0.1, 0.1, 1), vec4(1, 0, 0.1));
	albedo blue(vec4(0.1, 0.1, 0.8, 1), vec4(1, 0, 0.1));
	scn.world.add_mat(gnd, mat_ggx);
	scn.world.add_mat(trans, mat_mix);
	scn.world.add_mat(red, mat_ggx);
	scn.world.add_mat(trans2, mat_mix);
	scn.world.add_mat(blue, mat_ggx);
	scn.world.add_mesh(quad(vec4(-100, 0, -100), vec4(100, 0, -100), vec4(-100, 0, 100)), vec4(0, 0, 0, 1), 0);
	scn.world.add_mesh(sphere(vec4(0, 3.001, -3, 1)), vec4(0, 0, 0, 1), 1);
	scn.world.add_mesh(voxel(vec4(0, 3.001, -3, 0.576)), vec4(0, 0, 0, 1), 2);
	scn.world.add_mesh(voxel(vec4(0, 1.001, -3, 1)), vec4(0, 0, 0, 1), 3);
	scn.world.add_mesh(sphere(vec4(0, 1.001, -3, 0.999f)), vec4(0, 0, 0, 1), 4);
	scn.opt.ninv_fog = -100;
	scn.cam.exposure = 1.f;
	scn.sun_pos = rotat_mat4(vec4(1, 0, 0.32));
	scn.opt.bounces = 10;
	scn.opt.p_life = 0.9f;
	scn.opt.i_life = 1.f / 0.9f;
	scn.cam.setup(mat4(vec4(0, 1.7, 1), 0), 70, 1.f);
	scn.opt.en_bvh = 0;
}

void scn7(Scene& scn) {
	albedo red(vec4(0.63, 0.28, 0, 1), vec4(0.5, 5, 0.1));
	scn.world.add_mat(red, mat_las);
	scn.world.load_mesh("xyzrgb_dragon", vec4(0, 0, 0, 1), 0);
	scn.world.set_trans(0, mat4(vec4(0, 1.8, -0.5, 0.01f), vec4(0, -0.66, 0)));
	scn.cam.exposure = 1.f;
	scn.sun_pos = rotat_mat4(vec4(1, 0, 0.32));
	scn.opt.bounces = 10;
	scn.opt.p_life = 0.9f;
	scn.opt.i_life = 1.f / 0.9f;
	scn.opt.ninv_fog = -0.5;
	scn.opt.en_fog = true;
	scn.cam.setup(mat4(vec4(0.2, 1.7, 1), 0), 70, 64.f);
	scn.opt.en_bvh = 1;
}

void scn8(Scene& scn) {
	albedo r(vec4(0.8, 0.1, 0.1, 1), vec4(0, 33, 0));
	albedo g(vec4(0.1, 0.8, 0.1, 1), vec4(0, 33, 0));
	albedo b(vec4(0.1, 0.1, 0.8, 1), vec4(0, 33, 0));
	albedo a(vec4(0.95, 0.95, 0.95, 0), 0, vec4(0.5, 0.5, 1), 1, 1.5);
	scn.world.add_mat(a, mat_mix);
	scn.world.add_mat(r, mat_las);
	scn.world.add_mat(g, mat_las);
	scn.world.add_mat(b, mat_las);
	scn.world.add_mesh(sphere(vec4(0, 0, 0, 0.03)), vec4(0, 0, -0.1, 1), 0, 0, 1);
	scn.world.add_mesh(sphere(vec4(0, 0, 0, 0.01)), vec4(-0.05, 0.09, -0.1, 1), 1, 0, 1);
	scn.world.add_mesh(sphere(vec4(0, 0, 0, 0.01)), vec4(0, 0.1, -0.1, 1), 2, 0, 1);
	scn.world.add_mesh(sphere(vec4(0, 0, 0, 0.01)), vec4(0.05, 0.09, -0.1, 1), 3, 0, 1);
	scn.cam.exposure = 1.f;
	scn.opt.bounces = 20;
	scn.opt.samples = 2;
	scn.opt.p_life = 1.f;
	scn.opt.i_life = 1.f;
	scn.opt.ninv_fog = -1;
	scn.opt.en_fog = true;
	scn.opt.sky = 0;
	scn.cam.setup(mat4(0, 0), 70, 64.f);
	scn.opt.en_bvh = 0;
}
