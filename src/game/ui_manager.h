#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "engine.h"
#include "gui.h"

/// Usage
/*
    * Declare:
        UIManager ui; 
    
    * Call in update:
        ui.update();
    
    * Call in render:
        ui.render();

    * For long living elements (like a button in a menu) that doesn't change:
        Button start_button = Button(gw / 2, gh / 2, "Start");
	    start_button.on_click = [] {
		    new_game();
	    };
	    ui.add_element(start_button);

    * If you want to update an item every frame it's better to use an immediate mode item like:
        TextElement t;
        t.color = Colors::white;
        t.position = Point(pos.x, pos.y); // where pos can change
        t.text = std::to_string(CHANGING_VARIABLE) + ""; // CHANGING_VARIABLE can change
        ui.add_immediate_element(t);

        => call ui.frame(); whenever you want to clear all immediate items

    * Toasts are for showing text that disappears after som TTL
*/

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