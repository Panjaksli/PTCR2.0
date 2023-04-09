#include <omp.h>
#include "sensor.h"
void sensor::clear() {
//#pragma omp parallel for schedule(static,100)
	for (auto& buff : data)
		buff = vec3();
	for (auto& el : buff)
		el = vec3(vec3(), infp);
	spp = 0.f;
	time = 0;
}
void sensor::resize(uint _w, uint _h) {
	w = _w;
	h = _h;
	n = w * h;
	data.resize(n, vec3());
	buff.resize(n, vec3(vec3(), infp));
}

void sensor::outrgb() {

}