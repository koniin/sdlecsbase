#include "level_scene.h"
#include <chrono>


// Should be components for the engine/renderer really
// Could be nice to have a base component library 
// Then you can move the export function also
// ==============================================================
struct Position {
    Vector2 value;
    Vector2 last;

    Position() : value(Vector2()), last(Vector2()) {}
    Position(float x, float y) : Position(Vector2(x, y)) {}
    Position(Vector2 pos) : value(pos), last(Vector2()) {}
};

struct SpriteComponent {
    float scale;
    float rotation;
    int w, h;
    int16_t radius;
    int16_t color_r;
    int16_t color_g;
    int16_t color_b;
    int16_t color_a;
    size_t sprite_sheet_index;
    std::string sprite_name;
    int layer;
    bool line;
    Vector2 position;

    SpriteComponent() {}

    SpriteComponent(const std::string &sprite_sheet_name, std::string name) : sprite_name(name) {
        sprite_sheet_index = Resources::sprite_sheet_index(sprite_sheet_name);
        auto sprite = Resources::sprite_get_from_sheet(sprite_sheet_index, name);
        w = sprite.w;
        h = sprite.h;
        scale = 1.0f;
        rotation = 0.0f;
        color_r = color_g = color_b = color_a = 255;
        layer = 0;
        line = false;
    }
};
// ==============================================================

void LevelScene::initialize() {
	Engine::logn("Init level");
 	render_buffer.init(2048);
    Resources::sprite_sheet_load("combat_sprites", "combat_sprites.data");
}

void LevelScene::begin() {
	Engine::logn("Begin level");
    players = em.create_archetype<Position, SpriteComponent>(10);
    auto ent = em.create_entity(players);
	if(em.is_alive(players, ent)) {
		Position pos = Position(100, 100);
		em.set_component(players, ent, pos);
		Position &pos_get = em.get_component<Position>(players, ent);
		Engine::logn("pos_get: %f", pos_get.value.x);
        
        SpriteComponent s = SpriteComponent("combat_sprites", "ship1");
        s.layer = 10;
        em.set_component(players, ent, s);
	}
}

void LevelScene::end() {
	Engine::logn("end level");
	render_buffer.clear();
}

void LevelScene::update() {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	
    counter++;
	if(counter > 60) {
		Engine::logn("Update level");
		counter = 0;
	}
	if(Input::key_pressed(SDLK_SPACE)) {
		Scenes::set_scene("menu");
	}

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
	// room_render_ui();
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
}

void LevelScene::render_export() {
    render_buffer.clear();
    auto *sprite_sheets = &Resources::get_sprite_sheets();
    auto sprite_data_buffer = render_buffer.sprite_data_buffer;
    auto &sprite_count = render_buffer.sprite_count;

    auto ci2 = em.get_iterator<Position, SpriteComponent>();
	for(auto c : ci2.containers) {
        for(int i = 0; i < c->length; i++) {
			auto &pos = c->index<Position>(i);
            auto &sprite = c->index<SpriteComponent>(i);
			
            export_sprite_data(pos, sprite, sprite_data_buffer[sprite_count++], sprite_sheets);
		}
	}
    // for(int i = 0; i < _g->projectiles_player.length; ++i) {
    //     export_sprite_data(_g->projectiles_player, i, sprite_data_buffer[sprite_count++]);
    // }
}