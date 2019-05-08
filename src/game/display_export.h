#ifndef DISPLAY_EXPORT_H
#define DISPLAY_EXPORT_H

#include "engine.h"
#include "renderer.h"
#include "services.h"
#include "components.h"

void export_sprite_data(const Position &position, const SpriteComponent &sprite, SpriteBufferData &spr, std::vector<SpriteSheet> *sprite_sheets);

template<typename T>
void export_entities(std::vector<T> &entities, SpriteBufferData *sprite_data_buffer, std::vector<SpriteSheet> *sprite_sheets, int &sprite_count) {
    for(size_t i = 0; i < entities.size(); i++) {
        export_sprite_data(entities[i].position, entities[i].sprite, sprite_data_buffer[sprite_count++], sprite_sheets);
    }
}

void render_export(RenderBuffer &render_buffer) {
    render_buffer.clear();
    auto *sprite_sheets = &Resources::get_sprite_sheets();
    auto sprite_data_buffer = render_buffer.sprite_data_buffer;
    auto &sprite_count = render_buffer.sprite_count;

    export_entities(GameController::_motherships, sprite_data_buffer, sprite_sheets, sprite_count);
    // for(size_t i = 0; i < GameController::_motherships.size(); i++) {
    //     export_sprite_data(GameController::_motherships[i].position, GameController::_motherships[i].sprite, sprite_data_buffer[sprite_count++], sprite_sheets);
    // }

    export_entities(GameController::_fighter_ships, sprite_data_buffer, sprite_sheets, sprite_count);
    // for(size_t i = 0; i < GameController::_fighter_ships.size(); i++) {
    //     export_sprite_data(GameController::_fighter_ships[i].position, GameController::_fighter_ships[i].sprite, sprite_data_buffer[sprite_count++], sprite_sheets);
    // }
    
    export_entities(GameController::_projectiles, sprite_data_buffer, sprite_sheets, sprite_count);
    // for(size_t i = 0; i < GameController::_projectiles.size(); i++) {
    //     auto &projectile = GameController::_projectiles[i];
    //     export_sprite_data(projectile.position, projectile.sprite, sprite_data_buffer[sprite_count++], sprite_sheets);
    // }

    export_entities(GameController::_projectile_missed, sprite_data_buffer, sprite_sheets, sprite_count);
    // for(size_t i = 0; i < GameController::_projectile_missed.size(); i++) {
    //     auto &projectile = GameController::_projectile_missed[i];
    //     export_sprite_data(projectile.position, projectile.sprite, sprite_data_buffer[sprite_count++], sprite_sheets);
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

    for(auto &ship : GameController::_motherships) {
        if(ship.faction.faction == GameController::PLAYER_FACTION) {
            for(size_t i = 0; i < ship.weapons._weapons.size(); i++) {
                TextElement t;
                t.color = ship.weapons._reload_timer[i] < ship.weapons._weapons[i].reload_time ? Colors::red : Colors::green;
                t.align = UIAlign::Left;
                t.position = Point(10, gh - 60 + i * 15);
                float reload_time = ship.weapons._weapons[i].reload_time - ship.weapons._reload_timer[i];
                t.text = Text::format("%d. %s (%.2f)", i + 1, ship.weapons._weapons[i].name.c_str(), reload_time > 0.f ? reload_time : 0);
                Services::ui().add_element(t);
            }
        }
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

void export_sprite_data(const Position &position, const SpriteComponent &sprite, SpriteBufferData &spr, std::vector<SpriteSheet> *sprite_sheets) {
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

    auto &sprite_frame = sprite.get_current_frame();
    auto &sheet = sprite_sheets->at(sprite_frame.sprite_sheet_index);
    auto &region = sheet.sheet_sprites[sheet.sprites_by_name.at(sprite_frame.sprite_name)].region;

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

        spr.dest.w = sprite_frame.w;
        spr.dest.h = sprite_frame.h;

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