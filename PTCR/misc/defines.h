#pragma once
#include <cmath>
#include <memory>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <string>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
using uchar = unsigned char;
using uint = unsigned int;
#define TEST 0
#define GAMMA2 1
#define DEBUG 1
#define USE_SSE (1 && __SSE2__)
#define WIDTH 960
#define HEIGHT 720
#define SCALE 1.f
#define CLAMP ImGuiSliderFlags_AlwaysClamp
#define LOGFAST(x) if(rafl() > 0.9f) std::cout << x << "\n"
#define LOGSLOW(x) if(rafl() > 0.99999f) std::cout << x << "\n"
#define SELDOM if(rafl() > 0.99999f)
#define OFTEN if(rafl() > 0.9f)
#define ALWAYS if(1)
#define SMOOTH_SHADING 1
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
constexpr float infp = 1e6f;
constexpr float infn = -1e6f;
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
using std::sqrtf;
using std::tan;
using std::atan2;
using std::acos;
using std::asin;
using std::cout;
using std::string;
#define loop(i,j) for(uint i = 0 ; i < j ; i++)
#define println cout<<"\n"