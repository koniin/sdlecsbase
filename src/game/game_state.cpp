#include "game_state.h"

#include "engine.h"

void GameState::new_game() {
    Engine::logn("Making new game state");

    //seed = RNG::range_i(0, 3000000);
    seed = 15;

    // mothership.abilities;

    mothership.defense.hp = 100;
    mothership.defense.shield = 50;

    mothership.population_max = 1000;

    mothership.sprite_base = "mother1";
    
    WeaponConfig w;
    
    mothership.weapons.push_back(w);

    population = mothership.population_max / 4;
    resources = 40;
    fighters_max = 8;

    // std::vector<FighterConfig> fighters;

}