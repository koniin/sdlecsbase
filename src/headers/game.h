#ifndef GAME_H
#define GAME_H

#include "engine.h"
#include "renderer.h"
#include "level_scene.h"

struct MenuScene : Scene {
	void initialize() override {
		Engine::logn("Init menu");
	}

	void begin() override {
		Engine::logn("Begin menu");
	}

	void end() override {
		Engine::logn("end menu");
	}

	int counter = 0;
	void update() override {
		counter++;
		if(counter > 60) {
			Engine::logn("Update menu");
			counter = 0;
		}
		if(Input::key_pressed(SDLK_SPACE)) {
			Scenes::set_scene("level");
		}
	}

	void render() override {
		renderer_clear();
		// room_render();
		renderer_draw_render_target_camera();
		// room_render_ui();
		draw_text_centered_str((int)(gw / 2), (int)(gh / 2), Colors::white, "Press space to start!");
		renderer_flip();
	}

	void unload() override {
		Engine::logn("Unload menu");
	}
};

inline void game_load() {
	// Allocate memory and load resources
	Engine::set_base_data_folder("data");
	Font *font = Resources::font_load("normal", "pixeltype.ttf", 15);
	set_default_font(font);
    FrameLog::enable_at(5, 5);
	
	MenuScene *menu_scene = new MenuScene;
	LevelScene *level = new LevelScene;
	Scenes::setup_scene("menu", menu_scene);
	Scenes::setup_scene("level", level);

	Scenes::set_scene("menu");
}

#endif