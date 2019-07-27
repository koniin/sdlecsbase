#include "data_repository.h"
#include <fstream>

std::vector<FighterConfig> _fighters;

Weapon GLOBAL_BASE_WEAPON = {
    "GLOBAL_BASE", // std::string name = "Basic weapon"; // (Blaster MK2 etc)
    0.5f, //float reload_time = 1.0f; // in seconds (0.2f)
    1, // int damage = 1;
    0.8f, // float accuracy = 0.5f;
    ProjectileType::SmallBullet, // ProjectileType projectile_type; // name of sprite for projectile
    1, // int projectile_count = 1;
    0.0f, // float burst_delay = 0.0f;
    6, // int radius = 8;
    500.0f, // float projectile_speed = 500.0f;
    0.0f, // float projectile_speed_increase = 0.0f;
    0.0f // float projectile_speed_max = 0.0f;
};

const std::vector<Weapon> _weapons {
    {
        "Lazer", // std::string name = "Basic weapon"; // (Blaster MK2 etc)
        0.5f, //float reload_time = 1.0f; // in seconds (0.2f)
        1, // int damage = 1;
        0.8f, // float accuracy = 0.5f;
        ProjectileType::LazerBulletRed, // ProjectileType projectile_type; // name of sprite for projectile
        1, // int projectile_count = 1;
        0.0f, // float burst_delay = 0.0f;
        6, // int radius = 8;
        300.0f, // float projectile_speed = 500.0f;
        0.0f, // float projectile_speed_increase = 0.0f;
        0.0f // float projectile_speed_max = 0.0f;
    },
    { // 1
        "Dual Lazer", // std::string name = "Basic weapon"; // (Blaster MK2 etc)
        0.6f, //float reload_time = 1.0f; // in seconds (0.2f)
        1, // int damage = 1;
        0.8f, // float accuracy = 0.5f;
        ProjectileType::LazerBulletRed, // ProjectileType projectile_type; // name of sprite for projectile
        2, // int projectile_count = 1;
        0.15f, // float burst_delay = 0.0f;
        6, // int radius = 8;
        300.0f, // float projectile_speed = 500.0f;
        0.0f, // float projectile_speed_increase = 0.0f;
        0.0f // float projectile_speed_max = 0.0f;
    },
    { // 2
        "Burst Lazer", // std::string name = "Basic weapon"; // (Blaster MK2 etc)
        1.2f, //float reload_time = 1.0f; // in seconds (0.2f)
        1, // int damage = 1;
        0.8f, // float accuracy = 0.5f;
        ProjectileType::LazerBulletRed, // ProjectileType projectile_type; // name of sprite for projectile
        5, // int projectile_count = 1;
        0.1f, // float burst_delay = 0.0f;
        6, // int radius = 8;
        300.0f, // float projectile_speed = 500.0f;
        0.0f, // float projectile_speed_increase = 0.0f;
        0.0f // float projectile_speed_max = 0.0f;
    },
    { // 3
        "Heavy Lazer", // std::string name = "Basic weapon"; // (Blaster MK2 etc)
        0.5f, //float reload_time = 1.0f; // in seconds (0.2f)
        2, // int damage = 1;
        0.8f, // float accuracy = 0.5f;
        ProjectileType::LazerBulletRedLarge, // ProjectileType projectile_type; // name of sprite for projectile
        1, // int projectile_count = 1;
        0.0f, // float burst_delay = 0.0f;
        6, // int radius = 8;
        200.0f, // float projectile_speed = 500.0f;
        0.0f, // float projectile_speed_increase = 0.0f;
        0.0f // float projectile_speed_max = 0.0f;
    },
    { // 4
        "Dual Heavy Lazer", // std::string name = "Basic weapon"; // (Blaster MK2 etc)
        0.65f, //float reload_time = 1.0f; // in seconds (0.2f)
        2, // int damage = 1;
        0.8f, // float accuracy = 0.5f;
        ProjectileType::LazerBulletRedLarge, // ProjectileType projectile_type; // name of sprite for projectile
        2, // int projectile_count = 1;
        0.15f, // float burst_delay = 0.0f;
        6, // int radius = 8;
        200.0f, // float projectile_speed = 500.0f;
        0.0f, // float projectile_speed_increase = 0.0f;
        0.0f // float projectile_speed_max = 0.0f;
    },
    { // 5
        "Rocket Battery", // std::string name = "Basic weapon"; // (Blaster MK2 etc)
        1.65f, //float reload_time = 1.0f; // in seconds (0.2f)
        1, // int damage = 1;
        0.8f, // float accuracy = 0.5f;
        ProjectileType::Missile, // ProjectileType projectile_type; // name of sprite for projectile
        5, // int projectile_count = 1;
        0.15f, // float burst_delay = 0.0f;
        3, // int radius = 8;
        100.0f, // float projectile_speed = 500.0f;
        4.0f, // float projectile_speed_increase = 0.0f;
        300.0f // float projectile_speed_max = 0.0f;
    }
};

