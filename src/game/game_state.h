#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "engine.h"
#include "weapons.h"

struct DefenseConfig {
    int hp;
    int shield;
};

struct WeaponConfig {
    Weapon weapon;
    short targeting;
};

struct AbilityConfig {

};

struct MothershipConfig {
    int population_max;
    std::string sprite_base;
    DefenseConfig defense;
    std::vector<WeaponConfig> weapons;
    std::vector<AbilityConfig> abilities;
};

struct FighterConfig {
    std::string sprite_base;
    DefenseConfig defense;
    std::vector<WeaponConfig> weapons;
};

struct GameState {
    void new_game();

    int seed;
    int difficulty = 0;
    int node_distance = 0;

    MothershipConfig mothership;
    std::vector<FighterConfig> fighters;

    int population;
    int resources;
    int fighters_max;
};

#endif