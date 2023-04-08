#pragma once
template <class primitive>
class mesh {
public:
	mesh() {}
	mesh(const primitive& _prim, uint _mat) :prim(new primitive[1]{ _prim }), mat(_mat), size(1), lw(1.f) {
		fit();
	}
	mesh(const vector<primitive>& _prim, uint _mat) :prim(new primitive[_prim.size()]), mat(_mat), size(_prim.size()), lw(1.f / size) {
		memcpy(prim, &_prim[0], size * sizeof(primitive));
		fit();
	}
	mesh(const mesh& cpy) : bbox(cpy.bbox), P(cpy.P), A(cpy.A), prim(new primitive[cpy.size]), mat(cpy.mat), size(cpy.size), lw(cpy.lw) {
		memcpy(prim, cpy.prim, size * sizeof(primitive));
	}
	const mesh& operator=(const mesh& cpy) {
		bbox = cpy.bbox, P = cpy.P, A = cpy.A, prim = new primitive[cpy.size], mat = cpy.mat, size = cpy.size, lw = cpy.lw;
		memcpy(prim, cpy.prim, size * sizeof(primitive));
		return *this;
	}
	~mesh() {
		clean();
	}
	__forceinline bool hit(const ray& r, hitrec& rec) const
	{
		ray tr(r.O - P, r.D, r.iD);
		bool any_hit = false;
		for (uint i = 0; i < size; i++)
			any_hit |= prim[i].hit(tr, rec);
		rec.mat = any_hit ? mat : rec.mat;
		rec.P = any_hit ? rec.P + P : rec.P;
		return any_hit;
	}

	__forceinline bool hit(const ray& r, hitrec& rec, uint prim_id) const
	{
		ray tr(r.O - P, r.D, r.iD);
		bool any_hit = prim[prim_id].hit(tr, rec);
		rec.mat = any_hit ? mat : rec.mat;
		rec.P = any_hit ? rec.P + P : rec.P;
		return any_hit;
	}

	__forceinline float pdf(const ray& r)const {
		ray tr(r.O - P, r.D, r.iD);
		if (size == 1) return prim[0].trans(P).pdf(r);
		float y = 0.f;
		for (uint i = 0; i < size; i++)
		{
			y += lw * prim[i].trans(P).pdf(r);
		}
		return y;
	}
	__forceinline vec3 rand_to(vec3 O) const {
		if (size == 1) return prim[0].trans(P).rand_to(O);
		uint id = raint(size - 1);
		return prim[id].trans(P).rand_to(O);
	}
	__forceinline vec3 rand_from() const {
		if (size == 1) return prim[0].trans(P).rand_from();
		uint id = raint(size - 1);
		return prim[id].trans(P).rand_from();
	}

	inline aabb get_box()const { return bbox; }
	inline aabb get_box(uint prim_id)const { return prim[prim_id].trans(P).get_box(); }
	inline vector<primitive> get_data()const {
		return vector<primitive>(prim, prim + size);
	}
	inline primitive* get_data(uint i) const {
		return &(prim[i]);
	}
	inline uint get_mat()const {
		return mat;
	}
	inline uint get_size()const {
		return size;
	}
	inline matrix get_trans()const {
		return matrix(P, A);
	}
	inline void set_trans(const matrix& _T) {
		//matrix dT1(-P);
		matrix dT2(0, _T.A - A);
		//matrix dT3(_T.P(), 0);
		P = _T.P();
		A = _T.A;
#pragma omp parallel for schedule(static,64)
		for (uint i = 0; i < size; i++)
		{
			//prim[i] = prim[i].trans(dT1);
			prim[i] = prim[i].trans(dT2);
			//prim[i] = prim[i].trans(dT3);
		}
		fit();
	}
private:
	inline void transform(const matrix& dT) {
#pragma omp parallel for schedule(static,64)
		for (uint i = 0; i < size; i++)
			prim[i] = prim[i].trans(dT);
	}
	inline void build_cache(const matrix& dT) {
		transform(dT);
		fit();
	}
	inline void fit() {
		bbox = aabb();
		for (uint i = 0; i < size; i++)
		{
			bbox.join(get_box(i));
		}
	}
	inline void clean() {
		delete[]prim;
	}