void test_load() {

    std::string file = "weapons.data";
    std::string map_name = Engine::get_base_data_folder() + file;
    std::ifstream data(map_name);

    /*
    "Dual Heavy Lazer", // std::string name = "Basic weapon"; // (Blaster MK2 etc)
        0.65f, //float reload_time = 1.0f; // in seconds (0.2f)
        2, // int damage = 1;
        0.8f, // float accuracy = 0.5f;
        ProjectileType::LazerBulletRedLarge, // ProjectileType projectile_type; // name of sprite for projectile
        2, // int projectile_count = 1;
        0.15f, // float burst_delay = 0.0f;
        6, // int radius = 8;
        400.0f, // float projectile_speed = 500.0f;
        0.0f, // float projectile_speed_increase = 0.0f;
        0.0f // float projectile_speed_max = 0.0f;
        */
    if(data) {
        std::map<std::string, ProjectileType> proj = {
            {  "ProjectileType::Bullet", ProjectileType::Bullet } ,
            {  "ProjectileType::LazerBeamGreen", ProjectileType::LazerBeamGreen } ,
            {  "ProjectileType::LazerBulletRed", ProjectileType::LazerBulletRed } ,
            {  "ProjectileType::LazerBulletRedLarge", ProjectileType::LazerBulletRedLarge } ,
            {  "ProjectileType::Missile", ProjectileType::Missile } ,
            {  "ProjectileType::SmallBullet", ProjectileType::SmallBullet } 
        };
        Weapon w;
        std::string value;
        while ( data.good() )
        {
            std::getline(data, w.name, '|');
            std::getline(data, value, '|');
            w.reload_time = std::stof(value);
            std::getline(data, value, '|');
            w.damage = std::stoi(value);
            std::getline(data, value, '|');
            w.accuracy = std::stof(value);
            
            std::getline(data, value, '|');
            w.projectile_type = proj[value];

            std::getline(data, value, '|');
            w.projectile_count = std::stoi(value);

            std::getline(data, value, '|');
            w.burst_delay = std::stof(value);

            std::getline(data, value, '|');
            w.radius = std::stoi(value);

            std::getline(data, value, '|');
            w.projectile_speed = std::stof(value);

            std::getline(data, value, '|');
            w.projectile_speed_increase = std::stof(value);
            std::getline(data, value, '|');
            w.projectile_speed_max = std::stof(value);

            GLOBAL_BASE_WEAPON = w;
        }
    } else {
        Engine::log("\n[WARNING] unable to open tilemap file");
    } 

}

void DB::load() {
    test_load();

    // Interceptor
{
    FighterConfig f;
    f.defense = { 5, 0 };
    f.sprite_base = "interceptor_1";
    WeaponConfig wc;
    wc.targeting = 2;
    wc.weapon = _weapons[5];
    f.weapons.push_back(wc);
    f.id = _fighters.size();
    f.name = "Lazer Interceptor";
    f.cost = Cost(20, 5);
    f.type = FighterType::Interceptor;
    f.energy_cost = 20;
    _fighters.push_back(f);
}
    // Cruiser
 {
    FighterConfig f;
    f.defense = { 25, 5 };
    f.sprite_base = "cruiser_1";
    WeaponConfig wc;
    wc.targeting = 2;
    wc.weapon = _weapons[5];
    f.weapons.push_back(wc);
    f.id = _fighters.size();
    f.name = "Lazer Cruiser";
    f.cost = Cost(40, 10);
    f.type = FighterType::Cruiser;
    f.energy_cost = 40;
    _fighters.push_back(f);
 }
      // Destroyer
 {
    FighterConfig f;
    f.defense = { 60, 10 };
    f.sprite_base = "destroyer_1";
    WeaponConfig wc;
    wc.targeting = 2;
    wc.weapon = _weapons[5];
    f.weapons.push_back(wc);
    f.id = _fighters.size();
    f.name = "TEST Destroyer";
    f.cost = Cost(80, 20);
    f.type = FighterType::Destroyer;
    f.energy_cost = 60;
    _fighters.push_back(f);
 }
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

const Weapon &DB::get_ability_weapon(int id) {
    return GLOBAL_BASE_WEAPON;
    // return GLOBAL_BASE_WEAPON;
}