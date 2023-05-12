#include "obj_list.h"
void obj_list::clear()
{
	bvh.clear();
	obj_bvh.clear();
	objects.clear();
	materials.clear();
	lights.clear();
	nonbvh.clear();
	bbox = aabb();
	en_bvh = 0;
}
obj_flags obj_list::get_flag(uint id) const
{
	return objects[id].flag;
}
uint obj_list::get_id(const ray& r, hitrec& rec) const
{
	hit(r, rec);
	return rec.idx;
}
void obj_list::get_trans(uint id, mat4& T)const
{
	T = objects[id].get_trans();
}
void obj_list::set_trans(uint id, const mat4& T, uint node_size) {
	objects[id].transform(T);
	if (objects[id].bvh())
		update_bvh(0, node_size);
	fit();
}
void obj_list::fit() {
	bbox = aabb();
	for (const auto& obj : objects)
		bbox.join(obj.get_box());
	bbox.pmin -= eps;
	bbox.pmax += eps;
}
void obj_list::obj_create() {
	obj_bvh.clear();
	if (objects.empty()) return;
	uint size = 0;
	for (const auto& obj : objects)
		size += obj.get_size();
	obj_bvh.reserve(size);
	for (uint i = 0; i < objects.size(); i++) {
		if (objects[i].bvh()) {
			for (uint j = 0; j < objects[i].get_size(); j++)
				obj_bvh.emplace_back(objects[i].get_box(j), i, j);
		}
	}
}
void obj_list::update_lights() {
	lights.clear();
	lw_tot = 0;
	for (uint i = 0; i < objects.size(); i++)
	{
		if (objects[i].light()) {
			lw_tot++;
			lights.emplace_back(i);
		}
	}
	lw_tot = lw_tot > 0 ? 1.f / lw_tot : 0;
}
void obj_list::update_nonbvh() {
	nonbvh.clear();
	for (uint i = 0; i < objects.size(); i++) {
		if (!objects[i].bvh())nonbvh.emplace_back(i);
	}
}
void obj_list::update_lists() {
	update_lights();
	update_nonbvh();
	en_bvh = bvh.size() && obj_bvh.size() && objects.size();
}
void obj_list::update_all(uint node_size) {
	update_lists();
	build_bvh(1, node_size);
}
void obj_list::obj_update() {

}
void obj_list::rebuild_bvh(bool print, uint node_size) {
	bvh_builder(print, node_size);
	en_bvh = bvh.size() && obj_bvh.size() && objects.size();
}
void obj_list::build_bvh(bool print, uint node_size) {
	fit();
	obj_create();
	bvh_builder(print, node_size);
	en_bvh = bvh.size() && obj_bvh.size() && objects.size();
}
void obj_list::bvh_builder(bool print, uint node_size) {
	final_cost = 0;
	bvh.clear();
	if (obj_bvh.empty()) return;
	double t1 = timer();
	bvh.reserve(obj_bvh.size());
	if (bvh_lin) {
		bvh.emplace_back(box_from(0, obj_bvh.size()), 0, obj_bvh.size(), 0);
		split_bvh(0, node_size);
	}
	else split_bvh(0, obj_bvh.size(), node_size);
	if (print)
		printf("Built BVH, nodes: %d, took: %g, cost: %g\n", (int)bvh.size(), timer(t1), final_cost);
}

void obj_list::update_bvh(bool print, uint node_size) {
	double t1 = timer();
	float cost = 0;
	for (int i = bvh.size() - 1; i >= 0; i--) {
		auto& node = bvh[i];
		if (!node.parent) {
			node.bbox = box_from(node.n1, node.n2);
			cost += node.bbox.area() * (node.n2 - node.n1);
		}
		else {
			node.bbox = bvh[node.n1].bbox + bvh[node.n2].bbox;
		}
	}
	if (cost > 1.1 * final_cost) {
		rebuild_bvh(print, node_size);
	}
	if (print)
		printf("Updated BVH, nodes: %d, took: %g\n", (int)bvh.size(), timer(t1));
}

aabb obj_list::box_from(uint beg, uint end) {
	aabb box;
	for (uint i = beg; i < end; i++) {
		box.join(obj_bvh[i].get_box(objects.data()));
	}
	return box;
}

