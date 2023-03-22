#include <regex>
#include "scenes.h"
//C++ is terrible,but really TERRIBLE at parsing text
//Note to self, never ever use C/C++ again for parsing anything
void scn_load(scene& scn, const char* filename) {
	std::string name(filename);
	if (name.empty())return;
	if (name.find(".scn") == std::string::npos)
		name = name + ".scn";
	std::ifstream file(name);
	if (!file.is_open())
	{
		name = "scenes/" + name;
		file = std::ifstream(name);
		if (!file.is_open())
		{
			printf("File not found !\n");
			return;
		}
	}
	scn.world.clear();
	//scn.opt = scene_opt();
	std::stringstream file_buff;
	file_buff << file.rdbuf();
	std::string line = "";
	std::string pref = "";
	while (std::getline(file_buff, line))
	{
		if (line[0] == '*')continue;
		vector<char> buff1(40);
		vector<char> buff2(40);
		vector<char> buff3(40);
		uint mat = 0, bvh = 1, lig = 0;
		float scl = 0;
		float3 off, q, a, b, c;
		float r = 0;
		if (sscanf_s(line.c_str(), "poly mat=%u bvh=%u lig=%u off=%f,%f,%f,%f data=%f,%f,%f,%f,%f,%f,%f,%f,%f",
			&mat, &bvh, &lig, &off.x, &off.y, &off.z, &scl, &a.x, &a.y, &a.z, &b.x, &b.y, &b.z, &c.x, &c.y, &c.z) > 0)
		{
			scn.world.add_mesh(poly(a, b, c), off, scl, mat, !bvh, lig);
		}
		else if (sscanf_s(line.c_str(), "quad mat=%u bvh=%u lig=%u off=%f,%f,%f,%f data=%f,%f,%f,%f,%f,%f,%f,%f,%f",
			&mat, &bvh, &lig, &off.x, &off.y, &off.z, &scl, &a.x, &a.y, &a.z, &b.x, &b.y, &b.z, &c.x, &c.y, &c.z) > 0)
		{
			scn.world.add_mesh(quad(a, b, c), off, scl, mat, !bvh, lig);
		}
		else if (sscanf_s(line.c_str(), "sphere mat=%u bvh=%u lig=%u off=%f,%f,%f,%f data=%f,%f,%f,%f",
			&mat, &bvh, &lig, &off.x, &off.y, &off.z, &scl, &q.x, &q.y, &q.z, &r) > 0)
		{
			scn.world.add_mesh(sphere(q, r), off, scl, mat, !bvh, lig);
		}
		else if (sscanf_s(line.c_str(), "voxel mat=%u bvh=%u lig=%u off=%f,%f,%f,%f data=%f,%f,%f,%f",
			&mat, &bvh, &lig, &off.x, &off.y, &off.z, &scl, &q.x, &q.y, &q.z, &r) > 0)
		{
			scn.world.add_mesh(voxel(q, r), off, scl, mat, !bvh, lig);
		}
		else if (sscanf_s(line.c_str(), "mesh mat=%u bvh=%u lig=%u off=%f,%f,%f,%f data=%s",
			&mat, &bvh, &lig, &off.x, &off.y, &off.z, &scl, buff1.data(), 40) > 0)
		{
			scn.world.add_mesh(load_mesh(buff1.data()), off, scl, mat, !bvh, lig);
		}
		else if (sscanf_s(line.c_str(), "albedo type=%u rgb=%s mer=%s nor=%s scl=%f ir=%f",
			&mat, buff1.data(), 40, buff2.data(), 40, buff3.data(), 40, &scl, &r) > 0)
		{
			vec3 rgb, mer, nor;
			//WHY ?
			texture t1 = sscanf_s(buff1.data(), "%f,%f,%f,%f", &rgb[0], &rgb[1], &rgb[2], &rgb[3]) >= 3 ? texture(rgb) : texture(buff1.data());
			texture t2 = sscanf_s(buff2.data(), "%f,%f,%f,%f", &mer[0], &mer[1], &mer[2], &mer[3]) >= 3 ? texture(mer) : texture(buff2.data());
			texture t3 = sscanf_s(buff3.data(), "%f,%f,%f,%f", &nor[0], &nor[1], &nor[2], &nor[3]) >= 3 ? texture(nor) : texture(buff3.data());
			scn.world.add_mat(albedo(t1, t2, t3, scl, r), (mat_enum)mat);
		}
		else if (sscanf_s(line.c_str(), "skybox rgb=%s",buff1.data(), 40) > 0)
		{
			vec3 rgb;
			texture t1 = sscanf_s(buff1.data(), "%f,%f,%f,%f", &rgb[0], &rgb[1], &rgb[2], &rgb[3]) >= 3 ? texture(rgb) : texture(buff1.data());
			albedo sky(t1, vec3(0, 1, 0));
			scn.set_skybox(sky);
		}
	}
	file.close();
	scn.opt.en_bvh = scn.world.en_bvh;
	scn.cam.moving = 1;
	scn.world.build_bvh(1, scn.opt.node_size);
}
void scn_load(scene& scn, int n) {
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
	scn.opt.en_bvh = scn.world.en_bvh;
	scn.cam.moving = 1;
#if TEST
	scn.opt.en_fog = 0;
	scn.cam.bokeh = 0;
#endif
	scn.world.build_bvh(1, scn.opt.node_size);
}
void scn1(scene& scn) {
	albedo gre(vec3(0.7, 0.9, 0.7, 0), vec3(0, 0, 0), vec3(0.5, 0.5, 1), 1, 1.2);
	albedo carpet(vec3(0.8, 0.2, 0.2, 1), vec3(0, 0, 1), "snow_normal", 10);
	albedo sky("hotel", vec3(0, 1, 0));
	albedo blu(vec3(0.1, 0.28, 0.8, 1), vec3(0.5, 0, 0.1));
	scn.set_skybox(sky);
	scn.world.add_mat(carpet, mat_mix);
	scn.world.add_mat(gre, mat_mix);
	scn.world.add_mat(blu, mat_ggx);
	//scn.world.add_mat(sky, mat_lig);
	vector<poly> bunny = load_mesh("bunny", 0);
	vector<poly> teapot = load_mesh("teapot", 0);
	//scn.world.add_mesh(generate_mesh(1, 0, 1), vec3(0, 0, 0), 1.f, 0, 0);
	scn.world.add_mesh(quad(vec3(-10, 0, -10), vec3(-10, 0, 10), vec3(10, eps, -10)), vec3(0, 1, 0), 1.f, 0, 1, 0);
	scn.world.add_mesh(bunny, vec3(-0.44, 0.8, -1), 6.f, 1);
	scn.world.add_mesh(teapot, vec3(0.7, 1, -1), 0.25f, 2);
	//scn.world.add_mesh(sphere(0,1), 0, 100, 3,1,1,1);
	scn.cam.autofocus = 0;
	scn.cam.foc_t = 1.556;
	scn.cam.exposure = 1.f;
	scn.sun_pos = rotat_mat4(vec3(1, 0, 0.32));
	scn.opt.bounces = 10;
	scn.opt.ninv_fog = -5;
	scn.opt.p_life = 0.9f;
	scn.opt.i_life = 1.f / 0.9f;
	scn.cam.setup(mat4(vec3(0.1, 1.4, 0.8), vec3(0, 0, 0)), 70, 1.f);
	scn.opt.en_fog = false;
	scn.world.en_bvh = 1;
}

