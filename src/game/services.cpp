#include "services.h"

namespace Services {
    std::shared_ptr<UIManager> _ui_manager;
    std::shared_ptr<EventHub> _event_hub;
    std::shared_ptr<GameState> _game_state;
    std::shared_ptr<NodeEventManager> _node_event_manager;
    std::shared_ptr<DB> _db;

    void init() {
        _ui_manager = std::make_shared<UIManager>();
        _event_hub = std::make_shared<EventHub>();
        _game_state = std::make_shared<GameState>();
        _node_event_manager = std::make_shared<NodeEventManager>();
        _db = std::make_shared<DB>();
    }

    std::shared_ptr<UIManager> ui() {
        return _ui_manager;
    }

    std::shared_ptr<EventHub> events() {
        return _event_hub;
    }

    std::shared_ptr<GameState> game_state() {
        return _game_state;
    }
    
    std::shared_ptr<NodeEventManager> node_event_manager() {
        return _node_event_manager;
    }

    std::shared_ptr<DB> db() {
        return _db;
    }
}
