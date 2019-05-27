#include "menu_scene.h"
#include "game_input_wrapper.h"
#include "services.h"

void MenuScene::initialize() {
	Engine::logn("[MENU] Init");
}

void MenuScene::begin() {
	Engine::logn("[MENU] Begin");

	Button start_button = Button(gw / 2, gh / 2 - 20, "Start");
	start_button.on_click = [] {
		Services::game_state()->new_game();
		Scenes::set_scene("map");
	};
	Button quit_button = Button(gw / 2, gh / 2 + 20, "Quit");
	quit_button.on_click = [] {
		Engine::exit();
	};
	Services::ui().add_element(start_button);
	Services::ui().add_element(quit_button);
}

void MenuScene::end() {
	Engine::logn("[MENU] End");
	Services::ui().clear();
}

void MenuScene::update() {
	Services::ui().update();
	if(GInput::pressed(GInput::Action::Start)) {
		Services::game_state()->new_game();
		Scenes::set_scene("map");
	}
}

void MenuScene::render() {
	renderer_clear();
	// room_render();
	renderer_draw_render_target_camera();
	
	Services::ui().render();
	draw_text_centered_str((int)(gw / 2), (int)(gh - 20), Colors::white, "Press start to play new game.");
	
	renderer_flip();
}

void MenuScene::unload() {
	Engine::logn("[MENU] Unload");
}
