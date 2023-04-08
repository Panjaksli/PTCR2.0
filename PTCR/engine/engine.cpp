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
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11");
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
	scn_load(Scene, 1);
}

void engine::resize() {
	reproject = 0;
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
	if (ImGui::GetIO().WantCaptureKeyboard) handle.reset_keys();
	if (handle[SDL_SCANCODE_LSHIFT])move_mul = 2.0;
	else move_mul = 1.0;
	vec3 movement = 10 * move_mul * vec3(handle[SDL_SCANCODE_W] - handle[SDL_SCANCODE_S], handle[SDL_SCANCODE_SPACE] - handle[SDL_SCANCODE_LCTRL], handle[SDL_SCANCODE_D] - handle[SDL_SCANCODE_A]);
	Scene.cam_move(movement, dt);
	if (handle[SDL_SCANCODE_F1])overlay = !overlay, handle[SDL_SCANCODE_F1] = 0;
	Scene.get_trans(T);
	TP = T.P(); TA = T.A(); TD = todeg(TA);
}

void engine::process_overlay() {
	if (overlay)
	{
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		camera_menu();
		object_menu();
		add_object();
		add_material();
		ImGui::Render();
	}
}
void engine::draw_scene() {
	uint* disp = nullptr;
	int pitch = viewport.w * 4;
	SDL_LockTexture(frame, 0, (void**)&disp, &pitch);
	if (reproject) {
		Scene.Reproject(proj, disp, pitch / 4);
	}
	else
	{
		proj = projection(Scene.cam.T, Scene.cam.CCD.w, Scene.cam.CCD.h, Scene.cam.tfov);
		Scene.Render(disp, pitch / 4);
	}
	if(Scene.opt.framegen)reproject = !reproject;
	SDL_UnlockTexture(frame);
	SDL_RenderCopy(renderer, frame, 0, &viewport);
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	SDL_RenderPresent(renderer);
}

void engine::add_object() {
	if (add_obj) {
		static int obj_type = 0;
		static bool is_mesh = false;
		static bool skip_bvh = false;
		static bool is_light = false;
		static bool is_foggy = false;
		static int mat = 0;
		static vec3 a, b, c, q;
		static vec3 offset = vec3(0, 0, 0, 1);
		static char filename[40];
		ImGui::Begin("Add object", &add_obj);
		ImGui::DragFloat4("Offscale", offset._xyz, 0.01f);
		ImGui::SliderInt("Mat##1", &mat, -1, Scene.world.materials.size() - 1, "%d", CLAMP);
		ImGui::Checkbox("No bvh##2", &skip_bvh);
		ImGui::SameLine();
		ImGui::Checkbox("Light##2", &is_light);
		ImGui::SameLine();
		ImGui::Checkbox("Foggy##2", &is_foggy);
		ImGui::SliderInt("Obj type", &obj_type, 0, 4, obj_enum_str(obj_type, 1), CLAMP);
		is_mesh = obj_type == 4;
		if (is_mesh) {
			skip_bvh = false;
			ImGui::InputText("Mesh name", filename, 40);
		}
		else if (obj_type == o_pol || obj_type == o_qua) {
			ImGui::DragFloat3("A", a._xyz, 0.01);
			ImGui::DragFloat3("B", b._xyz, 0.01);
			ImGui::DragFloat3("C", c._xyz, 0.01);
		}
		else if (obj_type == o_sph || obj_type == o_vox) {
			ImGui::DragFloat4("Q+a", q._xyz, 0.01);
		}
		if (ImGui::Button("Load")) {
			if (is_mesh) {
				Scene.world.add_mesh(load_mesh(filename), offset, offset.w(), mat, skip_bvh, is_light, is_foggy);
				if (!skip_bvh)Scene.opt.en_bvh = Scene.world.en_bvh = true;
			}
			else if (obj_type == o_pol) Scene.world.add_mesh(poly(a, b, c), offset, offset.w(), mat, skip_bvh, is_light, is_foggy);
			else if (obj_type == o_qua) Scene.world.add_mesh(quad(a, b, c), offset, offset.w(), mat, skip_bvh, is_light, is_foggy);
			else if (obj_type == o_sph)	Scene.world.add_mesh(sphere(q), offset, offset.w(), mat, skip_bvh, is_light, is_foggy);
			else if (obj_type == o_vox) Scene.world.add_mesh(voxel(q), offset, offset.w(), mat, skip_bvh, is_light, is_foggy);
			Scene.opt.selected = Scene.world.objects.size() - 1;
			Scene.world.update_all();
			Scene.cam.moving = true;
		}
		ImGui::End();
	}
}

