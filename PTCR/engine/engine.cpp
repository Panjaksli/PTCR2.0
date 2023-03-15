#include "engine.h"

#if DEBUG
bool use_normal_maps = 1;
#endif

engine::~engine() {
	SDL_DestroyTexture(frame);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

engine::engine(double max_fps, SDL_Rect screen, SDL_Rect viewport, SDL_Rect menu) : Scene(viewport.w, viewport.h, 90), screen(screen), viewport(viewport), menu(menu), max_fps(max_fps) {
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d9");
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
	window = SDL_CreateWindow("PTCR", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen.w, screen.h, SDL_WINDOW_RESIZABLE);
	SDL_SetWindowMinimumSize(window, 200, 200);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	format = SDL_PIXELFORMAT_ARGB8888;
	frame = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, viewport.w, viewport.h);
	SDL_SetTextureScaleMode(frame, SDL_ScaleModeBest);
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer_Init(renderer);
	scn_load(Scene, scn_n);
}

void engine::resize() {
	viewport = { 0,0,3 * screen.w / 4,screen.h };
	menu = { 3 * screen.w / 4,0, screen.w / 4,screen.h };
	Scene.cam.resize(viewport.w, viewport.h, scale);
	SDL_DestroyTexture(frame);
	frame = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, Scene.cam.w, Scene.cam.h);
	SDL_SetTextureScaleMode(frame, SDL_ScaleModeBest);
}
void engine::process_input() {
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
		handle.scan(event);
		if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE))
			running = false;
		if (event.type == SDL_WINDOWEVENT)
		{
			if (event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
				running = false;
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				screen = { 0,0,event.window.data1, event.window.data2 };
				resize();
			}
		}

		if (!ImGui::GetIO().WantCaptureMouse) {
			if (handle.rmb) {
				int x, y;
				SDL_GetMouseState(&x, &y);
				Scene.get_id(scale * y, scale * x);
				if (tap_to_focus)
					Scene.cam_manufocus(scale * y, scale * x);
			}
			if (handle.motion && handle.lmb)
				Scene.cam.rotate(0, -torad(0.9 * handle.xvec), -torad(0.9 * handle.yvec));
		}
	}
	if (handle[SDL_SCANCODE_LSHIFT])move_mul = 2.0;
	else move_mul = 1.0;
	vec3 movement = 10 * move_mul * vec3(handle[SDL_SCANCODE_W] - handle[SDL_SCANCODE_S], handle[SDL_SCANCODE_SPACE] - handle[SDL_SCANCODE_LCTRL], handle[SDL_SCANCODE_D] - handle[SDL_SCANCODE_A]);
	Scene.cam_move(movement, dt);
	if (handle[SDL_SCANCODE_F1])overlay = !overlay, handle[SDL_SCANCODE_F1] = 0;
	Scene.get_trans(T);
	TP = T.P(); TA = T.A();
}