	aabb bbox;
	vec3 P, A;
	primitive* prim;
	uint mat;
	uint size;
	float lw;
};


struct mesh_raw {
	mesh_raw(const mesh_var* obj, uint prim_id) : bbox(obj->get_box(prim_id)), obj(obj), prim_id(prim_id) {}

	__forceinline bool hit(const ray& r, hitrec& rec) const
	{
		if (!bbox.hit(r))return false;
		return obj->hit(r, rec, prim_id);
	}

	inline aabb get_box()const {
		return bbox;
	}
	inline void update_box() {
		bbox = obj->get_box(prim_id);
	}
	aabb bbox;
	const mesh_var* obj;
	uint prim_id;
};

//obj_bvh.emplace_back(mesh_raw(&obj, j));


void median_filter(const vector<vec3>& in, vector<vec3>& out, int h, int w)
{
#pragma omp parallel for collapse(2) schedule(dynamic,100)
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			vec3 window[9];
			for (int k = 0; k < 3; k++)
				for (int l = 0; l < 3; l++)
				{
					int a = i + k - 1;
					int b = j + l - 1;
					a = (a < 0 || a >= h) ? i : a;
					b = (b < 0 || b >= w) ? j : b;
					window[k * 3 + l] = in[a * w + b];
				}
			out[i * w + j] = med9(window);
		}
	}
}


if (march) {
	ray r = sr;
	vec3 P = r.O;
	float t = 0;
	for (uint i = 0; i < 32; i++)
	{
		uint id = 0;
		float tmin = infp;
		for (uint j = 0; j < objects.size(); j++) {
			float d = objects[j].SDF(P);
			if (d < tmin) {
				tmin = d;
				id = j;
			}
		}
		t += tmin;
		P = r.at(t);
		if (fabsf(t) >= infp)return false;
		if (fabsf(tmin) < 1e-3f) {
			rec.face = tmin > 0;
			rec.mat = objects[id].get_mat();
			float x = objects[id].SDF(P + vec3(eps, 0, 0)) - objects[id].SDF(P + vec3(-eps, 0, 0));
			float y = objects[id].SDF(P + vec3(0, eps, 0)) - objects[id].SDF(P + vec3(0, -eps, 0));
			float z = objects[id].SDF(P + vec3(0, 0, eps)) - objects[id].SDF(P + vec3(0, 0, -eps));
			rec.N = norm(vec3(x, y, z));
			rec.P = P;
			rec.t = fabsf(t);
			rec.u = rec.v = 0;
			return true;
		}

	}
	return false;
}

albedo pbrcol(vec3(0.8f, 0.1f, 0.1f), vec3(0.1, 100, 0.1), vec3(0.5, 0.5, 1), 10);
scn.world.add_mat(pbrcol, mat_lig);
scn.world.add(vec3(0, 0, -3), sphere(vec3(0, 0, 0, 1)), 0);
scn.opt.en_fog = 0;
scn.sun_pos.set_A(vec3(1, 0, 1));
scn.cam.setup(matrix(vec3(1, 1, 1), vec3(0, 0, 0)), 47, 10);
scn.world.en_bvh = 0;


void display(double delay, SDL_Renderer* renderer, SDL_Texture* f1, SDL_Texture* f2, SDL_Rect* move) {
	delay *= 333;
	ImGui::Render();
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	SDL_RenderCopy(renderer, f1, NULL, &out);
	SDL_RenderPresent(renderer);
	SDL_Delay(delay);

	ImGui::Render();
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	SDL_RenderCopy(renderer, f1, NULL, move);
	SDL_RenderPresent(renderer);
	SDL_Delay(delay);

	ImGui::Render();
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	SDL_RenderCopy(renderer, f2, NULL, &out);
	SDL_RenderPresent(renderer);
	SDL_Delay(delay);
}



