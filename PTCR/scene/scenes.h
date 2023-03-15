#pragma once
#include "scene.h"
constexpr int no_scn = 8;

void scn1(scene& scn);
void scn2(scene& scn);
void scn3(scene& scn);
void scn4(scene& scn);
void scn5(scene& scn);
void scn6(scene& scn);
void scn7(scene& scn);
void scn8(scene& scn);
void OBJ_to_MSH(const char* filename);
std::vector<poly> load_mesh(const char* name, vec3 off = 0, float scale = 1.f, bool flip_face = 0);
std::vector<poly> load_OBJ(const char* name, vec3 off = 0, float scale = 1.f, bool flip_face = 0);
std::vector<poly> load_MSH(const char* name, vec3 off = 0, float scale = 1.f, bool flip_face = 0);
std::vector<poly> generate_mesh(uint seed, vec3 off, float scale, bool flip = 0);
inline void scn_load(scene& scn, int n) {
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
