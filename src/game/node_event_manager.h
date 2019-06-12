#ifndef NODE_EVENT_MANAGER_H
#define NODE_EVENT_MANAGER_H

#include "engine.h"
#include "renderer.h"
#include "map_node.h"

struct EventScreenOption {
    std::string text;
    std::function<void(void)> on_action;
};

struct EventScreen {
    std::string description;
    std::vector<EventScreenOption> options;
};

struct NodeEventManager {
    std::vector<EventScreen> screens;
    SDL_Color text_color = Colors::white;

    void clear();
    void next_screen();
    void update();
    void render();
    void start_event(const Node &n);
};

#endif