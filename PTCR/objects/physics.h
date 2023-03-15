#pragma once
#include "vec3.h"
class obj_phys{
public:
	obj_phys(vec3 X =0, vec3 V = 0, float m = 1):X(X,m),V(V){}
	float mass() {
		return X.w();
	}
	void tick(vec3 F, float t)
	{
		V += F/mass() * t;
		X += V * t;
	}
	vec3 X, V;
};