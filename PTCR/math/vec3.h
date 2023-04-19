#pragma once
#include "defines.h"
#if USE_SSE
#include "vec3_SSE.h"
#else 
#include "vec3_scalar.h"
#endif

//common functions

inline vec3 todeg(vec3 a) { return a * (180.0f / pi); }
inline vec3 torad(vec3 a) { return a * (pi / 180.0f); }

inline bool operator==(vec3 u, vec3 v) {
	return near0(u - v);
}
inline bool operator!=(vec3 u, vec3 v) {
	return !(u == v);
}

inline vec3 fma(vec3 a, vec3 b, vec3 c) {
	return a * b + c;
}
inline vec3 fract(vec3 u) {
	return u - floor(u);
}
inline vec3 mod(vec3 u, vec3 v) {
	return u - v * floor(u / v);
}
inline vec3 vec_eq_tol(vec3 u, vec3 v, float tol = eps) {
	vec3 w = fabs(u - v);
	return vec_lt(w, tol);
}
inline float sum(vec3 u)
{
	return dot(1.f, u);
}
inline float avg(vec3 u)
{
	return dot(1.f / 3.f, u);
}
inline vec3 min(vec3 u, vec3 v, vec3 w)
{
	return min(u, min(v, w));
}
inline vec3 max(vec3 u, vec3 v, vec3 w)
{
	return max(u, max(v, w));
}
inline vec3 mix(vec3 x, vec3 y, vec3 t)
{
	return (1.f - t) * x + t * y;
}
inline vec3 clamp(vec3 u, vec3 lo, vec3 hi)
{
	return max(min(u, hi), lo);
}
inline vec3 fixnan(vec3 u)
{
	return max(u, 0.0f);
}
inline vec3 saturate(vec3 u)
{
	return clamp(u, 0.f, 1.f);
}
inline vec3 reflect(vec3 v, vec3 n)
{
	return v - 2.f * dot(v, n) * n;
}
inline vec3 refract(vec3 v, vec3 n, float ir) {
	float NoV = dot(v, n);
	float k = 1.f - ir * ir * (1.f - NoV * NoV);
	return ir * v - (ir * NoV + sqrtf(fabsf(k))) * n;
}
inline vec3 poly_nis(vec3 u, vec3 v) {
	vec3 uv = cross(u, v);
	vec3 N = norm(uv);
	N.xyz[3] = 0.5f * uv.len();
	return N;
}
inline vec3 qua_nis(vec3 u, vec3 v) {
	vec3 uv = cross(u, v);
	vec3 N = norm(uv);
	N.xyz[3] = uv.len();
	return N;
}
inline vec3 fast_sin(vec3 x)
{
	vec3 x2 = x * x;
	return x + x * x2 * (-1.6666656684e-1f + x2 * (8.3330251389e-3f + x2 * (-1.9807418727e-4f + x2 * 2.6019030676e-6f)));
}
inline vec3 sin(vec3 x)
{
	x = mod(x - hpi, pi2);
	x = fabs(x - pi) - hpi;
	return fast_sin(x);
}
inline vec3 cos(vec3 x)
{
	x = mod(x, pi2);
	x = fabs(x - pi) - hpi;
	return fast_sin(x);
}
inline vec3 cossin(float x) {
	vec3 t(x, x - hpi);
	t = mod(t, pi2);
	t = fabs(t - pi) - hpi;
	vec3 y = fast_sin(t);
	return vec3(y.x(), y.y());
}
inline vec3 sincos(float x) {
	vec3 t(x - hpi, x);
	t = mod(t, pi2);
	t = fabs(t - pi) - hpi;
	vec3 y = fast_sin(t);
	return vec3(y.x(), y.y());
}

inline float luminance(vec3 rgb)
{
	return dot(vec3(0.2126f, 0.7152f, 0.0722f), rgb);
}
inline uint pack_rgb(vec3 rgb)
{
	return pack_rgb(rgb[0], rgb[1], rgb[2], rgb[3]);
}
inline uint pack_bgr(vec3 rgb)
{
	return pack_bgr(rgb[0], rgb[1], rgb[2], rgb[3]);
}
inline vec3 vec8bit(vec3 col) {
#if GAMMA2
	col = sqrt(col); //gamma correction to sRGB
#endif
	col = saturate(col);//clamp 0 - 1
	col *= 255.f; //multiply by 8bit max value
	col += 0.5f * ravec();//dithering using noise
	return col;
}
inline uint vec2bgr(vec3 col)
{
	return pack_bgr(vec8bit(col)); //store to bgr pixel
}
inline uint vec2rgb(vec3 col)
{
	return pack_rgb(vec8bit(col));
}

