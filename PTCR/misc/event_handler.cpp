#include "event_handler.h"
void event_handler::scan(SDL_Event& event)
{
	motion = 0;
	if (event.type == SDL_KEYDOWN)
	{
		int code = event.key.keysym.scancode;
		keys[code] = true;
	}
	if (event.type == SDL_KEYUP)
	{
		int code = event.key.keysym.scancode;
		keys[code] = false;
	}
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		uint code = event.button.button;
		if (code == SDL_BUTTON_LEFT)lmb = true;
		if (code == SDL_BUTTON_MIDDLE)mmb = true;
		if (code == SDL_BUTTON_RIGHT)rmb = true;

	}
	if (event.type == SDL_MOUSEBUTTONUP) {
		uint code = event.button.button;
		if (code == SDL_BUTTON_LEFT)lmb = false;
		if (code == SDL_BUTTON_MIDDLE)mmb = false;
		if (code == SDL_BUTTON_RIGHT)rmb = false;
	}
	if (event.type == SDL_MOUSEMOTION)
	{
		motion = 1;
		xvec = event.motion.xrel;
		yvec = event.motion.yrel;
	}
	if (event.type == SDL_MOUSEWHEEL)
	{

	}
}
bool event_handler::operator[](uint code)const {
	return keys[code];
}
bool& event_handler::operator[](uint code) {
	return keys[code];
}

bool event_handler::any_mb() {
	return lmb || mmb || rmb;
}
bool event_handler::any_kb() {
	bool res = false;
	for (int i = 0; i < 512; i++)
		res = res || keys[i];
	return res;
}


void event_handler::reset_mouse() {
	lmb = 0, mmb = 0, rmb = 0;
}
void event_handler::reset_all() {
	for (uint i = 0; i < 512; i++)
		keys[i] = false;
	lmb = 0, mmb = 0, rmb = 0;
}
void event_handler::reset_keys(std::initializer_list<int> key) {
	for (auto i : key)
		keys[i] = false;
}
void event_handler::reset_keys() {
	for (int i = 0; i < 512; i++)
		keys[i] = false;
}