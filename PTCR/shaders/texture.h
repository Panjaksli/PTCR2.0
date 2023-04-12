#pragma once
#include "vec3.h"

class texture {
public:

	texture() : rgb(0.5, 0.5, 1, 1) {}
	texture(vec3 _rgb, const char* filename) {
		*this = texture(filename);
		this->set_col(_rgb);
	}
	texture(vec3 _rgb) : rgb(_rgb) { flags.set(0, 1); }
	texture(const char* _filename) {
		clear();
		if (_filename != nullptr) {
			std::string filename = std::string(_filename);
			bool found = load(filename) || load(filename + ".jpg") || load(filename + ".png") || load(filename + ".gif") || load(filename + ".tga")
				|| load("textures/" + filename) || load("textures/" + filename + ".jpg") || load("textures/" + filename + ".png") ||
				load("textures/" + filename + ".gif") || load("textures/" + filename + ".tga");
			if (!found) {
				flags.set(0, 1);
				rgb = vec3(0.5, 0.5, 1, 1);
				std::cerr << "Texture not found!'" << filename << "'.\n";
			}
			flags.set(0, 0);
		}
		else flags.set(0, 0);
	}
	~texture() {
		clear();
	}

	texture(const texture& cpy) {
		//Did leak memory before fix :)
		clear();
		flags = cpy.flags;
		rgb = cpy.rgb;
		if(cpy.data != nullptr)
		{
			w = cpy.w, h = cpy.h;
			data = (uchar*)malloc(w * h * 4);
			memcpy(data, cpy.data, w * h * 4);
		}
	}
	const texture& operator=(const texture& cpy) {
		clear();
		flags = cpy.flags;
		rgb = cpy.rgb;
		if (cpy.data != nullptr) {
			w = cpy.w, h = cpy.h;
			data = (uchar*)malloc(w * h * 4);
			memcpy(data, cpy.data, w * h * 4);
		}
		return *this;
	}

	__forceinline vec3 sample(float u, float v) const {
		if(get_solid()||data==nullptr)return rgb;
		uint x = u * (w - 1);
		uint y = v * (h - 1);
		x %= w;
		y %= h;
		uint off = (x + y * w) * 4;
		uchar r = data[off];
		uchar g = data[off + 1];
		uchar b = data[off + 2];
		uchar a = data[off + 3];
		return vec3(r, g, b, a) * (1 / 255.f);
	}
	void set_solid(bool x) {flags.set(0, x);}
	void set_checker(bool x) {flags.set(1, x);}
	bool get_solid()const { return flags[0]; }
	bool get_checker()const { return flags[1]; }
	void set_tex(const char* _filename) { *this = texture(rgb,_filename); }
	void set_col(vec3 _rgb) {rgb = _rgb;}
	vec3 get_col(){return rgb;}
	void clear() { w = 0; h = 0; free(data); data = nullptr; }
	
	
private:
	vec3 rgb;
	uchar* data = nullptr;
	uint w = 0, h = 0;
	bitfield<uint> flags;
	bool load(const std::string filename);
};
