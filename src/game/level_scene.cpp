#include "level_scene.h"
#include "game_controller.h"
#include "systems.h"
#include "display_export.h"
#include "particles.h"

#include <chrono>

void LevelScene::initialize() {
    Engine::logn("Init level");
 	render_buffer.init(2048);
    Resources::sprite_sheet_load("combat_sprites", "combat_sprites.data");
    GameController::initialize();

    Resources::sprite_load("background", "bkg1.png");
}

void LevelScene::begin() {
	Engine::logn("Begin level");
    GameController::create_player_mothership();
    GameController::create_enemy_mothership();
    GameController::create_player_fighters();
    GameController::create_enemy_fighters();
}

void LevelScene::end() {
    Engine::logn("end level");
    GameController::clear();
	render_buffer.clear();
}

void LevelScene::update() {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	
    GameController::update();
    Particles::update(GameController::particles, Time::delta_time);
    Services::events().emit();
    Services::ui().update();
    
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
    renderer_draw_render_target_camera();
	renderer_flip();
}

void LevelScene::unload() {
	Engine::logn("Unload level");
}