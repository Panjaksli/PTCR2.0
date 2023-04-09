#pragma once
#include "vec3.h"


class sensor {
public:
	sensor() {}
	sensor(uint _w, uint _h) : data(_w* _h), buff(_w* _h, vec3(vec3(), infp)), w(_w), h(_h), n(_w* _h) {}
	vector<vec3> data;
	vector<vec3> buff;
	uint* disp = nullptr;
	uint w = 0, h = 0, n = 0, pitch = 0;
	float time = 0.f;
	float spp = 0.f;
	inline void clear(uint i) {
		data[i] = vec3(vec3(), infp);
	}
	inline void clear(uint i, uint j) {
		uint off = i * w + j;
		data[off] = vec3(vec3(), infp);
	}
	__forceinline void add(uint i, uint j, vec3 rgb) {
		uint off = i * w + j;
		rgb = fixnan(rgb);
		//rolling sum 
		data[off] = vec3(((time - 1.f) * data[off] + rgb) * inv_time, fminf(data[off][3], rgb[3]));
	}
	inline void set(uint i, uint j, const vec3& rgb) {
		uint off = i * w + j;
		data[off] = fixnan(rgb);
	}
	inline vec3 get(uint i, uint j)const {
		uint off = i * w + j;
		return data[off];
	}
	inline vec3 get_med(uint i, uint j, float thr) const {
		return median2d3(data.data(), i, j, h, w, thr);
	}
	__forceinline void out(uint i, uint j) {
		uint off = i * w + j;
		uint off2 = i * pitch + j;
		disp[off2]= vec2bgr(data[off]);
	}
	__forceinline void out(uint i, uint j, vec3 rgb) {
		uint off = i * pitch + j;
		disp[off] = vec2bgr(rgb);
	}
	__forceinline void out_med(uint i, uint j, float thr) {
		uint off = i * pitch + j;
		disp[off] = vec2bgr(median2d3(data.data(), i, j, h, w, thr));
	}
	inline void set_disp(uint* _disp, uint _pitch) {
		disp = _disp;
		pitch = _pitch;
	}
	inline void dt(float _t = 1.f) {
		spp += _t;
		time += 1.f;
		inv_time = 1.f / time;
	}
	inline void reset() {
		spp = time = 0;
	}
	void clear();
	void resize(uint _w, uint _h);
	void outrgb();
private:
	vec3 inv_time = 0;
};
