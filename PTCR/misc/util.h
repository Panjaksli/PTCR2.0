#pragma once
#include "defines.h"
#if USE_SSE
#include "SSE.h"
#endif

extern float GAUSS_3x3[9];
extern float GAUSS_5x5[25];

inline static constexpr float todeg(float a) { return a * (180.0f / pi); }
inline static constexpr float torad(float a) { return a * (pi / 180.0f); }

inline float fsqrt(float x)
{
	uint i = *(uint*)&x;
	i = 0x1fbd5f5f + (i >> 1); //532316802 //0x1fbd5f5f
	return *(float*)&i;
}

inline float flogf(float x)
{
	return  *(uint*)&x * 8.2546954e-8f - 87.94167f;
}

inline constexpr int copysigni(int a, int b) {
	return b > 0 ? a : -a;
}
inline float signf(float u)
{
	return copysignf(1.f, u);
}
inline float fast_atan(float x)
{
	//http://nghiaho.com/?p=997
	return qpi * x - x * (fabsf(x) - 1.f) * (0.2447f + 0.0663f * fabsf(x));
}
inline float mod(float u, float v) {
	return u - v * floorf(u / v);
}
inline float fast_sin(float x)
{
	float x2 = x * x;
	return x + x * x2 * (-1.6666656684e-1f + x2 * (8.3330251389e-3f + x2 * (-1.9807418727e-4f + x2 * 2.6019030676e-6f)));
}
inline float fsin(float x)
{
	x = mod(x - hpi, pi2);
	x = fabsf(x - pi) - hpi;
	return fast_sin(x);
}
inline float fcos(float x)
{
	x = mod(x, pi2);
	x = fabsf(x - pi) - hpi;
	return fast_sin(x);
}
inline void sincos(float x, float& sinx, float& cosx)
{
	sinx = fsin(x);
	cosx = fcos(x);
}

inline float ftan(float x)
{
	float sinx, cosx;
	sincos(x, sinx, cosx);
	return sinx / cosx;
}
inline float fcot(float x)
{
	float sinx, cosx;
	sincos(x, sinx, cosx);
	return cosx / sinx;
}

//https://developer.download.nvidia.com/cg/index_stdlib.html
inline float fasin(float x)
{
	float sign = signf(x);
	x = fabs(x);
	float y = -0.0187293f;
	y = y * x + 0.0742610f;
	y = y * x - 0.2121144f;
	y = y * x + 1.5707288f;
	y = hpi - sqrtf(1.0f - x) * y;
	return sign * y;
}
inline float facos(float x)
{
	return hpi - fasin(x);
}
//https://developer.download.nvidia.com/cg/index_stdlib.html
inline float fatan2(float y, float x)
{
	float fx, fy;
	float t0, t1, t2, t3;

	t2 = fx = fabsf(x);
	t1 = fy = fabsf(y);
	t0 = fmaxf(t2, t1);
	t1 = fminf(t2, t1);
	t2 = 1.f / t0;
	t2 = t1 * t2;

	t3 = t2 * t2;
	t0 = -0.013480470f;
	t0 = t0 * t3 + 0.057477314f;
	t0 = t0 * t3 - 0.121239071f;
	t0 = t0 * t3 + 0.195635925f;
	t0 = t0 * t3 - 0.332994597f;
	t0 = t0 * t3 + 0.999995630f;
	t2 = t0 * t2;

	t2 = (fy > fx) ? hpi - t2 : t2;
	t2 = (x < 0) ? pi - t2 : t2;
	t2 = copysignf(t2, y);

	return t2;
}


template <typename T>
inline T pow2n(T x, int n)
{
	for (int i = 0; i < n; i++)
		x = x * x;
	return x;
}

template <typename T>
inline T pow5(T x)
{
	return x * x * x * x * x;
}

inline float mix(float x, float y, float t)
{
	return x * (1.f - t) + y * t;
}

inline float lerpf(float x, float y, float t)
{
	return x * (1.f - t) + y * t;
}

inline float minp(float t1, float t2)
{
	if (t1 < eps2)t1 = infp;
	if (t2 < eps2)t2 = infp;
	return fminf(t1, t2);
}

inline float clamp(float x, float min, float max)
{
	return fmaxf(min, fminf(max, x));
}

inline bool within(float t, float min, float max)
{
	return (t >= min) && (t <= max);
}
inline bool inside(float t, float min, float max)
{
	return (t > min) && (t < max);
}

inline uint xorshift32() {
	thread_local static uint x = 0x6f9f;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return x;
}
inline uint fastrand() {
	thread_local static uint x = 0x6f9f;
	x = (214013U * x + 2531011U);
	return x;
}