void scn2(scene& scn) {
	scn.opt.li_sa = 1;
	albedo iron(vec3(0.7, 0.7, 0.9), vec3(1, 0, 0));
	albedo white(vec3(0.8, 0.8, 0.8), vec3(0, 5, 0));
	scn.world.add_mat(albedo(vec3(0.8, 0.8, 0.8, 1), vec3(0, 0, 1)), mat_mix);
	scn.world.add_mat(iron, mat_ggx);
	scn.world.add_mat(white, mat_lig);
	vector<quad> room(6);
	room.emplace_back(quad(vec3(-1, 0, -1), vec3(3, 0, -1), vec3(-1, 0, 1)));
	room.emplace_back(quad(vec3(-1, 0, -1), vec3(-1, 2, -1), vec3(-1, 0, 1)));
	room.emplace_back(quad(vec3(3, 0, -1), vec3(3, 1.5, -1), vec3(3, 0, 1)));
	room.emplace_back(quad(vec3(-1, 2, -1), vec3(3, 2, -1), vec3(-1, 2, 1)));
	room.emplace_back(quad(vec3(-1, 0, -1), vec3(3, 0, -1), vec3(-1, 2, -1)));
	room.emplace_back(quad(vec3(-1, 0, 1), vec3(3, 0, 1), vec3(-1, 2, 1)));
	scn.opt.ninv_fog = -100;
	scn.world.add_mesh(room, 0, 1, 0);
	scn.world.add_mesh(quad(vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0.999)), 0, 1, 1); //0.24 0 0.67
	scn.world.add_mesh(voxel(vec3(0, 1.9, 0, 0.1)), 0, 1, 2, 0, 1);
	scn.sun_pos = rotat_mat4(vec3(-1, 0, 0));
	scn.cam.setup(mat4(vec3(2, 1, 0), vec3(0, hpi, 0)), 90, 10);
	scn.world.en_bvh = 1;
}
void scn3(scene& scn) {
	for (int i = 0; i <= 10; i++) {
		for (int j = 0; j <= 10; j++) {
			albedo pbrcol(vec3(0.8f, 0.1f, 0.1f), vec3(0.1 * i, 0, 0.1 * j), vec3(0.5, 0.5, 1), 10);
			scn.world.add_mat(pbrcol, mat_ggx);
			scn.world.add_mesh(sphere(vec3(0, 0, 0, 0.5)), vec3(6 - i, 6 - j, -3), 1, i * 11 + j);
		}
	}
	scn.opt.en_fog = 0;
	scn.sun_pos = rotat_mat4(vec3(1, 0, 1));
	scn.cam.setup(mat4(vec3(1, 1, 10), vec3(0, 0, 0)), 47, 10);
	scn.world.en_bvh = 1;
}
void scn4(scene& scn) {
	albedo gre(vec3(0.7, 0.9, 0.7, 0), vec3(0, 0, 0), vec3(0.5, 0.5, 1), 1, 1.2);
	albedo yel(vec3(0.5, 0.3, 0.0, 1), vec3(0, 0, 1), "snow_normal", 10);
	albedo wat("water", vec3(1, 0, 0), "water_normal", 20, 1.333);
	albedo blu(vec3(0.1, 0.28, 0.8, 1), vec3(0.5, 0, 0.1));
	scn.world.add_mat(yel, mat_mix);
	scn.world.add_mat(gre, mat_mix);
	scn.world.add_mat(blu, mat_ggx);
	scn.world.add_mat(wat, mat_mix);
	vector<poly> bunny = load_mesh("bunny", 0);
	vector<poly> teapot = load_mesh("teapot", 0);
	scn.world.add_mesh(quad(vec3(-10, 0, -10), vec3(-10, 0, 10), vec3(10, eps, -10)), vec3(0, 1, 0), 1.f, 0, 1, 0);
	scn.world.add_mesh(bunny, vec3(-0.44, 0.8, -1), 6.f, 1);
	scn.world.add_mesh(teapot, vec3(0.7, 1, -1), 0.25f, 2);
	scn.world.add_mesh(voxel(vec3(0, 0, 0), 50), vec3(0, -48.8, 0), 1.f, 3, 1, 0, 1);
	scn.cam.autofocus = 0;
	scn.cam.foc_t = 1.556;
	scn.cam.exposure = 1.f;
	scn.sun_pos = rotat_mat4(vec3(0, 0, 0.32));
	scn.opt.bounces = 10;
	scn.opt.ninv_fog = -100;
	scn.opt.p_life = 0.9f;
	scn.opt.i_life = 1.f / 0.9f;
	scn.cam.setup(mat4(vec3(0.1, 1.4, 0.8), vec3(0, 0, 0)), 70, 1.f);
	scn.world.en_bvh = 1;
	scn.opt.en_fog = 1;
}
void scn5(scene& scn) {

	albedo white(vec3(0.73, 0.73, 0.73, 1), vec3(0, 0, 1));
	albedo green(vec3(0.12, 0.45, 0.12, 1), vec3(0, 0, 1));
	albedo red(vec3(0.45, 0.12, 0.12, 1), vec3(0, 0, 1));
	albedo blue(vec3(0.12, 0.12, 0.45, 1), vec3(0, 0, 1));
	albedo light(vec3(1), vec3(0, 100, 0));
	albedo clear(vec3(0.73, 0.73, 0.73, 0), vec3(1, 0, 0.1), vec3(0.5, 0.5, 1), 1, 1.5);
	albedo mirror(vec3(0.73, 0.73, 0.73, 1.f), vec3(1, 0, 0.1));
	scn.world.add_mat(white, mat_mix);
	scn.world.add_mat(red, mat_mix);
	scn.world.add_mat(green, mat_mix);
	scn.world.add_mat(blue, mat_mix);
	scn.world.add_mat(clear, mat_mix);
	scn.world.add_mat(mirror, mat_ggx);
	scn.world.add_mat(light, mat_lig);
	constexpr float l = 0.555f;
	scn.world.add_mesh(quad(vec3(0, 0, 0), vec3(l, 0, 0), vec3(0, 0, -l)), 0, 1, 0);
	scn.world.add_mesh(quad(vec3(0, l, 0), vec3(l, l, 0), vec3(0, l, -l)), 0, 1, 0);
	scn.world.add_mesh(quad(vec3(0, 0, -l), vec3(l, 0, -l), vec3(0, l, -l)), 0, 1, 3);
	scn.world.add_mesh(quad(vec3(0, 0, 0), vec3(0, 0, -l), vec3(0, l, 0)), 0, 1, 1);
	scn.world.add_mesh(quad(vec3(l, 0, 0), vec3(l, 0, -l), vec3(l, l, 0)), 0, 1, 2);
	scn.world.add_mesh(quad(vec3(0.213, l - eps, -0.227), vec3(0.343, l - eps, -0.227), vec3(0.213, l - eps, -0.332)), 0, 1, 6, 0, 1);
	scn.world.add_mesh(sphere(vec3(l / 4, 0.1, -l / 4, 0.1)), 0, 1, 0);
	scn.world.add_mesh(sphere(vec3(l - l / 4, 0.1, -l / 4, 0.1)), 0, 1, 4);
	scn.world.add_mesh(sphere(vec3(l / 2, 0.1, -l + l / 4, 0.1)), 0, 1, 5);
	scn.opt.ninv_fog = -2;
	scn.opt.sky = false;
	scn.cam.exposure = 1.f;
	scn.opt.bounces = 10;
	scn.opt.p_life = 0.9f;
	scn.opt.i_life = 1.f / 0.9f;
	scn.cam.setup(mat4(vec3(l / 2, 0.2f, l), vec3(0)), 40, 16.f);
	scn.world.en_bvh = 0;
}

