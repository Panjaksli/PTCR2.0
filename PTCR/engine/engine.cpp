#include "Engine.h"
namespace PTCR
{

	Engine::~Engine() {
		SDL_DestroyTexture(frame);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	Engine::Engine(double max_fps, SDL_Rect screen, SDL_Rect viewport, SDL_Rect menu) : scene(viewport.w, viewport.h, 90), screen(screen), viewport(viewport), menu(menu), max_fps(max_fps) {
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
		scn_load(scene, 1);
	}

	void Engine::Resize() {
		reproject = 0;
		viewport = { 0,0,3 * screen.w / 4,screen.h };
		menu = { 3 * screen.w / 4,0, screen.w / 4,screen.h };
		scene.cam.resize(viewport.w, viewport.h, scale);
		SDL_DestroyTexture(frame);
		frame = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, scene.cam.w, scene.cam.h);
		SDL_SetTextureScaleMode(frame, SDL_ScaleModeBest);
	}
	void Engine::Process_input() {
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
					Resize();
				}
			}
			if (event.type == SDL_DROPFILE) {
				string name = event.drop.file;
				if (name.find(".scn") != std::string::npos) scn_load(scene, name.c_str());
				else if (name.find(".msh") != std::string::npos || name.find(".obj") != std::string::npos) {
					int x, y;
					SDL_GetMouseState(&x, &y);
					vec4 P = scene.get_point(scale * y, scale * x, 1);
					if (scene.world.load_mesh(name.c_str(), vec4(P, 1), -1))
					{
						scene.opt.selected = scene.world.objects.size() - 1;
						scene.world.build_bvh();
						scene.cam.moving = true;
					}
				}
			}
			if (!ImGui::GetIO().WantCaptureMouse) {
				int x, y;
				SDL_GetMouseState(&x, &y);
				if (handle.mmb || (handle.rmb && handle[SDL_SCANCODE_LALT])) {
					scene.get_id(scale * y, scale * x);
					if (tap_to_focus)
						scene.cam_manufocus(scale * y, scale * x);
				}
				else if (handle.motion && handle.rmb && scene.opt.selected != -1) {
					float dist = (scene.selected().get_box().pmid() - scene.cam.P).len();
					vec4 dir = vec4(event.motion.xrel, -event.motion.yrel, 0, 0) / vec4(viewport.w, viewport.h, 1, 1);
					vec4 trans = scene.selected().get_trans().P() + scene.cam.T.vec(2 * dist * dir);
					vec4 rot = scene.selected().get_trans().A();
					trans[3] = scene.selected().get_trans().P()[3];
					scene.set_trans(mat4(trans,rot));
					scene.cam.moving = true;
				}
				if (handle.motion && handle.lmb)
					scene.cam.rotate(sensitivity * torad(vec4(0, -handle.xvec, -handle.yvec)));
			}
		}
		if (ImGui::GetIO().WantCaptureKeyboard) handle.reset_keys();
		if (handle[SDL_SCANCODE_LSHIFT])move_mul = 2.0;
		else move_mul = 1.0;
		vec4 dir = 10 * move_mul * vec4(handle[SDL_SCANCODE_W] - handle[SDL_SCANCODE_S], handle[SDL_SCANCODE_SPACE] - handle[SDL_SCANCODE_LCTRL], handle[SDL_SCANCODE_D] - handle[SDL_SCANCODE_A]);
		vec4 pan = vec4(0, handle[SDL_SCANCODE_LEFT] - handle[SDL_SCANCODE_RIGHT], handle[SDL_SCANCODE_UP] - handle[SDL_SCANCODE_DOWN]);
		scene.cam_move(dir, dt);
		scene.cam.rotate(sensitivity * pan * dt);
		if (handle[SDL_SCANCODE_F1])overlay = !overlay, handle[SDL_SCANCODE_F1] = 0;
		scene.get_trans(T);
		TP = T.P(); TA = T.A(); TD = todeg(TA);
	}

	void Engine::Process_overlay() {
		if (overlay) {
			ImGui_ImplSDLRenderer_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();
			Camera_menu();
			Object_menu();
			Add_object();
			Add_material();
			ImGui::Render();
		}
	}

	void Engine::Render() {
		ft = timer();
		uint* disp = nullptr;
		int pitch = viewport.w * 4;
		scene.cam.moving |= !den_view;
		SDL_LockTexture(frame, 0, (void**)&disp, &pitch);
		if (!scene.opt.framegen || reproject) {
			if (reproject) {
				scene.Reproject(proj, disp, pitch / 4);
				scene.cam.moving = false;
			}
			else {
				proj = projection(scene.cam.T, scene.cam.CCD.w, scene.cam.CCD.h, scene.cam.tfov);
				scene.Render(disp, pitch / 4);
			}
			SDL_UnlockTexture(frame);
			SDL_RenderCopy(renderer, frame, 0, &viewport);
			ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
			SDL_RenderPresent(renderer);
		}
		else
		{
			//static double tmr = 0;
			if (frame_id) {
				gen_t = timer();
				scene.Reproject(proj, disp, pitch / 4);
				gen_t = timer(gen_t);
				SDL_UnlockTexture(frame);
				SDL_RenderCopy(renderer, frame, 0, &viewport);
				ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
				double delay = fmax(0, nor_t - gen_t);
				Delay(delay);
				SDL_RenderPresent(renderer);
			}
			else {
				proj = projection(scene.cam.T, scene.cam.CCD.w, scene.cam.CCD.h, scene.cam.tfov);
				nor_t = timer();
				scene.Render(disp, pitch / 4);
				nor_t = timer(nor_t);
				SDL_UnlockTexture(frame);
				SDL_RenderCopy(renderer, frame, 0, &viewport);
				ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
				SDL_RenderPresent(renderer);
			}
			frame_id = !frame_id;
		}
		ft = timer(ft);
	}

	void Engine::Add_object() {
		if (add_obj) {
			static int obj_type = 0;
			static bool is_mesh = false;
			static bool is_bvh = true;
			static bool is_light = false;
			static bool is_foggy = false;
			static int mat = 0;
			static vec4 a, b, c, q;
			static vec4 offset = vec4(0, 0, 0, 1);
			static vec4 angle = vec4(0, 0, 0, 0);
			mat4 T;
			static char filename[40];
			ImGui::Begin("Add object", &add_obj);
			ImGui::DragFloat4("Offscale", offset._xyz, 0.001f);
			ImGui::DragFloat4("Rotation", angle._xyz, 0.01f);
			ImGui::SliderInt("Mat##1", &mat, -1, scene.world.materials.size() - 1, "%d", CLAMP);
			ImGui::Checkbox("No bvh##2", &is_bvh);
			ImGui::SameLine();
			ImGui::Checkbox("Light##2", &is_light);
			ImGui::SameLine();
			ImGui::Checkbox("Foggy##2", &is_foggy);
			ImGui::SliderInt("Obj type", &obj_type, 0, 4, obj_enum_str(obj_type, 1), CLAMP);
			is_mesh = obj_type == 4;
			T = mat4(offset, angle);
			if (is_mesh) {
				is_bvh = true;
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
					scene.world.load_mesh(filename, T, mat, is_bvh, is_light, is_foggy);
					if (is_bvh)scene.opt.en_bvh = scene.world.en_bvh = true;
				}
				else if (obj_type == o_pol) scene.world.add_mesh(poly(a, b, c), T, mat, is_bvh, is_light, is_foggy);
				else if (obj_type == o_qua) scene.world.add_mesh(quad(a, b, c), T, mat, is_bvh, is_light, is_foggy);
				else if (obj_type == o_sph)	scene.world.add_mesh(sphere(q), T, mat, is_bvh, is_light, is_foggy);
				else if (obj_type == o_vox) scene.world.add_mesh(voxel(q), T, mat, is_bvh, is_light, is_foggy);
				scene.opt.selected = scene.world.objects.size() - 1;
				scene.world.update_all();
				scene.cam.moving = true;
			}
			ImGui::End();
		}
	}

	void Engine::Add_material() {
		if (add_mat) {
			static int type = 0;
			static float ir = 1, scl = 1;
			static bool s1 = 1, s2 = 1, s3 = 1;
			static vec4 rgb(1), mer(0), nor(0.5, 0.5, 1), tint(0);
			static char srgb[40], smer[40], snor[40];
			ImGui::Begin("Add material", &add_mat);
			ImGui::SliderInt("Type", &type, 0, mat_cnt - 1, mat_enum_str(type));
			ImGui::Checkbox("RGB", &s1); ImGui::SameLine();
			s1 ? ImGui::ColorEdit4("##col1", rgb._xyz, ImGuiColorEditFlags_Float) : ImGui::InputText("##text1", srgb, 40);
			ImGui::Checkbox("MER", &s2); ImGui::SameLine();
			s2 ? ImGui::ColorEdit3("##col2", mer._xyz, ImGuiColorEditFlags_Float) : ImGui::InputText("##text2", smer, 40);
			ImGui::Checkbox("NOR", &s3); ImGui::SameLine();
			s3 ? ImGui::ColorEdit3("##col3", nor._xyz, ImGuiColorEditFlags_Float) : ImGui::InputText("##text3", snor, 40);
			ImGui::ColorEdit4("Tint", tint._xyz, ImGuiColorEditFlags_Float);
			if (ImGui::Button("Add")) {
				texture t1 = s1 ? texture(rgb) : texture(srgb);
				texture t2 = s2 ? texture(mer) : texture(smer);
				texture t3 = s3 ? texture(nor) : texture(snor);
				scene.world.add_mat(albedo(t1, t2, t3, scl, ir, tint), mat_enum(type));
			}
			ImGui::End();
		}
	}

	void Engine::Camera_menu() {
		ImGui::SetNextWindowPos(ImVec2(menu.x, 0));
		ImGui::SetNextWindowSize(ImVec2(menu.w, menu.h / 2));
		ImGui::Begin("Camera settings");
		static char scn_name[40];
		if (ImGui::InputText("SCN File", scn_name, 40, ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::Button("Load scene")) {
			if (scn_load(scene, scn_name, 0)) {
				scn_n = -1, scene.opt.en_bvh = 1;
				reproject = false;
			}
			else {
				scn_save(scene, scn_name);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Update scene")) {
			if (scn_load(scene, scn_name, 1)) {
				scn_n = -1, scene.opt.en_bvh = 1;
				reproject = false;
			}
		}
		if (ImGui::Button("Save scene")) {
			scn_save(scene, scn_name);
		}
		ImGui::SameLine();
		if (ImGui::Button("Screenshot")) {
			scene.Screenshot(reproject);
		}
		if (ImGui::SliderInt("scene", &scn_n, 1, no_scn)) {
			scn_load(scene, scn_n);
			reproject = false;
			fogdens = log10f(-scene.opt.ninv_fog);
		}
		vec4 pos = vec4(scene.cam.T.P(), scene.cam.speed);
		vec4 rot = vec4(scene.cam.T.A(), sensitivity);
		if (ImGui::DragFloat4("Position##cam", pos._xyz, 0.001, infp, infp, "%g"))
		{
			scene.cam.set_P(pos);
			scene.cam.speed = pos[3];
		}
		if (ImGui::DragFloat4("Rotation##cam", rot._xyz, 0.001, infp, infp, "%g"))
		{
			scene.cam.set_A(rot);
			sensitivity = rot[3];
		}
		if (ImGui::DragFloat("FOV", &scene.cam.fov, 0.1, 0.0001, 179, "%g"))
		{
			scene.cam.moving = true;
			scene.cam.update();
		}
		if (ImGui::Checkbox("Auto focus", &scene.cam.autofocus))
		{
			if (scene.cam.autofocus)tap_to_focus = 0;
			scene.cam.moving = true;
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Tap to focus", &tap_to_focus))
		{
			if (tap_to_focus)scene.cam.autofocus = 0;
		}
		if (ImGui::DragFloat("Foc dist", &scene.cam.foc_t, 0.001, 0.001, infp, "%g"))
		{
			scene.cam.moving = true;
			scene.cam.autofocus = 0;
		}
		scene.cam.moving |= ImGui::DragFloat("Exposure", &scene.cam.exposure, 0.1, 0.01, 100.f, "%g", ImGuiSliderFlags_Logarithmic);
		//scene.cam.moving |= ImGui::DragFloat("Lens correction", &scene.cam.lens_cor, 0.001, -1.f, 1.f, "%g");
		scene.cam.moving |= ImGui::DragFloat("F-stop", &scene.cam.fstop, 0.1, 0.1f, 64.f, "%g", ImGuiSliderFlags_Logarithmic);
		if (ImGui::DragFloat("Res scale", &scale, 0.01, 0.1f, 4, " %g", CLAMP)) Resize(), scene.cam.moving = true;
		if (ImGui::DragFloat("Ray lifetime", &scene.opt.p_life, 0.01, 0.01, 1.f, "%g", CLAMP))
		{
			scene.opt.i_life = 1.f / scene.opt.p_life;
			scene.cam.moving = true;
		}
		ImGui::DragInt("Samples", &scene.opt.samples, 0.1, 0, 1000, 0, CLAMP);
		scene.cam.moving |= ImGui::DragInt("Bounces", &scene.opt.bounces, 0.1, 0, 1000, 0, CLAMP);
		ImGui::DragFloat("MED filter", &scene.opt.med_thr, 0.001f, 0.0f, 1.f, "%g", CLAMP);
		scene.cam.moving |= ImGui::Checkbox("Light sampling", &scene.opt.li_sa); ImGui::SameLine();
		scene.cam.moving |= ImGui::Checkbox("Sun sampling", &scene.opt.sun_sa);
		scene.cam.moving |= ImGui::Checkbox("FPS/Freecam", &scene.cam.free); ImGui::SameLine();
		scene.cam.moving |= ImGui::Checkbox("Collision", &scene.cam.collision);
		scene.cam.moving |= ImGui::Checkbox("Denoise", &den_view); ImGui::SameLine();
		scene.cam.moving |= ImGui::Checkbox("Bokeh", &scene.cam.bokeh);
		scene.cam.moving |= ImGui::Checkbox("Perf mode", &scene.opt.p_mode); ImGui::SameLine();
		scene.cam.moving |= ImGui::Checkbox("Resample", &scene.opt.re_sam);
#if DEBUG
		scene.cam.moving |= ImGui::Checkbox("Debug Aten", &scene.opt.dbg_at); ImGui::SameLine();
		scene.cam.moving |= ImGui::Checkbox("Debug N", &scene.opt.dbg_n);
		scene.cam.moving |= ImGui::Checkbox("Debug UV", &scene.opt.dbg_uv);	ImGui::SameLine();
		scene.cam.moving |= ImGui::Checkbox("Debug Face", &scene.opt.dbg_f);
		scene.cam.moving |= ImGui::Checkbox("Debug BVH", &scene.opt.dbg_bvh); ImGui::SameLine();
		scene.cam.moving |= ImGui::Checkbox("Debug Edge", &scene.opt.dbg_e);
		scene.cam.moving |= ImGui::Checkbox("Debug t", &scene.opt.dbg_t); ImGui::SameLine();
		scene.cam.moving |= ImGui::Checkbox("Debug lighting", &scene.opt.dbg_light);
		scene.cam.moving |= ImGui::Checkbox("Normal maps", &use_normal_maps);
		scene.cam.moving |= ImGui::Checkbox("Reproject", &reproject);
		ImGui::SameLine();
		//WIP
		ImGui::Checkbox("Framegen", &scene.opt.framegen);
#endif
		int maxfps = max_fps;
		ImGui::DragInt("Max FPS", &maxfps, 1, 10, 240);
		max_fps = maxfps;
		ImGui::DragInt("Max SPP", &scene.opt.max_spp, 1000, 1, infp, "%d", ImGuiSliderFlags_Logarithmic);
		ImGui::Text("%.2f ms %.2f FPS,SPP %.3d", 1000.0f / fps, fps, (int)scene.cam.CCD.spp); ImGui::SameLine();
		ImGui::Checkbox("Pause", &scene.opt.paused);
		ImGui::Text("%g FPS %g ms %d frames", avg_fps / fps_cnt, 1000.0f * fps_cnt / avg_fps, fps_cnt); ImGui::SameLine();
		if (ImGui::Button("Measure perf")) {
			avg_fps = 0;
			fps_cnt = 0;
			fps_meas = true;
		}
		ImGui::End();
	}

	void Engine::Object_menu() {
		ImGui::SetNextWindowPos(ImVec2(menu.x, menu.h / 2));
		ImGui::SetNextWindowSize(ImVec2(menu.w, menu.h / 2));
		ImGui::Begin("World properties");
		static char filename[40];
		scene.cam.moving |= ImGui::Checkbox("Sky", &scene.opt.sky); ImGui::SameLine();
		scene.cam.moving |= ImGui::Checkbox("Skybox", &scene.opt.skybox); ImGui::SameLine();
		scene.cam.moving |= ImGui::Checkbox("Fog", &scene.opt.en_fog);
		if (scene.opt.en_fog) {
			if (ImGui::DragFloat("Fog density", &fogdens, -0.01, -3, 12, "1e-%g")) {
				scene.opt.ninv_fog = -1.f * pow(10, fogdens); scene.cam.moving = true;
			}
			scene.cam.moving |= ImGui::ColorEdit3("Fog color", scene.opt.fog_col._xyz, ImGuiColorEditFlags_Float);
		}
		if (scene.opt.sky) {
			if (scene.opt.skybox) {
				if (ImGui::InputText("Load skybox", filename, 40, ImGuiInputTextFlags_EnterReturnsTrue)) {
					scene.set_skybox(filename);
					scene.cam.moving = true;
				}
				ImGui::Text("Skybox: %s", scene.skybox.name.text());
			}
			else
			{
				scene.cam.moving |= ImGui::ColorEdit3("Sun noon", scene.opt.sun_noon._xyz, ImGuiColorEditFlags_Float);
				scene.cam.moving |= ImGui::ColorEdit3("Sun dawn", scene.opt.sun_dawn._xyz, ImGuiColorEditFlags_Float);
				scene.cam.moving |= ImGui::ColorEdit3("Sky noon", scene.opt.sky_noon._xyz, ImGuiColorEditFlags_Float);
				scene.cam.moving |= ImGui::ColorEdit3("Sky dawn", scene.opt.sky_dawn._xyz, ImGuiColorEditFlags_Float);
			}
		}
		else {
			scene.cam.moving |= ImGui::ColorEdit3("Ambient", scene.opt.ambient._xyz, ImGuiColorEditFlags_Float);
		}
		ImGui::Checkbox("BVH", &scene.opt.en_bvh);  ImGui::SameLine();
		if (ImGui::Checkbox("Linear", &scene.world.bvh_lin)) {
			scene.world.rebuild_bvh(1, scene.opt.node_size);
		}
		ImGui::SameLine();
		if (ImGui::Button("Rebuild BVH")) {
			scene.world.rebuild_bvh(1, scene.opt.node_size);
			scene.cam.moving = true;
		}
		if (ImGui::DragInt("Leaf nodes", &scene.opt.node_size, 1, 1, 32, 0, CLAMP)) {
			scene.world.rebuild_bvh(1, scene.opt.node_size);
		}
		ImGui::Text("Object menu");
		if (ImGui::Button("Add object")) {
			add_obj = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Add material")) {
			add_mat = true;
		}
		ImGui::SliderInt("Object ID", &(int&)scene.opt.selected, -1, scene.world.objects.size() - 1);
		ImGui::Text("%s", scene.get_name());
		if (scene.opt.selected < scene.world.objects.size()) {
			uint id = scene.opt.selected;
			mesh_var& obj = scene.object_at(id);
			bool is_light = obj.light();
			bool is_inbvh = obj.bvh();
			bool is_foggy = obj.fog();
			vec4 DT = TA;
			ImGui::SameLine();
			if (ImGui::Button("Erase")) {
				scene.cam.moving = true;
				scene.world.remove_mesh(id);
			}
			ImGui::SameLine();
			if (ImGui::Button("Duplicate")) {
				scene.cam.moving = true;
				scene.world.duplicate_mesh(id);
				scene.opt.selected = scene.world.objects.size() - 1;
			}
			if (ImGui::Checkbox("IN BVH", &is_inbvh)) {
				obj.flag.set_bvh(is_inbvh || obj.get_size() > 1000);
				scene.world.update_all(scene.opt.node_size); scene.cam.moving = true;
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Importance", &is_light)) {
				obj.flag.set_lig(is_light);
				scene.world.update_lights(); scene.cam.moving = true;
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Foggy", &is_foggy)) {
				obj.flag.set_fog(is_foggy);
				scene.cam.moving = true;
			}
			if (ImGui::DragFloat4("Pos", TP._xyz, 0.001, infp, infp, "%g"))
			{
				scene.set_trans(mat4(TP, TA));
				scene.cam.moving = true;
			}
			DT = TA;
			if (ImGui::DragFloat3("Rot", TA._xyz, 0.01, infp, infp, "%g") && not0(DT - mod(TA, pi2)))
			{
				scene.set_trans(mat4(TP, TA));
				scene.cam.moving = true;
			}
			DT = TD;
			if (ImGui::DragFloat3("Rot (deg)", TD._xyz, 1, infn, infp, "%g") && not0(DT - mod(TD, 360)))
			{
				scene.set_trans(mat4(TP, torad(TD)));
				scene.cam.moving = true;
			}
			uint mat = obj.get_mat();
			int max_mat = scene.world.materials.size();
			if (ImGui::SliderInt("Mat", &(int&)mat, -1, max_mat - 1)) {
				scene.cam.moving = true;
				obj.set_mat(mat);
			}
			if (mat < scene.world.materials.size()) {
				albedo& alb = scene.world.materials[mat].tex;
				int type = (int)scene.world.materials[mat].type;
				vec4 col = alb._rgb.get_col(), mer = alb._mer.get_col(), nor = alb._nor.get_col();
				bool s1 = alb._rgb.get_solid(), s2 = alb._mer.get_solid(), s3 = alb._nor.get_solid();
				if (ImGui::SliderInt("Mat type", &type, 0, mat_cnt - 1, mat_enum_str(type))) {
					scene.cam.moving = true;
					scene.world.materials[mat].type = (mat_enum)type;
				}
				ImGui::Text("Material: "); ImGui::SameLine();
				if (ImGui::Button("Erase##mat")) {
					scene.cam.moving = true;
					scene.world.remove_mat(mat);
				}
				ImGui::SameLine();
				if (ImGui::Button("Duplicate##mat")) {
					scene.cam.moving = true;
					scene.world.duplicate_mat(mat);
					obj.set_mat(max_mat);
				}
				static char srgb[40], smer[40], snor[40];
				if (ImGui::Checkbox("RGB##2", &s1)) {
					alb._rgb.set_solid(s1);
					if (!s1)alb._rgb.name.copy(srgb, 40);
					scene.cam.moving = true;
				}
				ImGui::SameLine();
				if (!s1) {
					if (ImGui::InputText("##t1", srgb, 40, ImGuiInputTextFlags_EnterReturnsTrue)) {
						scene.cam.moving = true;
						alb._rgb.set_tex(srgb);
					}
				}
				else {
					if (ImGui::ColorEdit4("##s1", col._xyz, ImGuiColorEditFlags_Float)) {
						scene.cam.moving = true;
						alb._rgb.set_col(col);
					}
				}
				if (ImGui::Checkbox("MER##2", &s2)) {
					alb._mer.set_solid(s2);
					scene.cam.moving = true;
				}
				ImGui::SameLine();
				if (!s2) {
					if (ImGui::InputText("##t2", smer, 40, ImGuiInputTextFlags_EnterReturnsTrue)) {
						scene.cam.moving = true;
						if (!s2)alb._mer.name.copy(smer, 40);
						alb._mer.set_tex(smer);
					}
				}
				else {
					if (ImGui::ColorEdit3("##s2", mer._xyz, ImGuiColorEditFlags_Float)) {
						scene.cam.moving = true;
						alb._mer.set_col(mer);
					}
				}
				if (ImGui::Checkbox("NOR##2", &s3)) {
					alb._nor.set_solid(s3);
					if (!s3)alb._nor.name.copy(snor, 40);
					scene.cam.moving = true;
				}
				ImGui::SameLine();
				if (!s3) {
					if (ImGui::InputText("##t3", snor, 40, ImGuiInputTextFlags_EnterReturnsTrue)) {
						scene.cam.moving = true;
						alb._nor.set_tex(snor);
					}
				}
				else {
					if (ImGui::ColorEdit3("##s3", nor._xyz, ImGuiColorEditFlags_Float)) {
						scene.cam.moving = true;
						alb._nor.set_col(nor);
					}
				}
				scene.cam.moving |= ImGui::ColorEdit4("Tint", alb.tint._xyz, ImGuiColorEditFlags_Float);
				scene.cam.moving |= ImGui::DragFloat("Translucent", &alb.trans, 0.01f, 0.f, 1.f, "%g");
				scene.cam.moving |= ImGui::DragFloat("Scale", &alb.rep, 0.1f, -1000.f, 1000.f, "%g");
				scene.cam.moving |= ImGui::DragFloat("IOR", &alb.ir, 0.001f, 1.f, 4.f, "%g");
			}
		}
		else if (!scene.opt.skybox && scene.opt.sky) {
			if (ImGui::DragFloat3("Rot", TA._xyz, 0.01))
			{
				scene.set_trans(mat4(TP, TA));
				scene.cam.moving = true;
			}
			vec4 dTD = TD;
			if (ImGui::DragFloat3("Rot (deg)", TD._xyz, 1, 0, 359.999f, "%g", CLAMP) && not0(dTD - TD))
			{
				scene.set_trans(mat4(TP, torad(TD)));
				scene.cam.moving = true;
			}
		}
		ImGui::End();
	}
	void Engine::Meas_fps() {
		if (fps_meas) {
			if (fps_cnt < 500) {
				fps_cnt++;
				avg_fps += 1 / ft;
			}
			else {
				fps_meas = false;
			}
		}
	}
	void Engine::Render_loop() {
		while (running) {
			double tmr = timer();
			Process_input();
			Process_overlay();
			Render();
			double delay = 1.0 / max_fps - timer(tmr);
			Delay(delay);
			dt = timer(tmr);
			fps = ImGui::GetIO().Framerate;
			Meas_fps();
		}
	}

}//END PTCR