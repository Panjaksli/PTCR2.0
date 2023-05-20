#include "mesh.h"
std::vector<poly> load_mesh(const char* filename, vec4 off, float scale, bool flip) {
	std::string name(filename);
	if (name.find(".msh") != -1) {
		return load_MSH(name.c_str(), off, scale, flip);
	}
	else if (name.find(".obj") != -1) {
		OBJ_to_MSH(name.c_str());
		printf("Generated .msh file!\n");
		name.erase(name.length() - 4);
		return load_MSH((name + ".msh").c_str(), off, scale, flip);
	}
	else if (std::ifstream(name + ".msh").good())
		return load_MSH((name + ".msh").c_str(), off, scale, flip);
	else if (std::ifstream("objects/" + name + ".msh").good())
		return load_MSH(("objects/" + name + ".msh").c_str(), off, scale, flip);
	else if (std::ifstream(name + ".obj").good()) {
		OBJ_to_MSH((name + ".obj").c_str());
		printf("Generated .msh file!\n");
		return load_MSH((name + ".msh").c_str(), off, scale, flip);
	}
	else if (std::ifstream("objects/" + name + ".obj").good()) {
		OBJ_to_MSH(("objects/" + name + ".obj").c_str());
		printf("Generated .msh file!\n");
		return load_MSH((name + ".msh").c_str(), off, scale, flip);
	}
	else {
		printf("INVALID MESH: %s !!!!!\n", filename);
		return std::vector<poly>();
	}
}

void OBJ_to_MSH(const char* filename) {

	std::string name(filename);
	std::ifstream file(name);
	if (!file.is_open()) {
		printf("File not found !\n");
		return;
	}
	std::stringstream file_buff;
	file_buff << file.rdbuf();
	std::vector<float3> vert; vert.reserve(0xffff);
	std::vector<uint3> face; face.reserve(0xffff);
	std::string line = "";
	while (std::getline(file_buff, line)) {
		float3 ftmp; uint3 utmp; uint tmp4 = 0;
		if (sscanf_s(line.c_str(), "v %f %f %f", &ftmp.x, &ftmp.y, &ftmp.z) > 1) {
			vert.emplace_back(ftmp);
		}
		else if (sscanf_s(line.c_str(), "f %u%*[^ ]%u%*[^ ]%u%*[^ ]%u", &utmp.x, &utmp.y, &utmp.z, &tmp4) >= 3) {
			utmp.x -= 1; utmp.y -= 1; utmp.z -= 1; tmp4 -= 1;
			face.emplace_back(utmp);
			if (tmp4 != -1) face.emplace_back(uint3(utmp.x, utmp.z, tmp4));
		}
		else if (sscanf_s(line.c_str(), "f %u %u %u %u", &utmp.x, &utmp.y, &utmp.z, &tmp4) >= 3) {
			utmp.x -= 1; utmp.y -= 1; utmp.z -= 1; tmp4 -= 1;
			face.emplace_back(utmp);
			if (tmp4 != -1) face.emplace_back(uint3(utmp.x, utmp.z, tmp4));
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

std::vector<poly> load_OBJ(const char* filename, vec4 off, float scale, bool flip) {
	std::string name(filename);
	std::ifstream file(name);
	if (!file.is_open()) {
		printf("File not found !\n");
		return vector<poly>();
	}
	std::stringstream file_buff;
	file_buff << file.rdbuf();
	std::vector<vec4> vert; vert.reserve(0xffff);
	std::vector<uint3> face; face.reserve(0xffff);
	std::string line = "";
	std::string pref = "";
	double t1 = timer();
	while (std::getline(file_buff, line)) {
		//C version is 2x faster
#if 1
		float3 ftmp; uint3 utmp; uint tmp4 = 0;
		if (sscanf_s(line.c_str(), "v %f %f %f", &ftmp.x, &ftmp.y, &ftmp.z) > 1) {
			vert.emplace_back(ftmp);
		}
		else if (sscanf_s(line.c_str(), "f %u%*[^ ]%u%*[^ ]%u%*[^ ]%u", &utmp.x, &utmp.y, &utmp.z, &tmp4) >= 3) {
			utmp.x -= 1; utmp.y -= 1; utmp.z -= 1; tmp4 -= 1;
			face.emplace_back(utmp);
			if (tmp4 != -1) face.emplace_back(uint3(utmp.x, utmp.z, tmp4));
		}
		else if (sscanf_s(line.c_str(), "f %u %u %u %u", &utmp.x, &utmp.y, &utmp.z, &tmp4) >= 3) {
			utmp.x -= 1; utmp.y -= 1; utmp.z -= 1; tmp4 -= 1;
			face.emplace_back(utmp);
			if (tmp4 != -1) face.emplace_back(uint3(utmp.x, utmp.z, tmp4));
		}
#else
		std::stringstream ss;
		ss.str(line);
		ss >> pref;
		if (pref == "v") {
			vec4 tmp;
			ss >> tmp._xyz[0] >> tmp._xyz[1] >> tmp._xyz[2];
			vert.emplace_back(tmp);
		}
		else if (pref == "f") {
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
	for (const auto& f : face) {
		vec4 a = vert[f.x];
		vec4 b = vert[f.y];
		vec4 c = vert[f.z];
		polys.emplace_back(flip ? poly(a, c, b) : poly(a, b, c));
	}

#endif
	std::cout << "Loaded: " << name << " Polygons: " << polys.size() << " Took: " << timer(t1) << "\n";
	return polys;
}


std::vector<poly> load_MSH(const char* filename, vec4 off, float scale, bool flip) {
	std::string name(filename);
	std::ifstream file(name, std::ios_base::in | std::ios_base::binary);
	if (!file.is_open()) {
		printf("File not found !\n");
		return vector<poly>();
	}
	double t1 = timer();
	uint vf[2] = {};
	file.read((char*)vf, 2 * sizeof(uint));
	std::vector<float3> vert(vf[0]);
	std::vector<uint3> face(vf[1]);
	file.read((char*)&vert[0], sizeof(float3) * vf[0]);
	file.read((char*)&face[0], sizeof(uint3) * vf[1]);
	file.close();
	if (not0(off) || scale != 1.f)
		for (auto& v : vert) {
			vec4 t = vec4(v) * scale + off;
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
	for (const auto& f : face) {
		vec4 a = vert[f.x];
		vec4 b = vert[f.y];
		vec4 c = vert[f.z];
		polys.emplace_back(flip ? poly(a, c, b) : poly(a, b, c));
	}
#endif
	std::cout << "Loaded: " << name << " Polygons: " << polys.size() << " Took: " << timer(t1) << "\n";
	return polys;
}