void scn6(scene& scn) {
	albedo gnd(vec3(0.8, 0.4, 0.3, 1), vec3(0, 0, 0));
	albedo trans(vec3(0.7, 0.7, 0.99, 0), vec3(1, 0, 0.2), vec3(0.5, 0.5, 1), 1, 1.2);
	albedo trans2(vec3(0.99, 0.7, 0.7, 0), vec3(1, 0, 0.2), vec3(0.5, 0.5, 1), 1, 1.2);
	albedo red(vec3(0.8, 0.1, 0.1, 1), vec3(1, 0, 0.1));
	albedo blue(vec3(0.1, 0.1, 0.8, 1), vec3(1, 0, 0.1));
	scn.world.add_mat(gnd, mat_ggx);
	scn.world.add_mat(trans, mat_mix);
	scn.world.add_mat(red, mat_ggx);
	scn.world.add_mat(trans2, mat_mix);
	scn.world.add_mat(blue, mat_ggx);
	scn.world.add_mesh(quad(vec3(-100, 0, -100), vec3(100, 0, -100), vec3(-100, 0, 100)), 0, 1, 0);
	scn.world.add_mesh(sphere(vec3(0, 3.001, -3, 1)), 0, 1, 1);
	scn.world.add_mesh(voxel(vec3(0, 3.001, -3, 0.576)), 0, 1, 2);
	scn.world.add_mesh(voxel(vec3(0, 1.001, -3, 1)), 0, 1, 3);
	scn.world.add_mesh(sphere(vec3(0, 1.001, -3, 0.999f)), 0, 1, 4);
	scn.opt.ninv_fog = -100;
	scn.cam.exposure = 1.f;
	scn.sun_pos = rotat_mat4(vec3(1, 0, 0.32));
	scn.opt.bounces = 10;
	scn.opt.p_life = 0.9f;
	scn.opt.i_life = 1.f / 0.9f;
	scn.cam.setup(mat4(vec3(0, 1.7, 1), vec3(0, 0, 0)), 70, 1.f);
	scn.world.en_bvh = 0;
}

