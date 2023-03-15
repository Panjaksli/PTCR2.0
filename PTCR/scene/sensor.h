#pragma once
#include "vec3.h"


class sensor {
public:
	sensor() {}
	sensor(uint _w, uint _h) : data(_w* _h), w(_w), h(_h), n(_w* _h) {}
	vector<vec3> data;
	uint* disp = nullptr;
	uint w = 0, h = 0, n = 0, pitch = 0;
	float spp = 0.f;
	inline void clear(uint i) {
		data[i] = vec3();
	}
	inline void clear(uint i, uint j) {
		uint off = i * w + j;
		data[off] = vec3();
	}

	__forceinline void add(uint i, uint j, const vec3& rgb) {
		uint off = i * w + j;
		data[off] += fixnan(rgb);
	}
	inline void set(uint i, uint j, const vec3& rgb) {
		uint off = i * w + j;
		data[off] = fixnan(rgb);
	}
	inline vec3 get(uint i, uint j) {
		uint off = i * w + j;
		return data[off];
	}
	inline vec3 get_med(uint i, uint j, float thr) {
		return median2d3(data.data(), i, j, h, w, thr);
	}
	__forceinline void out(uint i, uint j) {
		uint off = i * w + j;
		uint off2 = i * pitch + j;
		bgr(data[off], disp[off2]);
	}
	__forceinline void out(uint i, uint j, vec3 rgb) {
		uint off = i * pitch + j;
		bgr(rgb, disp[off]);
	}
	__forceinline void out_med(uint i, uint j, float thr) {
		uint off = i * pitch + j;
		bgr(median2d3(data.data(),i,j,h,w, thr), disp[off]);
	}

	inline void set_disp(uint* _disp, uint _pitch) {
		disp = _disp;
		pitch = _pitch;
	}
	inline void dt(float _t = 1.f) {
		spp += _t;
	}
	void clear();
	void resize(uint _w, uint _h);
	void outrgb();
};
