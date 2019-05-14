#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "engine.h"

enum UIAlign {
    Left,
    Center
};

struct Element {
    int layer = 0;
    Point position;
};

struct TextElement : public Element {
    std::string text;
    SDL_Color color;
    UIAlign align = UIAlign::Center;
};

struct ImageElement : public Element {
    
};

class UIManager {
    private:
        struct Toast {
            Vector2 pos;
            std::string text;
            float ttl;
            float timer;
        };
        std::vector<Toast> _toasts;
        
        bool is_game_over = false;
        bool is_battle_over = false;

        std::vector<TextElement> _textElements;

    public:
        void frame();
        void update();
        void render();
        void show_text_toast(Vector2 position, std::string text, float ttl);
        void add_element(TextElement t);
        void add_element(ImageElement t);
        void game_over();
        void battle_win();
};

#endif