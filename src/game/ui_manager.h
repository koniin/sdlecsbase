#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "engine.h"

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

    public:
        void update();
        void render();
        void show_text_toast(Vector2 position, std::string text, float ttl);
        void game_over();
        void battle_win();
};

#endif