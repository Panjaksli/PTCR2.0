#pragma once
#include "vec4.h"

struct mat4 {
	union {
		vec4 R[4];
		struct { vec4 x, y, z, w; };
	};
	mat4() {
		x = vec4(1, 0, 0, 0);
		y = vec4(0, 1, 0, 0);
		z = vec4(0, 0, 1, 0);
		w = vec4(0, 0, 0, 1);
	}
	mat4(vec4 P, vec4 A = 0, vec4 S = 1) {
		Compose(P, A, S);
	}
	mat4(vec4 r0, vec4 r1, vec4 r2, vec4 r3) : R{ r0,r1,r2,r3 } {}
	inline vec4 operator[](uchar id) const { return R[id]; }
	inline vec4& operator[](uchar id) { return R[id]; }

	inline vec4 P() const { return vec4(x.w(), y.w(), z.w(), w.w()); }
	inline vec4 S() const { return vec4(x.len(), y.len(), z.len()); }
	inline vec4 A() const { return w; }

	void add_A(vec4 A) {
		Rotation(w + A);
	}
	void set_A(vec4 A) {
		Rotation(A);
	}
	void set_GS(float GS) {
		w.xyz[3] = GS;
	}
	void add_P(vec4 P) {
		x.xyz[3] += P.x();
		y.xyz[3] += P.y();
		z.xyz[3] += P.z();
	}
	void set_P(vec4 P) {
		x.xyz[3] = P.x();
		y.xyz[3] = P.y();
		z.xyz[3] = P.z();
	}
	inline mat4 algebraic() const {
		mat4 T = *this;
		T.R[3] = vec4(0, 0, 0, 1);
		return T;
	}

#if USE_SSE
	inline mat4 transpose() const {
		mat4 T = *this;
		_MM_TRANSPOSE4_PS(T.x.xyz, T.y.xyz, T.z.xyz, T.w.xyz);
		return T;
	}
	inline vec4 vec(vec4 u) const {
		return vec4(*this * vec4(u, 0), u.w());
	}
	inline vec4 pnt(vec4 u) const {
		return vec4(*this * vec4(u, 1), u.w());
	}
	inline vec4 operator*(vec4 u) const {
		return col_mul(u.xyz);
	}
	inline mat4 operator*(mat4 T) const {
		vec4 r1 = row_mul(T[0].xyz);
		vec4 r2 = row_mul(T[1].xyz);
		vec4 r3 = row_mul(T[2].xyz);
		vec4 r4 = vec4(R[3] + T.R[3], R[3].w() * T.R[3].w());
		return mat4(r1, r2, r3, r4);
	}
private:
	inline __m128 row_mul(const __m128& v) const {
		return v[0] * R[0].xyz + v[1] * R[1].xyz + v[2] * R[2].xyz + _mm_setr_ps(0, 0, 0, v[3]);
	}
	inline __m128 col_mul(const __m128& v) const {
		mat4 T = transpose();
		return v[0] * T.R[0].xyz + v[1] * T.R[1].xyz + v[2] * T.R[2].xyz + v[3] * T.R[3].xyz;
	}
public:
#else
	inline vec4 vec(vec4 u) const {
		return vec4(dot(u, x), dot(u, y), dot(u, z), u.w());
	}
	inline vec4 pnt(vec4 u) const {
		vec4 v = vec4(u, 1);
		return vec4(dot4(v, x), dot4(v, y), dot4(v, z), u.w());
	}
	inline vec4 operator*(vec4 u) const {
		return vec4(dot4(u, x), dot4(u, y), dot4(u, z), dot4(u, w));
	}
	inline mat4 operator*(mat4 T) const {
		vec4 a = T.w;
		T = T.transpose();
		return
			mat4(
				vec4(dot(x, T[0]), dot(x, T[1]), dot(x, T[2]), dot4(x, vec4(T[3], 1))),
				vec4(dot(y, T[0]), dot(y, T[1]), dot(y, T[2]), dot4(y, vec4(T[3], 1))),
				vec4(dot(z, T[0]), dot(z, T[1]), dot(z, T[2]), dot4(z, vec4(T[3], 1))),
				vec4(w + a, w.w() * a.w())//vec4(dot4(w, T[0]), dot4(w, T[1]), dot4(w, T[2]), dot4(w, T[3]))
			);
	}
	inline mat4 transpose() const {
		return mat4(
			vec4(x.x(), y.x(), z.x(), w.x()),
			vec4(x.y(), y.y(), z.y(), w.y()),
			vec4(x.z(), y.z(), z.z(), w.z()),
			vec4(x.w(), y.w(), z.w(), w.w())
		);
	}

#endif
	inline mat4 inverse() const {
		vec4 t = -P();
		mat4 Rt = transpose();
		mat4 T = Rt;
		T.x.xyz[3] = dot(Rt.x, t);
		T.y.xyz[3] = dot(Rt.y, t);
		T.z.xyz[3] = dot(Rt.z, t);
		T.x /= x.len2();
		T.y /= y.len2();
		T.z /= z.len2();
		T.w = vec4(-w, 1.f / w.w());
		return T;
	}
	inline mat4 operator-(mat4 T) const {
		return mat4(x - T.x, y - T.y, z - T.z, w - T.w);
	}
	inline mat4 operator+(mat4 T) const {
		return mat4(x + T.x, y + T.y, z + T.z, w + T.w);
	}
	inline mat4& operator-=(mat4 T) {
		return *this = *this - T;
	}
	inline mat4& operator+=(mat4 T) {
		return *this = *this + T;
	}
	inline mat4& operator*=(mat4 T) {
		return *this = *this * T;
	}
	void decompose(vec4& P, vec4& A, vec4& S) const {
		mat4 T = *this;
		P = vec4(T[0].w(), T[1].w(), T[2].w());
		S = vec4(T[0].len(), T[1].len(), T[2].len());
		A = w;
	}
	void print() {
		for (int i = 0; i < 4; i++)
			R[i].print4();
	}

private:
	void Compose(vec4 P, vec4 A, vec4 S) {
		*this = mat4();
		if (not0(A))Rotation(A);
		Scale(S);
		Position(P);
	}
	void Scale(vec4 S) {
		x *= S; y *= S; z *= S;
	}
	void Position(vec4 P) {
		x.xyz[3] = P.x();
		y.xyz[3] = P.y();
		z.xyz[3] = P.z();
		w.xyz[3] = P.w();
	}
	void Rotation(vec4 A) {
		w = vec4(mod(A, pi2), w.w());
		vec4 sin_A = sin(w);
		vec4 cos_A = cos(w);
		x.xyz[0] = cos_A.x() * cos_A.y();
		x.xyz[1] = cos_A.x() * sin_A.y() * sin_A.z() - sin_A.x() * cos_A.z();
		x.xyz[2] = cos_A.x() * sin_A.y() * cos_A.z() + sin_A.x() * sin_A.z();
		y.xyz[0] = sin_A.x() * cos_A.y();
		y.xyz[1] = sin_A.x() * sin_A.y() * sin_A.z() + cos_A.x() * cos_A.z();
		y.xyz[2] = sin_A.x() * sin_A.y() * cos_A.z() - cos_A.x() * sin_A.z();
		z.xyz[0] = -sin_A.y();
		z.xyz[1] = cos_A.y() * sin_A.z();
		z.xyz[2] = cos_A.y() * cos_A.z();
	}
};
inline mat4 rotat_mat4(vec4 A = 0) {
	return mat4(0, A);
}
inline mat4 trans_mat4(vec4 P = 0) {
	return mat4(P);
}
inline mat4 scale_mat4(vec4 S = 1) {
	return mat4(0, 0, S);
}
inline mat4 compl_mat4(vec4 P, vec4 A = 0, vec4 S = 1) {
	return mat4(P, A, S);
}