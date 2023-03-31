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
}
obj_flags obj_list::get_flag(int id) const
{
	return objects[id].flag;
}
int obj_list::get_id(const ray& r, hitrec& rec) const
{
	if (hit(r, rec))return rec.idx;
	else return -1;
}
void obj_list::get_trans(const int id, mat4& T)const
{
	T = objects[id].get_trans();
}
void obj_list::set_trans(int id, const mat4& T, uint node_size) {
	objects[id].set_trans(T);
	if (objects[id].bvh())
		update_bvh(0, node_size);
	fit();
}
void obj_list::fit() {
	bbox = aabb();
	for (const auto& obj : objects)
		bbox.join(obj.get_box());
}
void obj_list::obj_create() {
	if (objects.size() == 0) {
		en_bvh = false;
		return;
	}
	obj_bvh.clear();
	size_t size = 0;
	for (uint i = 0; i < objects.size(); i++) {
		if (objects[i].bvh())
			size += objects[i].get_size();
	}
	obj_bvh.reserve(size);
	for (uint i = 0; i < objects.size(); i++) {
		if (objects[i].bvh()) {
			for (uint j = 0; j < objects[i].get_size(); j++)
				obj_bvh.emplace_back(mesh_raw(objects[i].get_box(j), i, j));
		}
	}
}
void obj_list::update_lights() {
	lights.clear();
	float none = 0;
	float total = 0;
	float bvh = 0;
	for (uint i = 0; i < objects.size(); i++)
	{
		if (objects[i].light()) {
			if (objects[i].bvh()) bvh += objects[i].get_size();
			else none += 1.f;
			total += 1;
			lights.emplace_back(i);
		}
	}
	lw_tot = total > 0 ? 1.f / total : 0;
	lw_non = none > 0 ? 1.f / none : 0;
	lw_bvh = bvh > 0 ? 1.f / bvh : 0;
}
void obj_list::update_nonbvh() {
	nonbvh.clear();
	for (uint i = 0; i < objects.size(); i++)
	{
		if (!objects[i].bvh())nonbvh.emplace_back(i);
	}
}
void obj_list::update_lists() {
	update_lights();
	update_nonbvh();
}
void obj_list::update_all(uint node_size) {
	update_lists();
	build_bvh(1,node_size);
}
void obj_list::obj_update() {
	//#pragma omp parallel for schedule(static,64)
		//for (auto& obj : obj_bvh)
		//	obj.update_box(objects[obj.obje_id].get_box(obj.prim_id));
}
void obj_list::rebuild_bvh(bool print, uint node_size) {
	//obj_update();
	bvh_builder(print, node_size);
}
void obj_list::build_bvh(bool print, uint node_size) {
	fit();
	obj_create();
	bvh_builder(print, node_size);
}
void obj_list::bvh_builder(bool print, uint node_size) {
	bvh.clear();
	if (obj_bvh.size() == 0 || objects.size() == 0) {
		en_bvh = false;
		return;
	}
	time_t t1 = clock();
	bvh.reserve(obj_bvh.size());
	if (bvh_lin) {
		bvh.emplace_back(bvh_node(box_from(0, obj_bvh.size()), 0, obj_bvh.size(), 0));
		split_bvh(0, node_size);
	}
	else split_bvh(0, obj_bvh.size(), node_size);
	bvh.shrink_to_fit();
	float t2 = clock() - t1;
	if (print)
		printf("%d %f\n", (int)bvh.size(), t2 / CLOCKS_PER_SEC);
}

void obj_list::update_bvh(bool print, uint node_size) {
	time_t t1 = clock();
	obj_update();
	for (int i = bvh.size() - 1; i >= 0; i--) {
		auto& node = bvh[i];
		if (!node.parent) {
			node.bbox = box_from(node.n1, node.n2);
		}
		else {
			node.bbox = bvh[node.n1].bbox + bvh[node.n2].bbox;
		}
	}
	float t2 = clock() - t1;
	if (print)
		printf("%d %f\n", (int)bvh.size(), t2 / CLOCKS_PER_SEC);
}

aabb obj_list::box_from(uint beg, uint end) {
	aabb box;
	for (uint i = beg; i < end; i++)
	{
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

void obj_list::split_bvh(uint parent, uint node_size) {
	aabb bbox = bvh[parent].bbox;
	uint be = bvh[parent].n1;
	uint en = bvh[parent].n2;
	uint size = en - be;
	if (size > node_size)
	{
		float pcost = bbox.area() * size;
		uint mi = be + size / 2;
		float cost = split_cost(be, en, mi, bbox);
		if (mi > be && mi < en && cost < pcost)
		{
			bvh[parent].parent = true;
			bvh[parent].n1 = bvh.size();
			bvh[parent].n2 = bvh[parent].n1 + 1;
			aabb lbox = box_from(be, mi);
			aabb rbox = box_from(mi, en);
			bvh.emplace_back(bvh_node(lbox, be, mi, 0));
			bvh.emplace_back(bvh_node(rbox, mi, en, 0));
			split_bvh(bvh[parent].n1, node_size);
			split_bvh(bvh[parent].n2, node_size);
		}
	}
}

void obj_list::split_bvh(uint be, uint en, uint node_size) {
	aabb bbox = box_from(be, en);
	uint size = en - be;
	if (size > node_size)
	{
		float pcost = bbox.area() * size;
		uint mi = be + size / 2;
		float cost = split_cost(be, en, mi, bbox);
		if (mi != be && mi != en && cost < pcost)
		{
			uint n = bvh.size();
			bvh.emplace_back(bvh_node(bbox, n + 1, 0, 1));
			split_bvh(be, mi, node_size);
			bvh[n].n2 = bvh.size();
			split_bvh(mi, en, node_size);
		}
		else bvh.emplace_back(bvh_node(bbox, be, en, 0));
	}
	else bvh.emplace_back(bvh_node(bbox, be, en, 0));
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


