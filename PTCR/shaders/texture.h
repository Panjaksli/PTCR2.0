#pragma once
#include "vec4.h"

class texture {
public:

	texture() {}
	texture(vec4 _rgb, const char* filename) {
		*this = texture(filename);
		rgb = _rgb;
	}
	texture(vec4 _rgb) : rgb(_rgb) { flags.set(0, 1); }
	texture(const char* _filename) {
		clear();
		if (_filename != nullptr && strcmp(_filename, "0") != 0) {
			std::string filename = std::string(_filename);
			bool found = load(filename) || load(filename + ".jpg") || load(filename + ".png") || load(filename + ".gif") || load(filename + ".tga")
				|| load("textures/" + filename) || load("textures/" + filename + ".jpg") || load("textures/" + filename + ".png") ||
				load("textures/" + filename + ".gif") || load("textures/" + filename + ".tga");
			if (found) {
				name = _filename;
				flags.set(0, 0);
				return;
			}
		}
		flags.set(0, 1);
	}
	~texture() {
		clear();
	}

	texture(const texture& cpy) {
		flags = cpy.flags;
		name = cpy.name;
		rgb = cpy.rgb;
		if (cpy.data != nullptr)
		{
			w = cpy.w, h = cpy.h;
			data = (uchar*)malloc(w * h * 4);
			memcpy(data, cpy.data, w * h * 4);
		}
	}
	texture(texture&& cpy) noexcept : texture() {
		swap(*this, cpy);
	}
	texture& operator=(texture cpy) {
		swap(*this, cpy);
		return *this;
	}

	__forceinline vec4 sample(float u, float v) const {
		if (get_solid() || data == nullptr)return rgb;
		uint x = u * (w - 1);
		uint y = v * (h - 1);
		x %= w;
		y %= h;
		uint off = (x + y * w) * 4;
		uchar r = data[off];
		uchar g = data[off + 1];
		uchar b = data[off + 2];
		uchar a = data[off + 3];
		return vec4(r, g, b, a) * (1 / 255.f);
	}
	void set_solid(bool x) { flags.set(0, x); }
	void set_checker(bool x) { flags.set(1, x); }
	bool get_solid()const { return flags[0]; }
	bool get_checker()const { return flags[1]; }
	void set_tex(const char* _filename) { *this = texture(rgb, _filename); }
	void clear() { if (data != nullptr)free(data); name.clear(); data = nullptr; }
	friend void swap(texture& t1, texture& t2){
		std::swap(t1.rgb, t2.rgb);
		std::swap(t1.name, t2.name);
		std::swap(t1.data, t2.data);
		std::swap(t1.w, t2.w);
		std::swap(t1.h, t2.h);
		std::swap(t1.flags, t2.flags);
	}
	vec4 rgb = vec4(0,0,0,1);
	c_str name;
private:
	uchar* data = nullptr;
	uint w = 0, h = 0;
	bitfield<uint> flags;
	bool load(const std::string filename);
};
