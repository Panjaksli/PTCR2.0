#pragma once
#include "util.h"
struct __declspec(align(16))
vec3
{
	vec3() : xyz{} {}
	vec3(float t) : xyz{ t,t,t,t } {}
	vec3(float3 t) : xyz{ t.x,t.y,t.z,0 } {}
	vec3(vec3 v, float w) :xyz{ v.x(),v.y(),v.z(),w } {}
	vec3(float x, float y, float z = 0, float w = 0) : xyz{ x,y,z,w } {}
	inline float x() const { return xyz[0]; }
	inline float y() const { return xyz[1]; }
	inline float z() const { return xyz[2]; }
	inline float w() const { return xyz[3]; }
	inline float operator[](uint i) const { return xyz[i]; }
	inline float& operator[](uint i) { return _xyz[i]; }
	inline vec3 operator-() const { return vec3(-x(), -y(), -z(), -w()); }
	inline vec3& operator+=(vec3 u)
	{
		xyz[0] += u.x();
		xyz[1] += u.y();
		xyz[2] += u.z();
		xyz[3] += u.w();
		return *this;
	}
	inline vec3& operator-=(vec3 u)
	{
		xyz[0] -= u.x();
		xyz[1] -= u.y();
		xyz[2] -= u.z();
		xyz[3] -= u.w();
		return *this;
	}
	inline vec3& operator*=(vec3 u)
	{
		xyz[0] *= u.x();
		xyz[1] *= u.y();
		xyz[2] *= u.z();
		xyz[3] *= u.w();
		return *this;
	}
	inline vec3& operator/=(vec3 u)
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
	inline vec3 dir() const
	{
		float inv = 1.f / len();
		return vec3(x() * inv, y() * inv, z() * inv, 0);
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
		float _xyz[4];
		float xyz[4];
	}__declspec(align(16));
};

inline vec3 norm(vec3 u) { return u.dir(); }
inline vec3 operator+(vec3 u, vec3 v) { return u += v; }
inline vec3 operator-(vec3 u, vec3 v) { return u -= v; }
inline vec3 operator*(vec3 u, vec3 v) { return u *= v; }
inline vec3 operator/(vec3 u, vec3 v) { return u /= v; }

inline float operator&(vec3 u, vec3 v) {
	return u.x() * v.x() + u.y() * v.y() + u.z() * v.z();
}
inline vec3 operator%(vec3 u, vec3 v)
{
	float x = u.y() * v.z() - u.z() * v.y();
	float y = u.z() * v.x() - u.x() * v.z();
	float z = u.x() * v.y() - u.y() * v.x();
	return vec3(x, y, z);
}
inline vec3 expf(vec3 u)
{
	return vec3(expf(u.x()), expf(u.y()), expf(u.z()), expf(u.w()));
}
inline vec3 sqrt(vec3 u)
{
	return vec3(sqrtf(u.x()), sqrtf(u.y()), sqrtf(u.z()), sqrtf(u.w()));
}
inline vec3 abs(vec3 u)
{
	return vec3(fabsf(u.x()), fabsf(u.y()), fabsf(u.z()), fabsf(u.w()));
}
inline vec3 fabs(vec3 u)
{
	return abs(u);
}
inline vec3 copysign(vec3 u, vec3 v)
{
	return vec3(copysignf(u.x(), v.x()), copysignf(u.y(), v.y()), copysignf(u.z(), v.z()), copysignf(u.w(), v.w()));
}
inline vec3 sign(vec3 u) {
	return vec3(signf(u.x()), signf(u.y()), signf(u.z()), signf(u.w()));
}
inline vec3 toint(vec3 u) {
	return vec3(int(u.x()), int(u.y()), int(u.z()), int(u.w()));
}
inline vec3 floor(vec3 u) {
	return vec3(floorf(u.x()), floorf(u.y()), floorf(u.z()), floorf(u.w()));
}
inline vec3 ceil(vec3 u) {
	return vec3(ceilf(u.x()), ceilf(u.y()), ceilf(u.z()), ceilf(u.w()));
}

inline vec3 cross(vec3 u, vec3 v) { return u % v; }
inline float dot(vec3 u, vec3 v) { return u & v; }
inline float dot4(vec3 u, vec3 v) { return u.x() * v.x() + u.y() * v.y() + u.z() * v.z() + u.w() * v.w(); }
inline float posdot(vec3 u, vec3 v) { return fmaxf(0.f, u & v); }
inline float absdot(vec3 u, vec3 v) { return fabsf(u & v); }
inline float dotabs(vec3 u, vec3 v) { return (abs(u) & abs(v)); }

inline vec3 rotl3(vec3 u) {
	float t = u.x();
	u.xyz[0] = u.y();
	u.xyz[1] = u.z();
	u.xyz[2] = t;
	return u;
}
inline vec3 rotl4(vec3 u) {
	float t = u.x();
	u.xyz[0] = u.y();
	u.xyz[1] = u.z();
	u.xyz[2] = u.w();
	u.xyz[3] = t;
	return u;
}

