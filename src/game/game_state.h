#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "engine.h"
#include "abilities.h"
#include "maze.h"
#include "data_repository.h"

struct AbilityConfig {
    enum { IsAbility, IsWeapon } type;

    Weapon weapon;
    short targeting;

    int usage_cost = 0;
};

struct MothershipConfig {
    int population_max;
    std::string sprite_base;
    DefenseConfig defense;
    std::vector<AbilityConfig> abilities;
};

struct FighterData {
    int id;
    int count = 0;
    enum Type {
        Interceptor,
        Cruiser,
        Destroyer
    } fighter_type;

    FighterData(int f_id, int c, Type t) {
        id = f_id;
        count = c;
        fighter_type = t;
    }
};

struct GameState {
    void new_game();
    void set_current_node(const Point &next_node);
    void set_current_node_completed();
    bool is_visited(const Point &node);

    int seed;
    int difficulty = 0;
    int node_distance = 0;

    Maze maze;
    Point start_node;
    Point current_node;
    Point _last_completed_node;

    std::vector<Point> _visited_nodes;

    MothershipConfig mothership;
    std::vector<FighterData> fighters;

    int population;
    int resources;
    int fighters_max;
    int spawn_count;
};

#endif