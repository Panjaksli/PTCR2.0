#include "Engine.h"
namespace PTCR {

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
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			handle.scan(event);
			if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE))
				running = false;
			if (event.type == SDL_WINDOWEVENT) {
				if (event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
					running = false;
				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					screen = { 0,0,event.window.data1, event.window.data2 };
					Resize();
				}
			}
			if (event.type == SDL_DROPFILE) {
				string name = event.drop.file;
				if (name.find(".scn") != -1) {
					if (scn_load(scene, name.c_str())) {
						scn_n = -1;
						scn_name = name.c_str();
						reproject = false;
					}
				}
				else if (name.find(".msh") != -1 || name.find(".obj") != -1) {
					int x, y;
					SDL_GetMouseState(&x, &y);
					vec4 P = scene.get_point(scale * y, scale * x, 1);
					if (scene.world.load_mesh(name.c_str(), vec4(P, 1), scene.world.materials.size())) {
						scene.world.add_mat(albedo(), mat_ggx);
						scene.opt.selected = scene.world.objects.size() - 1;
						scene.world.build_bvh(1, scene.opt.node_size);
						scene.opt.en_bvh = true;
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
					handle.rmb = false;
					handle.mmb = false;
				}
				else if (handle.motion && handle.rmb && scene.opt.selected != -1) {
					float dist = (scene.selected().get_box().pmid() - scene.cam.P).len();
					vec4 dir = vec4(event.motion.xrel, -event.motion.yrel, 0, 0) / vec4(viewport.w, viewport.h, 1, 1);
					vec4 trans = scene.selected().get_trans().P() + scene.cam.T.vec(2 * dist * dir);
					vec4 rot = scene.selected().get_trans().A();
					trans[3] = scene.selected().get_trans().P()[3];
					scene.set_trans(mat4(trans, rot));
					scene.cam.moving = true;
				}
				if (handle.motion && handle.lmb)
					scene.cam_rotate(torad(vec4(0, -handle.xvec, -handle.yvec)), 1);
			}
		}
		if (ImGui::GetIO().WantCaptureKeyboard) handle.reset_keys();
		if (handle[SDL_SCANCODE_LSHIFT])move_mul = 2.0;
		else move_mul = 1.0;
		vec4 dir = 10 * move_mul * vec4(handle[SDL_SCANCODE_W] - handle[SDL_SCANCODE_S], handle[SDL_SCANCODE_SPACE] - handle[SDL_SCANCODE_LCTRL], handle[SDL_SCANCODE_D] - handle[SDL_SCANCODE_A]);
		vec4 pan = vec4(0, handle[SDL_SCANCODE_LEFT] - handle[SDL_SCANCODE_RIGHT], handle[SDL_SCANCODE_UP] - handle[SDL_SCANCODE_DOWN]);
		scene.cam_move(dir, dt);
		scene.cam_rotate(pan, dt);
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
		SDL_RenderClear(renderer);
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
		}
		else {
			if (frame_id) {
				gen_t = timer();
				scene.Reproject(proj, disp, pitch / 4);
				gen_t = timer(gen_t);
				Delay(nor_t - gen_t);
			}
			else {
				proj = projection(scene.cam.T, scene.cam.CCD.w, scene.cam.CCD.h, scene.cam.tfov);
				nor_t = timer();
				scene.Render(disp, pitch / 4);
				nor_t = timer(nor_t);
			}
			frame_id = !frame_id;
		}
		SDL_UnlockTexture(frame);
		SDL_RenderCopy(renderer, frame, 0, &viewport);
		if (overlay)ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(renderer);
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
			static c_str filename;
			mat4 T; vec4 deg;
			ImGui::Begin("Add object", &add_obj);
			ImGui::DragFloat4("Offset", offset._xyz, 0.001f, infn, infp, "%g", NOROUND);
			ImGui::DragFloat4("Rotation", angle._xyz, 0.001f, infn, infp, "%g", NOROUND);
			deg = todeg(angle);
			if (ImGui::DragFloat4("Rot (deg)", deg._xyz, 0.1f, infn, infp, "%.1f")) angle = torad(deg);
			ImGui::SliderInt("Mat##1", &mat, -1, scene.world.materials.size() - 1, "%d", CLAMP);
			ImGui::Checkbox("Bvh##2", &is_bvh);
			ImGui::SameLine();
			ImGui::Checkbox("Light##2", &is_light);
			ImGui::SameLine();
			ImGui::Checkbox("Foggy##2", &is_foggy);
			ImGui::SliderInt("Obj type", &obj_type, 0, 4, obj_enum_str(obj_type, 1), CLAMP);
			is_mesh = obj_type == 4;
			T = mat4(offset, angle);
			if (is_mesh) {
				is_bvh = true;
				ImGui::InputText("Mesh name", filename, c_str::max_len);
			}
			else if (obj_type == o_pol || obj_type == o_qua) {
				ImGui::DragFloat3("A", a._xyz, 0.001);
				ImGui::DragFloat3("B", b._xyz, 0.001);
				ImGui::DragFloat3("C", c._xyz, 0.001);
			}
			else if (obj_type == o_sph || obj_type == o_vox) {
				ImGui::DragFloat4("Q+a", q._xyz, 0.001);
			}
			if (ImGui::Button("Load")) {
				if (is_mesh) {
					if (scene.world.load_mesh(filename, T, mat, is_bvh, is_light, is_foggy)) {
						if (is_bvh) scene.opt.en_bvh = true;
						scene.opt.selected = scene.world.objects.size() - 1;
						scene.world.update_all(scene.opt.node_size);
						scene.cam.moving = true;
					}
				}
				else {
					if (obj_type == o_pol) scene.world.add_mesh(poly(a, b, c), T, mat, is_bvh, is_light, is_foggy);
					else if (obj_type == o_qua) scene.world.add_mesh(quad(a, b, c), T, mat, is_bvh, is_light, is_foggy);
					else if (obj_type == o_sph)	scene.world.add_mesh(sphere(q), T, mat, is_bvh, is_light, is_foggy);
					else if (obj_type == o_vox) scene.world.add_mesh(voxel(q), T, mat, is_bvh, is_light, is_foggy);
					scene.opt.selected = scene.world.objects.size() - 1;
					scene.world.update_all(scene.opt.node_size);
					scene.cam.moving = true;
				}
			}
			ImGui::End();
		}
	}

	void Engine::Add_material() {
		if (add_mat) {
			static int type = 0;
			static float ir = 1, scl = 1;
			static bool s1 = 1, s2 = 1, s3 = 1;
			static bool alpha = 0, checker = 0;
			static vec4 rgb(1), mer(0), nor(0.5, 0.5, 1), tint(0);
			static c_str srgb, smer, snor;
			ImGui::Begin("Add material", &add_mat);
			ImGui::SliderInt("Type", &type, 0, mat_cnt - 1, mat_enum_str(type));
			s1 ? ImGui::ColorEdit4("##col1", rgb._xyz, FLOATCOL) : ImGui::InputText("##text1", srgb, c_str::max_len);
			ImGui::SameLine(); ImGui::Checkbox("RGB", &s1);
			s2 ? ImGui::ColorEdit3("##col2", mer._xyz, FLOATCOL) : ImGui::InputText("##text2", smer, c_str::max_len);
			ImGui::SameLine(); ImGui::Checkbox("MER", &s2);
			s3 ? ImGui::ColorEdit3("##col3", nor._xyz, FLOATCOL) : ImGui::InputText("##text3", snor, c_str::max_len);
			ImGui::SameLine(); ImGui::Checkbox("NOR", &s3);
			ImGui::ColorEdit4("Tint", tint._xyz, FLOATCOL);
			ImGui::DragFloat("Scale", &scl, 0.1f, -1000.f, 1000.f, "%g");
			ImGui::DragFloat("IOR", &ir, 0.01f, 1.f, 4.f, "%.2f");
			ImGui::Checkbox("Alpha##add", &alpha); ImGui::SameLine();
			ImGui::Checkbox("Checker##add", &checker);
			if (ImGui::Button("Add")) {
				scene.world.add_mat(albedo(texture(rgb, srgb, s1), texture(mer, smer, s2), texture(nor, snor, s3), scl, ir, tint, alpha, checker), mat_enum(type));
			}
			ImGui::End();
		}
	}

	void Engine::Camera_menu() {
		ImGui::SetNextWindowPos(ImVec2(menu.x, 0));
		ImGui::SetNextWindowSize(ImVec2(menu.w, menu.h / 2));
		ImGui::Begin("Engine properties");
		if (ImGui::SliderInt("Scene", &scn_n, 1, no_scn)) {
			scn_load(scene, scn_n);
			reproject = false;
		}
		if (ImGui::InputText("SCN File", scn_name, c_str::max_len, INPTEXT) + ImGui::Button("Load scene")) {
			if (scn_load(scene, scn_name, 0)) {
				scn_n = -1;
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
		vec4 pos = vec4(scene.cam.T.P(), scene.cam.speed);
		vec4 rad = vec4(scene.cam.T.A(), scene.cam.sens);
		vec4 deg = todeg(scene.cam.T.A());
		if (ImGui::DragFloat4("Position##cam", pos._xyz, 0.001, infp, infp, "%g", NOROUND)) {
			scene.cam.set_P(pos);
			scene.cam.speed = pos[3];
		}
		DT = rad;
		if (ImGui::DragFloat4("Rotation##cam", rad._xyz, 0.001, infp, infp, "%g", NOROUND) && not0(DT - mod(rad, pi2))) {
			scene.cam.set_A(rad);
			scene.cam.sens = rad[3];
		}
		DT = deg;
		if (ImGui::DragFloat3("Rot (deg)##cam", deg._xyz, 0.1, infp, infp, "%g", NOROUND) && not0(DT - mod(deg, 360))) {
			scene.cam.set_A(torad(deg));
		}
		if (ImGui::DragFloat("FOV", &scene.cam.fov, 0.1, 0.1, 179.9, "%.2f")) {
			scene.cam.moving = true;
			scene.cam.update();
		}
		if (ImGui::Checkbox("Auto focus", &scene.cam.autofocus)) {
			if (scene.cam.autofocus)tap_to_focus = 0;
			scene.cam.moving = true;
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Tap to focus", &tap_to_focus) && tap_to_focus) {
			scene.cam.autofocus = 0;
		}
		if (ImGui::DragFloat("Foc dist", &scene.cam.foc_t, 0.001, 0.001, infp, "%g", NOROUND & LOGSCL)) {
			scene.cam.moving = true;
			scene.cam.autofocus = 0;
		}
		scene.cam.moving |= ImGui::DragFloat("Exposure", &scene.cam.exposure, 0.1, 0.01, 100.f, "%.2f", LOGSCL);
		scene.cam.moving |= ImGui::DragFloat("F-stop", &scene.cam.fstop, 0.1, 0.1, 64, "%.2f", LOGSCL);
		if (ImGui::DragFloat("Res scale", &scale, 0.01, 0.1f, 4, " %.2f", CLAMP)) Resize(), scene.cam.moving = true;
		if (ImGui::DragFloat("Ray lifetime", &scene.opt.p_life, 0.01, 0.01, 1.f, "%.2f", CLAMP)) {
			scene.opt.i_life = 1.f / scene.opt.p_life;
			scene.cam.moving = true;
		}
		ImGui::DragInt("Samples", &scene.opt.samples, 0.1, 0, 1000, 0, CLAMP);
		scene.cam.moving |= ImGui::DragInt("Bounces", &scene.opt.bounces, 0.1, 0, 1000, 0, CLAMP);
		ImGui::DragFloat("MED filter", &scene.opt.med_thr, 0.002f, 0.0f, 1.f, "%.3f", CLAMP);
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
		ImGui::Checkbox("Outline", &scene.opt.outline);  ImGui::SameLine();
		scene.cam.moving |= ImGui::Checkbox("Normal maps", &use_normal_maps);
		scene.cam.moving |= ImGui::Checkbox("Reproject", &reproject);
		ImGui::SameLine();
		//WIP
		ImGui::Checkbox("Framegen", &scene.opt.framegen);
#endif
		int maxfps = max_fps;
		ImGui::DragInt("Max FPS", &maxfps, 1, 10, 240);
		max_fps = maxfps;
		ImGui::DragInt("Max SPP", &scene.opt.max_spp, 1000, 1, infp, "%d", LOGSCL);
		ImGui::Text("CUR: %.2f fps, %.2f ms, %.3d spp", fps, 1000.0 / fps, (int)scene.cam.CCD.spp);
		ImGui::Text("AVG: %.2f fps, %.2f ms, %.3d spp", avg_fps / fps_cnt, 1000.0 * fps_cnt / avg_fps, fps_cnt);
		if (ImGui::Button("Measure perf")) {
			avg_fps = 0;
			fps_cnt = 0;
			fps_meas = true;
		}
		ImGui::SameLine();
		ImGui::Checkbox("Pause", &scene.opt.paused);
		ImGui::End();
	}

	void Engine::Object_menu() {
		ImGui::SetNextWindowPos(ImVec2(menu.x, menu.h / 2));
		ImGui::SetNextWindowSize(ImVec2(menu.w, menu.h / 2));
		ImGui::Begin("World properties");
		c_str skybox = scene.skybox.name;
		scene.cam.moving |= ImGui::Checkbox("Sky", &scene.opt.sky); ImGui::SameLine();
		scene.cam.moving |= ImGui::Checkbox("Skybox", &scene.opt.skybox); ImGui::SameLine();
		scene.cam.moving |= ImGui::Checkbox("Fog", &scene.opt.en_fog);
		if (scene.opt.en_fog) {
			fogdens = log10f(-scene.opt.ninv_fog);
			if (ImGui::DragFloat("Fog density", &fogdens, -0.01, -3, 12, "1e-%.2f")) {
				scene.opt.ninv_fog = -1.f * pow(10, fogdens); scene.cam.moving = true;
			}
			scene.cam.moving |= ImGui::ColorEdit3("Fog color", scene.opt.fog_col._xyz, FLOATCOL);
		}
		if (scene.opt.sky) {
			if (scene.opt.skybox) {
				if (ImGui::InputText("Load skybox", skybox, c_str::max_len, INPTEXT)) {
					scene.set_skybox(skybox);
					scene.cam.moving = true;
				}
			}
			else {
				scene.cam.moving |= ImGui::ColorEdit3("Sun noon", scene.opt.sun_noon._xyz, FLOATCOL);
				scene.cam.moving |= ImGui::ColorEdit3("Sun dawn", scene.opt.sun_dawn._xyz, FLOATCOL);
				scene.cam.moving |= ImGui::ColorEdit3("Sky noon", scene.opt.sky_noon._xyz, FLOATCOL);
				scene.cam.moving |= ImGui::ColorEdit3("Sky dawn", scene.opt.sky_dawn._xyz, FLOATCOL);
			}
		}
		else {
			scene.cam.moving |= ImGui::ColorEdit3("Ambient", scene.opt.ambient._xyz, FLOATCOL);
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
		if (ImGui::DragInt("Leaf nodes", &scene.opt.node_size, 0.1f, 1, 32, 0, CLAMP)) {
			scene.world.rebuild_bvh(1, scene.opt.node_size);
		}
		if (ImGui::Button("Add object")) {
			add_obj = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Add material")) {
			add_mat = true;
		}
		ImGui::Text("Object menu:");
		ImGui::SliderInt("ID##obj", &(int&)scene.opt.selected, -1, scene.world.objects.size() - 1);
		if (scene.opt.selected < scene.world.objects.size()) {
			uint id = scene.opt.selected;
			mesh_var& obj = scene.object_at(id);
			bool is_light = obj.light();
			bool is_inbvh = obj.bvh();
			bool is_foggy = obj.fog();
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
			ImGui::SameLine();
			ImGui::Text("%s", scene.get_name());
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
			if (ImGui::DragFloat4("Pos", TP._xyz, 0.001, infp, infp, "%g", NOROUND)) {
				scene.set_trans(mat4(TP, TA));
				scene.cam.moving = true;
			}
			DT = TA;
			if (ImGui::DragFloat3("Rot", TA._xyz, 0.001, infp, infp, "%g", NOROUND) && not0(DT - mod(TA, pi2))) {
				scene.set_trans(mat4(TP, TA));
				scene.cam.moving = true;
			}
			DT = TD;
			if (ImGui::DragFloat3("Rot (deg)", TD._xyz, 0.1, infn, infp, "%.1f") && not0(DT - mod(TD, 360))) {
				scene.set_trans(mat4(TP, torad(TD)));
				scene.cam.moving = true;
			}
			uint mat = obj.get_mat();
			int max_mat = scene.world.materials.size();
			ImGui::Text("Material menu:");
			if (ImGui::SliderInt("Mat ID", &(int&)mat, -1, max_mat - 1)) {
				scene.cam.moving = true;
				obj.set_mat(mat);
			}
			if (mat < scene.world.materials.size()) {
				albedo& alb = scene.world.materials[mat].tex;
				c_str srgb = alb._rgb.name, smer = alb._mer.name, snor = alb._nor.name;
				int type = (int)scene.world.materials[mat].type;
				bool s1 = alb._rgb.solid(), s2 = alb._mer.solid(), s3 = alb._nor.solid();
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
				if (ImGui::SliderInt("Type", &type, 0, mat_cnt - 1, mat_enum_str(type))) {
					scene.cam.moving = true;
					scene.world.materials[mat].type = (mat_enum)type;
				}
				if (!s1) {
					if (ImGui::InputText("##t1", srgb, c_str::max_len, INPTEXT)) {
						scene.cam.moving = true;
						alb._rgb.set_tex(srgb);
					}
				}
				else {
					if (ImGui::ColorEdit4("##s1", alb._rgb.rgb._xyz, FLOATCOL)) {
						scene.cam.moving = true;
					}
				}
				ImGui::SameLine();
				if (ImGui::Checkbox("RGB##2", &s1)) {
					alb._rgb.set_solid(s1);
					scene.cam.moving = true;
				}

				if (!s2) {
					if (ImGui::InputText("##t2", smer, c_str::max_len, INPTEXT)) {
						scene.cam.moving = true;
						alb._mer.set_tex(smer);
					}
				}
				else {
					if (ImGui::ColorEdit3("##s2", alb._mer.rgb._xyz, FLOATCOL)) {
						scene.cam.moving = true;
					}
				}
				ImGui::SameLine();
				if (ImGui::Checkbox("MER##2", &s2)) {
					alb._mer.set_solid(s2);
					scene.cam.moving = true;
				}
				if (!s3) {
					if (ImGui::InputText("##t3", snor, c_str::max_len, INPTEXT)) {
						scene.cam.moving = true;
						alb._nor.set_tex(snor);
					}
				}
				else {
					if (ImGui::ColorEdit3("##s3", alb._nor.rgb._xyz, FLOATCOL)) {
						scene.cam.moving = true;
					}
				}
				ImGui::SameLine();
				if (ImGui::Checkbox("NOR##2", &s3)) {
					alb._nor.set_solid(s3);
					scene.cam.moving = true;
				}
				scene.cam.moving |= ImGui::ColorEdit4("Tint", alb.tint._xyz, FLOATCOL);
				scene.cam.moving |= ImGui::DragFloat("Scale", &alb.rep, 0.1f, -1000.f, 1000.f, "%g");
				scene.cam.moving |= ImGui::DragFloat("IOR", &alb.ir, 0.01f, 1.f, 4.f, "%.2f");
				bool alpha = alb.alpha();
				bool checker = alb.checker();
				scene.cam.moving |= ImGui::Checkbox("Alpha", &alpha); ImGui::SameLine();
				scene.cam.moving |= ImGui::Checkbox("Checker", &checker);
				alb.set_alpha(alpha);
				alb.set_checker(checker);
			}
		}
		else if (!scene.opt.skybox && scene.opt.sky) {
			DT = TA;
			if (ImGui::DragFloat3("Rot##sky", TA._xyz, 0.001, infp, infp, "%g", NOROUND) && not0(DT - mod(TA, pi2))) {
				scene.set_trans(mat4(0, TA));
				scene.cam.moving = true;
			}
			DT = TD;
			if (ImGui::DragFloat3("Rot (deg)##sky", TD._xyz, 0.1, infn, infp, "%.1f") && not0(DT - mod(TD, 360))) {
				scene.set_trans(mat4(0, torad(TD)));
				scene.cam.moving = true;
			}
		}
		ImGui::End();
	}
	void Engine::Meas_fps() {
		if (fps_meas) {
			if (fps_cnt < 500) {
				fps_cnt++;
				avg_fps += 1.0 / ft;
			}
			else {
				fps_meas = false;
			}
		}
	}
	void Engine::Render_loop() {
		while (running) {
			ct = timer();
			Process_input();
			Process_overlay();
			Render();
			Delay(1.0 / max_fps - timer(ct));
			dt = timer(ct);
			fps = ImGui::GetIO().Framerate;
			Meas_fps();
		}
	}

}//END PTCR