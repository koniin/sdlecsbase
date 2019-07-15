#include "game_state.h"
#include "engine.h"
#include "services.h"

void GameState::new_game() {
    Engine::logn("Making new game state");

    //seed = RNG::range_i(0, 3000000);
    seed = 15;

    maze_generate(maze, 22, 22);
	maze_grow_tree(&maze);

    // std::ostringstream out;
    // maze_log(&maze, out);
    // Engine::logn(out.str().c_str());
    
    start_node = Point(10, 10);
    current_node = start_node;
    //maze_open_all(&maze);
    
    // mothership.abilities;

    mothership.defense.hp = 40;
    mothership.defense.shield = 5;

    mothership.population_max = 1000;

    mothership.sprite_base = "mother1";
    
    
    AbilityConfig w;
    w.type = AbilityConfig::IsWeapon;
    
    // Initialise with base
    w.weapon = Services::db()->get_weapon(0);
    w.targeting = 1;

    w.weapon.name = "Dual Lazer";
    w.weapon.projectile_type = ProjectileType::LazerBulletRed;
    w.weapon.accuracy = 0.8f;
    w.weapon.projectile_count = 2;
    w.weapon.burst_delay = 0.1f;

    // Multi missile launcher
    // =========================
    // w.weapon.name = "Mothership blast cannon";
    // w.weapon.projectile_type = ProjectileType::Missile;
    // w.weapon.accuracy = 1.0f;
    // w.weapon.burst_delay = 0.1f;
    // w.weapon.damage = 3;
    // w.weapon.reload_time = 4.0f;
    // w.weapon.projectile_speed = 100.0f;
    // w.weapon.projectile_speed_increase = 1.031f;
    // w.weapon.projectile_count = 8;
    
    mothership.abilities.push_back(w);

    AbilityConfig a;
    a.type = AbilityConfig::IsAbility;
    a.abilityTest = 666;

    mothership.abilities.push_back(a);

    population = mothership.population_max / 4;
    resources = 30;
    fighters_max = 8; // per lane

    fighters.push_back(
        { 0, 28, FighterData::Type::Interceptor }
    );
    fighters.push_back(
        { 1, 1, FighterData::Type::Cruiser }
    );
}

void GameState::set_current_node(const Point &next_node) {
    current_node = next_node;
    
    int distance = (int)Math::distance_v(current_node.to_vector2(), start_node.to_vector2());
    node_distance = distance;
    _visited_nodes.push_back(current_node);    
}

void GameState::set_current_node_completed() {
    _last_completed_node = current_node;
}

bool GameState::is_visited(const Point &n) {
    bool is_node_visited = false;
    for(auto &node : _visited_nodes) {
        if(node == n) {
            is_node_visited = true;
            break;
        }
    }
    return is_node_visited;
}