#include "menu_scene.h"
#include "game_input_wrapper.h"
#include "services.h"

void MenuScene::initialize() {
	Engine::logn("[MENU] Init");
}

void MenuScene::begin() {
	Engine::logn("[MENU] Begin");
}

void MenuScene::end() {
	Engine::logn("[MENU] End");
}

void MenuScene::update() {
	if(GInput::pressed(GInput::Action::Start)) {
		Services::game_state()->new_game();
		Scenes::set_scene("map");
	}
}

void MenuScene::render() {
	renderer_clear();
	// room_render();
	renderer_draw_render_target_camera();
	
	draw_text_centered_str((int)(gw / 2), (int)(gh / 2), Colors::white, "Press start to continue...");
	
	renderer_flip();
}

void MenuScene::unload() {
	Engine::logn("[MENU] Unload");
}
