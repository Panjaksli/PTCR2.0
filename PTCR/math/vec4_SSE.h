#pragma once
#include "util.h"
#include "SSE.h"

struct vec4 {
	vec4() : xyz{} {}
	vec4(const __m128& t) :xyz(t) {}
	vec4(const float3& t) : xyz{ t.x,t.y,t.z,0 } {}
	vec4(float t) : xyz{ t,t,t,t } {}
	vec4(const vec4& v, float w) :xyz{ v.xyz[0],v.xyz[1],v.xyz[2],w } {}
	vec4(float x, float y, float z = 0, float w = 0) : xyz{ x,y,z,w } {}
	inline float x() const { return xyz[0]; }
	inline float y() const { return xyz[1]; }
	inline float z() const { return xyz[2]; }
	inline float w() const { return xyz[3]; }
	inline float operator[](uint i) const { return xyz[i]; }
	inline float& operator[](uint i) { return _xyz[i]; }
	inline vec4 operator-() const { return vec4(-xyz); }
	inline vec4& operator+=(const vec4& u) {
		xyz += u.xyz;
		return *this;
	}
	inline vec4& operator-=(const vec4& u) {
		xyz -= u.xyz;
		return *this;
	}
	inline vec4& operator*=(const vec4& u) {
		xyz *= u.xyz;
		return *this;
	}
	inline vec4& operator/=(const vec4& u) {
		xyz /= u.xyz;
		return *this;
	}
	inline float len2_v4() const {
		return _mm_dot_ps<0xFF>(xyz, xyz)[0];
	}
	inline float len_v4() const { return _mm_sqrt_ps(_mm_dot_ps<0xFF>(xyz, xyz))[0]; }
	inline float len2() const {
		return _mm_dot_ps<0x7F>(xyz, xyz)[0];
	}
	inline float len() const { return _mm_sqrt_ps(_mm_dot_ps<0x7F>(xyz, xyz))[0]; }
	inline vec4 dir() const {
		return _mm_norm_ps(xyz);
	}
	inline vec4 fact() const {
		vec4 t = *this;
		t /= w();
		return t;
	}

