#pragma once
#include <cstdio>
#include <iostream>
#include <vector>
template <typename T>
class darray {
public:
	darray() {}
	~darray() { free(); }
	darray(uint32_t size) : _data(new T[size]), _size(size), _capacity(size) {}
	darray(uint32_t size, const T& ref) : _data(new T[size]), _size(size), _capacity(size) {
		copy(_data, ref, _size);
	}
	darray(const std::vector<T>& ref) : _data(new T[ref.size()]), _size(ref.size()), _capacity(ref.size()) {
		copy(_data, ref.data(), _size);
	}
	darray(const darray& cpy) {
		_capacity = cpy._capacity;
		_size = cpy._size;
		_data = new T[_size];
		copy(_data, cpy._data, _size);
	}
	const darray& operator=(const darray& cpy) {
		_capacity = cpy._capacity;
		_size = cpy._size;
		_data = new T[_size];
		copy(_data, cpy._data, _size);
		return *this;
	}
	inline void erase(uint32_t idx) {
		_size--;
		for (uint32_t i = idx; i < size(); i++) {
			_data[i] = _data[i + 1];
		}
	}
	inline void push_back(const T& ref) {
		if (_size >= _capacity)reserve(2 * _size + (_size == 0));
		_data[_size++] = ref;
	}
	inline void emplace_back(const T& ref) {
		if (_size >= _capacity)reserve(2 * _size + (_size == 0));
		_data[_size++] = ref;
	}
	inline void emplace_back(T&& ref) {
		if (_size >= _capacity)reserve(2 * _size + (_size == 0));
		_data[_size++] = ref;
	}
	inline T& operator[](uint32_t i) {
		return _data[i];
	}
	inline const T& operator[](uint32_t i) const {
		return _data[i];
	}
	inline const T& front()const {
		return _data[0];
	}
	inline const T& back()const {
		return _data[_size - 1];
	}
	inline const T* begin()const {
		return _data;
	}
	inline const T* end()const {
		return _data + _size;
	}
	inline const T* data()const {
		return _data;
	}
	inline T& front() {
		return _data[0];
	}
	inline T& back() {
		return _data[_size - 1];
	}
	inline T* begin() {
		return _data;
	}
	inline T* end() {
		return _data + _size;
	}
	inline T* data() {
		return _data;
	}
	inline uint32_t size()const {
		return _size;
	}
	inline uint32_t capacity()const {
		return _capacity;
	}
	inline void clear() {
		_size = 0;
	}
	inline void free() {
		delete[] _data;
		_capacity = 0;
		_size = 0;
	}
	inline void shrink_to_fit() {
		T* tmp = new T[_size];
		copy(tmp, _data, _size);
		delete[] _data;
		_data = tmp;
		_capacity = _size;
	}
	inline void reserve(uint32_t size) {
		if (size > _capacity)
		{
			T* tmp = new T[size];
			copy(tmp, _data, _size);
			delete[] _data;
			_data = tmp;
			_capacity = size;
		}
	}
	inline void resize(uint32_t size) {
		reserve(size);
		_size = size;
	}
	inline void dbg() const {
		printf("%d %d\n", size(), capacity());
	}
	inline void print() const {
		for (uint32_t i = 0; i < size(); i++)
			std::cout << _data[i] << " ";
		printf("\n");
	}
private:
	static void copy(T* dst, const T* src, uint32_t size) {
		for (uint32_t i = 0; i < size; i++) dst[i] = src[i];
	}
	static void copy(T* dst, const T& src, uint32_t size) {
		for (uint32_t i = 0; i < size; i++) dst[i] = src;
	}
	T* _data = nullptr;
	uint32_t _size = 0;
	uint32_t _capacity = 0;

};
