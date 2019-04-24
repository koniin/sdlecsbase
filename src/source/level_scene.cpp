#include "level_scene.h"
#include "systems.h"
#include "level_bootstrap.h"
#include <chrono>

void LevelScene::initialize() {
    Engine::logn("Init level");
 	render_buffer.init(2048);
    Resources::sprite_sheet_load("combat_sprites", "combat_sprites.data");
    LevelBootstrap::initialise(arch_manager);
}

void LevelScene::begin() {
	Engine::logn("Begin level");
    
    LevelBootstrap::create_player(arch_manager);

    LevelBootstrap::create_enemy(arch_manager);
    
}

void LevelScene::end() {
    arch_manager.clear();
	Engine::logn("end level");
	render_buffer.clear();
}

PlayerInputSystem system_player_input;
PlayerHandleInputSystem system_player_handle_input;
MoveForwardSystem system_move_forward;
TravelDistanceSystem system_travel_distance;
LifeTimeSystem system_lifetime;

void LevelScene::update() {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	
	if(Input::key_pressed(SDLK_SPACE)) {
		Scenes::set_scene("menu");
	}

    system_player_input.update(arch_manager);
    system_player_handle_input.update(arch_manager);

    system_move_forward.update(arch_manager);

    system_travel_distance.update(arch_manager);
    
    system_lifetime.update(arch_manager);
    system_player_handle_input.post_update();

    render_export();

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
	
    auto ci2 = arch_manager.get_iterator<Hull, Position>();
	for(auto c : ci2.containers) {
        for(int i = 0; i < c->length; i++) {
			auto &health = c->index<Hull>(i);
            auto &pos = c->index<Position>(i);
            draw_text_str((int)pos.value.x, gh - 20, Colors::white, std::to_string(health.amount));
        }
    }

	renderer_flip();
}

void LevelScene::unload() {
	Engine::logn("Unload level");
}

template<typename T, typename T2>
void export_sprite_data(const T &position, const T2 &sprite, SpriteBufferData &spr, std::vector<SpriteSheet> *sprite_sheets) {
    // handle camera, zoom and stuff here

    // also we can do culling here
    // intersects world_bounds etc

    // float globalScale = 0.05f;
    // spr.x = go.pos.x * globalScale;
    // spr.y = go.pos.y * globalScale;
    // spr.scale = go.sprite.scale * globalScale;
    // spr.x = entity_data.position[i].x - camera.x;
    // spr.x = entity_data.position[i].y - camera.y;

    const auto &camera = get_camera();

    auto &sheet = sprite_sheets->at(sprite.sprite_sheet_index);
    auto &region = sheet.sheet_sprites[sheet.sprites_by_name.at(sprite.sprite_name)].region;

    spr.tex = Resources::sprite_get(sheet.sprite_sheet_name)->image;
    spr.src = region;

    spr.angle = sprite.rotation;
    spr.layer = sprite.layer;

    // if(entity_data.sprite[i].line) {
    //     spr.dest.x = (int16_t)(entity_data.sprite[i].position.x - camera.x);
    //     spr.dest.y = (int16_t)(entity_data.sprite[i].position.y - camera.y);
    //     spr.dest.w = entity_data.sprite[i].w;
    //     spr.dest.h = entity_data.sprite[i].h;
    // } else {
        spr.dest.x = (int16_t)(position.value.x - camera.x);
        spr.dest.y = (int16_t)(position.value.y - camera.y);

        spr.dest.w = sprite.w;
        spr.dest.h = sprite.h;

        spr.dest.x = spr.dest.x - (spr.dest.w / 2);
        spr.dest.y = spr.dest.y - (spr.dest.h / 2);
    //}

    if(sprite.flip == 1) {
        spr.flip = SDL_FLIP_HORIZONTAL; 
    } else if(sprite.flip == 2) {
        spr.flip = SDL_FLIP_VERTICAL;
    } else {
        spr.flip = SDL_FLIP_NONE;
    }
}

void LevelScene::render_export() {
    render_buffer.clear();
    auto *sprite_sheets = &Resources::get_sprite_sheets();
    auto sprite_data_buffer = render_buffer.sprite_data_buffer;
    auto &sprite_count = render_buffer.sprite_count;

    auto ci2 = arch_manager.get_iterator<Position, SpriteComponent>();
	for(auto c : ci2.containers) {
        for(int i = 0; i < c->length; i++) {
			auto &pos = c->index<Position>(i);
            auto &sprite = c->index<SpriteComponent>(i);
			
            export_sprite_data(pos, sprite, sprite_data_buffer[sprite_count++], sprite_sheets);
		}
	}

    std::sort(sprite_data_buffer, sprite_data_buffer + sprite_count);
}