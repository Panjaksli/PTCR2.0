#pragma once
#include "sensor.h"
#include "mat4.h"
#include "ray.h"
#include "obj.h"
#include "physics.h"

class camera
{
public:
	camera() {}
	camera(uint _w, uint _h, float fov, mat4 _T = mat4()) :T(_T), CCD(_w, _h), P(T.P()), fov(fov), tfov(tan(0.5f * torad(fov))), iw(1.0 / _w), ih(1.0 / _h), asp((float)_w / _h), speed(1) {}
	mat4 T = mat4();
	sensor CCD = sensor(1280, 720);
	vec3 P = 0;
	vec3 V = 0;
	uint& w = CCD.w, & h = CCD.h;
	float fov = 90, tfov = tan(0.5f * torad(fov));
	float hfov = 0;
	float iw = 1.f / w, ih = 1.f / h, asp = float(w) / h;
	float speed = 1.f;
	float exposure = 1.f;
	float fstop = 16.f;
	float foc_l = 0.0216f;
	float foc_t = 1.f;
	float lens_cor = 0.0;
	const float frame = 0.035;
	const float diagonal = 0.0432;
	bool collision = 1;
	bool bokeh = 1;
	bool free = 0;
	bool autofocus = 1;
	bool moving = 1;

	__forceinline ray optical_ray(vec3 xy) const
	{
		if (!bokeh)return pinhole_ray(xy);
		xy = SS(xy);// +vec3(0, 0, lens_cor * (xy[0] / asp * xy[0] / asp + xy[1] * xy[1]));
		vec3 D = foc_t * xy;
		vec3 r = T.vec(foc_l * sa_disk() / fstop);
		return ray(P - r, T.vec(D + r), true);
	}
	__forceinline ray pinhole_ray(vec3 xy) const { return ray(P, T.vec(SS(xy)), true); }
	__forceinline ray optical_ray(float py, float px) const { return optical_ray(vec3(px, py)); }
	__forceinline ray pinhole_ray(float py, float px) const { return pinhole_ray(vec3(px, py)); }
	ray focus_ray(float py = 0.5f, float px = 0.5f) const { return focus_ray(vec3(px, py)); }
	ray focus_ray(vec3 xy) const { return pinhole_ray(xy * vec3(w, h)); }



	__forceinline void set(uint y, uint x, vec3 rgb)
	{
		CCD.set(y, x, rgb);
	}

	__forceinline void add(uint y, uint x, vec3 rgb)
	{
		CCD.add(y, x, vec3(rgb, 1.f / exposure));
	}

	__forceinline void add_raw(uint y, uint x, vec3 rgb)
	{
		CCD.add(y, x, vec3(rgb, 1));
	}
	__forceinline vec3 get(uint y, uint x)
	{
		return CCD.get(y, x);
	}
	__forceinline vec3 get_med(uint y, uint x, float thr)
	{
		return CCD.get_med(y, x, thr);
	}
	__forceinline void out(uint y, uint x)
	{
		CCD.out(y, x);
	}
	__forceinline void out(uint y, uint x, vec3 rgb)
	{
		CCD.out(y, x, rgb);
	}
	__forceinline void out_med(uint y, uint x, float thr)
	{
		CCD.out(y, x, thr);
	}
	void update();
	void set_P(vec3 P);

	void move(vec3 dir);
	void rotate(float alfa, float beta, float gamma);
	void resize(uint _w, uint _h, float scale);
	void reset_opt();
	void setup(mat4 _T, float _fov, float _fstop = 16);
	void set_fov(float _fov);

private:
	inline vec3 SS(vec3 xy) const {
		xy = 2.f * (xy + 0.5f) * vec3(iw, ih) - 1.f;
		xy *= tfov * vec3(asp, -1);
		return vec3(xy[0], xy[1], -1);
	}
	inline float m_to_fov(float m) {
		return 2.f * atanf(diagonal / (2.f * m));
	}
	inline float fov_to_m(float fov) {

		return diagonal / (2 * tanf(0.5f * torad(fov)));
	}

};

