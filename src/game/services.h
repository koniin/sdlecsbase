#ifndef SERVICES_H
#define SERVICES_H

#include <memory>
#include "engine.h"
#include "ui_manager.h"
#include "events.h"
#include "game_state.h"

namespace Services {
    void init();
    
    UIManager &ui();
    EventHub &events();
    std::shared_ptr<GameState> game_state();
}

#endif