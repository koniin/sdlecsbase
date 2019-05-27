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
    
    // Initialise with base
    w.weapon = GLOBAL_BASE_WEAPON;
    
    w.targeting = 1;
    w.weapon.name = "Mothership blast cannon";
    w.weapon.projectile_type = ProjectileType::Missile;
    w.weapon.accuracy = 1.0f;
    w.weapon.burst_delay = 0.1f;
    w.weapon.damage = 3;
    w.weapon.reload_time = 4.0f;
    w.weapon.projectile_speed = 100.0f;
    w.weapon.projectile_speed_increase = 1.031f;
    w.weapon.projectile_count = 8;
    
    mothership.weapons.push_back(w);

    population = mothership.population_max / 4;
    resources = 40;
    fighters_max = 8;

    // std::vector<FighterConfig> fighters;

}