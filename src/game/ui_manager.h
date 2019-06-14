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

        struct UIState {
            std::string name;
            bool enabled = true;
            std::vector<std::shared_ptr<Element>> elements;
            std::vector<std::shared_ptr<Element>> elements_to_add;
        };

        const std::string __default_state_name = "___default_ui_state___";

        std::unordered_map<std::string, std::shared_ptr<UIState>> _states;
        std::vector<std::shared_ptr<Element>> _immediate_elements;
        std::vector<std::string> _states_to_remove;

    public:
        UIManager() {
            _states[__default_state_name] = std::make_shared<UIState>();
            _states[__default_state_name]->name = __default_state_name;
        }

        void enable_state(std::string state);
        void hide_state(std::string state);
        void add_state(std::string state);
        void remove_state(std::string state);

        void frame();
        void update();
        void render();
        void show_text_toast(Vector2 position, std::string text, float ttl);
        
        template<typename TElement>
        void add_element(TElement e, std::string state = "___default_ui_state___") {
            _states[state]->elements_to_add.push_back(std::make_shared<TElement>(e));
        }

        void add_immediate_element(TextElement t);
        void add_immediate_element(Button b);
        
        void clear();
};

#endif