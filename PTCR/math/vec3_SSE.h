#pragma once
#include "util.h"
#include "SSE.h"

struct vec3
{
	vec3() : xyz{} {}
	vec3(__m128 t) :xyz(t) {}
	vec3(float3 t) : xyz{ t.x,t.y,t.z,0 } {}
	vec3(float t) : xyz{ t,t,t,t } {}
	vec3(vec3 v, float w) :xyz{ v.xyz[0],v.xyz[1],v.xyz[2],w } {}
	vec3(float x, float y, float z = 0, float w = 0) : xyz{ x,y,z,w } {}
	inline float x() const { return xyz[0]; }
	inline float y() const { return xyz[1]; }
	inline float z() const { return xyz[2]; }
	inline float w() const { return xyz[3]; }
	inline float operator[](uint i) const { return xyz[i]; }
	inline float& operator[](uint i) { return _xyz[i]; }
	inline vec3 operator-() const { return vec3(-xyz); }
	inline vec3& operator+=(vec3 u)
	{
		xyz += u.xyz;
		return *this;
	}
	inline vec3& operator-=(vec3 u)
	{
		xyz -= u.xyz;
		return *this;
	}
	inline vec3& operator*=(vec3 u)
	{
		xyz *= u.xyz;
		return *this;
	}
	inline vec3& operator/=(vec3 u)
	{
		xyz /= u.xyz;
		return *this;
	}
	inline float len2_v4() const {
		return _mm_cvtss_f32(_mm_dp_ps(xyz, xyz, 0xF1));
	}
	inline float len_v4() const { return sqrtf(len2_v4()); }
	inline float len2() const {
		return _mm_cvtss_f32(_mm_dp_ps(xyz, xyz, 0x71));
	}
	inline float len() const { return sqrtf(len2()); }
	inline vec3 dir() const
	{
		return _mm_div_ps(xyz, _mm_sqrt_ps(_mm_dp_ps(xyz, xyz, 0x7F)));
	}

	inline vec3 fact() const
	{
		vec3 t = *this;
		t /= w();
		return t;
	}

	void print()const {
		printf("%.8f %.8f %.8f\n", x(), y(), z());
	}
	void print4()const {
		printf("%f %f %f %f\n", x(), y(), z(), w());
	}
	void printM()const {
		printf("%f %f %f %f\n", x(), y(), z(), len());
	}
	union {
		__m128 xyz;
		float _xyz[4];
	};
};

inline vec3 norm(vec3 u) { return u.dir(); }
inline vec3 operator+(vec3 u, vec3 v) { return u += v; }
inline vec3 operator-(vec3 u, vec3 v) { return u -= v; }
inline vec3 operator*(vec3 u, vec3 v) { return u *= v; }
inline vec3 operator/(vec3 u, vec3 v) { return u /= v; }

inline float operator&(vec3 u, vec3 v) {
	return _mm_cvtss_f32(_mm_dp_ps(u.xyz, v.xyz, 0x71));
}
inline vec3 operator%(vec3 u, vec3 v)
{
	return _mm_cross_ps(u.xyz, v.xyz);
}
inline vec3 expf(vec3 u) {
	return _mm_expf_ps(u.xyz);
}
inline vec3 sqrt(vec3 u)
{
	return _mm_sqrt_ps(u.xyz);
}
inline vec3 abs(vec3 u)
{
	return _mm_abs_ps(u.xyz);
}
inline vec3 fabs(vec3 u)
{
	return abs(u);
}
inline vec3 copysign(vec3 u, vec3 v)
{
	return _mm_sign_ps(u.xyz, v.xyz);
}
inline vec3 flipsign(vec3 u, vec3 v)
{
	return _mm_flipsign_ps(u.xyz, v.xyz);
}
inline vec3 sign(vec3 u) {
	return _mm_sign_ps(u.xyz);
}
inline vec3 toint(vec3 u) {
	return _mm_cvtepi32_ps(_mm_cvttps_epi32(u.xyz));
}
inline vec3 floor(vec3 u) {
	return _mm_floor_ps(u.xyz);
}
inline vec3 ceil(vec3 u) {
	return _mm_ceil_ps(u.xyz);
}
inline vec3 cross(vec3 u, vec3 v) { return u % v; }
inline float dot(vec3 u, vec3 v) { return u & v; }
inline float dot4(vec3 u, vec3 v) { return _mm_cvtss_f32(_mm_dp_ps(u.xyz, v.xyz, 0xF1)); }
inline float posdot(vec3 u, vec3 v) { return fmaxf(0.f, u & v); }
inline float absdot(vec3 u, vec3 v) { return fabsf(u & v); }
inline float dotabs(vec3 u, vec3 v) { return (abs(u) & abs(v)); }

inline vec3 rotr3(vec3 x)
{
	return _mm_shuffle_ps(x.xyz, x.xyz, 0b11010010);
}
inline vec3 rotr4(vec3 x)
{
	return _mm_shuffle_ps(x.xyz, x.xyz, 0b10010011);
}

inline vec3 rotl3(vec3 x)
{
	return _mm_shuffle_ps(x.xyz, x.xyz, 0b11001001);
}
inline vec3 rotl4(vec3 x)
{
	return _mm_shuffle_ps(x.xyz, x.xyz, 0b00111001);
}

