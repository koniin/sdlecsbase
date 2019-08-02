#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "abilities.h"
#include "data_repository.h"
#include "maze.h"

struct Point;
struct Maze;

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
    
    FighterData(int f_id, int c) {
        id = f_id;
        count = c;
    }
};

struct ResourceContainer {
    int resource;
    int population;
};

inline bool resources_available(const ResourceContainer r, const Cost c, const int count) {
    return r.population >= c.population * count && r.resource >= c.resource * count;
}

inline void resources_use(ResourceContainer &r, const Cost c, const int count) {
    r.population -= c.population * count;
    r.resource -= c.resource * count;
}

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

    ResourceContainer resources;
    int fighters_max;
    int spawn_count;
};

#endif