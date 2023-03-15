#pragma once
#include "vec3.h"

/*
From:
https://schuttejoe.github.io/post/ggximportancesamplingpart1/
*/

inline float DGGX(float NoH, float a)
{
	float a2 = a * a;
	float d = NoH * NoH * (a2 - 1.0f) + 1.0f;
	return ipi * a2 / (d * d);
}
inline float MGGX(float NoV, float a)
{
	float a2 = a * a;
	float d3 = sqrtf(a2 + (1.f + a2) * NoV * NoV) + NoV;
	return 2.f * NoV / d3;

}
inline float GGX(float NoL, float NoV, float a)
{
	float a2 = a * a;
	float d1 = NoV * sqrtf(a2 + (1.0f - a2) * NoL * NoL);
	float d2 = NoL * sqrtf(a2 + (1.0f - a2) * NoV * NoV);
	return 2.0f * NoL * NoV / (d1 + d2);
}
inline float VNDF_GGX(float NoL, float NoV, float a)
{
	float a2 = a * a;
	float d1 = NoV * sqrtf(a2 + (1.0f - a2) * NoL * NoL);
	float d2 = NoL * sqrtf(a2 + (1.0f - a2) * NoV * NoV);
	float d3 = sqrtf(a2 + (1.f + a2) * NoV * NoV) + NoV;
	return NoL * d3 / (d1 + d2);
}
inline float fres_spec(float NoV, float F0)
{
	return F0 + (1.0f - F0) * pow5(1.0f - NoV);
}

inline vec3 fres_spec(float NoV, vec3 F0)
{
	return F0 + (1.0f - F0) * pow5(1.0f - NoV);
}

inline vec3 fres_blend(vec3 kd, vec3 ks, float NoL, float NoV) {
	kd = (ipi * 28.f / 23.f) * kd * (1.f - ks);
	kd *= 1.f - pow5(1.f - 0.5f * NoL);
	kd *= 1.f - pow5(1.f - 0.5f * NoV);
	return kd;
}


inline float fres_refl(float NoV, float ir) {
	float r0 = (1.f - ir) / (1.f + ir);
	r0 = r0 * r0;
	return r0 + (1.f - r0) * pow5(1.f - NoV);
}