static uint* disp = nullptr;
static int pitch = width * 4;
if (state == 0) {
	state = 1;
	SDL_LockTexture(f1, 0, (void**)&disp, &pitch);
	Scene.Render(disp, pitch / 4);
	SDL_UnlockTexture(f1);
	ftime = (SDL_GetPerformanceCounter() - ftime) / SDL_GetPerformanceFrequency();
	//display(ftime, renderer, f2, f1);
	SDL_Rect move = out;
	move.x = -(int)movement.z() * movmul;
	move.y = (int)movement.y() * movmul;
	std::future<void> tmp = std::async(display, ftime, renderer, f2, f1, &move);

}
else if (state == 1) {
	state = 0;
	SDL_LockTexture(f2, 0, (void**)&disp, &pitch);
	Scene.Render(disp, pitch / 4);
	SDL_UnlockTexture(f2);
	ftime = (SDL_GetPerformanceCounter() - ftime) / SDL_GetPerformanceFrequency();
	//display(ftime, renderer, f2, f1);
	SDL_Rect move = out;
	move.x = -(int)movement.z() * movmul;
	move.y = (int)movement.y() * movmul;
	std::future<void> tmp = std::async(display, ftime, renderer, f1, f2, &move);

}

//rec.N = (1 - u - v) * rec.N + u * n1 + v * n2;





vec3 x[9];
vec3 y;
for (int m = 1; m <= 4; m = m << 1) {
	for (int k = -1; k <= 1; k++) {
		for (int l = -1; l <= 1; l++) {
			int py = clamp_int(i + k * m, 0, h - 1);
			int px = clamp_int(j + l * m, 0, w - 1);
			int idx = (k + 1) * 3 + l + 1;
			x[idx] = data[py * w + px];
		}
	}
	y += med9(x);
}
return y;


//REPROJECTION
////////////////////////////////////
void scene::Reproject(const mat4& T, float tfov, uint* disp, uint pitch) {
	cam.CCD.set_disp(disp, pitch);
	for (int i = 0; i < cam.CCD.n; i++)cam.CCD.buff[i] = vec3(0, 0, 0, infp);
#pragma omp parallel for collapse(2) schedule(dynamic, 100)
	for (int i = 0; i < cam.h; i++) {
		for (int j = 0; j < cam.w; j++) {
			uint off = i * cam.w + j;
			vec3 xy = cam.SS(vec3(j, i));
			float dist = cam.CCD.data[off].w() / cam.CCD.time;
			vec3 pt = T.P() + norm(T.vec(xy)) * dist;
			vec3 spt = cam.T.inverse().pnt(pt);
			vec3 dir = spt / dist;
			vec3 uv = dir / fabsf(dir.z());
			if (fabsf(uv[2] + 1.f) > 0.01f)continue;
			uv *= tfov / cam.tfov;
			uv = cam.inv_SS(uv);
			//uv += rapvec();
			uint x = uv[0];
			uint y = uv[1];
			if (y < cam.h && x < cam.w) {
				if (dist <= cam.CCD.buff[y * cam.w + x].w()) cam.CCD.buff[y * cam.w + x] = vec3(cam.CCD.data[off], dist);
			}

		}
	}
#pragma omp parallel for collapse(2) schedule(dynamic, 100)
	for (int i = 0; i < cam.h; i++) {
		for (int j = 0; j < cam.w; j++) {
			uint off = i * cam.w + j;
			uint off2 = i * pitch + j;
			float fact = cam.CCD.time / cam.exposure;
			vec3 col[9];
			kernel(cam.CCD.buff.data(), col, i, j, cam.h, cam.w);
			vec3 out = max(col[4], med9(col));
			vec3 base = cam.CCD.data[off];
			if ((base - out).len2() < 0.001f) bgr(vec3(out, fact), cam.CCD.disp[off]);
			else bgr(vec3(base, fact), cam.CCD.disp[off]);
			bgr(vec3(out, fact), cam.CCD.disp[off2]);
		}
	}
}