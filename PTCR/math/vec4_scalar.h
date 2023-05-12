#pragma once
#include "util.h"
struct vec4
{
	vec4() : xyz{} {}
	vec4(float t) : xyz{ t,t,t,t } {}
	vec4(float3 t) : xyz{ t.x,t.y,t.z,0 } {}
	vec4(vec4 v, float w) :xyz{ v.x(),v.y(),v.z(),w } {}
	vec4(float x, float y, float z = 0, float w = 0) : xyz{ x,y,z,w } {}
	inline float x() const { return xyz[0]; }
	inline float y() const { return xyz[1]; }
	inline float z() const { return xyz[2]; }
	inline float w() const { return xyz[3]; }
	inline float operator[](uint i) const { return xyz[i]; }
	inline float& operator[](uint i) { return _xyz[i]; }
	inline vec4 operator-() const { return vec4(-x(), -y(), -z(), -w()); }
	inline vec4& operator+=(const vec4 &u)
	{
		xyz[0] += u.x();
		xyz[1] += u.y();
		xyz[2] += u.z();
		xyz[3] += u.w();
		return *this;
	}
	inline vec4& operator-=(const vec4& u)
	{
		xyz[0] -= u.x();
		xyz[1] -= u.y();
		xyz[2] -= u.z();
		xyz[3] -= u.w();
		return *this;
	}
	inline vec4& operator*=(const vec4& u)
	{
		xyz[0] *= u.x();
		xyz[1] *= u.y();
		xyz[2] *= u.z();
		xyz[3] *= u.w();
		return *this;
	}
	inline vec4& operator/=(const vec4& u)
	{
		xyz[0] /= u.x();
		xyz[1] /= u.y();
		xyz[2] /= u.z();
		xyz[3] /= u.w();
		return *this;
	}

	inline float len2_v4() const {
		return x() * x() + y() * y() + z() * z() + w() * w();
	}
	inline float len_v4() const { return sqrtf(len2_v4()); }
	inline float len2() const {
		return x() * x() + y() * y() + z() * z();
	}
	inline float len() const { return sqrtf(len2()); }
	inline vec4 dir() const
	{
		float inv = 1.f / len();
		return vec4(x() * inv, y() * inv, z() * inv, 0);
	}
	inline vec4 fact() const
	{
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
		float _xyz[4];
		float xyz[4];
	};
};
inline vec4 norm(vec4 u) { return u.dir(); }
inline float dot(vec4 u, vec4 v) { return u.x() * v.x() + u.y() * v.y() + u.z() * v.z(); }
inline vec4 cross(vec4 u, vec4 v) {
	float x = u.y() * v.z() - u.z() * v.y();
	float y = u.z() * v.x() - u.x() * v.z();
	float z = u.x() * v.y() - u.y() * v.x();
	return vec4(x, y, z);
}
inline float dot4(vec4 u, vec4 v) { return u.x() * v.x() + u.y() * v.y() + u.z() * v.z() + u.w() * v.w(); }

inline vec4 operator&(vec4 u, vec4 v) { return dot(u,v); }
inline vec4 operator%(vec4 u, vec4 v) { return cross(u,v); }
inline vec4 operator+(vec4 u, vec4 v) { return u += v; }
inline vec4 operator-(vec4 u, vec4 v) { return u -= v; }
inline vec4 operator*(vec4 u, vec4 v) { return u *= v; }
inline vec4 operator/(vec4 u, vec4 v) { return u /= v; }

inline vec4 expf(vec4 u)
{
	return vec4(expf(u.x()), expf(u.y()), expf(u.z()), expf(u.w()));
}
inline vec4 sqrt(vec4 u)
{
	return vec4(sqrtf(u.x()), sqrtf(u.y()), sqrtf(u.z()), sqrtf(u.w()));
}
inline vec4 abs(vec4 u)
{
	return vec4(fabsf(u.x()), fabsf(u.y()), fabsf(u.z()), fabsf(u.w()));
}
inline vec4 fabs(vec4 u)
{
	return abs(u);
}
inline vec4 copysign(vec4 u, vec4 v)
{
	return vec4(copysignf(u.x(), v.x()), copysignf(u.y(), v.y()), copysignf(u.z(), v.z()), copysignf(u.w(), v.w()));
}
inline vec4 sign(vec4 u) {
	return vec4(signf(u.x()), signf(u.y()), signf(u.z()), signf(u.w()));
}
inline vec4 toint(vec4 u) {
	return vec4(int(u.x()), int(u.y()), int(u.z()), int(u.w()));
}
inline vec4 floor(vec4 u) {
	return vec4(floorf(u.x()), floorf(u.y()), floorf(u.z()), floorf(u.w()));
}
inline vec4 ceil(vec4 u) {
	return vec4(ceilf(u.x()), ceilf(u.y()), ceilf(u.z()), ceilf(u.w()));
}

inline float posdot(vec4 u, vec4 v) { return fmaxf(0.f, u & v); }
inline float absdot(vec4 u, vec4 v) { return fabsf(u & v); }
inline float dotabs(vec4 u, vec4 v) { return (abs(u) & abs(v)); }