void scn7(scene& scn) {
	albedo red(vec3(0.63, 0.28, 0, 1), vec3(0.5, 20, 0.1));
	scn.world.add_mat(red, mat_las);
	vector<poly> dragon = load_mesh("xyzrgb_dragon", 0, 1);
	scn.world.add_mesh(dragon, 0, 1, 0);
	scn.world.set_trans(0, mat4(vec3(0, 1.8, -0.5, 0.01f), vec3(0, -0.66, 0)));
	scn.cam.exposure = 1.f;
	scn.sun_pos = rotat_mat4(vec3(1, 0, 0.32));
	scn.opt.bounces = 10;
	scn.opt.p_life = 0.9f;
	scn.opt.i_life = 1.f / 0.9f;
	scn.opt.ninv_fog = -0.5;
	scn.opt.en_fog = true;
	scn.cam.setup(mat4(vec3(0.2, 1.7, 1), vec3(0, 0, 0)), 70, 64.f);
	scn.world.en_bvh = 1;
}

void scn8(scene& scn) {
	albedo r(vec3(0.8, 0.1, 0.1, 1), vec3(0, 1000, 0));
	albedo g(vec3(0.1, 0.8, 0.1, 1), vec3(0, 1000, 0));
	albedo b(vec3(0.1, 0.1, 0.8, 1), vec3(0, 1000, 0));
	albedo a(vec3(0.95, 0.95, 0.95, 0), vec3(0, 0, 0), vec3(0.5, 0.5, 1), 1, 1.5);
	scn.world.add_mat(a, mat_mix);
	scn.world.add_mat(r, mat_las);
	scn.world.add_mat(g, mat_las);
	scn.world.add_mat(b, mat_las);
	scn.world.add_mesh(sphere(vec3(0, 0, 0, 0.03)), vec3(0, 0, -0.1), 1, 0, 0, 1);
	scn.world.add_mesh(sphere(vec3(0, 0, 0, 0.01)), vec3(-0.05, 0.09, -0.1), 1, 1, 0, 1);
	scn.world.add_mesh(sphere(vec3(0, 0, 0, 0.01)), vec3(0, 0.1, -0.1), 1, 2, 0, 1);
	scn.world.add_mesh(sphere(vec3(0, 0, 0, 0.01)), vec3(0.05, 0.09, -0.1), 1, 3, 0, 1);
	scn.cam.exposure = 1.f;
	scn.opt.bounces = 20;
	scn.opt.samples = 2;
	scn.opt.p_life = 1.f;
	scn.opt.i_life = 1.f;
	scn.opt.ninv_fog = -1;
	scn.opt.en_fog = true;
	scn.opt.sky = 0;
	scn.cam.setup(mat4(vec3(0, 0, 0), vec3(0, 0, 0)), 70, 64.f);
	scn.world.en_bvh = 0;
}

