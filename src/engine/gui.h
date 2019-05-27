#ifndef GUI_H
#define GUI_H

#include "engine.h"
#include "renderer.h"

enum UIAlign {
    Left,
    Center
};

struct Element {
    int layer = 0;
    int tag;
    Point position;

    virtual void update() = 0;
    virtual void render() = 0;
};

struct TextElement : public Element {
    std::string text;
    SDL_Color color;
    UIAlign align = UIAlign::Center;

    void update() override {
        // Point p;
        // Input::mouse_current(p);
        // if(rect.contains(p)) {
        //     color = highlighted;
        // } else {
        //     color = normal;
        // }
    }

    void render() override {
        if(align == UIAlign::Center) {
            draw_text_centered_str((int)position.x, (int)position.y, color, text);
        } else if(align == UIAlign::Left) {
            draw_text_str((int)position.x, (int)position.y, color, text);
        }
    }
};

// struct ImageElement : public Element {
    
// };

struct Button : public Element {
    Point size;
    Rectangle rect;
    std::string _text;

    std::function<void(void)> on_click = nullptr;

    SDL_Color color;
    SDL_Color text_color = Colors::white;
    SDL_Color normal = { 66, 134, 244, 255 };
    SDL_Color highlighted { 160, 196, 255, 255 };
    SDL_Color activated { 255, 120, 200, 255 };
    
    Button(int x, int y, std::string text) {
        position = { x, y };
        color = normal;
        TextCache::size(text.c_str(), &size.x, &size.y);
        size.x += 10;
        size.y += 5;

        _text = text;
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
        draw_g_rectangle_filled(rect.x, rect.y, rect.w, rect.h, color);
        draw_text_centered_str(position.x, position.y, text_color, _text);
    }
};

#endif