inline float rafl()
{
	uint x = 0x3f800000 | (xorshift32() & 0x007FFFFF);
	return *(float*)&x - 1.f;
}

inline float randf(uint &x)
{
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	x = 0x3f800000 | (x & 0x007FFFFF);
	return *(float*)&x - 1.f;
}

#if USE_SSE
inline void rafl_tuple(float r12[2]) {
	__m128 r = _mm_rafl_ps();
	r12[0] = r[0];
	r12[1] = r[1];
}
inline void rafl_tuple_sym(float r12[2]) {
	__m128 r = 2.f * _mm_rafl_ps() - 1.f;
	r12[0] = r[0];
	r12[1] = r[1];
}
#else
inline void rafl_tuple(float r12[2]) {
	r12[0] = rafl();
	r12[1] = rafl();
}
inline void rafl_tuple_sym(float r12[2]) {
	r12[0] = 1.f - 2.f * rafl();
	r12[1] = 1.f - 2.f * rafl();
}
#endif

template <int ks = 3, typename T>
inline void kernel(const T* in, T* out, int i, int j, int h, int w) {
	for (int k = 0; k < ks * ks; k++)
	{
		out[k] = in[clamp_int(i + k / ks - ks / 2, 0, h - 1) * w + clamp_int(j + k % ks - ks / 2, 0, w - 1)];
	}
}

template <typename T>
inline T gauss_3x3(const T* x) {
	T sum = 0;
	for (int i = 0; i < 9; i++)
		sum += GAUSS_3x3[i] * x[i];
	return sum;
}
template <typename T>
inline T gauss_5x5(const T* x) {
	T sum = 0;
	for (int i = 0; i < 25; i++)
		sum += GAUSS_5x5[i] * x[i];
	return sum;
}

inline float fract(float x) {
	return x - floorf(x);
}

inline float rafl(float min, float max) { return min + (max - min) * rafl(); }
inline float rafl(float max) { return max * rafl(); }
inline int raint(int min, int max)
{
	return rafl(min, max + 1);
}
inline uint raint(uint max)
{
	return rafl(max + 1);
}

inline constexpr int modulo(int a, int b)
{
	const int result = a % b;
	return result >= 0 ? result : result + b;
}

struct rgba8888 {
	rgba8888(uint rgba) : rgba(rgba) {}
	union {
		struct {
			uchar r, g, b, a;
		};
		uint rgba;
	};
	inline void unpack(uint _rgba) {
		rgba = _rgba;
	}
	inline uint pack() {
		return rgba;
	}
};

inline uint avg_rgb8888(rgba8888 x, rgba8888 y) {
	x.r = (x.r + y.r) / 2;
	x.g = (x.g + y.g) / 2;
	x.b = (x.b + y.b) / 2;
	x.a = (x.a + y.a) / 2;
	return x.rgba;
}

struct pbool {
	pbool() :byte(0) {}
	pbool(uchar _byte) :byte(_byte) {}

	inline operator bool() const {
		return byte;
	}
	inline operator uchar()const {
		return byte;
	}
	inline void set(bool data, uchar id) {
		uchar mask = data << id;
		uchar temp = byte & ~mask;
		byte = temp | mask;
	}
	inline bool operator[](uchar id) {
		return (byte >> id) & 1U;
	}

	uchar byte;
};

inline uint pack_rgb(uchar r, uchar g, uchar b, uchar a = 255)
{
	return r + (g << 8) + (b << 16) + (a << 24);
}
inline uint pack_bgr(uchar r, uchar g, uchar b, uchar a = 255)
{
	return b + (g << 8) + (r << 16) + (a << 24);
}

inline uint pack_rgb10(uint r, uint g, uint b, uchar a = 255)
{
	return b + (g << 10) + (r << 20) + (a << 30);// a + (r << 2) + (g << 12) + (b << 12);//b + (g << 12) + (r << 20) + (a << 30);
}

struct uint3 {
	uint3(uint x = 0, uint y = 0, uint z = 0) :xyz{ x,y,z } {}
	union {
		struct {
			uint x, y, z;
		};
		uint xyz[3];
	};
};
struct float3 {
	float3(float x = 0, float y = 0, float z = 0) :xyz{ x,y,z } {}
	union {
		struct {
			float x, y, z;
		};
		float xyz[3];
	};
};

struct float3x3 {
	float3 x[3];
};

inline int clamp_int(int x, int min, int max) {
	return x < min ? min : x > max ? max : x;
}
inline int fmin_int(int x, int y) {
	return x > y ? y : x;
}
inline int fmax_int(int x, int y) {
	return x < y ? y : x;
}

inline float gauss(float x, float s) {
	float is = 1.f / s;
	return expf(-0.5f * x * x * is * is) * (1.f / sqrtpi2) * is;
}