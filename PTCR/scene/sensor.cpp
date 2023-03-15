#include <omp.h>
#include "sensor.h"
void sensor::clear() {
#pragma omp parallel for schedule(static,100)
	for (auto& buff : data)
		buff = vec3();
	spp = 0.f;
}
void sensor::resize(uint _w, uint _h) {
	w = _w;
	h = _h;
	n = w * h;
	data.resize(n);
}

void sensor::outrgb() {

}