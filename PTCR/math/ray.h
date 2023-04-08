#pragma once
#include "vec3.h"

struct ray {
	ray() {}
	ray(vec3 _O, vec3 _D) :O(_O), D(_D), iD(1.f / D) {}
	ray(vec3 _O, vec3 _D, bool normalize) :O(_O), D(norm(_D)), iD(1.f / D) {}
	
	inline vec3 at(float t) const
	{
		return O + t * D;
	}
	inline ray off(vec3 P) const
	{
		return ray(O - P, D, iD);
	}
	vec3 O, D, iD;
private:
	ray(vec3 _O, vec3 _D, vec3 _iD) :O(_O), D(_D), iD(_iD) {}
};
