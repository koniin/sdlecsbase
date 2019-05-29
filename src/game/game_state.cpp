#include "game_state.h"
#include "engine.h"

void GameState::new_game() {
    Engine::logn("Making new game state");

    //seed = RNG::range_i(0, 3000000);
    seed = 15;

    maze_generate(maze, 22, 22);
	maze_grow_tree(&maze);

    // std::ostringstream out;
    // maze_log(&maze, out);
    // Engine::logn(out.str().c_str());

    current_node = Point(10, 10);
    //maze_open_all(&maze);
    
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

    
// if(w_choice == 0) {
//     weaponComponent = WeaponComponent(GLOBAL_BASE_WEAPON, "Missiles", _random_targeter, ProjectileType::Missile);
//     weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::Accuracy, 0.4f));
//     weaponComponent.add(ValueModifier<int>::make("temp", WeaponProperty::Damage, 1));
//     weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ProjectileSpeed, -400.0f));
//     weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ProjectileSpeedIncrease, 1.051f));
//     weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ProjectileSpeedMax, 300.5f));
//     weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ReloadTime, 2.0f));
// } else if(w_choice == 1) {
//     weaponComponent = WeaponComponent(GLOBAL_BASE_WEAPON, "Lazer Beam", _random_targeter, ProjectileType::GreenLazerBeam);
//     // Beams dont miss
//     weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::Accuracy, 0.5f));
//     weaponComponent.add(ValueModifier<int>::make("temp", WeaponProperty::Damage, 2));
//     weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ProjectileSpeed, -500.0f));
//     weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ReloadTime, 4.0f));
// } else {
//     weaponComponent = WeaponComponent(GLOBAL_BASE_WEAPON, "Lazer Gun", _random_targeter, ProjectileType::RedLazerBullet);
//     weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::Accuracy, 0.3f));        
// }


    for(int i = 0; i < 4; i++) {
        FighterConfig f;
        f.defense = { 10, 5 };
        f.sprite_base = "cs1";

        WeaponConfig wc;
        wc.targeting = 2;

        wc.weapon = GLOBAL_BASE_WEAPON;
        wc.weapon.name = "Lazer Gun";
        wc.weapon.projectile_type = ProjectileType::RedLazerBullet;
        wc.weapon.accuracy = 0.8f;

        f.weapons.push_back(wc);

        fighters.push_back(f);
    }
}

void GameState::prepare_node(const Point &next_node) {
    _next_node = next_node;
}

void GameState::end_node() {
    current_node = _next_node;
}