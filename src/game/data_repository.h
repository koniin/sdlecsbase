#ifndef DATA_REPOSITORY_H
#define DATA_REPOSITORY_H

#include "engine.h"
#include "abilities.h"

struct DefenseConfig {
    int hp;
    int shield;
};

struct WeaponConfig {
    Weapon weapon;
    short targeting;
};

struct Cost {
    int resource;
    int population;

    Cost() {}
    Cost(int res, int pop) : resource(res), population(pop) {}
};

enum FighterType {
    Interceptor,
    Cruiser,
    Destroyer
};

struct FighterConfig {
    int id;
    std::string name;
    std::string sprite_base;
    DefenseConfig defense;
    std::vector<WeaponConfig> weapons;
    Cost cost;
    FighterType type;
    int energy_cost;
};

struct DB {
    void load();
    std::vector<FighterConfig> &get_fighters();
    const FighterConfig &get_fighter_config(int id);
    const Weapon &get_ability_weapon(int id);
};

#endif