inline vec3 rotr3(vec3 u) {
	float t = u.z();
	u.xyz[2] = u.y();
	u.xyz[1] = u.x();
	u.xyz[0] = t;
	return u;
}
inline vec3 rotr4(vec3 u) {
	float t = u.w();
	u.xyz[3] = u.z();
	u.xyz[2] = u.y();
	u.xyz[1] = u.x();
	u.xyz[0] = t;
	return u;
}

inline vec3 vec_ins(vec3 u, vec3 min, vec3 max) {
	return vec3(u.x() > min.x() && u.x() < max.x(), u.y() > min.y() && u.y() < max.y(), u.z() > min.z() && u.z() < max.z(), u.w() > min.w() && u.w() < max.w());
}
inline vec3 vec_wit(vec3 u, vec3 min, vec3 max) {
	return vec3(u.x() >= min.x() && u.x() <= max.x(), u.y() >= min.y() && u.y() <= max.y(), u.z() >= min.z() && u.z() <= max.z(), u.w() >= min.w() && u.w() <= max.w());
}
inline vec3 vec_gt(vec3 u, vec3 v) {
	return vec3(u.x() > v.x(), u.y() > v.y(), u.z() > v.z(), u.w() > v.w());
}
inline vec3 vec_lt(vec3 u, vec3 v) {
	return vec3(u.x() < v.x(), u.y() < v.y(), u.z() < v.z(), u.w() < v.w());
}
inline vec3 vec_ge(vec3 u, vec3 v) {
	return vec3(u.x() >= v.x(), u.y() >= v.y(), u.z() >= v.z(), u.w() >= v.w());
}
inline vec3 vec_le(vec3 u, vec3 v) {
	return vec3(u.x() <= v.x(), u.y() <= v.y(), u.z() <= v.z(), u.w() <= v.w());
}
inline vec3 vec_eq(vec3 u, vec3 v) {
	return vec3(u.xyz[0] == v.x(), u.xyz[1] == v.y(), u.xyz[2] == v.z(), u.xyz[3] == v.w());
}
inline vec3 vec_neq(vec3 u, vec3 v) {
	return vec3(u.x() != v.x(), u.y() != v.y(), u.z() != v.z(), u.w() != v.w());
}

inline vec3 vec_near0(vec3 u)
{
	u = fabs(u);
	return vec3(u.x() < eps, u.y() < eps, u.z() < eps);
}
inline vec3 vec_not0(vec3 u) {
	u = fabs(u);
	return vec3(u.x() >= eps, u.y() >= eps, u.z() >= eps);
}
inline bool lt(vec3 u, vec3 v)
{
	return u.x() < v.x() && u.y() < v.y() && u.z() < v.z();
}
inline bool gt(vec3 u, vec3 v)
{
	return u.x() > v.x() && u.y() > v.y() && u.z() > v.z();
}
inline bool eq(vec3 u, vec3 v)
{
	return u.xyz[0] == v.x() && u.xyz[1] == v.y() && u.xyz[2] == v.z();
}
inline bool neq(vec3 u, vec3 v)
{
	return !eq(u, v);
}
inline bool eq_tol(vec3 u, vec3 v, float tol = eps)
{
	vec3 w = abs(u - v);
	return w.x() < eps && w.y() < eps && w.z() < eps;
}
inline bool eq0(vec3 u)
{
	return u.xyz[0] == 0 && u.xyz[1] == 0 && u.xyz[2] == 0;
}
inline bool neq0(vec3 u)
{
	return !eq0(u);
}

inline bool near0(vec3 u)
{
	u = fabs(u);
	return u.x() < eps && u.y() < eps && u.z() < eps;
}
inline bool not0(vec3 u) {
	return !near0(u);
}

inline float min(vec3 u)
{
	return fminf(u.x(), fminf(u.y(), u.z()));
}
inline float max(vec3 u)
{
	return fmaxf(u.x(), fmaxf(u.y(), u.z()));
}
inline vec3 min(vec3 u, vec3 v)
{
	return vec3(fminf(u.x(), v.x()), fminf(u.y(), v.y()), fminf(u.z(), v.z()), fminf(u.w(), v.w()));
}
inline vec3 max(vec3 u, vec3 v)
{
	return vec3(fmaxf(u.x(), v.x()), fmaxf(u.y(), v.y()), fmaxf(u.z(), v.z()), fmaxf(u.w(), v.w()));
}
inline vec3 rapvec() {
	return vec3(rafl(), rafl(), rafl());
}
inline vec3 ravec() {
	return 2.f * vec3(rafl(), rafl(), rafl()) - 1.f;
}
inline vec3 ravec(vec3 min, vec3 max) {
	return min + (max - min) * rapvec();
}
