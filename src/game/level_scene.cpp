#include "level_scene.h"
#include "game_controller.h"
#include "systems.h"
#include "display_export.h"
#include <chrono>

void LevelScene::initialize() {
    Engine::logn("Init level");
 	render_buffer.init(2048);
    Resources::sprite_sheet_load("combat_sprites", "combat_sprites.data");
    GameController::initialize();
}

void LevelScene::begin() {
	Engine::logn("Begin level");
    GameController::create_player();
    GameController::create_enemy();
}

void LevelScene::end() {
    Services::arch_manager().clear_entities();

	Engine::logn("end level");
	render_buffer.clear();
}

// PlayerInputSystem system_player_input;
// AIInputSystem system_ai_input;
// PlayerHandleInputSystem system_player_handle_input;
// MoveForwardSystem system_move_forward;
// TravelDistanceSystem system_travel_distance;
// LifeTimeSystem system_lifetime;
// ProjectileHitSystem system_projectilehit;
// RemoveNoHullEntitiesSystem system_remove_no_hull;
// RemoveNoParentAliveEntitiesSystem system_remove_no_parent;

void LevelScene::update() {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	
    // ECS::ArchetypeManager &arch_manager = Services::arch_manager();
    // system_player_input.update(arch_manager);
    // system_player_handle_input.update(arch_manager);
    // system_ai_input.update(arch_manager);
    // system_move_forward.update(arch_manager);
    // system_travel_distance.update(arch_manager);
    // system_projectilehit.update(arch_manager);
    // system_remove_no_hull.update(arch_manager);
    // system_remove_no_parent.update(arch_manager);
    // system_lifetime.update(arch_manager);

    // system_player_handle_input.post_update();
    // system_ai_input.post_update();

    GameController::update();

    Services::ui().update();
    Services::events().emit();

    render_export(render_buffer);

	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	auto diff = t2 - t1;
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( diff ).count();
	auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>( diff ).count();
	std::string frame_duration_mu = "update time mu: " + std::to_string(duration);
	std::string frame_duration_ms = "update time ms: " + std::to_string(duration_ms);
	FrameLog::log(frame_duration_mu);
	FrameLog::log(frame_duration_ms);
}

void LevelScene::render() {
	renderer_clear();
    draw_buffer(render_buffer);
	renderer_draw_render_target_camera();
	
    Services::ui().render();

	renderer_flip();
}

void LevelScene::unload() {
	Engine::logn("Unload level");
}