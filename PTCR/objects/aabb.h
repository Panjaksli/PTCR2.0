#pragma once
#include "vec4.h"
#include "ray.h"

struct aabb {
	aabb() {}
	aabb(const vec4 p1, const vec4 p2) : pmin(min(p1, p2)), pmax(max(p1, p2)) {}
	aabb(const vec4 p1, const vec4 p2, const vec4 p3) :pmin(min(p1, p2, p3)), pmax(max(p1, p2, p3)) {}
	aabb(const vec4 p1, const vec4 p2, const vec4 p3, const vec4 p4) :pmin(min(p1, p2, min(p3, p4))), pmax(max(p1, p2, max(p3, p4))) {}
	aabb(const aabb& box1, const aabb& box2) :pmin(min(box1.pmin, box2.pmin)), pmax(max(box1.pmax, box2.pmax)) {}
	inline aabb operator+(const aabb& box)const {
		aabb box1 = *this;
		box1.join(box);
		return box1;
	}
	inline aabb move(vec4 P) const {
		return { pmin + P, pmax + P };
	}
	inline void join(const aabb& box) {
		pmin = min(pmin, box.pmin), pmax = max(pmax, box.pmax);
	}
	inline void pad() {
		vec4 delta = eps * vec_lt(fabs(pmax - pmin), eps);
		pmin = pmin - delta;
		pmax = pmax + delta;
	}
	inline aabb padded() const {
		aabb nbox = *this;
		nbox.pad();
		return nbox;
	}
	inline vec4 pmid()const { return 0.5f * (pmin + pmax); }
	inline bool inside(vec4 O) const { return gt(O, pmin) && lt(O, pmax); }
	inline float axis_len(uchar axis) const {
		vec4 x = abs(pmax - pmin);
		return x[axis];
	}
	inline uchar get_longest_axis() const {
		vec4 x = abs(pmax - pmin);
		uchar axis = 0;
		if (x.y() > x.x()) axis = 1;
		if (x.z() > x[axis]) axis = 2;
		return axis;
	}
	void print()const {
		pmin.print();
		pmax.print();
	}
	__forceinline bool hit(const ray& r) const {
		float mint, maxt;
		bounds_check(r, mint, maxt);
		return mint < maxt && maxt > 0;
	}
	__forceinline bool hit(const ray& r, float& t) const {
		float mint, maxt;
		bounds_check(r, mint, maxt);
		if (mint < maxt && maxt > 0 && mint < t) {
			t = mint;
			return true;
		}
		return false;
	}
	__forceinline bool closer_hit(const ray& r, float t) const {
		float mint, maxt;
		bounds_check(r, mint, maxt);
		return mint < maxt && maxt > 0 && mint < t;
	}
	__forceinline bool hit_edge(const ray& r) const {
		float mint, maxt;
		bounds_check(r, mint, maxt);
		return mint < maxt && maxt > 0 && (maxt - mint) < eps;
	}
	__forceinline bool hit_edge(const ray& r, float& t, bool& edge) const {
		float mint, maxt;
		bounds_check(r, mint, maxt);
		if (mint < maxt && maxt > 0 && mint < t) {
			edge = lt(20 * fabsf(maxt - mint), (pmax - pmin));
			t = mint;
			return true;
		}
		return false;
	}
	__forceinline bool shift(ray& r, float& t) const {
		float mint, maxt;
		bounds_check(r, mint, maxt);
		if (mint < maxt && maxt > 0) {
			t = fmaxf(0, mint);
			r.O = r.at(t);
			return true;
		}
		return false;
	}
	//not scaled by 2
	inline float area()const {
		vec4 S = pmax - pmin;
		return dot(S, rotl3(S));
	}
	vec4 pmin = infp, pmax = infn;
private:
	/*:
	Edited from: https://tavianator.com/2011/ray_box.html
	*/
	__forceinline void bounds_check(const ray& r, float& mint, float& maxt) const {
		vec4 t1 = (pmin - r.O) * r.iD;
		vec4 t2 = (pmax - r.O) * r.iD;
		vec4 tmin = min(t1, t2);
		vec4 tmax = max(t1, t2);
		mint = max(tmin);
		maxt = min(tmax);
	}
};

