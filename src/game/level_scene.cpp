#include "level_scene.h"
#include "game_controller.h"
#include "systems.h"
#include "display_export.h"
#include "particles.h"
#include "services.h"
#include "game_input_wrapper.h"

#include <chrono>

bool battle_over = false;

void LevelScene::initialize() {
    Engine::logn("[LEVEL] Init");
 	render_buffer.init(2048);
    Resources::sprite_sheet_load("combat_sprites", "combat_sprites.data");
    GameController::initialize();

    Resources::sprite_load("background", "bkg1.png");
}

void LevelScene::begin() {
	Engine::logn("[LEVEL] Begin");
    GameController::create(Services::game_state());

    Services::events().listen<BattleOverEvent>([](BattleOverEvent e) { 
        Engine::logn("MAKE UI FOR END OF BATTLE!");
        // if(e.winner_faction == PLAYER_FACTION) {
        //     draw_text_centered_str((int)(gw / 2), (int)(gh / 2), Colors::white, "YOU ROCK!");
        //     draw_text_centered_str((int)(gw / 2), (int)(gh / 2) + 10, Colors::white, "Press start to continue...");
        // } else {
        //     draw_text_centered_str((int)(gw / 2), (int)(gh / 2), Colors::white, "GAME OVER!");
        //     draw_text_centered_str((int)(gw / 2), (int)(gh / 2) + 10, Colors::white, "Press start to continue...");
        // }
    });
}

void LevelScene::end() {
    Engine::logn("[LEVEL] End");
    GameController::end(Services::game_state());
	render_buffer.clear();
    Services::ui().clear();

    battle_over = false;
}

void LevelScene::update() {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	
    GameController::update();
    Particles::update(GameController::particles, Time::delta_time);
    Services::events().emit();
    Services::ui().update();
    
	if(battle_over && GInput::pressed(GInput::Action::Start)) {
		Scenes::set_scene("map");
        Services::game_state()->end_node();
	}

    if(Input::key_pressed(SDLK_m)) {
		Scenes::set_scene("map");
        Services::game_state()->end_node();
	}
    
    render_export(render_buffer);

	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	auto diff = t2 - t1;
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( diff ).count();
	auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>( diff ).count();
	std::string frame_duration_mu = "update time mu: " + std::to_string(duration);
	std::string frame_duration_ms = "update time ms: " + std::to_string(duration_ms);
	FrameLog::log(frame_duration_mu);
	FrameLog::log(frame_duration_ms);

    FrameLog::log("projectiles: " + std::to_string(GameController::_projectiles.size()));
    FrameLog::log("projectiles missed: " + std::to_string(GameController::_projectile_missed.size()));
}

void LevelScene::render() {
	renderer_clear();
    // Render Background
    draw_sprite(Resources::sprite_get("background"), 0, 0);
    
    draw_buffer(render_buffer);
    Particles::render_circles_filled(GameController::particles);
	Services::ui().render();
        
    int population = Services::game_state()->population;
    std::string population_text = "Population: " + std::to_string(population);
    draw_text_centered_str((int)(gw / 2), 10, Colors::white, population_text);
    
    renderer_draw_render_target_camera();
	renderer_flip();
}

void LevelScene::unload() {
	Engine::logn("[LEVEL] Unload");
}