void engine::add_material() {
	if (add_mat) {
		static int type = 0;
		static float ir = 1, scl = 1;
		static bool s1 = 1, s2 = 1, s3 = 1;
		static vec3 rgb(1), mer(0), nor(0.5, 0.5, 1), spec(0);
		static char srgb[40], smer[40], snor[40];
		ImGui::Begin("Add material", &add_mat);
		ImGui::SliderInt("Type", &type, 0, mat_cnt - 1, mat_enum_str(type));
		ImGui::Checkbox("RGB", &s1); ImGui::SameLine();
		s1 ? ImGui::ColorEdit4("##col1", rgb._xyz, ImGuiColorEditFlags_Float) : ImGui::InputText("##text1", srgb, 40);
		ImGui::Checkbox("MER", &s2); ImGui::SameLine();
		s2 ? ImGui::ColorEdit4("##col2", mer._xyz, ImGuiColorEditFlags_Float) : ImGui::InputText("##text2", smer, 40);
		ImGui::Checkbox("NOR", &s3); ImGui::SameLine();
		s3 ? ImGui::ColorEdit4("##col3", nor._xyz, ImGuiColorEditFlags_Float) : ImGui::InputText("##text3", snor, 40);
		ImGui::ColorEdit4("Specular", spec._xyz, ImGuiColorEditFlags_Float);
		if (ImGui::Button("Add")) {
			texture t1 = s1 ? texture(rgb) : texture(srgb);
			texture t2 = s2 ? texture(mer) : texture(smer);
			texture t3 = s3 ? texture(nor) : texture(snor);
			Scene.world.add_mat(albedo(t1, t2, t3, scl, ir, spec), mat_enum(type));
		}
		ImGui::End();
	}
}

