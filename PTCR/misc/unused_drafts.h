#pragma once


//struct instance {
//	mat4 T, Ti;
//	aabb bbox;
//	uint *_mat = nullptr;
//	uint mat;
//	uint idx;
//};

struct rgba8888 {
	rgba8888(uint rgba) : rgba(rgba) {}
	union {
		struct {
			uchar r, g, b, a;
		};
		uint rgba;
	};
	inline void unpack(uint _rgba) {
		rgba = _rgba;
	}
	inline uint pack() {
		return rgba;
	}
};

inline uint avg_rgb8888(rgba8888 x, rgba8888 y) {
	x.r = (x.r + y.r) / 2;
	x.g = (x.g + y.g) / 2;
	x.b = (x.b + y.b) / 2;
	x.a = (x.a + y.a) / 2;
	return x.rgba;
}
extern float GAUSS_3x3[9];
extern float GAUSS_5x5[25];
template <typename T>
inline T gauss_3x3(const T* x) {
	T sum = 0;
	for (int i = 0; i < 9; i++)
		sum += GAUSS_3x3[i] * x[i];
	return sum;
}
template <typename T>
inline T gauss_5x5(const T* x) {
	T sum = 0;
	for (int i = 0; i < 25; i++)
		sum += GAUSS_5x5[i] * x[i];
	return sum;
}
float GAUSS_3x3[9] = {
	1 / 16.f, 1 / 8.f, 1 / 16.f,
	1 / 8.f, 1 / 4.f, 1 / 8.f,
	1 / 16.f, 1 / 8.f, 1 / 16.f
};

float GAUSS_5x5[25] = {
	1 / 273.f,4 / 273.f,7 / 273.f,4 / 273.f,1 / 273.f,
	4 / 273.f,16 / 273.f,26 / 273.f,16 / 273.f,4 / 273.f,
	7 / 273.f,26 / 273.f,41 / 273.f,26 / 273.f,7 / 273.f,
	4 / 273.f,16 / 273.f,26 / 273.f,16 / 273.f,4 / 273.f,
	1 / 273.f,4 / 273.f,7 / 273.f,4 / 273.f,1 / 273.f
};
inline vec4 bilat_3x3(const vec4* x) {
	vec4 y = 0;
	float w = 0;
	float sd = 1, sr = 1;
	sd *= sd; sr *= sr;
	float d[9] = { 2,1,2,1,0,1,2,1,2 };
	for (int i = 0; i < 9; i++)
	{
		vec4 dl = x[4] / x[4].w() - x[i] / x[i].w();
		float wi = expf(-0.5f * (d[i] / sd + dl.len2() / sr));//GAUSS_3x3[i] * gauss(dl.len2(), 16);
		y += x[i] * wi;
		w += wi;
	}
	return y / w;
}

inline vec4 bilat_5x5(const vec4* x) {
	vec4 y = 0;
	float w = 0;
	float sd = 1, sr = 1;
	sd *= sd; sr *= sr;
	float d[25] =
	{
	8,5,4,5,8,
	5,2,1,2,5,
	4,1,0,1,4,
	5,2,1,2,5,
	8,5,4,5,8
	};
	for (int i = 0; i < 25; i++)
	{
		vec4 dl = x[12] / x[12].w() - x[i] / x[i].w();
		float wi = expf(-0.5f * (d[i] / sd + dl.len2() / sr));
		y += x[i] * wi;
		w += wi;
	}
	return y / w;
}
std::vector<poly> generate_mesh(uint seed, vec4 off, float scale, bool flip) {
	int dim = 128;
	std::vector<vec4> vert2(dim * dim);
	std::vector<vec4> vert(dim * dim);
	std::vector<uint3> face;

	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			vert2[i * dim + j] = vec4(i, randf(seed), j);
		}
	}

	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			vec4 x[9];
			kernel<3>(vert2.data(), x, i, j, dim, dim);
			vert[i * dim + j] = x[4] + gauss_3x3(x);
		}
	}


	for (int i = 0; i < dim - 1; i++) {
		for (int j = 0; j < dim - 1; j++)
		{
			int l = i * dim + j;
			int r = i * dim + j + 1;
			int dl = (i + 1) * dim + j;
			int dr = (i + 1) * dim + j + 1;
			face.emplace_back(uint3(l, r, dr));
			face.emplace_back(uint3(dl, l, dr));
		}
	}

