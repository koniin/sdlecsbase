#include "data_repository.h"
#include <fstream>

std::vector<FighterConfig> _fighters;
std::vector<Weapon> _weapons;

void load_weapons() {
    std::string file = "weapons.data";
    std::string file_name = Engine::get_base_data_folder() + file;
    std::ifstream weapon_data(file_name);

    if (!weapon_data.is_open()) {
        Engine::logn("ERROR: No data in weapons.data");
        Engine::exit();
        return;
    }
    
    if(weapon_data) {
        std::map<std::string, ProjectileType> proj = {
            {  "ProjectileType::Bullet", ProjectileType::Bullet } ,
            {  "ProjectileType::LazerBeamGreen", ProjectileType::LazerBeamGreen } ,
            {  "ProjectileType::LazerBulletRed", ProjectileType::LazerBulletRed } ,
            {  "ProjectileType::LazerBulletRedLarge", ProjectileType::LazerBulletRedLarge } ,
            {  "ProjectileType::Missile", ProjectileType::Missile } ,
            {  "ProjectileType::SmallBullet", ProjectileType::SmallBullet } 
        };
        Weapon w;
        
        std::string line;
        std::getline(weapon_data, line); // ignore header
        while (std::getline(weapon_data, line)) {
            std::istringstream data(line);
            std::string value;
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
        }
    } else {
        Engine::log("\n[WARNING] unable to open weapons file");
    } 
}

void load_fighters() {
    // std::string file = "weapons.data";
    // std::string file_name = Engine::get_base_data_folder() + file;
    // std::ifstream weapon_data(file_name);

    // if (!weapon_data.is_open()) {
    //     Engine::logn("ERROR: No data in weapons.data");
    //     Engine::exit();
    //     return;
    // }
    
    // if(weapon_data) {
    //     std::map<std::string, ProjectileType> proj = {
    //         {  "ProjectileType::Bullet", ProjectileType::Bullet } ,
    //         {  "ProjectileType::LazerBeamGreen", ProjectileType::LazerBeamGreen } ,
    //         {  "ProjectileType::LazerBulletRed", ProjectileType::LazerBulletRed } ,
    //         {  "ProjectileType::LazerBulletRedLarge", ProjectileType::LazerBulletRedLarge } ,
    //         {  "ProjectileType::Missile", ProjectileType::Missile } ,
    //         {  "ProjectileType::SmallBullet", ProjectileType::SmallBullet } 
    //     };
    //     Weapon w;
        
    //     std::string line;
    //     std::getline(weapon_data, line); // ignore header
    //     while (std::getline(weapon_data, line)) {
    //         std::istringstream data(line);
    //         std::string value;
    //         std::getline(data, w.name, '|');
    //         std::getline(data, value, '|');
    //         w.reload_time = std::stof(value);
    //         std::getline(data, value, '|');
    //         w.damage = std::stoi(value);
    //         std::getline(data, value, '|');
    //         w.accuracy = std::stof(value);
            
    //         std::getline(data, value, '|');
    //         w.projectile_type = proj[value];

    //         std::getline(data, value, '|');
    //         w.projectile_count = std::stoi(value);

    //         std::getline(data, value, '|');
    //         w.burst_delay = std::stof(value);

    //         std::getline(data, value, '|');
    //         w.radius = std::stoi(value);

    //         std::getline(data, value, '|');
    //         w.projectile_speed = std::stof(value);

    //         std::getline(data, value, '|');
    //         w.projectile_speed_increase = std::stof(value);
    //         std::getline(data, value, '|');
    //         w.projectile_speed_max = std::stof(value);
    //     }
    // } else {
    //     Engine::log("\n[WARNING] unable to open weapons file");
    // }
// Interceptor
{
    FighterConfig f;
    f.defense = { 5, 0 };
    f.sprite_base = "interceptor_1";
    WeaponConfig wc;
    wc.targeting = 2;
    wc.weapon_id = 0;
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
    wc.weapon_id = 5;
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
    wc.weapon_id = 5;
    f.weapons.push_back(wc);
    f.id = _fighters.size();
    f.name = "TEST Destroyer";
    f.cost = Cost(80, 20);
    f.type = FighterType::Destroyer;
    f.energy_cost = 60;
    _fighters.push_back(f);
 }
}

void DB::load() {
    load_weapons();
    load_fighters();  
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
    return _weapons[id];
    // return GLOBAL_BASE_WEAPON;
}