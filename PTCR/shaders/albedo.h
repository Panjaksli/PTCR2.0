#pragma once
#include "texture.h"
#pragma pack(push,4)
class albedo {
public:
	albedo(const texture& _rgb = texture(vec3(0.5, 0, 0.5, 1)), const texture& _mer = texture(vec3(0, 0, 1)),
		const texture& _nor = texture(vec3(0.5, 0.5, 1)), float _rep = 1.f, float _ir = 1.f, vec3 _spec = 0) :
		_rgb(_rgb), _mer(_mer), _nor(_nor), spec(_spec), rep(_rep), ir(_ir) {}
	__forceinline vec3 rgb(float u = 0, float v = 0)const {
		vec3 rgb = _rgb.sample(rep * u, rep * v);
		return GAMMA2 ? rgb * vec3(rgb, 1) : rgb;
	}
	__forceinline vec3 specular(float u = 0, float v = 0)const {
		return GAMMA2 ? spec * vec3(spec, 1) : spec;
	}
	__forceinline vec3 linear(float u = 0, float v = 0)const {
		return _rgb.sample(rep * u, rep * v);
	}
	__forceinline vec3 mer(float u = 0, float v = 0)const {
		vec3 col = _mer.sample(rep * u, rep * v);
		return GAMMA2 ? col * vec3(1,col[1],1,1) : col;
	}
	__forceinline vec3 nor(float u = 0, float v = 0)const {
		return _nor.sample(rep * u, rep * v);
	}
	void clear() {
		_rgb.clear();
		_mer.clear();
		_nor.clear();
	}

	
public:
	texture _rgb, _mer, _nor;
	vec3 spec = 0;
	float trans = 0.f;
	float rep = 1.f;
	float ir = 1.f;
};
#pragma pack(pop)