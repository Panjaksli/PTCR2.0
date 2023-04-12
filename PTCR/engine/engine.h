#pragma once
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>
#include <chrono>
#include "scenes.h"
#include "event_handler.h"
#include "defines.h"
//Using following naming conventions:
//What is super important class/function = capital (Scene,Engine, Render(), etc...)
//What is 20x nested or POD or math related = non-capital (basically everyhing else)
//Sometimes it doesn't match neither, then I follow every other C/C++ math/graphics library (do whatever you want)
namespace PTCR
{
class Engine {
public:
	Engine(double max_fps = 90, SDL_Rect screen = { 0,0,1280,720 }, SDL_Rect viewport = { 0,0,960,720 }, SDL_Rect menu = { 960,0, 320,720 });
	~Engine();
	void Resize();
	void Process_input();
	void Process_overlay();
	void Draw_scene();
	void Render_loop();
	void Object_menu();
	void Camera_menu();
	void Add_object();
	void Add_material();
	static void delay(double sec) {
		//mid precision, good efficiency
		SDL_Delay(1000 * fmax(0, sec));
	}
	static double timer() {
		auto t = std::chrono::high_resolution_clock::now();
		return std::chrono::duration<double>(t.time_since_epoch()).count();
	}
	static double timer(double t1) {
		auto t = std::chrono::high_resolution_clock::now();
		return std::chrono::duration<double>(t.time_since_epoch()).count() - t1;
	}
	static double timer_ms(double t1) {
		return 1000.0 * (SDL_GetPerformanceCounter() - t1) / SDL_GetPerformanceFrequency();
	}
private:
	Scene scene;
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
}