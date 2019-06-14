#include "ui_manager.h"
#include "engine.h"
#include "renderer.h"
#include "game_input_wrapper.h"


void UIManager::enable_state(std::string state) {
    _states[state]->enabled = true;
}

void UIManager::hide_state(std::string state) {
    _states[state]->enabled = false;
}

void UIManager::add_state(std::string state) {
    _states[state] = std::make_shared<UIState>();
    _states[state]->name = state;
}

void UIManager::remove_state(std::string state) {
    _states_to_remove.push_back(state);
}

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

    for(auto &s : _states) {
        if(s.second->enabled) {
            for(auto &t : s.second->elements) {
                t->update();
            }
        }
    }
    for(auto &t : _immediate_elements) {
        t->update();
    }

    for(auto &s : _states) {
        for(auto t : s.second->elements_to_add) {
            s.second->elements.push_back(t);
        }
        s.second->elements_to_add.clear();
    }

    for(auto &s : _states_to_remove) {
        _states.erase(s);
    }
    _states_to_remove.clear();
}

void UIManager::render() {
    for(auto t : _toasts) {
        draw_text_centered_str((int)t.pos.x, (int)t.pos.y, Colors::white, t.text);
    }
    for(auto &s : _states) {
        if(s.second->enabled) {
            for(auto &t : s.second->elements) {
                t->render();
            }
        }
    }
    for(auto &t : _immediate_elements) {
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

void UIManager::add_immediate_element(TextElement t) {
    _immediate_elements.push_back(std::make_shared<TextElement>(t));
}

void UIManager::add_immediate_element(Button b) {
    _immediate_elements.push_back(std::make_shared<Button>(b));
}

void UIManager::clear() {
    for(auto &s : _states) {
        s.second->elements.clear();
        s.second->elements_to_add.clear();
    }
    _immediate_elements.clear();
}