#pragma once
#include "defines.h"
#if USE_SSE
#include "vec4_SSE.h"
#else
#include "vec4_scalar.h"
#endif

// common functions

inline vec4 todeg(const vec4& a) { return a * (180.0f / pi); }
inline vec4 torad(const vec4& a) { return a * (pi / 180.0f); }

inline bool operator==(const vec4 &u, const vec4 &v) { return near0(u - v); }
inline bool operator!=(const vec4 &u, const vec4 &v) { return !(u == v); }

inline vec4 fma(const vec4 &a, const vec4& b, const vec4& c) { return a * b + c; }
inline vec4 fract(const vec4 &u) { return u - floor(u); }
inline vec4 mod(const vec4 &u, const vec4 &v) { return u - v * floor(u / v); }
inline vec4 vec_eq_tol(const vec4 &u, const vec4 &v, float tol = eps) {
	vec4 w = fabs(u - v);
	return vec_lt(w, tol);
}
inline float sum(const vec4 &u) { return dot(1.f, u); }
inline float avg(const vec4 &u) { return dot(1.f / 3.f, u); }
inline vec4 min(const vec4 &u, const vec4 &v, vec4 w) { return min(u, min(v, w)); }
inline vec4 max(const vec4 &u, const vec4 &v, vec4 w) { return max(u, max(v, w)); }
inline vec4 mix(const vec4& x, const vec4& y, const vec4& t) { return (1.f - t) * x + t * y; }
inline vec4 clamp(const vec4 &u, const vec4& lo, const vec4& hi) { return max(min(u, hi), lo); }
inline vec4 saturate(const vec4 &u) { return clamp(u, 0.f, 1.f); }
inline vec4 reflect(const vec4 &v, const vec4& n) { return v - 2.f * dot(v, n) * n; }
inline vec4 refract(const vec4 &v, const vec4& n, float ir) {
	float NoV = dot(v, n);
	float k = 1.f - ir * ir * (1.f - NoV * NoV);
	return ir * v - (ir * NoV + sqrtf(fabsf(k))) * n;
}
inline vec4 poly_ns(const vec4 &u, const vec4 &v) {
	vec4 uv = cross(u, v);
	vec4 N = norm(uv);
	N.xyz[3] = 0.5f * uv.len();
	return N;
}
inline vec4 quad_ns(const vec4 &u, const vec4 &v) {
	vec4 uv = cross(u, v);
	vec4 N = norm(uv);
	N.xyz[3] = uv.len();
	return N;
}
inline vec4 fast_sin(const vec4& x) {
	vec4 x2 = x * x;
	return x + x * x2 *
		(-1.6666656684e-1f +
			x2 * (8.3330251389e-3f +
				x2 * (-1.9807418727e-4f + x2 * 2.6019030676e-6f)));
}
inline vec4 sin(vec4 x) {
	x = mod(x - hpi, pi2);
	x = fabs(x - pi) - hpi;
	return fast_sin(x);
}
inline vec4 cos(vec4 x) {
	x = mod(x, pi2);
	x = fabs(x - pi) - hpi;
	return fast_sin(x);
}
inline vec4 cossin(float x) {
	vec4 t(x, x - hpi);
	t = mod(t, pi2);
	t = fabs(t - pi) - hpi;
	vec4 y = fast_sin(t);
	return vec4(y.x(), y.y());
}
inline vec4 sincos(float x) {
	vec4 t(x - hpi, x);
	t = mod(t, pi2);
	t = fabs(t - pi) - hpi;
	vec4 y = fast_sin(t);
	return vec4(y.x(), y.y());
}
inline float luminance(const vec4 &rgb) {
	return dot(vec4(0.2126f, 0.7152f, 0.0722f), rgb);
}
inline uint pack_rgb(const vec4& rgb) {
	return pack_rgb(rgb[0], rgb[1], rgb[2], rgb[3]);
}
inline uint pack_bgr(const vec4& rgb) {
	return pack_bgr(rgb[0], rgb[1], rgb[2], rgb[3]);
}
inline vec4 vec8bit(vec4 col) {
#if GAMMA2
	col = sqrt(col); // gamma correction to sRGB
#endif
	col = saturate(col);      // clamp 0 - 1
	col = col * 255.f;        // multiply by 8bit max value
	col += 0.999f * rapvec(); // dithering using noise
	return col;
}
inline uint vec2bgr(const vec4& col) {
	return pack_bgr(vec8bit(col)); // store to bgr pixel
}
inline uint vec2rgb(const vec4& col) { return pack_rgb(vec8bit(col)); }

