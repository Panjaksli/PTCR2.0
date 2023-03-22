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
std::vector<poly> load_mesh(const char* filename, vec3 off = 0, float scale = 1.f, bool flip_face = 0);
std::vector<poly> load_OBJ(const char* filename, vec3 off = 0, float scale = 1.f, bool flip_face = 0);
std::vector<poly> load_MSH(const char* filename, vec3 off = 0, float scale = 1.f, bool flip_face = 0);
std::vector<poly> generate_mesh(uint seed, vec3 off, float scale, bool flip = 0);
void scn_load(scene& scn, const char* filename);
void scn_load(scene& scn, int n);