inline vec4 rotl3(vec4 u) {
	float t = u.x();
	u.xyz[0] = u.y();
	u.xyz[1] = u.z();
	u.xyz[2] = t;
	return u;
}
inline vec4 rotl4(vec4 u) {
	float t = u.x();
	u.xyz[0] = u.y();
	u.xyz[1] = u.z();
	u.xyz[2] = u.w();
	u.xyz[3] = t;
	return u;
}

inline vec4 rotr3(vec4 u) {
	float t = u.z();
	u.xyz[2] = u.y();
	u.xyz[1] = u.x();
	u.xyz[0] = t;
	return u;
}
inline vec4 rotr4(vec4 u) {
	float t = u.w();
	u.xyz[3] = u.z();
	u.xyz[2] = u.y();
	u.xyz[1] = u.x();
	u.xyz[0] = t;
	return u;
}

inline vec4 vec_ins(vec4 u, vec4 min, vec4 max) {
	return vec4(u.x() > min.x() && u.x() < max.x(), u.y() > min.y() && u.y() < max.y(), u.z() > min.z() && u.z() < max.z(), u.w() > min.w() && u.w() < max.w());
}
inline vec4 vec_wit(vec4 u, vec4 min, vec4 max) {
	return vec4(u.x() >= min.x() && u.x() <= max.x(), u.y() >= min.y() && u.y() <= max.y(), u.z() >= min.z() && u.z() <= max.z(), u.w() >= min.w() && u.w() <= max.w());
}
inline vec4 vec_gt(vec4 u, vec4 v) {
	return vec4(u.x() > v.x(), u.y() > v.y(), u.z() > v.z(), u.w() > v.w());
}
inline vec4 vec_lt(vec4 u, vec4 v) {
	return vec4(u.x() < v.x(), u.y() < v.y(), u.z() < v.z(), u.w() < v.w());
}
inline vec4 vec_ge(vec4 u, vec4 v) {
	return vec4(u.x() >= v.x(), u.y() >= v.y(), u.z() >= v.z(), u.w() >= v.w());
}
inline vec4 vec_le(vec4 u, vec4 v) {
	return vec4(u.x() <= v.x(), u.y() <= v.y(), u.z() <= v.z(), u.w() <= v.w());
}
inline vec4 vec_eq(vec4 u, vec4 v) {
	return vec4(u.xyz[0] == v.x(), u.xyz[1] == v.y(), u.xyz[2] == v.z(), u.xyz[3] == v.w());
}
inline vec4 vec_neq(vec4 u, vec4 v) {
	return vec4(u.x() != v.x(), u.y() != v.y(), u.z() != v.z(), u.w() != v.w());
}

inline vec4 vec_near0(vec4 u)
{
	u = fabs(u);
	return vec4(u.x() < eps, u.y() < eps, u.z() < eps);
}
inline vec4 vec_not0(vec4 u) {
	u = fabs(u);
	return vec4(u.x() >= eps, u.y() >= eps, u.z() >= eps);
}
inline bool lt(vec4 u, vec4 v)
{
	return u.x() < v.x() && u.y() < v.y() && u.z() < v.z();
}
inline bool gt(vec4 u, vec4 v)
{
	return u.x() > v.x() && u.y() > v.y() && u.z() > v.z();
}
inline bool eq(vec4 u, vec4 v)
{
	return u.xyz[0] == v.x() && u.xyz[1] == v.y() && u.xyz[2] == v.z();
}
inline bool neq(vec4 u, vec4 v)
{
	return !eq(u, v);
}
inline bool eq_tol(vec4 u, vec4 v, float tol = eps)
{
	vec4 w = abs(u - v);
	return w.x() < eps && w.y() < eps && w.z() < eps;
}
inline bool eq0(vec4 u)
{
	return u.xyz[0] == 0 && u.xyz[1] == 0 && u.xyz[2] == 0;
}
inline bool neq0(vec4 u)
{
	return !eq0(u);
}

inline bool near0(vec4 u)
{
	u = fabs(u);
	return u.x() < eps && u.y() < eps && u.z() < eps;
}
inline bool not0(vec4 u) {
	return !near0(u);
}

inline float min(vec4 u)
{
	return fminf(u.x(), fminf(u.y(), u.z()));
}
inline float max(vec4 u)
{
	return fmaxf(u.x(), fmaxf(u.y(), u.z()));
}
inline vec4 min(vec4 u, vec4 v)
{
	return vec4(fminf(u.x(), v.x()), fminf(u.y(), v.y()), fminf(u.z(), v.z()), fminf(u.w(), v.w()));
}
inline vec4 max(vec4 u, vec4 v)
{
	return vec4(fmaxf(u.x(), v.x()), fmaxf(u.y(), v.y()), fmaxf(u.z(), v.z()), fmaxf(u.w(), v.w()));
}
inline vec4 rapvec() {
	return vec4(rafl(), rafl(), rafl());
}
inline vec4 ravec() {
	return 2.f * vec4(rafl(), rafl(), rafl()) - 1.f;
}
inline vec4 ravec(vec4 min, vec4 max) {
	return min + (max - min) * rapvec();
}
