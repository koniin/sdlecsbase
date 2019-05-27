#include "ui_manager.h"
#include "engine.h"
#include "renderer.h"
#include "game_input_wrapper.h"

void UIManager::frame() {
    clear();
}

void UIManager::update() {
    // Toasts
    for(auto &t : _toasts) {
        t.timer += Time::delta_time_fixed;
    }
    _toasts.erase(std::remove_if(_toasts.begin(), _toasts.end(), [](const Toast& t) { 
        return t.timer >= t.ttl;
    }), _toasts.end());

	if((is_game_over || is_battle_over) && GInput::pressed(GInput::Action::Start)) {
		Scenes::set_scene("map");
        is_game_over = false;
        is_battle_over = false;
	}
}

void UIManager::render() {
    for(auto t : _toasts) {
        draw_text_centered_str((int)t.pos.x, (int)t.pos.y, Colors::white, t.text);
    }
    for(auto t : _textElements) {
        if(t.align == UIAlign::Center) {
            draw_text_centered_str((int)t.position.x, (int)t.position.y, t.color, t.text);
        } else if(t.align == UIAlign::Left) {
            draw_text_str((int)t.position.x, (int)t.position.y, t.color, t.text);
        }
    }

    if(is_game_over) {
        draw_text_centered_str((int)(gw / 2), (int)(gh / 2), Colors::white, "GAME OVER");
        draw_text_centered_str((int)(gw / 2), (int)(gh / 2) + 10, Colors::white, "Press start to continue...");
    } else if(is_battle_over) {
        draw_text_centered_str((int)(gw / 2), (int)(gh / 2), Colors::white, "YOU ROCK!");
        draw_text_centered_str((int)(gw / 2), (int)(gh / 2) + 10, Colors::white, "Press start to continue...");
    }
}

void UIManager::show_text_toast(Vector2 position, std::string text, float ttl) {
    Toast t;
    t.pos = position;
    t.text = text;
    t.ttl = ttl;
    t.timer = 0;
    _toasts.push_back(t);
}

void UIManager::add_element(TextElement t) {
    _textElements.push_back(t);
}

void UIManager::add_element(ImageElement t) {

}

void UIManager::game_over() {
    is_game_over = true;
}

void UIManager::battle_win() {
    is_battle_over = true;
}

void UIManager::clear() {
    _textElements.clear();
}