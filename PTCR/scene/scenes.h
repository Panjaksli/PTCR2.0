#pragma once
#include "Scene.h"
constexpr int no_scn = 8;

void scn1(Scene& scn);
void scn2(Scene& scn);
void scn3(Scene& scn);
void scn4(Scene& scn);
void scn5(Scene& scn);
void scn6(Scene& scn);
void scn7(Scene& scn);
void scn8(Scene& scn);
bool scn_save(Scene& scn, const char* filename);
bool scn_load(Scene& scn, const char* filename, bool update_only = 0);
void scn_load(Scene& scn, int n);
