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
        draw_text_centered_str(rect.x, rect.y + rect.h, text_color, _text);
    }
};

void list_blueprints() {
    int x = gw / 2;
    int spacing = 40;
    int y_start = (gh / 2) - ((Services::game_state()->fighters.size() * spacing) / 2);
    int y = 0;
    for(auto &f_data : Services::game_state()->fighters) {
        auto &f = Services::db()->get_fighter_config(f_data.id);
        
        std::string text = std::to_string(f_data.count) + " : " + f.weapons[0].weapon.name;
        FighterUIElement fe = FighterUIElement(x, y_start + y * spacing, "combat_sprites", f.sprite_base, text);
        Services::ui()->add_element(fe, "fleet_ui");
        y++;
    }
}

void fleet_ui_show(std::function<void(void)> on_close) {
    Services::ui()->add_state("fleet_ui");

    list_blueprints();

    Button close_button = Button(gw / 2, gh - 20, "CLOSE");
    close_button.on_click = on_close;
	Services::ui()->add_element(close_button, "fleet_ui");
}

void fleet_ui_hide() {
    Services::ui()->remove_state("fleet_ui");
}

#endif