inline vec3 rgb2vec(const uint& rgb) {
	vec3 v;
	v.xyz[0] = rgb & 255;
	v.xyz[1] = (rgb >> 8) & 255;
	v.xyz[2] = (rgb >> 16) & 255;
	v.xyz[3] = (rgb >> 24) & 255;
	return v * (1.f / 255.f);
}

inline bool cmp_lum(const vec3& a, const vec3& b) {
	return luminance(a) < luminance(b);
};

inline bool cmp_avg(const vec3& a, const vec3& b) {
	return sum(a) < sum(b);
};
inline bool cmp_val(const float& a, const float& b) {
	return a < b;
};

inline vec3 avg9(vec3* x) {
	vec3 sum;
	for (int i = 0; i < 9; i++)
		sum += x[i];
	return sum * (1.f / 9.f);
}
inline vec3 avg_n(vec3* x, const int n) {
	vec3 sum;
	for (int i = 0; i < n; i++)
		sum += x[i];
	return sum * (1.f / n);
}
inline void swap_vec3(vec3* x, uint i, uint j) {
	vec3 t1 = x[i];
	vec3 t2 = x[j];
	x[i] = min(t1, t2);
	x[j] = max(t1, t2);
}
inline void min_vec3(vec3* x, uint i, uint j) {
	x[i] = min(x[i], x[j]);
}

inline void max_vec3(vec3* x, uint i, uint j) {
	x[j] = max(x[i], x[j]);
}

inline vec3 med3(const vec3* x) {
	vec3 y[3] = { x[0],x[1],x[2] };
	swap_vec3(y, 0, 2);
	min_vec3(y, 1, 2);
	max_vec3(y, 0, 1);
	return y[1];
}

inline vec3 med4(const vec3* x) {
	vec3 y[4] = { x[0],x[1],x[2],x[3] };
	swap_vec3(y, 1, 3);
	swap_vec3(y, 0, 2);
	min_vec3(y, 2, 3);
	max_vec3(y, 0, 1);
	return 0.5f * (y[1] + y[2]);
}
inline vec3 med_n4(vec3* x, uint n) {
	if (n == 1)return x[0];
	else if (n == 2)return 0.5f * (x[0] + x[1]);
	else if (n == 3)return med3(x);
	else return med4(x);
}
inline vec3 med9(const vec3* x) {
	vec3 y[9] = { x[0], x[1],x[2],x[3],x[4],x[5],x[6],x[7],x[8] };
	swap_vec3(y, 0, 1); swap_vec3(y, 3, 4); swap_vec3(y, 6, 7);
	swap_vec3(y, 1, 2); swap_vec3(y, 4, 5); swap_vec3(y, 7, 8);
	swap_vec3(y, 0, 1); swap_vec3(y, 3, 4); swap_vec3(y, 6, 7);
	max_vec3(y, 0, 3);  max_vec3(y, 3, 6); swap_vec3(y, 1, 4);
	min_vec3(y, 4, 7);  max_vec3(y, 1, 4); min_vec3(y, 5, 8);
	min_vec3(y, 2, 5);  swap_vec3(y, 2, 4);
	min_vec3(y, 4, 6);  max_vec3(y, 2, 4);
	return y[4];
}


inline vec3 bilat_3x3(const vec3* x) {
	vec3 y = 0;
	float w = 0;
	float sd = 1, sr = 1;
	sd *= sd; sr *= sr;
	float d[9] = { 2,1,2,1,0,1,2,1,2 };
	for (int i = 0; i < 9; i++)
	{
		vec3 dl = x[4] / x[4].w() - x[i] / x[i].w();
		float wi = expf(-0.5f * (d[i] / sd + dl.len2() / sr));//GAUSS_3x3[i] * gauss(dl.len2(), 16);
		y += x[i] * wi;
		w += wi;
	}
	return y / w;
}

inline vec3 bilat_5x5(const vec3* x) {
	vec3 y = 0;
	float w = 0;
	float sd = 1, sr = 1;
	sd *= sd; sr *= sr;
	float d[25] =
	{
	8,5,4,5,8,
	5,2,1,2,5,
	4,1,0,1,4,
	5,2,1,2,5,
	8,5,4,5,8
	};
	for (int i = 0; i < 25; i++)
	{
		vec3 dl = x[12] / x[12].w() - x[i] / x[i].w();
		float wi = expf(-0.5f * (d[i] / sd + dl.len2() / sr));
		y += x[i] * wi;
		w += wi;
	}
	return y / w;
}
inline vec3 median2d3(const vec3* data, int i, int j, int h, int w, float thr) {
	vec3 x[9];
	if (thr <= eps)return data[i * w + j];
	kernel<3>(data, x, i, j, h, w);
	return mix(x[4], med9(x), sqrtf(thr));
}

using vec4 = vec3;