std::vector<poly> load_mesh(const char* filename, vec3 off, float scale, bool flip)
{
	std::string name(filename);
	if (std::ifstream(name + ".msh").good() || std::ifstream("objects/" + name + ".msh").good())
		return load_MSH((name + ".msh").c_str(), off, scale, flip);
	else if (std::ifstream(name + ".obj").good() || std::ifstream("objects/" + name + ".obj").good())
	{
		OBJ_to_MSH((name + ".obj").c_str());
		printf("Generated .msh file!\n");
		return load_MSH((name + ".msh").c_str(), off, scale, flip);
	}
	else {
		printf("INVALID MESH: %s!!!!!\n", filename);
		return std::vector<poly>();
	}
}

void OBJ_to_MSH(const char* filename) {

	std::string name(filename);
	std::ifstream file(name);
	if (!file.is_open())
	{
		name = "objects/" + name;
		file = std::ifstream(name);
		if (!file.is_open())
		{
			printf("File not found !\n");
			return;
		}
	}
	std::stringstream file_buff;
	file_buff << file.rdbuf();
	std::vector<float3> vert; vert.reserve(0xfffff);
	std::vector<uint3> face; face.reserve(0xfffff);
	std::string line = "";
	std::string pref = "";
	while (std::getline(file_buff, line))
	{
		float3 ftmp;
		uint3 utmp;
		if (sscanf_s(line.c_str(), "v %f %f %f", &ftmp.x, &ftmp.y, &ftmp.z) > 1) {
			vert.emplace_back(ftmp);
		}
		else if (sscanf_s(line.c_str(), "f %u%*[^ ]%u%*[^ ]%u", &utmp.x, &utmp.y, &utmp.z) > 1) {
			utmp.x -= 1; utmp.y -= 1; utmp.z -= 1;
			face.emplace_back(utmp);
		}
		else if (sscanf_s(line.c_str(), "f %u %u %u", &utmp.x, &utmp.y, &utmp.z) > 1) {
			utmp.x -= 1; utmp.y -= 1; utmp.z -= 1;
			face.emplace_back(utmp);
		}
	}
	file.close();

	std::ofstream out;
	name.erase(name.length() - 4);
	out.open(name + ".msh", std::ios_base::binary | std::ios_base::out);
	uint vf[2] = { (uint)vert.size(),  (uint)face.size() };
	out.write((char*)vf, 2 * sizeof(uint));
	out.write((char*)&vert[0], sizeof(float3) * vf[0]);
	out.write((char*)&face[0], sizeof(uint3) * vf[1]);
	out.close();
}



