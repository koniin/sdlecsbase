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
    
    Button(int x, int y, std::string text);
    void update() override;
    void render() override;
};

struct SelectBox : public Element {
    int _select_active = 0;
    Point start;
    Point end;
    std::function<void(Rectangle r)> release_func;

    void update() override;
    void render() override;
};

struct ClickAction : public Element {
    std::function<void(Point p)> on_click;

    void update() override;
    void render() override;
};

#endif