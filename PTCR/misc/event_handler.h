#pragma once
#include <SDL.h>
#include <initializer_list>
#include "util.h"

class event_handler {
public:
	void scan(SDL_Event& event);
	bool operator[](uint code)const;
	bool& operator[](uint code);
	bool any_mb();
	bool any_kb();
	void reset_mouse();
	void reset_all();
	void reset_keys(std::initializer_list<int> key);
	void reset_keys();
	float xvec = 0, yvec = 0;
	bool keys[512] = {};
	bool lmb = 0, mmb = 0, rmb = 0;
	bool wheel = 0, motion = 0;
};

