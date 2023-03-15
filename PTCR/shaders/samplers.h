#pragma once
#include "vec3.h"
#include "onb.h"
inline vec3 sa_disk() {
	float r[2]; rafl_tuple(r);
	float phi = pi2 * r[0];
	return sqrtf(r[1]) * cossin(phi);
}
inline vec3 sa_sph()
{
	float r[2]; rafl_tuple(r);
	float phi = pi2 * r[0];
	r[1] = 2.f * r[1] - 1.f;
	vec3 d = sqrtf(1.f - r[1] * r[1]) * cossin(phi);
	return d + vec3(0, 0, r[1]);
}
inline vec3 sa_hem(vec3 v)
{
	vec3 u = sa_sph();
	float s = dot(u, v);
	return signf(s) * u;
}
inline vec3 sa_uni()
{
	float r[2]; rafl_tuple(r);
	const float phi = pi2 * r[0];
	vec3 d = sqrtf(1.f - r[1] * r[1]) * cossin(phi);
	return d + vec3(0, 0, r[1]);
}
inline vec3 sa_cos()
{
	float r[2]; rafl_tuple(r);
	const float phi = pi2 * r[0];
	vec3 d = sqrtf(r[1]) * cossin(phi);
	return d + vec3(0, 0, sqrtf(1.f - r[1]));
}
/*
From:
https://schuttejoe.github.io/post/ggximportancesamplingpart1/
Simplified by ME
*/
inline vec3 sa_ggx(float a) {
	float r[2]; rafl_tuple(r);
	const float phi = pi2 * r[0];
	float z2 = (1.0f - r[1]) / ((a * a - 1.0f) * r[1] + 1.0f);
	vec3 d = sqrtf(1.f - z2) * cossin(phi);
	return d + vec3(0, 0, sqrtf(z2));
}
/*https://hal.archives-ouvertes.fr/hal-01509746/document
Also simplified by ME
*/
inline vec3 sa_vndf(vec3 V_, float ro)
{
	float r[2]; rafl_tuple(r);
	vec3 scl = vec3(ro, ro, 1.f, 1.f);
	vec3 V = norm(scl * V_);
	vec3 T1 = (V.z() < 0.9999f) ? norm(cross(V, vec3(0, 0, 1))) : vec3(1, 0, 0);
	vec3 T2 = cross(T1, V);
	float a = 1.f / (1.f + V.z());
	float b = r[1] - a;
	bool neg = b < 0;
	float phi = neg ? pi * r[1] / a : pi + pi * b / (1.f - a);
	vec3 ang = sqrtf(r[0]) * cossin(phi);
	float P1 = ang.x();
	float P2 = neg ? ang.y() : ang.y() * V.z();
	float P3 = sqrtf(fabsf(1.f - P1 * P1 - P2 * P2));
	vec3 N = P1 * T1 + P2 * T2 + P3 * V;
	N.xyz[2] = fmaxf(N.z(), 0);
	N = norm(scl * N);
	return N;
}
