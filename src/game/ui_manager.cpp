#include "ui_manager.h"
#include "engine.h"
#include "renderer.h"
#include "game_input_wrapper.h"

void UIManager::frame() {
    _immediate_elements.clear();
}

void UIManager::update() {
    // Toasts
    for(auto &t : _toasts) {
        t.timer += Time::delta_time_fixed;
    }
    _toasts.erase(std::remove_if(_toasts.begin(), _toasts.end(), [](const Toast& t) { 
        return t.timer >= t.ttl;
    }), _toasts.end());

    for(auto t : _states[_current_state]->elements) {
        t->update();
    }
    for(auto t : _immediate_elements) {
        t->update();
    }
}

void UIManager::render() {
    for(auto t : _toasts) {
        draw_text_centered_str((int)t.pos.x, (int)t.pos.y, Colors::white, t.text);
    }
    for(auto t : _states[_current_state]->elements) {
        t->render();
    }
    for(auto t : _immediate_elements) {
        t->render();
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
    _states[_current_state]->elements.push_back(std::make_shared<TextElement>(t));
}

void UIManager::add_element(Button b) {
    _states[_current_state]->elements.push_back(std::make_shared<Button>(b));
}

void UIManager::add_immediate_element(TextElement t) {
    _immediate_elements.push_back(std::make_shared<TextElement>(t));
}

void UIManager::add_immediate_element(Button b) {
    _immediate_elements.push_back(std::make_shared<Button>(b));
}

void UIManager::clear() {
    _states[_current_state]->elements.clear();
    _immediate_elements.clear();
}