void engine::process_overlay() {
	if (overlay)
	{
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		ImGui::SetNextWindowPos(ImVec2(menu.x, 0));
		ImGui::SetNextWindowSize(ImVec2(menu.w, menu.h / 2));
		ImGui::Begin("Camera settings");
		ImGui::Text("%.4f %.4f %.4f", Scene.cam.T.P().x(), Scene.cam.T.P().y(), Scene.cam.T.P().z());
		if (ImGui::Button("Screenshot")) {
			Scene.Screenshot();
		}
		if (ImGui::SliderInt("Scene", &scn_n, 1, no_scn)) {
			scn_load(Scene, scn_n);
			fogdens = log10f(-Scene.opt.ninv_fog);
		}
		ImGui::DragFloat("Speed", &Scene.cam.speed, 0.001, 0.001, 1e3f, "% .2f");
		if (ImGui::DragFloat("FOV", &Scene.cam.fov, 0.1, 0.001, 179, "%.1f"))
		{
			Scene.cam.moving = 1;
			Scene.cam.update();
		}
		if (ImGui::Checkbox("Auto focus", &Scene.cam.autofocus))
		{
			if (Scene.cam.autofocus)tap_to_focus = 0;
			Scene.cam.moving = 1;
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Tap to focus", &tap_to_focus))
		{
			if (tap_to_focus)Scene.cam.autofocus = 0;
		}
		if (ImGui::InputFloat("Foc dist", &Scene.cam.foc_t, 0.001, 0.001))
		{
			Scene.cam.moving = 1;
			Scene.cam.autofocus = 0;
		}
		Scene.cam.moving |= ImGui::DragFloat("Exposure", &Scene.cam.exposure, 0.1, 0.01, 100.f, "% .2f");
		Scene.cam.moving |= ImGui::DragFloat("Lens correction", &Scene.cam.lens_cor, 0.001, -1.f, 1.f, "% .3f");
		Scene.cam.moving |= ImGui::DragFloat("F-stop", &Scene.cam.fstop, 0.1, 0.1f, 64.f, "% .2f");
		if (ImGui::DragFloat("Res scale", &scale, 0.01, 0.1, 4, " % .2f", CLAMP)) resize(), Scene.cam.moving = true;
		if (ImGui::DragFloat("Ray lifetime", &Scene.opt.p_life, 0.01, 0.01, 1.f, "%.2f", CLAMP))
			Scene.opt.i_life = 1.f / Scene.opt.p_life, Scene.cam.moving = 1;
		ImGui::DragInt("Samples", &Scene.opt.samples, 0.1, 0, 100, 0, CLAMP);
		Scene.cam.moving |= ImGui::DragInt("Bounces", &Scene.opt.bounces, 0.1, 0, 1000, 0, CLAMP);
		ImGui::DragFloat("MED filter", &Scene.opt.med_thr, 0.001f, 0.0f, 1.f, "% .3f", CLAMP);
		Scene.cam.moving |= ImGui::Checkbox("Light sampling", &Scene.opt.li_sa); ImGui::SameLine();
		Scene.cam.moving |= ImGui::Checkbox("Sun sampling", &Scene.opt.sun_sa);
		Scene.cam.moving |= ImGui::Checkbox("FPS/Freecam", &Scene.cam.free); ImGui::SameLine();
		Scene.cam.moving |= ImGui::Checkbox("Bokeh", &Scene.cam.bokeh);
		Scene.cam.moving |= ImGui::Checkbox("Collision", &Scene.cam.collision);  ImGui::SameLine();
		Scene.cam.moving |= ImGui::Checkbox("Perf mode", &Scene.opt.p_mode);
		Scene.cam.moving |= ImGui::Checkbox("MC", &den_view); ImGui::SameLine();
		Scene.cam.moving |= ImGui::Checkbox("Resample", &Scene.opt.re_sam);
#if DEBUG
		Scene.cam.moving |= ImGui::Checkbox("Debug Aten", &Scene.opt.dbg_at); ImGui::SameLine();
		Scene.cam.moving |= ImGui::Checkbox("Debug N", &Scene.opt.dbg_n);
		Scene.cam.moving |= ImGui::Checkbox("Debug UV", &Scene.opt.dbg_uv);	ImGui::SameLine();
		Scene.cam.moving |= ImGui::Checkbox("Debug t", &Scene.opt.dbg_t);
		Scene.cam.moving |= ImGui::Checkbox("Normal maps", &use_normal_maps); 
		Scene.cam.moving |= !den_view;
#endif
		ImGui::Text("%.2f ms %.1f FPS,SPP %.1fk", 1000.0f / fps, fps, Scene.cam.CCD.spp * 0.001f);
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(menu.x, menu.h / 2));
		ImGui::SetNextWindowSize(ImVec2(menu.w, menu.h / 2));

		ImGui::Begin("World properties");
		Scene.cam.moving |= ImGui::Checkbox("Sky", &Scene.opt.sky); ImGui::SameLine();
		Scene.cam.moving |= ImGui::Checkbox("Skybox", &Scene.opt.skybox); ImGui::SameLine();
		Scene.cam.moving |= ImGui::Checkbox("Fog", &Scene.opt.en_fog);
		if (Scene.opt.en_fog) {
			if (ImGui::DragFloat("Fog density", &fogdens, -0.01, -1, 12, "1e-%.2f", CLAMP)) {
				Scene.opt.ninv_fog = -1.f * pow(10, fogdens); Scene.cam.moving = 1;
			}
			Scene.cam.moving |= ImGui::ColorEdit3("Fog color", Scene.opt.fog_col._xyz, ImGuiColorEditFlags_Float);
		}
		if (Scene.opt.sky&&!Scene.opt.skybox) {
			Scene.cam.moving |= ImGui::ColorEdit3("Sun noon", Scene.opt.sun_noon._xyz, ImGuiColorEditFlags_Float);
			Scene.cam.moving |= ImGui::ColorEdit3("Sun dawn", Scene.opt.sun_dawn._xyz, ImGuiColorEditFlags_Float);
			Scene.cam.moving |= ImGui::ColorEdit3("Sky noon", Scene.opt.sky_noon._xyz, ImGuiColorEditFlags_Float);
			Scene.cam.moving |= ImGui::ColorEdit3("Sky dawn", Scene.opt.sky_dawn._xyz, ImGuiColorEditFlags_Float);
		}
		ImGui::Checkbox("BVH", &Scene.opt.en_bvh);  ImGui::SameLine();
		if (ImGui::Checkbox("Linear", &Scene.world.bvh_lin)) {
			Scene.world.rebuild_bvh(1, Scene.opt.node_size);
		}
		//Scene.cam.moving |= ImGui::Checkbox("Ray march", &Scene.world.march);
		if (ImGui::DragInt("Leaf nodes", &Scene.opt.node_size, 1, 1, 32, 0, CLAMP)) {
			Scene.world.rebuild_bvh(1, Scene.opt.node_size);
		}
		ImGui::SliderInt("Object", &Scene.opt.selected, -1, Scene.world.objects.size() - 1);
		ImGui::Text("Type: %d", Scene.get_flag().type());
		if (Scene.opt.selected != -1) {
			int id = Scene.opt.selected;
			mesh_var& obj = Scene.world.objects[id];
			bool is_light = obj.light();
			bool is_inbvh = obj.bvh();
			bool is_foggy = obj.fog();
			if (ImGui::Checkbox("IN BVH", &is_inbvh)) {
				obj.flag.set_bvh(is_inbvh);
				Scene.world.update_nonbvh(Scene.opt.node_size); Scene.cam.moving = true;
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Importance", &is_light)) {
				obj.flag.set_lig(is_light);
				Scene.world.update_lights(); Scene.cam.moving = true;
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Foggy", &is_foggy)) {
				obj.flag.set_fog(is_foggy);
				Scene.cam.moving = true;
			}
			if (ImGui::Button("Erase")) {
				Scene.cam.moving = true;
				Scene.world.remove_mesh(id);
				Scene.opt.selected = Scene.opt.selected < Scene.world.objects.size() ? Scene.opt.selected : Scene.world.objects.size() - 1;
			}
			albedo& alb = Scene.world.materials[obj.get_mat()].tex;
			int type = (int)Scene.world.materials[obj.get_mat()].type;
			vec3 col = alb.get_rgb(), mer = alb.get_mer();
			Scene.cam.moving |= ImGui::SliderInt("Mat", &type, 0, material::mat_cnt - 1);
			Scene.cam.moving |= ImGui::InputFloat("Rep", &alb.get_rep(), 1, 10);
			Scene.cam.moving |= ImGui::ColorEdit4("Col", col._xyz, ImGuiColorEditFlags_Float);
			Scene.cam.moving |= ImGui::ColorEdit3("Mer", mer._xyz, ImGuiColorEditFlags_Float);
			Scene.cam.moving |= ImGui::DragFloat("Ir", &alb.get_ir(), 0.01f, 1.f, 4.f);
			Scene.world.materials[obj.get_mat()].type = (mat_enum)type;
			alb.set_rgb(col);
			alb.set_mer(mer);
		}
		if (ImGui::DragFloat4("Pos", TP._xyz, 0.01))
		{
			Scene.set_trans(mat4(TP, TA));
			Scene.cam.moving = 1;
		}
		if (ImGui::DragFloat3("Rot", TA._xyz, 0.01))
		{
			Scene.set_trans(mat4(TP, TA));
			Scene.cam.moving = 1;
		}
		ImGui::End();
		ImGui::Render();
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	}
}
void engine::draw_scene() {
	uint* disp = nullptr;
	int pitch = viewport.w * 4;
	SDL_LockTexture(frame, 0, (void**)&disp, &pitch);
	Scene.Render(disp, pitch / 4);
	SDL_UnlockTexture(frame);
	SDL_RenderCopy(renderer, frame, 0, &viewport);
	SDL_RenderPresent(renderer);
}

void engine::render_loop() {
	while (running) {
		ft = SDL_GetPerformanceCounter();
		process_input();
		process_overlay();
		draw_scene();
		ft = (SDL_GetPerformanceCounter() - ft) / SDL_GetPerformanceFrequency();
		double delay = fmax(0, 1.0 / max_fps - ft);
		SDL_Delay(1e3 * delay);
		dt = ft + delay;//(SDL_GetPerformanceCounter() - dt) / SDL_GetPerformanceFrequency();
		fps = ImGui::GetIO().Framerate;
	}
}