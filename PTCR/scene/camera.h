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
	camera(uint _w, uint _h, float fov, mat4 _T = mat4()) :T(_T), CCD(_w, _h), fov(fov), tfov(tan(0.5f * torad(fov))), iw(1.0 / _w), ih(1.0 / _h), asp((float)_w / _h), speed(0.02) {}
	mat4 T = mat4();
	sensor CCD = sensor(1280, 720);
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

	__forceinline ray optical_ray(float py, float px) const
	{
		SS(py, px);
		vec3 D = foc_t * vec3(px, py, -1 + lens_cor * (px/asp * px/asp + py * py));
		//Defocus blur
		vec3 r = bokeh ? T * (foc_l * sa_disk() / fstop) : 0;
		return ray(T.P() - r, T * D + r, true);
	}

	__forceinline ray pinhole_ray(float py, float px) const
	{
		SS(py, px);
		return ray(T.P(), T * vec3(px, py, -1), true);
	}

	ray focus_ray(float py = 0.5f, float px = 0.5f) const
	{
		py *= h; px *= w;
		return pinhole_ray(py,px);
	}

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
		CCD.add(y, x, rgb);
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
	inline void SS(float& py, float& px) const {
		float SSX = 2.f * (px + 0.5f) * iw - 1.f;
		float SSY = 1.f - 2.f * (py + 0.5f) * ih;
		px = SSX * tfov * asp;
		py = SSY * tfov;
	}
	inline float m_to_fov(float m) {
		return 2.f * atanf(diagonal / (2.f * m));
	}
	inline float fov_to_m(float fov) {

		return diagonal / (2 * tanf(0.5f * torad(fov)));
	}

};