std::vector<poly> load_OBJ(const char* filename, vec3 off, float scale, bool flip)
{
	std::string name(filename);
	std::ifstream file(name);
	if (!file.is_open())
	{
		file = std::ifstream("objects/" + name);
		if (!file.is_open())
		{
			printf("File not found !\n");
			return std::vector<poly>();
		}
	}
	std::stringstream file_buff;
	file_buff << file.rdbuf();
	std::vector<vec3> vert; vert.reserve(0xfffff);
	std::vector<uint3> face; face.reserve(0xfffff);
	std::string line = "";
	std::string pref = "";
	float t1 = clock();
	while (std::getline(file_buff, line))
	{
		//C version is 2x faster
#if 1
		float3 ftmp;
		uint3 utmp;
		if (sscanf_s(line.c_str(), "v %f %f %f", &ftmp.x, &ftmp.y, &ftmp.z) > 1) {
			vert.emplace_back(ftmp);
		}
		else if (sscanf_s(line.c_str(), "f %u%*[^ ]%u%*[^ ]%u", &utmp.x, &utmp.y, &utmp.z) > 1) {
			utmp.x -= 1; utmp.y -= 1; utmp.z -= 1;
			face.emplace_back(utmp);
		}
		else if (sscanf_s(line.c_str(), "f %u %u %u", &utmp.x, &utmp.y, &utmp.z) > 1) {
			utmp.x -= 1; utmp.y -= 1; utmp.z -= 1;
			face.emplace_back(utmp);
		}
#else
		std::stringstream ss;
		ss.str(line);
		ss >> pref;
		if (pref == "v")
		{
			vec3 tmp;
			ss >> tmp._xyz[0] >> tmp._xyz[1] >> tmp._xyz[2];
			vert.emplace_back(tmp);
		}
		else if (pref == "f")
		{
			uint3 tmp = {};
			ss >> tmp.x >> tmp.y >> tmp.z;
			tmp.x -= 1; tmp.y -= 1; tmp.z -= 1;
			face.emplace_back(tmp);
		}
#endif
	}
	file.close();
	if (not0(off) || scale != 1.f)
		for (auto& v : vert)
			v = v * scale + off;
#if SMOOTH_SHADING

	//per-vertex normals
	std::vector<poly> polys(face.size());
	std::vector<vec3> nrms(vert.size(), vec3());
	for (uint j = 0; j < face.size(); j++) {
		flip ? polys[j].set_quv(vert[face[j].x], vert[face[j].z], vert[face[j].y])
			: polys[j].set_quv(vert[face[j].x], vert[face[j].y], vert[face[j].z]);
		vec3 n = cross(polys[j].U, polys[j].V);
		nrms[face[j].x] += n;
		nrms[face[j].y] += n;
		nrms[face[j].z] += n;
	}

	//polygons from triangles and normals
	for (uint j = 0; j < face.size(); j++)
		polys[j].set_nor(nrms[face[j].x], nrms[face[j].y], nrms[face[j].z]);
#else
	std::vector<poly> polys; polys.reserve(face.size());
	for (const auto& f : face)
	{
		vec3 a = vert[f.x];
		vec3 b = vert[f.y];
		vec3 c = vert[f.z];
		polys.emplace_back(flip ? poly(a, c, b) : poly(a, b, c));
	}

#endif
	std::cout << "Loaded: " << name << "\n";
	std::cout << "No of tris: " << polys.size() << " Took: " << (clock() - t1) / CLOCKS_PER_SEC << "\n";
	return polys;
}


