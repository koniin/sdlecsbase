#ifndef GAME_H
#define GAME_H

#include "engine.h"
#include "renderer.h"
#include "services.h"
#include "menu_scene.h"
#include "level_scene.h"
#include "map_scene.h"
#include "game_input_wrapper.h"
// #include "_engine_test.h"

inline void game_load() {
	// Allocate memory and load resources
	Engine::set_base_data_folder("data");
	Font *font = Resources::font_load("normal", "pixeltype.ttf", 15);
	set_default_font(font);
    FrameLog::enable_at(5, 5);
	
	Services::init();

	Services::db()->load();

	MenuScene *menu_scene = new MenuScene;
	LevelScene *level = new LevelScene;
	MapScene *map = new MapScene;
	Scenes::setup_scene("menu", menu_scene);
	Scenes::setup_scene("map", map);
	Scenes::setup_scene("level", level);

	Scenes::set_scene("menu");

	SDL_ShowCursor(SDL_ENABLE);
	// engine_test();
}

#endif