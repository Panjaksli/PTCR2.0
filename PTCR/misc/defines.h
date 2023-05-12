#pragma once
#include <math.h>
#include <cstdio>
#include <vector>
#include <string>
#include <ctime>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include "c_str.h"

#define uchar unsigned char
#define uint unsigned int
#define TEST 0
#define GAMMA2 1
#define DEBUG 1
#define USE_SSE (1 && __SSE2__)
#define SMOOTH_SHADING 1
#define WIDTH 960
#define HEIGHT 720
#define SCALE 1.f
#define CLAMP ImGuiSliderFlags_AlwaysClamp
#define NOROUND ImGuiSliderFlags_NoRoundToFormat
#define LOGSCL ImGuiSliderFlags_Logarithmic
#define FLOATCOL ImGuiColorEditFlags_Float
#define INPTEXT ImGuiInputTextFlags_EnterReturnsTrue
#define CONSOLE_SLOW(x) do {if(rafl() > 0.99999f) std::cout << x << "\n"}while(0)
#define CONSOLE(x) do {std::cout << x << "\n"}while(0)

constexpr double pi_dbl = 3.14159265358979323846;
constexpr float sqrtpi = 1.77245385091;
constexpr float sqrtpi2 = 2.50662827463;
constexpr float qpi = 0.25 * pi_dbl;
constexpr float hpi = 0.5 * pi_dbl;
constexpr float pi = pi_dbl;
constexpr float pi2 = 2.0 * pi_dbl;
constexpr float pi4 = 4.0 * pi_dbl;
constexpr float ipi = 1.0 / pi_dbl;
constexpr float ipi2 = 0.5 / pi_dbl;
constexpr float ihpi = 2.0 / pi_dbl;
constexpr float infp = 1e4f;
constexpr float infn = -1e4f;
constexpr float eps = 1e-4f;
constexpr float eps2 = 1e-6f;
using std::vector;
using std::sin;
using std::cos;
using std::fabs;
using std::fmaxf;
using std::fminf;
using std::abs;
using std::fabs;
using std::fmax;
using std::fmin;
using std::sqrt;
using std::tan;
using std::atan2;
using std::acos;
using std::asin;
using std::cout;
using std::string;
