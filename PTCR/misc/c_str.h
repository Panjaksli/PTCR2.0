#pragma once
#include <stdint.h>
#include <iostream>
#include <memory>
#define uint unsigned int
//static cstring, null terminated, stored on heap
class c_str {
public:
	c_str() {}
	c_str(char* buff) { copy(buff); }
	c_str(const char* buff) { copy(buff); }
	c_str(const c_str& cpy) { copy(cpy.data()); }
	c_str(c_str&& cpy) noexcept : c_str() { swap(*this, cpy); }
	~c_str() { delete[] _data; }
	c_str& operator=(c_str cpy) {
		swap(*this, cpy);
		return *this;
	}
	operator const char* ()const { return _data; }
	operator char* () { return _data; }
	const char& operator[](uint i)const { return _data[i]; }
	char& operator[](uint i) { return _data[i]; }
	uint size()const { return size_of(_data); }
	char* begin() { return _data; }
	char* data() { return _data; }
	char* end() { return _data + size(); }
	const char* begin() const { return _data; }
	const char* data() const { return _data; }
	const char* end() const { return _data + size(); }
	bool empty() const { return size() <= 1; }
	const char* text() const { return empty() ? "0" : _data; }

	void set(const char var) {
		for (uint i = 0; i < max_size; i++)
			_data[i] = var;
	}
	
	const c_str& copy(const char* buff) {
		for (uint i = 0; i < size_of(buff); i++)
			if (i < max_size) _data[i] = buff[i];
		return *this;
	}
	uint paste(char* buff, uint buff_size)const {
		uint msize = size() < buff_size ? size() : buff_size;
		for (uint i = 0; i < msize; i++)
			buff[i] = _data[i];
		return msize;
	}
	const c_str& append(const char* buff) {
		uint off = size() - 1;
		for (uint i = 0; i < size_of(buff); i++)
			if (i + off < max_size) _data[i + off] = buff[i];
		return *this;
	}
	void replace(char what, char with) {
		replace(_data, what, with, max_size);
	}
	static void replace(char* str, char what, char with, uint size) {
		for (uint i = 0; i < size; i++) {
			if (str[i] == what)
				str[i] = with;
		}
	}
	static uint size_of(const char* buff) {
		if (buff == nullptr)return 0;
		uint i = 0;
		while (buff[i++] != '\0');
		return i;
	}
	void clear() {
		set(0);
	}
	void print()const { printf("%s\n", text()); }
	friend void swap(c_str &s1, c_str &s2) {
		std::swap(s1._data, s2._data);
	}
	friend c_str operator+(c_str s1, c_str s2) {
		s1.append(s2);
		return s1;
	}
	static constexpr uint max_size = 128;
private:
	char* _data = new char[max_size] {};
};

/*
class c_str {
public:
	c_str() {}
	c_str(const char* _data) {
		uint size = size_of(_data);
		if (size <= 1) { return; }
		data = new char[size];
		std::copy(_data, _data + size, data);
	}
	c_str(const c_str& cpy) {
		uint size = size_of(cpy.data);
		if (size <= 1) return;
		data = new char[size];
		std::copy(cpy.data, cpy.data + size, data);
	}
	~c_str() { delete[] data; }
	const c_str& operator=(const c_str& cpy) {
		if (this != &cpy) {
			delete[]data;
			uint size = size_of(cpy.data);
			if (size <= 1) return *this;
			data = new char[size];
			std::copy(cpy.data, cpy.data + size, data);
		}
		return *this;
	}
	operator const char* ()const {
		return data;
	}
	operator char* () {
		return data;
	}
	const char& operator[](uint i)const {
		return data[i];
	}
	char& operator[](uint i) {
		return data[i];
	}
	uint size()const {
		return size_of(data);
	}
	char* begin() {
		return data;
	}
	char* end() {
		return data + size();
	}
	bool empty() const {
		return size() <= 1;
	}
	const char* text() const {
		return empty() ? "0" : data;
	}
	int copy(char* buff, uint buff_size = 0)const {
		uint min_size = size() < buff_size ? size() : buff_size;
		for (uint i = 0; i < min_size; i++)
			buff[i] = data[i];
		return min_size;
	}
	void clear() {
		if (data != nullptr)delete[] data; data = nullptr;
	}
	static void replace(char* str, char what, char with, uint size) {
		for (uint i = 0; i < size; i++) {
			if (str[i] == what)
				str[i] = with;
		}
	}
	static uint size_of(const char* _data) {
		if (_data == nullptr)return 0;
		uint i = 0;
		while (_data[i++] != '\0');
		return i;
	}
	char* data = nullptr;
private:

};
*/