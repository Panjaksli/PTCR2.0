#pragma once
#include "vec4.h"
/*Unused for now*/
class obj_phys {
public:
	obj_phys(vec4 X = 0, vec4 V = 0, float m = 1) :X(X, m), V(V) {}
	float mass() {
		return X.w();
	}
	void tick(vec4 F, float t) {
		V += F / mass() * t;
		X += V * t;
	}
	vec4 X, V;
};