inline vec3 vec_ins(vec3 u, vec3 min, vec3 max) {
	__m128 gt = _mm_cmpgt_ps(u.xyz, min.xyz);
	__m128 lt = _mm_cmplt_ps(u.xyz, max.xyz);
	return _mm_and_ps(_mm_and_ps(gt, lt), _mm_set1_ps(1.f));
}
inline vec3 vec_wit(vec3 u, vec3 min, vec3 max) {
	__m128 gt = _mm_cmpge_ps(u.xyz, min.xyz);
	__m128 lt = _mm_cmple_ps(u.xyz, max.xyz);
	return _mm_and_ps(_mm_and_ps(gt, lt), _mm_set1_ps(1.f));
}
inline vec3 vec_gt(vec3 u, vec3 v) {
	__m128 mask = _mm_cmpgt_ps(u.xyz, v.xyz);
	return _mm_and_ps(mask, _mm_set1_ps(1.f));
}
inline vec3 vec_lt(vec3 u, vec3 v) {
	__m128 mask = _mm_cmplt_ps(u.xyz, v.xyz);
	return _mm_and_ps(mask, _mm_set1_ps(1.f));
}
inline vec3 vec_ge(vec3 u, vec3 v) {
	__m128 mask = _mm_cmpge_ps(u.xyz, v.xyz);
	return _mm_and_ps(mask, _mm_set1_ps(1.f));
}
inline vec3 vec_le(vec3 u, vec3 v) {
	__m128 mask = _mm_cmple_ps(u.xyz, v.xyz);
	return _mm_and_ps(mask, _mm_set1_ps(1.f));
}
inline vec3 vec_eq(vec3 u, vec3 v) {
	__m128 mask = _mm_cmpeq_ps(u.xyz, v.xyz);
	return _mm_and_ps(mask, _mm_set1_ps(1.f));
}
inline vec3 vec_neq(vec3 u, vec3 v) {
	__m128 mask = _mm_cmpneq_ps(u.xyz, v.xyz);
	return _mm_and_ps(mask, _mm_set1_ps(1.f));
}

inline vec3 vec_near0(vec3 u)
{
	u = fabs(u);
	__m128 v = _mm_cmplt_ps(u.xyz, _mm_set1_ps(eps));
	return _mm_and_ps(v, _mm_set1_ps(1.f));
}
inline vec3 vec_not0(vec3 u) {
	u = fabs(u);
	__m128 v = _mm_cmpge_ps(u.xyz, _mm_set1_ps(eps));
	return _mm_and_ps(v, _mm_set1_ps(1.f));
}

inline bool gt(vec3 u, vec3 v) {
	__m128 mask = _mm_cmplt_ps(u.xyz, v.xyz);
	return !(_mm_movemask_ps(mask) & 0b0111);
}
inline bool lt(vec3 u, vec3 v) {
	__m128 mask = _mm_cmpgt_ps(u.xyz, v.xyz);
	return !(_mm_movemask_ps(mask) & 0b0111);
}

inline bool eq(vec3 u, vec3 v)
{
	__m128 w = _mm_cmpneq_ps(u.xyz, v.xyz);
	return !(_mm_movemask_ps(w) & 0b0111);
}
inline bool neq(vec3 u, vec3 v)
{
	__m128 w = _mm_cmpneq_ps(u.xyz, v.xyz);
	return (_mm_movemask_ps(w) & 0b0111);
}
inline bool eq_tol(vec3 u, vec3 v, float tol = eps)
{
	__m128 w = _mm_cmpge_ps(fabs(u - v).xyz, _mm_set1_ps(tol));
	return !(_mm_movemask_ps(w) & 0b0111);
}
inline bool eq0(vec3 u)
{
	__m128 v = _mm_cmpneq_ps(u.xyz, _mm_setzero_ps());
	return !(_mm_movemask_ps(v) & 0b0111);
}
inline bool neq0(vec3 u)
{
	__m128 v = _mm_cmpneq_ps(u.xyz, _mm_setzero_ps());
	return (_mm_movemask_ps(v) & 0b0111);
}

inline bool near0(vec3 u)
{
	u = fabs(u);
	__m128 v = _mm_cmpge_ps(u.xyz, _mm_set1_ps(eps));
	return !(_mm_movemask_ps(v) & 0b0111);
}
inline bool not0(vec3 u) {
	u = fabs(u);
	__m128 v = _mm_cmpge_ps(u.xyz, _mm_set1_ps(eps));
	return (_mm_movemask_ps(v) & 0b0111);
}

inline float min(vec3 u)
{
	return fminf(u.xyz[0], fminf(u.xyz[1], u.xyz[2]));
}
inline float max(vec3 u)
{
	return fmaxf(u.xyz[0], fmaxf(u.xyz[1], u.xyz[2]));
}
inline vec3 min(vec3 u, vec3 v)
{
	return _mm_min_ps(u.xyz, v.xyz);
}
inline vec3 max(vec3 u, vec3 v)
{
	return _mm_max_ps(u.xyz, v.xyz);
}
inline vec3 rapvec() {
	return vec3(_mm_rafl_ps());
}
//(-1,1)
inline vec3 ravec() {
	return 2 * vec3(_mm_rafl_ps()) - 1;
}
inline vec3 ravec(vec3 min, vec3 max) {
	return min + (max - min) * _mm_rafl_ps();
}



