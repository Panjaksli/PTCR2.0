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
	uint len()const { return size_of(_data) - 1; }
	char* begin() { return _data; }
	char* data() { return _data; }
	char* end() { return _data + size(); }
	const char* begin() const { return _data; }
	const char* data() const { return _data; }
	const char* end() const { return _data + size(); }
	bool empty() const { return size() <= 1; }
	const char* text() const { return empty() ? "0" : _data; }

	void set(const char var) {
		for (uint i = 0; i < max_len; i++)
			_data[i] = var;
	}

	const c_str& copy(const char* buff) {
		if (!buff) 
			return *this;
		uint i = 0;
		while (i < max_len && buff[i]) {
			_data[i] = buff[i];
			i++;
		}
		_data[i] = 0;
		return *this;
	}
	uint paste(char* buff, uint bsize)const {
		if (!buff || !bsize) return 0;
		uint i = 0;
		while (i < bsize - 1 && _data[i]) {
			buff[i] = _data[i];
			i++;
		}
		buff[i] = 0;
		return i;
	}
	const c_str& append(const char* buff) {
		if (!buff)
			return *this;
		uint i = 0;
		uint off = size() - 1;
		while (off + i < max_len && buff[i]) {
			_data[off + i] = buff[i];
			i++;
		}
		_data[off + i] = 0;
		return *this;
	}
	void replace(char what, char with) {
		replace(_data, what, with, max_len);
	}
	static void replace(char* str, char what, char with, uint size) {
		for (uint i = 0; i < size; i++) {
			if (str[i] == what)
				str[i] = with;
		}
	}
	void clear() {
		set(0);
	}
	void print()const { printf("%s\n", text()); }
	friend void swap(c_str& s1, c_str& s2) {
		std::swap(s1._data, s2._data);
	}
	friend c_str operator+(c_str s1, c_str s2) {
		s1.append(s2);
		return s1;
	}
	static uint size_of(const char* buff) {
		if (!buff)return 0;
		uint i = 0;
		while (buff[i++] != '\0');
		return i;
	}
	static constexpr uint max_size = 256;
	static constexpr uint max_len = max_size - 1;
private:
	char* _data = new char[max_size] {};
};
