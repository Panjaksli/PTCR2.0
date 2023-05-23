#include "mesh.h"
vector<poly> load_mesh(const char* filename, vec4 off, float scale, bool flip) {
	path name(u8path(filename));
	if (!name.has_parent_path())
		name = "objects" / name;
	path msh = name.replace_extension(".msh");
	path obj = name.replace_extension(".obj");
	if (exists(msh)) {
		return load_MSH(msh, off, scale, flip);
	}
	else if (exists(obj) && OBJ_to_MSH(obj)) {
		return load_MSH(msh, off, scale, flip);
	}
	else {
		printf("INVALID MESH: %s !!!!!\n", filename);
		return vector<poly>();
	}
}

bool OBJ_to_MSH(path name) {
	std::ifstream file(name);
	if (!file.is_open()) {
		cout << "File: " << name << " not found !" << "\n";
		return false;
	}
	vector<float3> vert; vert.reserve(0xffff);
	vector<uint3> face; face.reserve(0xffff);
	string line; line.reserve(128);
	while (std::getline(file, line)) {
		float3 f3; uint3 u3; uint u4 = 0; const char* cstr = line.c_str();
		if (sscanf_s(cstr, "v %f %f %f", &f3.x, &f3.y, &f3.z) > 1) {
			vert.emplace_back(f3);
		}
		else if (sscanf_s(cstr, "f %u%*[^ ]%u%*[^ ]%u%*[^ ]%u", &u3.x, &u3.y, &u3.z, &u4) > 2) {
			u3.x -= 1; u3.y -= 1; u3.z -= 1; u4 -= 1;
			face.emplace_back(u3);
			if (u4 != -1)
				face.emplace_back(uint3(u3.x, u3.z, u4));
		}
		else if (sscanf_s(cstr, "f %u %u %u %u", &u3.x, &u3.y, &u3.z, &u4) > 2) {
			u3.x -= 1; u3.y -= 1; u3.z -= 1; u4 -= 1;
			face.emplace_back(u3);
			if (u4 != -1)
				face.emplace_back(uint3(u3.x, u3.z, u4));
		}
	}
	name.replace_extension(".msh");
	std::ofstream out(name, std::ios_base::binary | std::ios_base::out);
	uint vf[2] = { (uint)vert.size(),  (uint)face.size() };
	out.write((char*)vf, 2 * sizeof(uint));
	out.write((char*)&vert[0], sizeof(float3) * vf[0]);
	out.write((char*)&face[0], sizeof(uint3) * vf[1]);
	printf("Generated .msh file!\n");
	return true;
}

vector<poly> load_MSH(path name, vec4 off, float scale, bool flip) {
	std::ifstream file(name, std::ios_base::in | std::ios_base::binary);
	if (!file.is_open()) {
		cout << "File: " << name << " not found !" << "\n";
		return vector<poly>();
	}
	double time = timer();
	uint vf[2] = {};
	file.read((char*)vf, 2 * sizeof(uint));
	vector<float3> vert(vf[0]);
	vector<uint3> face(vf[1]);
	file.read((char*)&vert[0], sizeof(float3) * vf[0]);
	file.read((char*)&face[0], sizeof(uint3) * vf[1]);
	if (not0(off) || scale != 1.f)
		for (auto& v : vert) {
			vec4 t = vec4(v) * scale + off;
			v = float3(t.x(), t.y(), t.z());
		}
#if SMOOTH_SHADING
	//per-vertex normals
	vector<poly> polys(face.size());
	vector<vec4> nrms(vert.size(), vec4());
	for (uint j = 0; j < face.size(); j++) {
		if (flip)
			polys[j].set_quv(vert[face[j].x], vert[face[j].z], vert[face[j].y]);
		else
			polys[j].set_quv(vert[face[j].x], vert[face[j].y], vert[face[j].z]);
		vec4 n = cross(polys[j].U, polys[j].V);
		nrms[face[j].x] += n;
		nrms[face[j].y] += n;
		nrms[face[j].z] += n;
	}

	//polygons from triangles and normals
	for (uint j = 0; j < face.size(); j++)
		polys[j].set_nor(nrms[face[j].x], nrms[face[j].y], nrms[face[j].z]);
#else
	vector<poly> polys; polys.reserve(face.size());
	for (const auto& f : face) {
		vec4 a = vert[f.x];
		vec4 b = vert[f.y];
		vec4 c = vert[f.z];
		polys.emplace_back(flip ? poly(a, c, b) : poly(a, b, c));
}
#endif
	cout << "Loaded: " << name << " Polygons: " << polys.size() << " Took: " << timer(time) << "\n";
	return polys;
}
