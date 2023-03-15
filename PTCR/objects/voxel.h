#pragma once
#include "obj.h"
//voxel vec4(x,y,z,a) 16B
class voxel {
public:
	voxel() {}
	//a = half side length
	voxel(vec3 Q, float a) :Qa(Q, a) {}
	voxel(vec4 _Qa) :Qa(_Qa) {}

	inline bool hit(const ray& r, hitrec& rec) const
	{
		//Based on AABB intersection method:
		vec3 t1 = (A() - r.O) * r.iD;
		vec3 t2 = (B() - r.O) * r.iD;
		vec3 tmin = min(t1, t2);
		vec3 tmax = max(t1, t2);
		float mint = max(tmin);
		float maxt = min(tmax);
		if (mint > maxt || maxt < 0) return false;
		bool face = mint > 0;
		float t = face ? mint : maxt;
		if (inside(t, eps2, rec.t))
		{
			vec3 P = r.at(t);
			vec3 W = (P - Qa) / Qa.w();
			vec3 N = norm(toint(1.00001f * W));
			vec3 UV = 0.5f * (1.f + W - N);
			rec.N = face ? N : -N;
			rec.P = r.at(t);
			rec.t = t;
			rec.u = fabs(UV.x() - 0.5f) > eps2 ? UV.x() : UV.z();
			rec.v = fabs(UV.y() - 0.5f) > eps2 ? UV.y() : UV.z();
			rec.face = face;
			return true;
		}
		return false;
	}
	inline aabb get_box()const {
		return aabb(A(), B());
	}

	inline voxel trans(const mat4& T) const {
		return voxel(T.pnt(Qa));
	}
	inline voxel move(vec3 P) const {
		return voxel(vec3(Qa + P,Qa.w() * P.w()));
	}
	inline float pdf(const ray& r)const {
		hitrec rec;
		if (!hit(r, rec))return 0;
		float S = 4.f * Qa.w() * Qa.w();
		float NoL = absdot(r.D, rec.N);
		if (!rec.face)return rec.t * rec.t / (6.f * S * NoL);
		else {
			//computes visible faces (took looooong enough to figure out, ~10 hours)
			//if point's bound is inside 2 coordinates of box, it can see only one face,
			//if only one bound is inside, it can see 2 faces,
			//and when none coordinates are inside, 3 faces are visible
			//multiply S by number of visible faces to get correct pdf
			vec3 faces = vec_ins(r.O, A(), B());
			float visible = 3.f - sum(faces);
			return rec.t * rec.t / (visible * S * NoL);
		}
	}
	inline vec3 rand_to(vec3 O) const {
		//sample until visible face is hit, or point is inside cuboid
		bool inside = gt(O, A()) && lt(O, B());
		while (1)
		{
#if 1
			vec3 r = ravec();
			vec3 W = r / max(fabs(r));
			vec3 N = toint(1.0000001f * W);
			bool dir = rafl() < 0.5f;
			//smaller waste of rejection sampling, pick one side, than eventually rotate to all 6
			//gotta choose rotated side randomly to get unbiased results; left or right depending on beginning condition
			for (int i = 0; i < 3; i++) {
				vec3 P = Qa + Qa.w() * W;
				vec3 L = P - O;
				if (inside || dot(L, N) < 0) return norm(L);
				P = Qa - Qa.w() * W;
				L = P - O;
				if (dot(L, N) > 0) return norm(L);
				W = dir ? rotl3(W) : rotr3(W);
				N = dir ? rotl3(N) : rotr3(N);
			}
#else
			vec3 r = ravec();
			vec3 W = r / max(fabs(r));
			vec3 N = toint(1.00001f * W);
			vec3 P = Qa + Qa.w() * W;
			vec3 L = P - O;
			if (inside || dot(L, N) < 0)return norm(L);
			P = Qa - Qa.w() * W;
			L = P - O;
			if (dot(L, N) > 0)return norm(L);
#endif
		}
	}
	inline ray rand_from() const {
		vec3 r = ravec();
		vec3 W = r / max(fabs(r));
		vec3 N = toint(1.0000001f * W);
		vec3 O = Qa + N * Qa.w();
		vec3 L = onb(rafl() < 0.5f ? N : -N).world(sa_cos());
		return ray(O, L);
	}
	inline float area()const {
		return 24.f * Qa.w() * Qa.w();
	}
	vec4 Qa;
private:
	inline vec3 A()const {
		return Qa - Qa.w();
	}
	inline vec3 B()const {
		return Qa + Qa.w();
	}
};