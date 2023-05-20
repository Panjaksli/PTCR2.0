#pragma once
#include "vec4.h"

struct ray {
	ray() {}
	ray(vec4 _O, vec4 _D) :O(_O), D(_D), iD(1.f / D) {}
	ray(vec4 _O, vec4 _D, bool normalize) :O(_O), D(norm(_D)), iD(1.f / D) {}

	inline vec4 at(float t) const {
		return O + t * D;
	}
	inline ray off(vec4 P) const {
		return ray(O - P, D, iD);
	}
	vec4 O, D, iD;
private:
	ray(vec4 _O, vec4 _D, vec4 _iD) :O(_O), D(_D), iD(_iD) {}
};