#if SMOOTH_SHADING
	//per-vertex normals
	std::vector<poly> polys(face.size());
	std::vector<vec4> nrms(vert.size(), vec4());
	for (uint j = 0; j < face.size(); j++) {
		flip ? polys[j].set_quv(vert[face[j].x], vert[face[j].z], vert[face[j].y])
			: polys[j].set_quv(vert[face[j].x], vert[face[j].y], vert[face[j].z]);
		vec4 n = cross(polys[j].U, polys[j].V);
		nrms[face[j].x] += n;
		nrms[face[j].y] += n;
		nrms[face[j].z] += n;
	}

	//polygons from triangles and normals
	for (uint j = 0; j < face.size(); j++)
		polys[j].set_nor(nrms[face[j].x], nrms[face[j].y], nrms[face[j].z]);
#else
	std::vector<poly> polys; polys.reserve(face.size());
	for (const auto& f : face)
	{
		vec4 a = vert[f.x];
		vec4 b = vert[f.y];
		vec4 c = vert[f.z];
		polys.emplace_back(flip ? poly(a, c, b) : poly(a, b, c));
	}
#endif
	return polys;
}
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
	__forceinline vec4 rand_to(vec4 O) const {
		if (size == 1) return prim[0].trans(P).rand_to(O);
		uint id = raint(size - 1);
		return prim[id].trans(P).rand_to(O);
	}
	__forceinline vec4 rand_from() const {
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
	vec4 P, A;
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


void median_filter(const vector<vec4>& in, vector<vec4>& out, int h, int w)
{
#pragma omp parallel for collapse(2) schedule(dynamic,100)
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			vec4 window[9];
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
	vec4 P = r.O;
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
			float x = objects[id].SDF(P + vec4(eps, 0, 0)) - objects[id].SDF(P + vec4(-eps, 0, 0));
			float y = objects[id].SDF(P + vec4(0, eps, 0)) - objects[id].SDF(P + vec4(0, -eps, 0));
			float z = objects[id].SDF(P + vec4(0, 0, eps)) - objects[id].SDF(P + vec4(0, 0, -eps));
			rec.N = norm(vec4(x, y, z));
			rec.P = P;
			rec.t = fabsf(t);
			rec.u = rec.v = 0;
			return true;
		}

	}
	return false;
}

albedo pbrcol(vec4(0.8f, 0.1f, 0.1f), vec4(0.1, 100, 0.1), vec4(0.5, 0.5, 1), 10);
scn.world.add_mat(pbrcol, mat_lig);
scn.world.add(vec4(0, 0, -3), sphere(vec4(0, 0, 0, 1)), 0);
scn.opt.en_fog = 0;
scn.sun_pos.set_A(vec4(1, 0, 1));
scn.cam.setup(matrix(vec4(1, 1, 1), vec4(0, 0, 0)), 47, 10);
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





vec4 x[9];
vec4 y;
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
	for (int i = 0; i < cam.CCD.n; i++)cam.CCD.buff[i] = vec4(0, 0, 0, infp);
#pragma omp parallel for collapse(2) schedule(dynamic, 100)
	for (int i = 0; i < cam.h; i++) {
		for (int j = 0; j < cam.w; j++) {
			uint off = i * cam.w + j;
			vec4 xy = cam.SS(vec4(j, i));
			float dist = cam.CCD.data[off].w() / cam.CCD.time;
			vec4 pt = T.P() + norm(T.vec(xy)) * dist;
			vec4 spt = cam.T.inverse().pnt(pt);
			vec4 dir = spt / dist;
			vec4 uv = dir / fabsf(dir.z());
			if (fabsf(uv[2] + 1.f) > 0.01f)continue;
			uv *= tfov / cam.tfov;
			uv = cam.inv_SS(uv);
			//uv += rapvec();
			uint x = uv[0];
			uint y = uv[1];
			if (y < cam.h && x < cam.w) {
				if (dist <= cam.CCD.buff[y * cam.w + x].w()) cam.CCD.buff[y * cam.w + x] = vec4(cam.CCD.data[off], dist);
			}

		}
	}
