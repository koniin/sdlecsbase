#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "engine.h"
#include "gui.h"

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

        std::vector<std::shared_ptr<Element>> _elements;
        std::vector<std::shared_ptr<Element>> _immediate_elements;

    public:
        void frame();
        void update();
        void render();
        void show_text_toast(Vector2 position, std::string text, float ttl);
        void add_element(TextElement t);
        void add_element(Button b);

        void add_immediate_element(TextElement t);
        void add_immediate_element(Button b);

        void clear();
};

#endif