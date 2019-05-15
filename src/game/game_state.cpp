#include "game_state.h"

#include "engine.h"

void GameState::new_game() {
    Engine::logn("Making new game state");

    //seed = RNG::range_i(0, 3000000);
    seed = 15;
}