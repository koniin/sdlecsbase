#ifndef GAME_INPUT_WRAPPER_H
#define GAME_INPUT_WRAPPER_H

#include "engine.h"

namespace GInput {
    enum Action {
        Start = 0,
        Cancel = 1,
        Pause = 2,
        Fire_1 = 3,
        Fire_2 = 4,
        Fire_3 = 5,
        ACTION_COUNT
    };

    const static SDL_Keycode input_map[ACTION_COUNT] = {
        SDLK_RETURN, SDLK_BACKSPACE, SDLK_p, SDLK_1, SDLK_2, SDLK_3
    };

    // inline void direction(Vector2 &direction) {
    //     if(Input::key_down_k(input_map[Action::Left])) {
    //         direction.x = -1;
    //     } else if(Input::key_down_k(input_map[Action::Right])) {
    //         direction.x = 1;
    //     }

    //     if(Input::key_down_k(input_map[Action::Up])) {
    //         direction.y = 1;
    //     } else if(Input::key_down_k(input_map[Action::Down])) {
    //         direction.y = -1;
    //     }
    // }

    inline bool down(const Action &action) {
        return (action == Action::Start && Input::key_down_k(SDLK_SPACE)) 
            || Input::key_down_k(input_map[action]);
    }

    inline bool pressed(const Action &action) {
        return (action == Action::Start && Input::key_pressed(SDLK_SPACE)) 
            || Input::key_pressed(input_map[action]);
    }

    inline int pressed_weapon_id() {
        if(pressed(GInput::Action::Fire_1)) {
            return 0;
        } else if(pressed(GInput::Action::Fire_2)) {
            return 1;
        } else if(pressed(GInput::Action::Fire_3)) {
            return 2;
        }

        return -1;
    }
}

#endif