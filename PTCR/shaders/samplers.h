#pragma once
#include "vec4.h"
#include "onb.h"
inline vec4 sa_disk() {
	float r[2]; rafl_tuple(r);
	float phi = pi2 * r[0];
	return sqrtf(r[1]) * cossin(phi);
}
inline vec4 sa_sph()
{
	float r[2]; rafl_tuple(r);
	float phi = pi2 * r[0];
	r[1] = 2.f * r[1] - 1.f;
	vec4 d = sqrtf(1.f - r[1] * r[1]) * cossin(phi);
	return d + vec4(0, 0, r[1]);
}
inline vec4 sa_hem(vec4 v)
{
	vec4 u = sa_sph();
	float s = dot(u, v);
	return signf(s) * u;
}
inline vec4 sa_uni()
{
	float r[2]; rafl_tuple(r);
	const float phi = pi2 * r[0];
	vec4 d = sqrtf(1.f - r[1] * r[1]) * cossin(phi);
	return d + vec4(0, 0, r[1]);
}
inline vec4 sa_cos()
{
	float r[2]; rafl_tuple(r);
	const float phi = pi2 * r[0];
	vec4 d = sqrtf(r[1]) * cossin(phi);
	return d + vec4(0, 0, sqrtf(1.f - r[1]));
}
/*
From:
https://schuttejoe.github.io/post/ggximportancesamplingpart1/
Simplified by ME
*/
inline vec4 sa_ggx(float a) {
	float r[2]; rafl_tuple(r);
	const float phi = pi2 * r[0];
	float z2 = (1.0f - r[1]) / (r[1] * (a * a - 1.0f) + 1.0f);
	vec4 d = sqrtf(1.f - z2) * cossin(phi);
	return d + vec4(0, 0, sqrtf(z2));
}
/*https://hal.archives-ouvertes.fr/hal-01509746/document
Also simplified by ME
*/
inline vec4 sa_vndf(vec4 V_, float ro)
{
	float r[2]; rafl_tuple(r);
	vec4 scl = vec4(ro, ro, 1.f, 1.f);
	vec4 V = norm(scl * V_);
	vec4 T1 = V.z() < 0.999f ? norm(cross(V, vec4(0, 0, 1))) : vec4(1, 0, 0);
	vec4 T2 = cross(T1, V);
	float a = 1.f / (1.f + V.z());
	float b = r[1] - a;
	bool neg = b < 0;
	float phi = neg ? pi * r[1] / a : pi + pi * b / (1.f - a);
	vec4 ang = sqrtf(r[0]) * cossin(phi);
	float P1 = ang.x();
	float P2 = neg ? ang.y() : ang.y() * V.z();
	float P3 = sqrtf(fabsf(1.f - P1 * P1 - P2 * P2));
	vec4 N = P1 * T1 + P2 * T2 + P3 * V;
	N = norm(scl * N);
	return N;
}
