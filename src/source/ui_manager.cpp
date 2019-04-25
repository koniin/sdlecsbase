#include "ui_manager.h"
#include "engine.h"
#include "renderer.h"

void UIManager::update() {
    // Toasts
    for(auto &t : _toasts) {
        t.timer += Time::delta_time_fixed;
    }
    _toasts.erase(std::remove_if(_toasts.begin(), _toasts.end(), [](const Toast& t) { 
        return t.timer >= t.ttl;
    }), _toasts.end());
}

void UIManager::render() {
    for(auto t : _toasts) {
        draw_text_centered_str((int)t.pos.x, (int)t.pos.y, Colors::white, t.text);
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
