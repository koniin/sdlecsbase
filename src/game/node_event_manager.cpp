#include "node_event_manager.h"
#include "services.h"

void NodeEventManager::clear() {
    screens.clear();
}

void NodeEventManager::next_screen() {
    // remove first item
    screens.erase(screens.begin());
}

void NodeEventManager::update() {
    if(screens.size() == 0) {   
        Services::ui()->enable_state("map_nav");
        return;
    }

    Services::ui()->hide_state("map_nav");

    auto &scr = screens[0];
    int i = 0;
    for(auto &option : scr.options) {
        if(i == 0 && Input::key_pressed(SDLK_1)) {
            option.on_action();
        }
        if(i == 1 && Input::key_pressed(SDLK_2)) {
            option.on_action();
        }
        if(i == 2 && Input::key_pressed(SDLK_3)) {
            option.on_action();
        }
        if(i == 3 && Input::key_pressed(SDLK_4)) {
            option.on_action();
        }
        i++;
    }
}

void NodeEventManager::render() {
    if(screens.size() == 0) {
        return;
    }
    SDL_Color bkg_color = Colors::blue;
    draw_g_rectangle_filled(150, 100, 340, 160, bkg_color);
    auto &scr = screens[0];
    draw_text_str(gw / 2 - 100, 120, text_color, scr.description);
    int i = 1;
    for(auto &option : scr.options) {
        draw_text_str(gw / 2 - 100, 120 + i * 20, text_color, option.text);
        i++;
    }
}

void NodeEventManager::start_event(const Node &n) {
    if(n.type == 1) {
        {
            EventScreen e;
            e.description = "You jumped straight into an ambush!";
            e.options.push_back( { "Continue", [&]() { Scenes::set_scene("level"); next_screen(); } } );
            screens.push_back(e);
            EventScreen e_after_battle;
            e_after_battle.description = "Yay battle is done! You got 50 resources, cool eh?";
            e_after_battle.options.push_back( { "Continue", [&]() { 
                    Services::game_state()->resources += 50;
                    next_screen();
                } 
            });
            screens.push_back(e_after_battle);
        }
    } else if(n.type == 2) {
        {
            EventScreen e;
            e.description = "Your sensors pick up small fleet in the outskirts of this system...";
            e.options.push_back( { "Continue", [&]() { next_screen(); } } );
            screens.push_back(e);
        }
        {
            EventScreen e;
            e.description = "Do you want to investigate?";
            e.options.push_back( { "yes", [&]() { Engine::logn("yes"); next_screen(); } } );
            e.options.push_back( { "no", [&]() { Engine::logn("no"); next_screen(); } } );
            screens.push_back(e);
        }
    } else if(n.type == 3) {
        Engine::logn("Node type 3 clicked, what to do?");
    } else {
        ASSERT_WITH_MSG(false, "get_node: returned non specified node");
    }
}