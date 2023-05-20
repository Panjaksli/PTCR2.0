#pragma once
#include "vec4.h"

/*
From:
https://schuttejoe.github.io/post/ggximportancesamplingpart1/
*/

inline float DGGX(float NoH, float a) {
	float a2 = a * a;
	float d = NoH * NoH * (a2 - 1.0f) + 1.0f;
	return ipi * a2 / (d * d);
}
inline float MGGX(float NoV, float a) {
	float a2 = a * a;
	float d3 = sqrtf(a2 + (1.0f - a2) * NoV * NoV) + NoV;
	return 2.0f * NoV / d3;
}
inline float GGX(float NoL, float NoV, float a) {
	float a2 = a * a;
	float d1 = NoV * sqrtf(a2 + (1.0f - a2) * NoL * NoL);
	float d2 = NoL * sqrtf(a2 + (1.0f - a2) * NoV * NoV);
	return 2.0f * NoL * NoV / (d1 + d2);
}

inline float VNDF_GGX(float NoL, float NoV, float a) {
	return GGX(NoL, NoV, a) / MGGX(NoV, a);
}
inline float fres_spec(float NoV, float F0) {
	return F0 + (1.0f - F0) * pow5(1.0f - NoV);
}
inline vec4 fres_blend(vec4 n1, vec4 n2) {
	return n2 + (1.0f - n2) * pow5(1.0f - n1);
}
inline vec4 fres_spec(float NoV, vec4 F0) {
	return F0 + (1.0f - F0) * pow5(1.0f - NoV);
}

inline float fres_refl(float NoV, float n) {
	float r0 = (1.f - n) / (1.f + n);
	r0 = r0 * r0;
	return r0 + (1.f - r0) * pow5(1.f - NoV);
}

inline float fres_refl(float NoV, float n1, float n2) {
	float r0 = (n1 - n2) / (n1 + n2);
	r0 = r0 * r0;
	return r0 + (1.f - r0) * pow5(1.f - NoV);
}

/*https://otik.zcu.cz/bitstream/11025/11214/1/Lazanyi.pdf*/
inline float fres_conductor(float NoV, float ir, float k) {
	float nom = (ir - 1.f) * (ir - 1.f) + 4.f * ir * pow5(1.f - NoV) + k * k;
	float denom = (ir + 1.f) * (ir + 1.f) + k * k;
	return nom / denom;
}

inline float fresnel(float NoV, float n1, float n2) {
	float n = (n1 / n2);
	if (n1 > n2) {
		float sinx2 = n * n * (1.0f - NoV * NoV);
		if (sinx2 > 1.0f) return 1.0f;
		NoV = sqrtf(1.0f - sinx2);
	}
	return fres_refl(NoV, n);
}

inline float fresnel(float& NoV, float n1, float n2, float mu) {
	float n = (n1 / n2);
	if (n1 > n2) {
		float sinx2 = n * n * (1.0f - NoV * NoV);
		if (sinx2 > 1.0f) return 1.0f;
		NoV = sqrtf(1.0f - sinx2);
	}
	return fres_conductor(NoV, n, mu);
}