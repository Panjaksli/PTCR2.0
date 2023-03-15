#pragma once
#include <cstdlib>
#include <iostream>
#include "vec3.h"

#pragma pack(push,2)
class texture {
public:

	texture() : rgb(0.5, 0.5, 1, 1), solid(1) {}
	texture(vec3 _rgb) :rgb(_rgb), solid(1) {}
	texture(const char* _filename) : solid(0) {
		if (_filename == nullptr)
		{
			solid = 1;
			rgb = 0;
		}
		else {
		std::string filename = std::string(_filename);
		bool found = load(filename) || load(filename + ".jpg") || load(filename + ".png") || load(filename + ".gif") || load(filename + ".tga")
			|| load("textures/" + filename) || load("textures/" + filename + ".jpg") || load("textures/" + filename + ".png") ||
			load("textures/" + filename + ".gif") || load("textures/" + filename + ".tga");
		if (!found) {
			solid = 1;
			rgb = vec3(0.5, 0.5, 1, 1);
			std::cerr << "Texture not found!'" << filename << "'.\n";
		}
		}
	}
	texture(const texture& cpy){
		//Did leak memory before fix :)
		clear();
		solid = cpy.solid;
		if (solid) {
			rgb = cpy.rgb;
		}
		else {
			w = cpy.w, h = cpy.h;
			data = (uchar*)malloc(w * h * 4);
			memcpy(data, cpy.data, w * h * 4);
		}
	}
	const texture & operator=(const texture& cpy){
		clear();
		solid = cpy.solid;
		if (solid) {
			rgb = cpy.rgb;
		}
		else {
			w = cpy.w, h = cpy.h;
			//malloc because of compatibility with stbi_load
			data = (uchar*)malloc(w * h * 4);
			memcpy(data, cpy.data, w * h * 4);
		}
		return *this;
	}
	~texture() {
		clear();
	}
	__forceinline vec3 sample(float u, float v) const {
		if (solid)return rgb;
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
	void set_col(vec3 _rgb) {
		if (solid)rgb = _rgb;
	}
	vec3 get_col()
	{
		return solid ? rgb : 0;
	}
	void clear() { 
		if (!solid) {
			w = h = 0;
			free(data);
		}
		else rgb = 0;
		solid = true;
	}
private:
	union {
		struct {
			uint w, h;
			uchar* data;
		};
		vec3 rgb;
	};
	bool solid = 1;
	bool load(const std::string filename);
	
};
#pragma pack(pop)
