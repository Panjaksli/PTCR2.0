#include "util.h"
float GAUSS_3x3[9] = {
	1 / 16.f, 1 / 8.f, 1 / 16.f,
	1 / 8.f, 1 / 4.f, 1 / 8.f,
	1 / 16.f, 1 / 8.f, 1 / 16.f
};

float GAUSS_5x5[25] = {
	1 / 273.f,4 / 273.f,7 / 273.f,4 / 273.f,1 / 273.f,
	4 / 273.f,16 / 273.f,26 / 273.f,16 / 273.f,4 / 273.f,
	7 / 273.f,26 / 273.f,41 / 273.f,26 / 273.f,7 / 273.f,
	4 / 273.f,16 / 273.f,26 / 273.f,16 / 273.f,4 / 273.f,
	1 / 273.f,4 / 273.f,7 / 273.f,4 / 273.f,1 / 273.f
};