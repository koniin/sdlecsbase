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

            _weapons.push_back(w);
        }
    } else {
        Engine::log("\n[WARNING] unable to open weapons file");
    } 
}

void load_fighters() {
    std::string file = "fighter.data";
    std::string file_name = Engine::get_base_data_folder() + file;
    std::ifstream fighter_data(file_name);

    if (!fighter_data.is_open()) {
        Engine::logn("ERROR: No data in fighter.data");
        Engine::exit();
        return;
    }
    
    if(fighter_data) {
        std::map<std::string, FighterType> f_type = {
            {  "FighterType::Interceptor", FighterType::Interceptor } ,
            {  "FighterType::Cruiser", FighterType::Cruiser } ,
            {  "FighterType::Destroyer", FighterType::Destroyer }
        };
        FighterConfig f;
        std::string line;
        std::getline(fighter_data, line); // ignore header
        // id|name|type|cost|energy_cost|sprite|defense|weapons
        while (std::getline(fighter_data, line)) {
            std::istringstream data(line);
            std::string value;
            std::getline(data, value, '|');
            f.id = std::stoi(value);
            std::getline(data, value, '|');
            f.name = value;
            std::getline(data, value, '|');
            f.type = f_type[value];
            
            std::getline(data, value, '|');
            auto index = value.find_first_of(',');
            f.cost.resource = std::stoi(value.substr(0, index));
            f.cost.population = std::stoi(value.substr(index + 1, value.size() - 1));
            
            std::getline(data, value, '|');
            f.energy_cost = std::stoi(value);
            
            std::getline(data, value, '|');
            f.sprite_base = value;

            std::getline(data, value, '|');
            auto defense_index = value.find_first_of(',');
            f.defense.hp = std::stoi(value.substr(0, defense_index)); 
            f.defense.shield = std::stoi(value.substr(defense_index + 1, value.size() - 1));
            
            std::getline(data, value, '|');
            auto weapon_index = value.find_first_of(',');
            WeaponConfig w;
            w.targeting = (short)std::stoi(value.substr(0, weapon_index)); 
            w.weapon_id = std::stoi(value.substr(weapon_index + 1, value.size() - 1));
            f.weapons.push_back(w);
            
            _fighters.push_back(f);
        }
    }
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