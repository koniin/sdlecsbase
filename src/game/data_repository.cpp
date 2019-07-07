#include "data_repository.h"

std::vector<FighterConfig> _fighters;

Weapon GLOBAL_BASE_WEAPON = {
    "GLOBAL_BASE", // std::string name = "Basic weapon"; // (Blaster MK2 etc)
    0.5f, //float reload_time = 1.0f; // in seconds (0.2f)
    1, // int damage = 1;
    0.8f, // float accuracy = 0.5f;
    ProjectileType::LazerBulletRed, // ProjectileType projectile_type; // name of sprite for projectile
    1, // int projectile_count = 1;
    0.0f, // float burst_delay = 0.0f;
    6, // int radius = 8;
    500.0f, // float projectile_speed = 500.0f;
    0.0f, // float projectile_speed_increase = 0.0f;
    0.0f // float projectile_speed_max = 0.0f;
};

void DB::load() {
    FighterConfig f;
    f.defense = { 10, 5 };
    f.sprite_base = "cs1";

    WeaponConfig wc;
    wc.targeting = 2;

    wc.weapon = GLOBAL_BASE_WEAPON;
    
    f.weapons.push_back(wc);

    f.id = _fighters.size();
    f.name = "Lazer frigate";

    _fighters.push_back(f);
}

std::vector<FighterConfig> &DB::get_fighters() {
    return _fighters;
}

const FighterConfig &DB::get_fighter_config(int id) {
    for(auto &f : _fighters) {
        if(f.id == id) {
            return f;
        }
    }
    ASSERT_WITH_MSG(false, "Fighter config not found!");
}

const Weapon &DB::get_weapon(int id) {
    return GLOBAL_BASE_WEAPON;
}