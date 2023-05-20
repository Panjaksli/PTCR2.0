#include <omp.h>
#include "sensor.h"
void sensor::clear() {
	//#pragma omp parallel for schedule(static,100)
	for (auto& buff : data)
		buff = 0;
	for (auto& el : buff)
		el = 0;
	spp = 0.f;
	time = 0;
}
void sensor::resize(uint _w, uint _h) {
	w = _w;
	h = _h;
	n = w * h;
	data.resize(n, 0);
	buff.resize(n, 0);
}