#pragma omp parallel for collapse(2) schedule(dynamic, 100)
	for (int i = 0; i < cam.h; i++) {
		for (int j = 0; j < cam.w; j++) {
			uint off = i * cam.w + j;
			uint off2 = i * pitch + j;
			float fact = cam.CCD.time / cam.exposure;
			vec4 col[9];
			kernel(cam.CCD.buff.data(), col, i, j, cam.h, cam.w);
			vec4 out = max(col[4], med9(col));
			vec4 base = cam.CCD.data[off];
			if ((base - out).len2() < 0.001f) bgr(vec4(out, fact), cam.CCD.disp[off]);
			else bgr(vec4(base, fact), cam.CCD.disp[off]);
			bgr(vec4(out, fact), cam.CCD.disp[off2]);
		}
	}
}
/*float scl = 1.f / 1.001f;
			float y = (i / (float)cam.h) - 0.5f;
			float x = (j / (float)cam.w) - 0.5f;
			int iy = clamp_int((y * scl + 0.5f) * cam.h, 0, cam.h - 1);
			int ix = clamp_int((x * scl + 0.5f) * cam.w, 0, cam.w - 1);*/
			//near0(cam.CCD.buff[off]) ? cam.CCD.data[iy * cam.w + ix] :
/*mat4 iTp = proj.T.inverse();
#pragma omp parallel for collapse(2) schedule(dynamic, 100)
	for (int i = 0; i < cam.h; i++) {
		for (int j = 0; j < cam.w; j++) {
			uint off = i * cam.w + j;
			float dist = cam.CCD.data[off].w();
			vec4 xy = cam.SS(vec4(j, i), proj);
			vec4 pt = iTp.P() + iTp.vec(xy) * dist;
			vec4 spt = cam.T.pnt(pt);
			if (spt.z() < 0) [[likely]] {
				vec4 dir = spt / dist;
				vec4 uv = dir / fabsf(dir.z());
				uv = cam.inv_SS(uv);
				uint x = uv[0];
				uint y = uv[1];
				if (x < cam.w && y < cam.h && near0(cam.CCD.buff[y * cam.w + x]))
					cam.CCD.buff[y * cam.w + x] = vec4(cam.CCD.data[off], dist);
			}
		}
	}*/

	//
//#pragma omp parallel for
int mid = cam.w / 2;
for (int i = 0; i < cam.h; i++) {
	for (int j = mid; j >= 0; j--) {
		if (near0(buff[j + i * cam.w])) {
			vec4 col = buff[j + 1 + i * cam.w];
			buff[i * cam.w + j] = col;
			int k;
			for (k = j - 1; k >= 0; k--) {
				if (near0(buff[i * cam.w + k])) {
					buff[i * cam.w + k] = col;
				}
				else break;
			}
			j = k;
		}
	}
	for (int j = mid; j < cam.w; j++) {
		if (near0(buff[j + i * cam.w])) {
			vec4 col = buff[j + -1 + i * cam.w];
			buff[i * cam.w + j] = col;
			int k;
			for (k = j + 1; k < cam.w; k++) {
				if (near0(buff[i * cam.w + k])) {
					buff[i * cam.w + k] = col;
				}
				else break;
			}
			j = k;
		}
	}
}
		/*#pragma omp parallel for collapse(2) schedule(dynamic, 100)
			for (int i = 0; i < cam.h; i++) {
				for (int j = 0; j < cam.w; j++) {
					uint off = i * cam.w + j;
					uint off2 = i * pitch + j;
					vec4 base = cam.CCD.data[off];
					vec4 changed = cam.CCD.buff[off];
					float fact = cam.CCD.time / cam.exposure;
					if((base-changed).len2() < 0.001f) bgr(vec4(changed, fact), cam.CCD.disp[off]);
					else bgr(vec4(base, fact), cam.CCD.disp[off2]);

				}
			}*/


