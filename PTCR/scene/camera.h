#pragma once
#include "sensor.h"
#include "mat4.h"
#include "ray.h"
#include "obj.h"

struct projection {
	projection() {}
	projection(mat4 T, float w, float h, float tfov) :T(T), w(w), h(h), iw(1 / w), ih(1 / h), asp(w / h), tfov(tfov) {}
	mat4 T;
	float w, h;
	float iw, ih;
	float asp;
	float tfov;
	bool operator==(const projection& proj)const {
		return T.x == proj.T.x&& T.y == proj.T.y && T.z == proj.T.z && T.w == proj.T.w
			&& w == proj.w && h == proj.h && asp == proj.asp && asp == proj.asp;
	}
	bool operator!=(const projection& proj)const {
		return !(*this == proj);
	}
};

class camera
{
public:
	camera() {}
	camera(uint w, uint h, float fov, mat4 _T = mat4()) :T(_T), CCD(w, h), P(T.P()), iw(1.0 / w), ih(1.0 / h), asp((float)w / h), fov(fov), tfov(tan(0.5f * torad(fov))), speed(1) {}
	mat4 T = mat4();
	sensor CCD;
	vec4 P = 0;
	vec4 V = 0;
	uint& w = CCD.w, & h = CCD.h;
	float iw = 1.f / w, ih = 1.f / h, asp = float(w) / h;
	float fov = 90, tfov = tan(0.5f * torad(fov)), hfov = 0;
	float fstop = 16.f,exposure = 1.f;
	float foc_t = 1.f, foc_l = 0.0216f;
	const float frame = 0.035, diagonal = 0.0432;
	float speed = 1.f;
	bool autofocus = 1, bokeh = 1;
	bool moving = 1, free = 0, collision = 1;

	__forceinline ray optical_ray(vec4 xy) const
	{
		if (!bokeh)return pinhole_ray(xy);
		vec4 D = T.vec(foc_t * SS(xy));
		vec4 r = T.vec(foc_l * sa_disk() / fstop);
		return ray(P - r, D + r, true);
	}
	__forceinline ray pinhole_ray(vec4 xy) const { return ray(P, T.vec(SS(xy)), true); }
	__forceinline ray optical_ray(float py, float px) const { return optical_ray(vec4(px, py)); }
	__forceinline ray pinhole_ray(float py, float px) const { return pinhole_ray(vec4(px, py)); }
	ray focus_ray(float py = 0.5f, float px = 0.5f) const { return focus_ray(vec4(px, py)); }
	ray focus_ray(vec4 xy) const { return pinhole_ray(xy * vec4(w, h)); }
	__forceinline void set(uint y, uint x, vec4 rgb)
	{
		CCD.set(y, x, rgb);
	}
	__forceinline void add(uint y, uint x, vec4 rgb)
	{
		CCD.add(y, x, rgb);
	}
	__forceinline void add_raw(uint y, uint x, vec4 rgb)
	{
		CCD.add(y, x, rgb);
	}
	__forceinline vec4 get(uint y, uint x)
	{
		return CCD.get(y, x);
	}
	__forceinline vec4 get_med(uint y, uint x, float thr) const
	{
		return CCD.get_med(y, x, thr);
	}
	__forceinline void out(uint y, uint x)
	{
		CCD.out(y, x);
	}
	__forceinline void display(uint y, uint x, vec4 rgb)
	{
		CCD.out(y, x, (GAMMA2 ? exposure : 1) * exposure * rgb);
	}
	void update();
	void set_P(vec4 P);
	void set_A(vec4 P);

	void move(vec4 dir);
	void rotate(float alfa, float beta, float gamma);
	void rotate(vec4 angles);
	void resize(uint _w, uint _h, float scale);
	void reset_opt();
	void setup(mat4 _T, float _fov, float _fstop = 16);
	void set_fov(float _fov);
	inline vec4 SS(vec4 xy) const {
		xy = 2.f * (xy + 0.5f) * vec4(iw, ih) - 1.f;
		xy *= tfov * vec4(asp, -1);
		return vec4(xy[0], xy[1], -1);
	}
	inline vec4 inv_SS(vec4 xy) const {
		xy /= tfov * vec4(asp, -1);
		xy = (xy + 1.f) * 0.5f * vec4(w, h);
		return xy;
	}
	inline vec4 SS(vec4 xy, const projection& proj) const {
		xy = 2.f * (xy + 0.5f) * vec4(proj.iw, proj.ih) - 1.f;
		xy *= proj.tfov * vec4(proj.asp, -1);
		return vec4(xy[0], xy[1], -1);
	}
	inline vec4 inv_SS(vec4 xy, const projection& proj) const {
		xy /= proj.tfov * vec4(proj.asp, -1);
		xy = (xy + 1.f) * 0.5f * vec4(proj.w, proj.h);
		return xy;
	}
private:

	inline float m_to_fov(float m) {
		return 2.f * atanf(diagonal / (2.f * m));
	}
	inline float fov_to_m(float fov) {

		return diagonal / (2 * tanf(0.5f * torad(fov)));
	}

};

