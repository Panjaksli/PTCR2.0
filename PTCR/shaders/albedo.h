#pragma once
#include "texture.h"
#pragma pack(push,4)
class albedo {
public:
	albedo(const texture& rgb = texture(vec4(0.5, 0.5, 0.5, 1)), const texture& mer = texture(vec4(0, 0, 1)),
		const texture& nor = texture(vec4(0.5, 0.5, 1)), float rep = 1.f, float ir = 1.f, vec4 tint = 0, bool alpha=0) :
		_rgb(rgb), _mer(mer), _nor(nor), tint(tint), rep(rep), ir(ir), alpha(alpha) {}
	__forceinline vec4 rgb(float u = 0, float v = 0)const {
		vec4 rgb = _rgb.sample(rep * u, rep * v);
		return GAMMA2 ? rgb * vec4(rgb, 1) : rgb;
	}
	__forceinline vec4 tinted(float u = 0, float v = 0)const {
		return GAMMA2 ? tint * vec4(tint, 1) : tint;
	}
	__forceinline vec4 linear(float u = 0, float v = 0)const {
		return _rgb.sample(rep * u, rep * v);
	}
	__forceinline vec4 mer(float u = 0, float v = 0)const {
		vec4 col = _mer.sample(rep * u, rep * v);
		return GAMMA2 ? col * vec4(1,col[1],1,1) : col;
	}
	__forceinline vec4 nor(float u = 0, float v = 0)const {
		return _nor.sample(rep * u, rep * v);
	}
	void clear() {
		_rgb.clear();
		_mer.clear();
		_nor.clear();
	}

	
public:
	texture _rgb, _mer, _nor;
	vec4 tint = 0;
	float rep = 1.f;
	float ir = 1.f;
	bool alpha = 0;
};
#pragma pack(pop)