#include "gui.h"

Button::Button(int x, int y, std::string text) {
    position = { x, y };
    color = normal;
    TextCache::size(text.c_str(), &size.x, &size.y);
    size.x += 10;
    size.y += 5;
    _text = text;
}

void Button::update() {
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

void Button::render() {
    draw_g_rectangle_filled(rect.x, rect.y, rect.w, rect.h, color);
    draw_text_centered_str(position.x, position.y, text_color, _text);
}

void SelectBox::update() {
    if(_select_active == 1 && Input::mouse_left_up) {
        _select_active = 0;
        
        Rectangle select_area;
        release_func(select_area);
    }

    if(_select_active == 0 && Input::mouse_left_down) {
        _select_active = 1;
    }
}

void SelectBox::render() {
    if(_select_active == 1) {
        draw_g_rectangle_RGBA(100, 100, 100, 100, 255, 0, 0, 255);
    }
}
