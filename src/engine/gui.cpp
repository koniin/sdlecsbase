#include "gui.h"

void click_check(Rectangle &rect, 
        SDL_Color &color, 
        SDL_Color highlighted, 
        SDL_Color activated, 
        SDL_Color normal, 
        std::function<void(void)> on_click) {
    Point p;
    Input::mouse_current(p);
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

void TextElement::update() {
        // Point p;
        // Input::mouse_current(p);
        // if(rect.contains(p)) {
        //     color = highlighted;
        // } else {
        //     color = normal;
        // }
}

void TextElement::render() {
        if(align == UIAlign::Center) {
            draw_text_centered_str((int)position.x, (int)position.y, color, text);
        } else if(align == UIAlign::Left) {
            draw_text_str((int)position.x, (int)position.y, color, text);
        } else if(align == UIAlign::Right) {
            draw_text_right_str((int)position.x, (int)position.y, color, text);
        }
    }

Button::Button(int x, int y, std::string text) {
    position = { x, y };
    color = normal;
    TextCache::size(text.c_str(), &size.x, &size.y);
    size.x += 10;
    size.y += 5;
    _text = text;
}

void Button::update() {
    rect.x = position.x - size.x / 2;
    rect.y = position.y - size.y / 2;
    rect.w = size.x;
    rect.h = size.y;
    click_check(rect, color, highlighted, activated, normal, on_click);
}

void Button::render() {
    draw_g_rectangle_filled(rect.x, rect.y, rect.w, rect.h, color);
    draw_text_centered_str(position.x, position.y, text_color, _text);
}

void SelectBox::update() {
    if(_select_active == 1 && Input::mouse_left_up) {
        _select_active = 0;
        
        Rectangle select_area;
        select_area.x = start.x > end.x ? end.x : start.x;
        select_area.y = start.y > end.y ? end.y : start.y;
        select_area.w = start.x > end.x ? start.x - end.x : end.x - start.x;
        select_area.h = start.y > end.y ? start.y - end.y : end.y - start.y;
        release_func(select_area);
    }

    if(_select_active == 0 && Input::mouse_left_down) {
        _select_active = 1;

        start.x = Input::mousex;
        start.y = Input::mousey;
    }

    if(_select_active == 1) {
        end.x = Input::mousex;
        end.y = Input::mousey;
    }
}

void SelectBox::render() {
    if(_select_active == 1) {
        draw_g_rectangle_RGBA(start.x, start.y, end.x - start.x, end.y - start.y, 0, 255, 0, 255);
    }
}

void ClickAction::update() {
    if(Input::mouse_right_down) {
        Point p;
        Input::mouse_current(p);
        on_click(p);
    }
}

void ClickAction::render() {}