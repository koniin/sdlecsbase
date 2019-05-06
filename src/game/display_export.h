#ifndef DISPLAY_EXPORT_H
#define DISPLAY_EXPORT_H

#include "engine.h"
#include "renderer.h"
#include "services.h"
#include "components.h"

void export_sprite_data(const Position &position, const SpriteRender &sprite, SpriteBufferData &spr, std::vector<SpriteSheet> *sprite_sheets);

void render_export(RenderBuffer &render_buffer) {
    render_buffer.clear();
    auto *sprite_sheets = &Resources::get_sprite_sheets();
    auto sprite_data_buffer = render_buffer.sprite_data_buffer;
    auto &sprite_count = render_buffer.sprite_count;

    for(size_t i = 0; i < GameController::_motherships.size(); i++) {
        auto &sprite = GameController::_motherships[i].sprite;
        export_sprite_data(GameController::_motherships[i].position, sprite.animations[sprite.current_animation].render_data, sprite_data_buffer[sprite_count++], sprite_sheets);
    }

    for(size_t i = 0; i < GameController::_fighter_ships.size(); i++) {
        auto &sprite = GameController::_fighter_ships[i].sprite;
        export_sprite_data(GameController::_fighter_ships[i].position, sprite.animations[sprite.current_animation].render_data, sprite_data_buffer[sprite_count++], sprite_sheets);
    }
    
    for(size_t i = 0; i < GameController::_projectiles.size(); i++) {
        auto &projectile = GameController::_projectiles[i];
        auto &sprite = projectile.sprite;
        export_sprite_data(projectile.position, sprite.animations[sprite.current_animation].render_data, sprite_data_buffer[sprite_count++], sprite_sheets);
    }
    // auto ci = Services::arch_manager().get_iterator<Position, SpriteComponent>();
	// for(auto c : ci.containers) {
    //     for(int i = 0; i < c->length; i++) {
	// 		auto &pos = c->index<Position>(i);
    //         auto &sprite = c->index<SpriteComponent>(i);
			
    //         export_sprite_data(pos, sprite, sprite_data_buffer[sprite_count++], sprite_sheets);
	// 	}
	// }

    std::sort(sprite_data_buffer, sprite_data_buffer + sprite_count);

    // Update UI State
    // =================================

    Services::ui().frame();
    
    for(auto &ship : GameController::_fighter_ships) {
        auto pos = ship.position.value;
        TextElement t;
        t.color = Colors::white;
        t.position = Point((int)pos.x, (int)pos.y - 12);
        t.text = std::to_string(ship.hull.amount);
        Services::ui().add_element(t);
    }

    // auto ci2 = Services::arch_manager().get_iterator<PlayerInput, InputTriggerComponent, WeaponConfigurationComponent>();
	// for(auto c : ci2.containers) {
    //     for(int i = 0; i < c->length; i++) {
	// 		auto &input = c->index<PlayerInput>(i);
    //         auto &trigger = c->index<InputTriggerComponent>(i);
    //         auto &wc = c->index<WeaponConfigurationComponent>(i);
    //         TextElement t;
    //         t.color = input.fire_cooldown > 0.0f ? Colors::red : Colors::green;
    //         t.align = UIAlign::Left;
    //         t.position = Point(10, gh - 60 + i * 15);
    //         t.text = Text::format("%d. %s (%.2f)", trigger.trigger, wc.name.c_str(), input.fire_cooldown);
    //         Services::ui().add_element(t);
    //     }
    // }
}

void export_sprite_data(const Position &position, const SpriteRender &sprite, SpriteBufferData &spr, std::vector<SpriteSheet> *sprite_sheets) {
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

#endif