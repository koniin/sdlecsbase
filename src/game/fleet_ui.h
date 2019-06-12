#ifndef FLEET_UI_H
#define FLEET_UI_H

#include "engine.h"
#include "renderer.h"
#include "gui.h"
#include "game_state.h"

struct FighterUIElement : public Element {
    Point size;
    Rectangle rect;
    std::string _text;

    std::function<void(void)> on_click = nullptr;

    SDL_Color color;
    SDL_Color text_color = Colors::white;
    SDL_Color normal = { 66, 134, 244, 255 };
    SDL_Color highlighted { 160, 196, 255, 255 };
    SDL_Color activated { 255, 120, 200, 255 };
    
    std::string sheet_name;
    std::string sprite;

    FighterUIElement(int x, int y, std::string sprite_sheet, std::string sprite_name, std::string text) {
        position = { x, y };
        color = normal;
        TextCache::size(text.c_str(), &size.x, &size.y);
        size.x += 10;
        size.y += 5;

        _text = text;

        sheet_name = sprite_sheet;
        sprite = sprite_name;
    }

    void update() override {
        Point p;
        Input::mouse_current(p);

        rect.x = position.x - size.x / 2;
        rect.y = position.y - size.y / 2;
        rect.w = size.x;
        rect.h = size.y;

        if(rect.contains(p)) {
            color = highlighted;

            if(Input::mouse_left_down) {
                color = activated;
                if(on_click != nullptr) {
                    on_click();
                }
            }
        } else {
            color = normal;
        }
    }

    void render() override {
        draw_spritesheet_name_centered(Resources::sprite_sheet_get(sheet_name), sprite, rect.x, rect.y);
        // draw_g_rectangle_filled(rect.x, rect.y, rect.w, rect.h, color);
        draw_text_centered_str(position.x, position.y, text_color, _text);
    }
};

#endif