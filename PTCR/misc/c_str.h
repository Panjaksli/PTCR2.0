#pragma once
#include <stdint.h>
#include <iostream>
#include <memory>
#define uint unsigned int
//dynamic cstring, null terminated
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