uint obj_list::sort(uint be, uint en, uchar axis, float plane) {
	int i = be;
	int j = en - 1;
	while (i <= j)
	{
		if (obj_bvh[i].get_box(objects.data()).pmid().xyz[axis] < plane)
			i++;
		else
			std::swap(obj_bvh[i], obj_bvh[j--]);
	}
	return i;
}

void obj_list::split_bvh(uint node, uint node_size) {
	aabb bbox = bvh[node].bbox;
	uint be = bvh[node].n1, en = bvh[node].n2;
	uint size = en - be, mi = (be + en) / 2;
	float pcost = bbox.area() * size;
	if (size > node_size) {
		float cost = split_cost(be, en, mi, bbox);
		if (mi > be && mi < en && cost < pcost) {
			bvh[node].parent = true;
			bvh[node].n1 = bvh.size();
			bvh[node].n2 = bvh[node].n1 + 1;
			bvh.emplace_back(box_from(be, mi), be, mi, 0);
			bvh.emplace_back(box_from(mi, en), mi, en, 0);
			split_bvh(bvh[node].n1, node_size);
			split_bvh(bvh[node].n2, node_size);
		}
		else final_cost += pcost;
	}
	else final_cost += pcost;
}

void obj_list::split_bvh(uint be, uint en, uint node_size) {
	aabb bbox = box_from(be, en);
	uint size = en - be;
	float pcost = bbox.area() * size;
	if (size > node_size)
	{
		uint mi = be + size / 2;
		float cost = split_cost(be, en, mi, bbox);
		if (mi != be && mi != en && cost < pcost)
		{
			uint n = bvh.size();
			bvh.emplace_back(bbox, n + 1, 0, 1);
			split_bvh(be, mi, node_size);
			bvh[n].n2 = bvh.size();
			split_bvh(mi, en, node_size);
		}
		else bvh.emplace_back(bbox, be, en, 0), final_cost += pcost;
	}
	else bvh.emplace_back(bbox, be, en, 0), final_cost += pcost;
}

/*
Based on Binned BVH build:
https://jacco.ompf2.com/2022/04/21/how-to-build-a-bvh-part-3-quick-builds/
*/
float obj_list::split_cost(uint be, uint en, uint& split, aabb bbox) {
	uchar axis = bbox.get_longest_axis();
	float fplane = bbox.pmid()[axis];
	float fcost = 1e30f;
	constexpr uint no_bins = 32;
	struct bins {
		aabb box;
		uint cnt = 0;
	};
	struct layer {
		float area = 0;
		uint cnt = 0;
	};
	for (uchar a = 0; a < 3; a++) {
		bins bin[no_bins];
		float scale = no_bins / (bbox.pmax[a] - bbox.pmin[a]);
		for (uint i = be; i < en; i++)
		{
			aabb box = obj_bvh[i].get_box(objects.data());
			int id = clamp_int((box.pmid()[a] - bbox.pmin[a]) * scale, 0, no_bins - 1);
			bin[id].cnt++;
			bin[id].box.join(box);
		}
		layer L[no_bins - 1], R[no_bins - 1];
		bins lbin, rbin;
		for (int i = 0; i < no_bins - 1; i++) {
			lbin.box.join(bin[i].box);
			rbin.box.join(bin[no_bins - 1 - i].box);
			L[i].area = lbin.box.area();
			R[no_bins - 2 - i].area = rbin.box.area();
			L[i].cnt = lbin.cnt += bin[i].cnt;
			R[no_bins - 2 - i].cnt = rbin.cnt += bin[no_bins - 1 - i].cnt;
		}
		float step = (bbox.pmax[a] - bbox.pmin[a]) / no_bins;
		float plane = bbox.pmin[a];
		for (uint i = 0; i < no_bins - 1; i++) {
			plane += step;
			float cost = L[i].area * L[i].cnt + R[i].area * R[i].cnt;
			if (cost < fcost) {
				axis = a;
				fplane = plane;
				fcost = cost;
			}
		}
	}
	split = sort(be, en, axis, fplane);
	return fcost;
}


