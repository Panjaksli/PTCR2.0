#include "mesh.h"
std::vector<poly> load_mesh(const char* filename, vec4 off, float scale, bool flip)
{
	std::string name(filename);
	if (name.find(".msh") != -1) {
		return load_MSH((name).c_str(), off, scale, flip);
	}
	else if (name.find(".obj") != -1) {
		OBJ_to_MSH((name + ".obj").c_str());
		printf("Generated .msh file!\n");
		return load_MSH((name + ".msh").c_str(), off, scale, flip);
	}
	else if (std::ifstream(name + ".msh").good() || std::ifstream("objects/" + name + ".msh").good())
		return load_MSH((name + ".msh").c_str(), off, scale, flip);
	else if (std::ifstream(name + ".obj").good() || std::ifstream("objects/" + name + ".obj").good())
	{
		OBJ_to_MSH((name + ".obj").c_str());
		printf("Generated .msh file!\n");
		return load_MSH((name + ".msh").c_str(), off, scale, flip);
	}
	else {
		printf("INVALID MESH: %s!!!!!\n", filename);
		return std::vector<poly>();
	}
}

void OBJ_to_MSH(const char* filename) {

	std::string name(filename);
	std::ifstream file(name);
	if (!file.is_open())
	{
		name = "objects/" + name;
		file = std::ifstream(name);
		if (!file.is_open())
		{
			printf("File not found !\n");
			return;
		}
	}
	std::stringstream file_buff;
	file_buff << file.rdbuf();
	std::vector<float3> vert; vert.reserve(0xfffff);
	std::vector<uint3> face; face.reserve(0xfffff);
	std::string line = "";
	while (std::getline(file_buff, line))
	{
		float3 ftmp; uint3 utmp;
		if (sscanf_s(line.c_str(), "v %f %f %f", &ftmp.x, &ftmp.y, &ftmp.z) > 1) {
			vert.emplace_back(ftmp);
		}
		else if (sscanf_s(line.c_str(), "f %u%*[^ ]%u%*[^ ]%u", &utmp.x, &utmp.y, &utmp.z) > 1) {
			utmp.x -= 1; utmp.y -= 1; utmp.z -= 1;
			face.emplace_back(utmp);
		}
		else if (sscanf_s(line.c_str(), "f %u %u %u", &utmp.x, &utmp.y, &utmp.z) > 1) {
			utmp.x -= 1; utmp.y -= 1; utmp.z -= 1;
			face.emplace_back(utmp);
		}
	}
	file.close();
	std::ofstream out;
	name.erase(name.length() - 4);
	out.open(name + ".msh", std::ios_base::binary | std::ios_base::out);
	uint vf[2] = { (uint)vert.size(),  (uint)face.size() };
	out.write((char*)vf, 2 * sizeof(uint));
	out.write((char*)&vert[0], sizeof(float3) * vf[0]);
	out.write((char*)&face[0], sizeof(uint3) * vf[1]);
	out.close();
}



std::vector<poly> load_OBJ(const char* filename, vec4 off, float scale, bool flip)
{
	std::string name(filename);
	std::ifstream file(name);
	if (!file.is_open())
	{
		file = std::ifstream("objects/" + name);
		if (!file.is_open())
		{
			printf("File not found !\n");
			return std::vector<poly>();
		}
	}
	std::stringstream file_buff;
	file_buff << file.rdbuf();
	std::vector<vec4> vert; vert.reserve(0xfffff);
	std::vector<uint3> face; face.reserve(0xfffff);
	std::string line = "";
	std::string pref = "";
	float t1 = clock();
	while (std::getline(file_buff, line))
	{
		//C version is 2x faster
#if 1
		float3 ftmp;
		uint3 utmp;
		if (sscanf_s(line.c_str(), "v %f %f %f", &ftmp.x, &ftmp.y, &ftmp.z) > 1) {
			vert.emplace_back(ftmp);
		}
		else if (sscanf_s(line.c_str(), "f %u%*[^ ]%u%*[^ ]%u", &utmp.x, &utmp.y, &utmp.z) > 1) {
			utmp.x -= 1; utmp.y -= 1; utmp.z -= 1;
			face.emplace_back(utmp);
		}
		else if (sscanf_s(line.c_str(), "f %u %u %u", &utmp.x, &utmp.y, &utmp.z) > 1) {
			utmp.x -= 1; utmp.y -= 1; utmp.z -= 1;
			face.emplace_back(utmp);
		}
#else
		std::stringstream ss;
		ss.str(line);
		ss >> pref;
		if (pref == "v")
		{
			vec4 tmp;
			ss >> tmp._xyz[0] >> tmp._xyz[1] >> tmp._xyz[2];
			vert.emplace_back(tmp);
		}
		else if (pref == "f")
		{
			uint3 tmp = {};
			ss >> tmp.x >> tmp.y >> tmp.z;
			tmp.x -= 1; tmp.y -= 1; tmp.z -= 1;
			face.emplace_back(tmp);
		}
#endif
	}
	file.close();
	if (not0(off) || scale != 1.f)
		for (auto& v : vert)
			v = v * scale + off;
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
	std::cout << "Loaded: " << name << "\n";
	std::cout << "No of tris: " << polys.size() << " Took: " << (clock() - t1) / CLOCKS_PER_SEC << "\n";
	return polys;
}


std::vector<poly> load_MSH(const char* filename, vec4 off, float scale, bool flip)
{
	std::string name(filename);
	std::ifstream file(name, std::ios_base::in | std::ios_base::binary);
	if (!file.is_open())
	{
		file = std::ifstream("objects/" + name, std::ios_base::in | std::ios_base::binary);
		if (!file.is_open())
			throw "File not found !";
	}
	float t1 = clock();
	uint vf[2] = {};
	file.read((char*)vf, 2 * sizeof(uint));
	std::vector<float3> vert(vf[0]);
	std::vector<uint3> face(vf[1]);
	file.read((char*)&vert[0], sizeof(float3) * vf[0]);
	file.read((char*)&face[0], sizeof(uint3) * vf[1]);
	file.close();
	if (not0(off) || scale != 1.f)
		for (auto& v : vert)
		{
			vec4 t = v * scale + off;
			v = float3(t.x(), t.y(), t.z());
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

	std::cout << "Loaded: " << name << "\n";
	std::cout << "No of tris: " << polys.size() << " Took: " << (clock() - t1) / CLOCKS_PER_SEC << "\n";
	return polys;
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