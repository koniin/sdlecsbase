#include "level_scene.h"
#include "battle_controller.h"
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
    BattleController::initialize();

    Resources::sprite_load("background", "bkg1.png");
}

void LevelScene::begin() {
	Engine::logn("[LEVEL] Begin");
    BattleController::create(Services::game_state());

    SelectBox s_box;
    s_box.release_func = [](Rectangle r) {
        Engine::logn("There was a release triggered");
    };
    Services::ui()->add_element(s_box);

    Services::events()->listen<BattleOverEvent>([](BattleOverEvent e) { 
        battle_over = true;

        TextElement t, t2;
        t.align = t2.align = UIAlign::Center;
        t.color = t2.color = Colors::white;
        t.position = Point((int)(gw / 2), (int)(gh / 2));
        t2.position = Point(t.position.x, t.position.y + 10);

        if(e.winner_faction == PLAYER_FACTION) {
            t.text = "YOU ROCK!";
        } else {
            t.text = "GAME OVER";
        }
        
        t2.text = "Press start to continue...";

        Services::ui()->add_element(t);
        Services::ui()->add_element(t2);
    });
}

void LevelScene::end() {
    Engine::logn("[LEVEL] End");
    BattleController::end(Services::game_state());
	render_buffer.clear();
    Services::ui()->clear();
    
    battle_over = false;
}

void LevelScene::update() {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	
    BattleController::update();
    Particles::update(BattleController::particles, Time::delta_time);
    Services::events()->emit();
    Services::ui()->update();
    
	if(battle_over && GInput::pressed(GInput::Action::Start)) {
		Scenes::set_scene("map");
        Services::game_state()->set_current_node_completed();
	}

    if(Input::key_pressed(SDLK_m)) {
		Scenes::set_scene("map");
        Services::game_state()->set_current_node_completed();
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

    FrameLog::log("projectiles: " + std::to_string(BattleController::_projectiles.size()));
    FrameLog::log("projectiles missed: " + std::to_string(BattleController::_projectile_missed.size()));
}

void LevelScene::render() {
	renderer_clear();
    // Render Background
    draw_sprite(Resources::sprite_get("background"), 0, 0);
    
    draw_buffer(render_buffer);
    Particles::render_circles_filled(BattleController::particles);
    
    Services::ui()->render();

    int population = Services::game_state()->population;
    std::string population_text = "Population: " + std::to_string(population);
    draw_text_centered_str((int)(gw / 2), 10, Colors::white, population_text);
    
    renderer_draw_render_target_camera();
	renderer_flip();
}

void LevelScene::unload() {
	Engine::logn("[LEVEL] Unload");
}