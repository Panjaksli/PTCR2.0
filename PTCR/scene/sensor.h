#pragma once
#include "vec4.h"


class sensor {
public:
	sensor() {}
	sensor(uint w, uint h) : data(w* h), buff(w* h), w(w), h(h), n(w* h) {}
	vector<vec4> data;
	vector<vec4> buff;
	uint* disp = nullptr;
	uint w = 0, h = 0, n = 0, pitch = 0;
	double time = 0.f;
	double spp = 0.f;
private:
	float delta = 0;
	float inv_time = 0;
public:
	inline void clear(uint i) {
		data[i] = 0;
	}
	inline void clear(uint i, uint j) {
		uint off = i * w + j;
		data[off] = 0;
	}
	__forceinline void add(uint i, uint j, vec4 rgb) {
		uint off = i * w + j;
		rgb = fixnan(rgb); //Fix possible NaN values
		data[off] = delta * data[off] + rgb * inv_time; //rolling average
	}
	inline void set(uint i, uint j, const vec4& rgb) {
		uint off = i * w + j;
		data[off] = fixnan(rgb);
	}
	inline vec4 get(uint i, uint j)const {
		uint off = i * w + j;
		return data[off];
	}
	inline vec4 get_med(uint i, uint j, float thr) const {
		return median2d3(data.data(), i, j, h, w, thr);
	}
	__forceinline void out(uint i, uint j) {
		uint off = i * w + j;
		uint off2 = i * pitch + j;
		disp[off2]= vec2bgr(data[off]);
	}
	__forceinline void out(uint i, uint j, vec4 rgb) {
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
		time += 1.0;
		inv_time = 1.0 / time;
		delta = (time - 1.0) / time;
	}
	inline void reset() {
		delta = inv_time = spp = time = 0;
	}
	void clear();
	void resize(uint _w, uint _h);
};