	void print()const {
		printf("%f %f %f\n", x(), y(), z());
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

inline vec4 norm(const vec4& u) { return _mm_norm_ps(u.xyz); }
inline float dot(const vec4& u, const vec4& v) { return _mm_dot_ps<0x7F>(u.xyz, v.xyz)[0]; }
inline vec4 cross(const vec4& u, const vec4& v) { return _mm_cross_ps(u.xyz, v.xyz); }
template <int imm8>
inline __m128 dot(const vec4& u, const vec4& v) { return _mm_dot_ps<imm8>(u.xyz, v.xyz); }
inline float dot4(const vec4& u, const vec4& v) { return _mm_dot_ps<0xFF>(u.xyz, v.xyz)[0]; }
inline float operator&(const vec4& u, const vec4& v) { return dot(u, v); }
inline vec4 operator%(const vec4& u, const vec4& v) { return cross(u, v); }
inline vec4 operator+(const vec4& u, const vec4& v) { return u.xyz + v.xyz; }
inline vec4 operator-(const vec4& u, const vec4& v) { return u.xyz - v.xyz; }
inline vec4 operator*(const vec4& u, const vec4& v) { return u.xyz * v.xyz; }
inline vec4 operator/(const vec4& u, const vec4& v) { return u.xyz / v.xyz; }

inline vec4 expf(const vec4& u) {
	return _mm_expf_ps(u.xyz);
}
inline vec4 sqrt(const vec4& u) {
	return _mm_sqrt_ps(u.xyz);
}
inline vec4 abs(const vec4& u) {
	return _mm_abs_ps(u.xyz);
}
inline vec4 fabs(const vec4& u) {
	return abs(u);
}
inline vec4 copysign(const vec4& u, const vec4& v) {
	return _mm_sign_ps(u.xyz, v.xyz);
}
inline vec4 flipsign(const vec4& u, const vec4& v) {
	return _mm_flipsign_ps(u.xyz, v.xyz);
}
inline vec4 sign(const vec4& u) {
	return _mm_sign_ps(u.xyz);
}
inline vec4 toint(const vec4& u) {
	return _mm_cvtepi32_ps(_mm_cvttps_epi32(u.xyz));
}
inline vec4 floor(const vec4& u) {
	return _mm_floor_ps(u.xyz);
}
inline vec4 round(const vec4& u) {
	return _mm_round_ps(u.xyz, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}
inline vec4 ceil(const vec4& u) {
	return _mm_ceil_ps(u.xyz);
}

inline float posdot(const vec4& u, const vec4& v) { return fmaxf(0.f, u & v); }
inline float absdot(const vec4& u, const vec4& v) { return fabsf(u & v); }
inline float dotabs(const vec4& u, const vec4& v) { return (abs(u) & abs(v)); }

inline vec4 rotr3(vec4 x) {
	return _mm_shuffle_ps(x.xyz, x.xyz, 0b11010010);
}
inline vec4 rotr4(vec4 x) {
	return _mm_shuffle_ps(x.xyz, x.xyz, 0b10010011);
}

inline vec4 rotl3(vec4 x) {
	return _mm_shuffle_ps(x.xyz, x.xyz, 0b11001001);
}
inline vec4 rotl4(vec4 x) {
	return _mm_shuffle_ps(x.xyz, x.xyz, 0b00111001);
}

inline vec4 vec_ins(const vec4& u, const vec4 &min, const vec4 &max) {
	__m128 gt = _mm_cmpgt_ps(u.xyz, min.xyz);
	__m128 lt = _mm_cmplt_ps(u.xyz, max.xyz);
	return _mm_and_ps(_mm_and_ps(gt, lt), _mm_set1_ps(1.f));
}
inline vec4 vec_wit(const vec4& u, const vec4& min, const vec4& max) {
	__m128 gt = _mm_cmpge_ps(u.xyz, min.xyz);
	__m128 lt = _mm_cmple_ps(u.xyz, max.xyz);
	return _mm_and_ps(_mm_and_ps(gt, lt), _mm_set1_ps(1.f));
}
inline vec4 vec_gt(const vec4& u, const vec4& v) {
	__m128 mask = _mm_cmpgt_ps(u.xyz, v.xyz);
	return _mm_and_ps(mask, _mm_set1_ps(1.f));
}
inline vec4 vec_lt(const vec4& u, const vec4& v) {
	__m128 mask = _mm_cmplt_ps(u.xyz, v.xyz);
	return _mm_and_ps(mask, _mm_set1_ps(1.f));
}
inline vec4 vec_ge(const vec4& u, const vec4& v) {
	__m128 mask = _mm_cmpge_ps(u.xyz, v.xyz);
	return _mm_and_ps(mask, _mm_set1_ps(1.f));
}
inline vec4 vec_le(const vec4& u, const vec4& v) {
	__m128 mask = _mm_cmple_ps(u.xyz, v.xyz);
	return _mm_and_ps(mask, _mm_set1_ps(1.f));
}
inline vec4 vec_eq(const vec4& u, const vec4& v) {
	__m128 mask = _mm_cmpeq_ps(u.xyz, v.xyz);
	return _mm_and_ps(mask, _mm_set1_ps(1.f));
}
inline vec4 vec_neq(const vec4& u, const vec4& v) {
	__m128 mask = _mm_cmpneq_ps(u.xyz, v.xyz);
	return _mm_and_ps(mask, _mm_set1_ps(1.f));
}

inline vec4 vec_near0(const vec4& u) {
	__m128 v = _mm_cmplt_ps(fabs(u).xyz, _mm_set1_ps(eps));
	return _mm_and_ps(v, _mm_set1_ps(1.f));
}
inline vec4 vec_not0(const vec4& u) {
	__m128 v = _mm_cmpge_ps(fabs(u).xyz, _mm_set1_ps(eps));
	return _mm_and_ps(v, _mm_set1_ps(1.f));
}

inline bool gt(const vec4& u, const vec4& v) {
	__m128 mask = _mm_cmplt_ps(u.xyz, v.xyz);
	return !(_mm_movemask_ps(mask) & 0b0111);
}
inline bool lt(const vec4& u, const vec4& v) {
	__m128 mask = _mm_cmpgt_ps(u.xyz, v.xyz);
	return !(_mm_movemask_ps(mask) & 0b0111);
}

inline bool eq(const vec4& u, const vec4& v) {
	__m128 w = _mm_cmpneq_ps(u.xyz, v.xyz);
	return !(_mm_movemask_ps(w) & 0b0111);
}
inline bool neq(const vec4& u, const vec4& v) {
	__m128 w = _mm_cmpneq_ps(u.xyz, v.xyz);
	return (_mm_movemask_ps(w) & 0b0111);
}
inline bool eq_tol(const vec4& u, const vec4& v, float tol = eps) {
	__m128 w = _mm_cmpge_ps(fabs(u - v).xyz, _mm_set1_ps(tol));
	return !(_mm_movemask_ps(w) & 0b0111);
}
inline bool eq0(const vec4& u) {
	__m128 v = _mm_cmpneq_ps(u.xyz, _mm_setzero_ps());
	return !(_mm_movemask_ps(v) & 0b0111);
}
inline bool neq0(const vec4& u) {
	__m128 v = _mm_cmpneq_ps(u.xyz, _mm_setzero_ps());
	return (_mm_movemask_ps(v) & 0b0111);
}

inline bool near0(const vec4& u) {
	__m128 v = _mm_cmpge_ps(fabs(u).xyz, _mm_set1_ps(eps));
	return !(_mm_movemask_ps(v) & 0b0111);
}
inline bool not0(const vec4& u) {
	__m128 v = _mm_cmpge_ps(fabs(u).xyz, _mm_set1_ps(eps));
	return (_mm_movemask_ps(v) & 0b0111);
}

inline float min(const vec4& u) {
	return fminf(u.xyz[0], fminf(u.xyz[1], u.xyz[2]));
}
inline float max(const vec4& u) {
	return fmaxf(u.xyz[0], fmaxf(u.xyz[1], u.xyz[2]));
}
inline vec4 min(const vec4& u, const vec4& v) {
	return _mm_min_ps(u.xyz, v.xyz);
}
inline vec4 max(const vec4& u, const vec4& v) {
	return _mm_max_ps(u.xyz, v.xyz);
}
inline vec4 rapvec() {
	return vec4(_mm_rafl_ps());
}
//(-1,1)
inline vec4 ravec() {
	return 2 * vec4(_mm_rafl_ps()) - 1;
}
inline vec4 ravec(const vec4& min, const vec4& max) {
	return min + (max - min) * _mm_rafl_ps();
}
inline vec4 fixnan(const vec4& u) {
	__m128 mask = _mm_isnan_ps(u.xyz);
	return _mm_andnot_ps(mask, u.xyz);
}


