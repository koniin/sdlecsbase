#ifndef DISPLAY_EXPORT_H
#define DISPLAY_EXPORT_H

#include "engine.h"
#include "renderer.h"
#include "services.h"
#include "components.h"
#include "battle_controller.h"

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

    export_entities(BattleController::_motherships, sprite_data_buffer, sprite_sheets, sprite_count);
    export_entities(BattleController::_fighter_ships, sprite_data_buffer, sprite_sheets, sprite_count);
    export_entities(BattleController::_projectiles, sprite_data_buffer, sprite_sheets, sprite_count);
    export_entities(BattleController::_projectile_missed, sprite_data_buffer, sprite_sheets, sprite_count);
    
    std::sort(sprite_data_buffer, sprite_data_buffer + sprite_count);

    // Update UI State
    // =================================

    Services::ui().frame();
    
    for(auto &ship : BattleController::_fighter_ships) {
        auto pos = ship.position.value;
        TextElement t;
        t.color = Colors::white;
        t.position = Point((int)pos.x, (int)pos.y - 12);
        t.text = std::to_string(ship.defense.hp) + "/" + std::to_string(ship.defense.shield);
        Services::ui().add_immediate_element(t);
    }

    for(auto &ship : BattleController::_motherships) {
        auto pos = ship.position.value;
        TextElement t;
        t.color = Colors::white;
        t.position = Point((int)pos.x, (int)pos.y - 12);
        t.text = std::to_string(ship.defense.hp) + "/" + std::to_string(ship.defense.shield);
        Services::ui().add_immediate_element(t);
    }

    for(auto &ship : BattleController::_motherships) {
        if(ship.faction.faction == PLAYER_FACTION) {
            int i = 0;
            for(auto ability_id : ship.abilities.ids()) {
                auto cooldown = ship.abilities.get_cooldown(ability_id);
                auto name = ship.abilities.get_name(ability_id);
                auto reload_timer = ship.abilities.get_timer(ability_id);
                TextElement t;
                t.color = reload_timer < cooldown ? Colors::red : Colors::green;
                t.align = UIAlign::Left;
                t.position = Point(10, gh - 60 + i * 15);
                float reload_time = cooldown - reload_timer;
                t.text = Text::format("%d. %s (%.2f)", i + 1, name.c_str(), reload_time > 0.f ? reload_time : 0);
                Services::ui().add_immediate_element(t);
                i++;
            }
        }
    }
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

    if(sprite.line) {
         spr.dest.x = (int16_t)(position.last.x - camera.x);
         spr.dest.y = (int16_t)(position.last.y - camera.y);
         spr.dest.w = sprite.w;
         spr.dest.h = sprite.h;
    } else {
        spr.dest.x = (int16_t)(position.value.x - camera.x);
        spr.dest.y = (int16_t)(position.value.y - camera.y);

        spr.dest.w = sprite_frame.w;
        spr.dest.h = sprite_frame.h;

        spr.dest.x = spr.dest.x - (spr.dest.w / 2);
        spr.dest.y = spr.dest.y - (spr.dest.h / 2);
    }

    if(sprite.flip == 1) {
        spr.flip = SDL_FLIP_HORIZONTAL; 
    } else if(sprite.flip == 2) {
        spr.flip = SDL_FLIP_VERTICAL;
    } else {
        spr.flip = SDL_FLIP_NONE;
    }
}

#endif