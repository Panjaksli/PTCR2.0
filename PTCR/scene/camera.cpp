#include "camera.h"
void camera::update()
{
	asp = (float)w / h;
	fov = fminf(179, fmaxf(1, fov));
	tfov = tan(0.5f * torad(fov));
	hfov = todeg(2 * atan(tfov * asp));
	foc_l = fov_to_m(fov);
}
void camera::move(vec4 dir) {
	if (near0(dir))return;
	P += dir;
	T.add_P(dir);
	moving = 1;
}
void camera::set_P(vec4 pos)
{
	P = pos;
	T.set_P(pos);
	moving = 1;
}
void camera::set_A(vec4 rot)
{
	T.set_A(rot);
	moving = 1;
}
void camera::rotate(vec4 angles)
{
	if (near0(angles))return;
	vec4 A = T.A() + fov * (1.f / 90.f) * angles;
	A._xyz[2] = (A._xyz[2] > hpi - 0.01f && A._xyz[2] < 3 * hpi + 0.01f) ? T.A()._xyz[2] : A._xyz[2];
	T.set_A(A);
	moving = 1;
}
void camera::rotate(float alfa, float beta, float gamma)
{
	vec4 angles(alfa, beta, gamma);
	if (near0(angles))return;
	vec4 A = T.A() + fov * (1.f / 90.f) * angles;
	A._xyz[2] = (A._xyz[2] > hpi - 0.01f && A._xyz[2] < 3 * hpi + 0.01f) ? T.A()._xyz[2] : A._xyz[2];
	T.set_A(A);
	moving = 1;
}
void camera::resize(uint _w, uint _h, float scale)
{
	w = _w * scale;
	h = _h * scale;
	iw = 1.0 / w;
	ih = 1.0 / h;
	CCD.resize(w, h);
	update();
	moving = 1;
}
void camera::reset_opt() {
	fov = 90, tfov = tan(0.5f * torad(fov));
	iw = 1.f / w, ih = 1.f / h, asp = float(w) / h;
	speed = 1.f;
	exposure = 1.f;
	fstop = 16.f;
	foc_l = 0.0216f;
	foc_t = 1.f;
	autofocus = 1;
	moving = 1;
	update();
}
void camera::setup(mat4 _T, float _fov, float _fstop) {
	T = _T;
	P = T.P();
	fov = _fov;
	fstop = _fstop;
	update();
}
void camera::set_fov(float _fov) {
	fov = _fov;
	update();
}
