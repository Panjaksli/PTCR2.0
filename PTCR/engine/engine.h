#pragma once
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>
#include "scenes.h"
#include "event_handler.h"
#include "defines.h"

class engine {
public:
	engine(double max_fps = 90, SDL_Rect screen = { 0,0,1280,720 }, SDL_Rect viewport = { 0,0,960,720 }, SDL_Rect menu = { 960,0, 320,720 });
	~engine();
	void resize();
	void process_input();
	void process_overlay();
	void draw_scene();
	void render_loop();
	void object_menu();
	void camera_menu();
	void add_object();
	void add_material();
	void delay(double sec) {
		//mid precision, good efficiency
		SDL_Delay(1000 * fmax(0, sec));
		//double t1 = timer();
		//while (timer(t1) < sec);
		//std::this_thread::sleep_for(std::chrono::duration<double>(sec));
	}
	double timer() {
		auto t = std::chrono::high_resolution_clock::now();
		return std::chrono::duration<double>(t.time_since_epoch()).count();
	}
	double timer(double t1) {
		auto t = std::chrono::high_resolution_clock::now();
		return std::chrono::duration<double>(t.time_since_epoch()).count() - t1;
	}
	double timer_ms(double t1) {
		return 1000.0 * (SDL_GetPerformanceCounter() - t1) / SDL_GetPerformanceFrequency();
	}
private:
	scene Scene;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* frame;
	SDL_PixelFormatEnum format;
	SDL_Rect screen;
	SDL_Rect viewport;
	SDL_Rect menu;
	SDL_Event event;
	event_handler handle;
	projection proj;
	mat4 T;
	vec3 TP = 0, TA = 0, TD = 0;
	double max_fps = 90;
	double fps = 0;
	double ft = 0;
	double nor_t = 0, gen_t = 0;
	double dt = 1;
	float sensitivity = 1.f;
	float scale = 1.f;
	float fogdens = 0;
	float move_mul = 1.f;
	int scn_n = 0;
	bool reproject = false;
	bool frame_id = false;
	bool den_view = true;
	bool overlay = true;
	bool tap_to_focus = false;
	bool running = true;
	bool add_obj = false;
	bool add_mat = false;
	//bool obj_delta = false;
};