void engine::camera_menu() {
	ImGui::SetNextWindowPos(ImVec2(menu.x, 0));
	ImGui::SetNextWindowSize(ImVec2(menu.w, menu.h / 2));
	ImGui::Begin("Camera settings");
	if (ImGui::Button("Screenshot")) {
		Scene.Screenshot(reproject);
	}
	static char scn_name[40];
	ImGui::InputText("SCN File", scn_name, 40);
	if (ImGui::Button("Load Scene")) {
		if (scn_load(Scene, scn_name, 0)) {
			scn_n = -1, Scene.opt.en_bvh = 1;
			reproject = false;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Update Scene")) {
		if (scn_load(Scene, scn_name, 1)) {
			scn_n = -1, Scene.opt.en_bvh = 1;
			reproject = false;
		}
	}
	if (ImGui::SliderInt("Scene", &scn_n, 1, no_scn)) {
		scn_load(Scene, scn_n);
		reproject = false;
		fogdens = log10f(-Scene.opt.ninv_fog);
	}
	vec3 pos = Scene.cam.T.P();
	if (ImGui::DragFloat3("Position", pos._xyz, 0.001))
		Scene.cam.set_P(pos);
	ImGui::DragFloat("Speed", &Scene.cam.speed, 0.001, 0.001, 1e3f, "%.2f");
	if (ImGui::DragFloat("FOV", &Scene.cam.fov, 0.1, 0.0001, 179, "%.2f"))
	{
		Scene.cam.moving = true;
		Scene.cam.update();
	}
	if (ImGui::Checkbox("Auto focus", &Scene.cam.autofocus))
	{
		if (Scene.cam.autofocus)tap_to_focus = 0;
		Scene.cam.moving = true;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Tap to focus", &tap_to_focus))
	{
		if (tap_to_focus)Scene.cam.autofocus = 0;
	}
	if (ImGui::DragFloat("Foc dist", &Scene.cam.foc_t, 0.001, 0.001, infp, "%.3f"))
	{
		Scene.cam.moving = true;
		Scene.cam.autofocus = 0;
	}
	Scene.cam.moving |= ImGui::DragFloat("Exposure", &Scene.cam.exposure, 0.1, 0.01, 100.f, "%.2f", ImGuiSliderFlags_Logarithmic);
	Scene.cam.moving |= ImGui::DragFloat("Lens correction", &Scene.cam.lens_cor, 0.001, -1.f, 1.f, "%.3f");
	Scene.cam.moving |= ImGui::DragFloat("F-stop", &Scene.cam.fstop, 0.1, 0.1f, 64.f, "%.2f", ImGuiSliderFlags_Logarithmic);
	if (ImGui::DragFloat("Res scale", &scale, 0.01, 0.1f, 4, " %.2f", CLAMP)) resize(), Scene.cam.moving = true;
	if (ImGui::DragFloat("Ray lifetime", &Scene.opt.p_life, 0.01, 0.01, 1.f, "%.2f", CLAMP))
	{
		Scene.opt.i_life = 1.f / Scene.opt.p_life;
		Scene.cam.moving = true;
	}
	ImGui::DragInt("Samples", &Scene.opt.samples, 0.1, 0, 1000, 0, CLAMP);
	Scene.cam.moving |= ImGui::DragInt("Bounces", &Scene.opt.bounces, 0.1, 0, 1000, 0, CLAMP);
	ImGui::DragFloat("MED filter", &Scene.opt.med_thr, 0.001f, 0.0f, 1.f, "%.3f", CLAMP);
	Scene.cam.moving |= ImGui::Checkbox("Light sampling", &Scene.opt.li_sa); ImGui::SameLine();
	Scene.cam.moving |= ImGui::Checkbox("Sun sampling", &Scene.opt.sun_sa);
	Scene.cam.moving |= ImGui::Checkbox("FPS/Freecam", &Scene.cam.free); ImGui::SameLine();
	Scene.cam.moving |= ImGui::Checkbox("Collision", &Scene.cam.collision);
	Scene.cam.moving |= ImGui::Checkbox("Denoise", &den_view); ImGui::SameLine();
	Scene.cam.moving |= ImGui::Checkbox("Bokeh", &Scene.cam.bokeh);
	Scene.cam.moving |= ImGui::Checkbox("Perf mode", &Scene.opt.p_mode); ImGui::SameLine();
	Scene.cam.moving |= ImGui::Checkbox("Resample", &Scene.opt.re_sam);
#if DEBUG
	Scene.cam.moving |= ImGui::Checkbox("Debug Aten", &Scene.opt.dbg_at); ImGui::SameLine();
	Scene.cam.moving |= ImGui::Checkbox("Debug N", &Scene.opt.dbg_n);
	Scene.cam.moving |= ImGui::Checkbox("Debug UV", &Scene.opt.dbg_uv);	ImGui::SameLine();
	Scene.cam.moving |= ImGui::Checkbox("Debug Face", &Scene.opt.dbg_f);
	Scene.cam.moving |= ImGui::Checkbox("Debug Edge", &Scene.opt.dbg_e);	ImGui::SameLine();
	Scene.cam.moving |= ImGui::Checkbox("Debug t", &Scene.opt.dbg_t);
	Scene.cam.moving |= ImGui::Checkbox("Debug lighting", &Scene.opt.dbg_light); ImGui::SameLine();
	if (Scene.opt.dbg_light)
	{
		ImGui::SameLine();
		Scene.cam.moving |= ImGui::Checkbox("Direct", &Scene.opt.dbg_direct);
	}
	Scene.cam.moving |= ImGui::Checkbox("Normal maps", &use_normal_maps);
	ImGui::Checkbox("Reproject", &reproject); ImGui::SameLine();
	//WIP
	if (ImGui::Checkbox("Framegen", &Scene.opt.framegen)){
		reproject = 0;
	}
#endif
	int maxfps = max_fps;
	ImGui::DragInt("Max FPS", &maxfps, 1, 10, 240);
	max_fps = maxfps;
	ImGui::DragInt("Max SPP", &Scene.opt.max_spp, 1000, 1, infp, "%d", ImGuiSliderFlags_Logarithmic);
	ImGui::Text("%.2f ms %.1f FPS,SPP %.3d", 1000.0f / fps, fps, (int)Scene.cam.CCD.spp); ImGui::SameLine();
	ImGui::Checkbox("Pause", &Scene.opt.paused);
	ImGui::End();
}

void engine::object_menu() {
	ImGui::SetNextWindowPos(ImVec2(menu.x, menu.h / 2));
	ImGui::SetNextWindowSize(ImVec2(menu.w, menu.h / 2));
	ImGui::Begin("World properties");
	static char filename[40];
	if (ImGui::InputText("Load skybox", filename, 40, ImGuiInputTextFlags_EnterReturnsTrue)) {
		Scene.set_skybox(albedo(filename, Scene.skybox.mer()));
		Scene.cam.moving = true;
	}
	Scene.cam.moving |= ImGui::Checkbox("Sky", &Scene.opt.sky); ImGui::SameLine();
	Scene.cam.moving |= ImGui::Checkbox("Skybox", &Scene.opt.skybox); ImGui::SameLine();
	Scene.cam.moving |= ImGui::Checkbox("Fog", &Scene.opt.en_fog);
	if (Scene.opt.en_fog) {
		if (ImGui::DragFloat("Fog density", &fogdens, -0.01, -3, 12, "1e-%.2f")) {
			Scene.opt.ninv_fog = -1.f * pow(10, fogdens); Scene.cam.moving = true;
		}
		Scene.cam.moving |= ImGui::ColorEdit3("Fog color", Scene.opt.fog_col._xyz, ImGuiColorEditFlags_Float);
	}
	if (Scene.opt.sky && !Scene.opt.skybox) {
		Scene.cam.moving |= ImGui::ColorEdit3("Sun noon", Scene.opt.sun_noon._xyz, ImGuiColorEditFlags_Float);
		Scene.cam.moving |= ImGui::ColorEdit3("Sun dawn", Scene.opt.sun_dawn._xyz, ImGuiColorEditFlags_Float);
		Scene.cam.moving |= ImGui::ColorEdit3("Sky noon", Scene.opt.sky_noon._xyz, ImGuiColorEditFlags_Float);
		Scene.cam.moving |= ImGui::ColorEdit3("Sky dawn", Scene.opt.sky_dawn._xyz, ImGuiColorEditFlags_Float);
	}
	ImGui::Checkbox("BVH", &Scene.opt.en_bvh);  ImGui::SameLine();
	if (ImGui::Checkbox("Linear", &Scene.world.bvh_lin)) {
		Scene.world.rebuild_bvh(1, Scene.opt.node_size);
	}
	ImGui::SameLine();
	if (ImGui::Button("Rebuild BVH")) {
		Scene.world.rebuild_bvh(1, Scene.opt.node_size);
		Scene.cam.moving = true;
	}
	if (ImGui::DragInt("Leaf nodes", &Scene.opt.node_size, 1, 1, 32, 0, CLAMP)) {
		Scene.world.rebuild_bvh(1, Scene.opt.node_size);
	}
	ImGui::Text("Object menu");
	if (ImGui::Button("Add object")) {
		add_obj = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Add material")) {
		add_mat = true;
	}
	ImGui::SliderInt("Object ID", &(int&)Scene.opt.selected, -1, Scene.world.objects.size() - 1);
	ImGui::Text("Type: %s", obj_enum_str(Scene.get_flag().type()));
	if (Scene.opt.selected < Scene.world.objects.size()) {
		uint id = Scene.opt.selected;
		mesh_var& obj = Scene.object_at(id);
		bool is_light = obj.light();
		bool is_inbvh = obj.bvh();
		bool is_foggy = obj.fog();
		vec3 DT = TA;
		ImGui::SameLine();
		if (ImGui::Button("Erase")) {
			Scene.cam.moving = true;
			Scene.world.remove_mesh(id);
		}
		ImGui::SameLine();
		if (ImGui::Button("Duplicate")) {
			Scene.cam.moving = true;
			Scene.world.duplicate_mesh(id);
		}
		if (ImGui::Checkbox("IN BVH", &is_inbvh)) {
			obj.flag.set_bvh(is_inbvh);
			Scene.world.update_all(Scene.opt.node_size); Scene.cam.moving = true;
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
		if (ImGui::DragFloat4("Pos", TP._xyz, 0.01))
		{
			Scene.set_trans(mat4(TP, TA));
			Scene.cam.moving = true;
		}
		DT = TA;
		if (ImGui::DragFloat3("Rot", TA._xyz, 0.01) && not0(DT - mod(TA, pi2)))
		{
			Scene.set_trans(mat4(TP, TA));
			Scene.cam.moving = true;
		}
		DT = TD;
		if (ImGui::DragFloat3("Rot (deg)", TD._xyz, 1, infn, infp, "%.2f") && not0(DT - mod(TD, 360)))
		{
			Scene.set_trans(mat4(TP, torad(TD)));
			Scene.cam.moving = true;
		}
		uint mat = obj.get_mat();
		int max_mat = Scene.world.materials.size();
		if (ImGui::SliderInt("Mat", &(int&)mat, -1, max_mat - 1)) {
			Scene.cam.moving = true;
			obj.set_mat(mat);
		}
		if (mat < Scene.world.materials.size()) {
			albedo& alb = Scene.world.materials[mat].tex;
			int type = (int)Scene.world.materials[mat].type;
			vec3 col = alb.get_rgb(), mer = alb.get_mer();
			if (ImGui::SliderInt("Mat type", &type, 0, mat_cnt - 1, mat_enum_str(type))) {
				Scene.cam.moving = true;
				Scene.world.materials[mat].type = (mat_enum)type;
			}
			ImGui::Text("Material: "); ImGui::SameLine();
			if (ImGui::Button("Erase##mat")) {
				Scene.cam.moving = true;
				Scene.world.remove_mat(mat);
			}
			ImGui::SameLine();
			if (ImGui::Button("Duplicate##mat")) {
				Scene.cam.moving = true;
				Scene.world.duplicate_mat(mat);
			}
			if (ImGui::ColorEdit4("Albedo", col._xyz, ImGuiColorEditFlags_Float)) {
				Scene.cam.moving = true;
				alb.set_rgb(col);
			}
			if (ImGui::ColorEdit3("MER", mer._xyz, ImGuiColorEditFlags_Float)) {
				Scene.cam.moving = true;
				alb.set_mer(mer);
			}
			Scene.cam.moving |= ImGui::ColorEdit4("Tint", alb.spec._xyz, ImGuiColorEditFlags_Float);
			Scene.cam.moving |= ImGui::DragFloat("Scale", &alb.rep, 1, 0);
			Scene.cam.moving |= ImGui::DragFloat("IOR", &alb.ir, 0.01f, 1.f, 4.f);
		}
	}
	else if(!Scene.opt.skybox&&Scene.opt.sky){
		if (ImGui::DragFloat3("Rot", TA._xyz, 0.01))
		{
			Scene.set_trans(mat4(TP, TA));
			Scene.cam.moving = true;
		}
		vec3 dTD = TD;
		if (ImGui::DragFloat3("Rot (deg)", TD._xyz, 1, 0, 359.999f, "%.2f", CLAMP) && not0(dTD - TD))
		{
			Scene.set_trans(mat4(TP, torad(TD)));
			Scene.cam.moving = true;
		}
	}
	ImGui::End();
}

void engine::render_loop() {
	while (running) {
		Scene.cam.moving |= !den_view;
		ft = SDL_GetPerformanceCounter();
		process_input();
		process_overlay();
		draw_scene();
		ft = (SDL_GetPerformanceCounter() - ft) / SDL_GetPerformanceFrequency();
		double delay = fmax(0, 1.0 / max_fps - ft);
		SDL_Delay(1e3 * delay);
		dt = ft + delay;//(SDL_GetPerformanceCounter() - dt) / SDL_GetPerformanceFrequency();
		avg_dt = 0.5 * (dt + avg_dt);
		fps = ImGui::GetIO().Framerate;
	}
}