inline vec4 rgb2vec(const uint& rgb) {
	vec4 v;
	v.xyz[0] = rgb & 255;
	v.xyz[1] = (rgb >> 8) & 255;
	v.xyz[2] = (rgb >> 16) & 255;
	v.xyz[3] = (rgb >> 24) & 255;
	return v * (1.f / 255.f);
}

inline bool cmp_lum(const vec4& a, const vec4& b) {
	return luminance(a) < luminance(b);
};

inline bool cmp_avg(const vec4& a, const vec4& b) { return sum(a) < sum(b); };
inline bool cmp_val(const float& a, const float& b) { return a < b; };

inline void sort(vec4* x, uint n) {
	for (int j = 1; j < n; j++) {
		for (int i = 0; i < n - j; i++) {
			vec4 mint = min(x[i], x[i + 1]);
			vec4 maxt = max(x[i], x[i + 1]);
			x[i] = mint;
			x[i + 1] = maxt;
		}
	}
}

inline vec4 avg(vec4* x, uint n) {
	vec4 sum;
	for (uint i = 0; i < n; i++)
		sum += x[i];
	return sum * (1.f / n);
}

inline vec4 min(vec4* x, uint n) {
	vec4 mint = x[0];
	for (uint i = 1; i < n; i++)
		mint = min(mint, x[i]);
	return mint;
}

inline vec4 max(vec4* x, uint n) {
	vec4 maxt = x[0];
	for (uint i = 1; i < n; i++)
		maxt = max(maxt, x[i]);
	return maxt;
}

inline void swap_vec(vec4* x, uint i, uint j) {
	vec4 t1 = x[i];
	vec4 t2 = x[j];
	x[i] = min(t1, t2);
	x[j] = max(t1, t2);
}
inline void min_vec(vec4* x, uint i, uint j) { x[i] = min(x[i], x[j]); }

inline void max_vec(vec4* x, uint i, uint j) { x[j] = max(x[i], x[j]); }

inline vec4 med3(const vec4* x) {
	vec4 y[3] = { x[0], x[1], x[2] };
	swap_vec(y, 0, 2);
	min_vec(y, 1, 2);
	max_vec(y, 0, 1);
	return y[1];
}

inline vec4 med4(const vec4* x) {
	vec4 y[4] = { x[0], x[1], x[2], x[3] };
	swap_vec(y, 1, 3);
	swap_vec(y, 0, 2);
	min_vec(y, 2, 3);
	max_vec(y, 0, 1);
	return 0.5f * (y[1] + y[2]);
}

inline vec4 med_n4(vec4* x, uint n) {
	if (n == 1)
		return x[0];
	else if (n == 2)
		return 0.5f * (x[0] + x[1]);
	else if (n == 3)
		return med3(x);
	else
		return med4(x);
}
inline vec4 med9(const vec4* x) {
	vec4 y[9] = { x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7], x[8] };
	swap_vec(y, 0, 1);
	swap_vec(y, 3, 4);
	swap_vec(y, 6, 7);
	swap_vec(y, 1, 2);
	swap_vec(y, 4, 5);
	swap_vec(y, 7, 8);
	swap_vec(y, 0, 1);
	swap_vec(y, 3, 4);
	swap_vec(y, 6, 7);
	max_vec(y, 0, 3);
	max_vec(y, 3, 6);
	swap_vec(y, 1, 4);
	min_vec(y, 4, 7);
	max_vec(y, 1, 4);
	min_vec(y, 5, 8);
	min_vec(y, 2, 5);
	swap_vec(y, 2, 4);
	min_vec(y, 4, 6);
	max_vec(y, 2, 4);
	return y[4];
}
inline vec4 median_3x3(const vec4* data, int i, int j, int h, int w,
	float thr) {
	vec4 x[9];
	if (thr <= eps)
		return data[i * w + j];
	kernel<3>(data, x, i, j, h, w);
	return mix(x[4], med9(x), thr);
}