__forceinline void mixed(const ray& r, const hitrec& rec, const albedo& tex, matrec& mat) {
	//simple mix of lambertian and mirror reflection/transmission + emission
	vec4 rgb = tex.rgb(rec.u, rec.v);
	vec4 mer = tex.mer(rec.u, rec.v);
	vec4 nor = tex.nor(rec.u, rec.v);
	float mu = mer.x();
	float em = mer.y();
	float ro = mer.z();
	float a = ro * ro;
	float wave = rafl();
	float ior = tex.ir;
	bool opaque = rafl() < rgb.w();
	if (rec.face && !opaque) {
		if (wave < 0.333f) {
			float um = 0.65;
			ior += 0.01f * ior / (um * um);
			rgb *= vec4(3, 0, 0);
		}
		else if (wave < 0.666f) {
			float um = 0.54;
			ior += 0.01f * ior / (um * um);
			rgb *= vec4(0, 3, 0);
		}
		else {
			float um = 0.43;
			ior += 0.01f * ior / (um * um);
			rgb *= vec4(0, 0, 3);
		}
	}
	float n1 = rec.face ? mat.ir : mat.ir == 1.f ? ior : mat.ir;
	float n2 = rec.face ? ior : ior == n1 ? 1.f : ior;
	vec4 N = normal_map(rec.N, nor);
	onb n(N);
	//perfect diffuse && solid
	if (mu < eps && ro > 1 - eps && opaque) {
		mat.aten = rgb;
		mat.L = n.world(sa_cos());
		mat.N = N;
		mat.P = rec.P + rec.N * eps;
		mat.emis = rgb * em;
		mat.refl = refl_diff * not0(mat.aten);
		return;
	}
	else if (opaque)return ggx(r, rec, tex, mat);
	vec4 H = n.world(sa_ggx(a));
	float HoV = absdot(H, r.D);
	float Fr = fresnel(HoV, n1, n2, mu);
	bool refl = Fr > rafl();
	if (refl) {
		mat.L = reflect(r.D, H);
		float NoV = absdot(-r.D, N);
		float NoL = dot(N, mat.L);
		float NoH = dot(N, H);
		if (NoL <= 0)return;
		vec4 F0 = mix(0.04f, vec4(rgb, 1), mu);
		vec4 F = fres_spec(HoV, F0).fact();
		vec4 Fs = fres_spec(HoV, tex.specular());
		mat.aten = rec.face ? mix(F, Fs, tex.spec.w()) * GGX(NoL, NoV, a) * HoV / (NoV * NoH) : rgb;
		mat.P = rec.P + rec.N * eps;
		mat.refl = rec.face ? refl_spec : refl_tran;
	}
	else {
		mat.aten = rgb;
		mat.P = rec.P - rec.N * eps;
		mat.L = refract(r.D, H, n1 / n2);
		mat.refl = refl_tran;
		mat.ir = n2;
	}
	//if (!rec.face)mat.aten *= saturate(expf(-20.f * rec.t * (1.f - rgb)));
	mat.a = a;
	mat.N = N;
	mat.emis = em * rgb;
	mat.refl *= not0(mat.aten);
}


//Quaternions ?
void rot(vec4 A) {
	w = A;//mod(A, pi2);
	if (not0(w)) {
		A = w * 0.5f;
		vec4 csy = cossin(A.x());
		vec4 csp = cossin(A.y());
		vec4 csr = cossin(A.z());
		float qx = csr[1] * csp[0] * csy[0] - csr[0] * csp[1] * csy[1];
		float qy = csr[0] * csp[1] * csy[0] + csr[1] * csp[0] * csy[1];
		float qz = csr[0] * csp[0] * csy[1] - csr[1] * csp[1] * csy[0];
		float qw = csr[0] * csp[0] * csy[0] + csr[1] * csp[1] * csy[1];
		x.xyz[0] = 1 - 2 * qy * qy - 2 * qz * qz;
		x.xyz[1] = 2 * qx * qy - 2 * qw * qz;
		x.xyz[2] = 2 * qx * qz + 2 * qw * qy;
		y.xyz[0] = 2 * qx * qy + 2 * qw * qz;
		y.xyz[1] = 1 - 2 * qx * qx - 2 * qz * qz;
		y.xyz[2] = 2 * qy * qz - 2 * qw * qx;
		z.xyz[0] = 2 * qx * qz - 2 * qw * qy;
		z.xyz[1] = 2 * qy * qz + 2 * qw * qx;
		z.xyz[2] = 1 - 2 * qx * qx - 2 * qy * qy;
	}
}