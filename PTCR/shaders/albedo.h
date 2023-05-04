#pragma once
#include "texture.h"
#pragma pack(push,4)
class albedo {
public:
	albedo(const texture& _rgb = texture(vec4(0.5, 0, 0.5, 1)), const texture& _mer = texture(vec4(0, 0, 1)),
		const texture& _nor = texture(vec4(0.5, 0.5, 1)), float _rep = 1.f, float _ir = 1.f, vec4 _tint = 0) :
		_rgb(_rgb), _mer(_mer), _nor(_nor), tint(_tint), rep(_rep), ir(_ir) {}
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
	float trans = 0.f;
	float rep = 1.f;
	float ir = 1.f;
};
#pragma pack(pop)