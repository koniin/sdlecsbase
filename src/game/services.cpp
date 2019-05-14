#include "services.h"

namespace Services {
    UIManager _ui_manager;
    EventHub _event_hub;
    std::shared_ptr<GameState> _game_state;

    void init() {
        _game_state = std::make_shared<GameState>();
    }

    UIManager &ui() {
        return _ui_manager;
    }

    EventHub &events() {
        return _event_hub;
    }

    std::shared_ptr<GameState> game_state() {
        return _game_state;
    }
}