std::vector<poly> load_MSH(const char* filename, vec3 off, float scale, bool flip)
{
	std::string name(filename);
	std::ifstream file(name, std::ios_base::in | std::ios_base::binary);
	if (!file.is_open())
	{
		file = std::ifstream("objects/" + name, std::ios_base::in | std::ios_base::binary);
		if (!file.is_open())
			throw "File not found !";
	}
	float t1 = clock();
	uint vf[2] = {};
	file.read((char*)vf, 2 * sizeof(uint));
	std::vector<float3> vert(vf[0]);
	std::vector<uint3> face(vf[1]);
	file.read((char*)&vert[0], sizeof(float3) * vf[0]);
	file.read((char*)&face[0], sizeof(uint3) * vf[1]);
	file.close();
	if (not0(off) || scale != 1.f)
		for (auto& v : vert)
		{
			vec3 t = v * scale + off;
			v = float3(t.x(), t.y(), t.z());
		}
#if SMOOTH_SHADING

	//per-vertex normals
	std::vector<poly> polys(face.size());
	std::vector<vec3> nrms(vert.size(), vec3());
	for (uint j = 0; j < face.size(); j++) {
		flip ? polys[j].set_quv(vert[face[j].x], vert[face[j].z], vert[face[j].y])
			: polys[j].set_quv(vert[face[j].x], vert[face[j].y], vert[face[j].z]);
		vec3 n = cross(polys[j].U, polys[j].V);
		nrms[face[j].x] += n;
		nrms[face[j].y] += n;
		nrms[face[j].z] += n;
	}

	//polygons from triangles and normals
	for (uint j = 0; j < face.size(); j++)
		polys[j].set_nor(nrms[face[j].x], nrms[face[j].y], nrms[face[j].z]);
#else
	std::vector<poly> polys; polys.reserve(face.size());
	for (const auto& f : face)
	{
		vec3 a = vert[f.x];
		vec3 b = vert[f.y];
		vec3 c = vert[f.z];
		polys.emplace_back(flip ? poly(a, c, b) : poly(a, b, c));
	}
#endif

	std::cout << "Loaded: " << name << "\n";
	std::cout << "No of tris: " << polys.size() << " Took: " << (clock() - t1) / CLOCKS_PER_SEC << "\n";
	return polys;
}


std::vector<poly> generate_mesh(uint seed, vec3 off, float scale, bool flip) {
	int dim = 128;
	std::vector<vec3> vert2(dim * dim);
	std::vector<vec3> vert(dim * dim);
	std::vector<uint3> face;

	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			vert2[i * dim + j] = vec3(i, randf(seed), j);
		}
	}

	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			vec3 x[9];
			kernel<3>(vert2.data(), x, i, j, dim, dim);
			vert[i * dim + j] = x[4] + gauss_3x3(x);
		}
	}


	for (int i = 0; i < dim - 1; i++) {
		for (int j = 0; j < dim - 1; j++)
		{
			int l = i * dim + j;
			int r = i * dim + j + 1;
			int dl = (i + 1) * dim + j;
			int dr = (i + 1) * dim + j + 1;
			face.emplace_back(uint3(l, r, dr));
			face.emplace_back(uint3(dl, l, dr));
		}
	}

	std::vector<poly> polys(face.size());
	std::vector<vec3> nrms(vert.size(), vec3());
	for (uint j = 0; j < face.size(); j++) {
		flip ? polys[j].set_quv(vert[face[j].x], vert[face[j].z], vert[face[j].y])
			: polys[j].set_quv(vert[face[j].x], vert[face[j].y], vert[face[j].z]);
		vec3 n = cross(polys[j].U, polys[j].V);
		nrms[face[j].x] += n;
		nrms[face[j].y] += n;
		nrms[face[j].z] += n;
	}

	//polygons from triangles and normals
	for (uint j = 0; j < face.size(); j++)
		polys[j].set_nor(nrms[face[j].x], nrms[face[j].y], nrms[face[j].z]);
	return polys;
}