#pragma once
#include "texture.h"
#pragma pack(push,4)
class albedo {
public:
	albedo(const texture& _rgb = texture(vec3(0.5, 0, 0.5, 1)), const texture& _mer = texture(vec3(0, 0, 1)),
		const texture& _nor = texture(vec3(0.5, 0.5, 1)), float _rep = 1.f, float _ir = 1.f) :
		_rgb(_rgb), _mer(_mer), _nor(_nor), rep(_rep), _ir(_ir) {}
	__forceinline vec3 rgb(float u = 0, float v = 0)const {
		vec3 rgb = _rgb.sample(rep * u, rep * v);
#if GAMMA2
		return rgb * vec3(rgb, 1);
#else 
		return rgb;
#endif
	}
	__forceinline vec3 mer(float u = 0, float v = 0)const {
		return _mer.sample(rep * u, rep * v);
	}
	__forceinline vec3 nor(float u = 0, float v = 0)const {
		return _nor.sample(rep * u, rep * v);
	}
	__forceinline float ir()const {
		return _ir;
	}
	vec3 get_rgb()
	{
		return _rgb.get_col();
	}
	vec3 get_mer()
	{
		return _mer.get_col();
	}
	vec3 get_nor()
	{
		return _nor.get_col();
	}
	void set_all(vec3 rgb, vec3 mer, vec3 nor) {
		_rgb.set_col(rgb);
		_mer.set_col(mer);
		_nor.set_col(nor);
	}
	void set_rgb(vec3 col)
	{
		_rgb.set_col(col);
	}
	void set_mer(vec3 col)
	{
		_mer.set_col(col);
	}
	void set_nor(vec3 col)
	{
		_nor.set_col(col);
	}
	float& get_rep() {
		return rep;
	}
	float& get_ir() {
		return _ir;
	}
	void clear() {
		_rgb.clear();
		_mer.clear();
		_nor.clear();
	}
private:
	texture _rgb, _mer, _nor;
	float rep;
	float _ir = 1.f;
};
#pragma pack(pop)