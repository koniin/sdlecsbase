#ifndef BATTLE_CONTROLLER_H
#define BATTLE_CONTROLLER_H

#include "components.h"
#include "game_state.h"
#include "particles.h"

namespace BattleController {
    extern std::vector<MotherShip> _motherships;
    extern std::vector<FighterShip> _fighter_ships;
    extern std::vector<Projectile> _projectiles;
    extern std::vector<ProjectileMiss> _projectile_missed;
    extern Particles::ParticleContainer particles;

    struct EnergySystem {
        int current;
        int max;
        float recharge_rate;
        float recharge_timer;
        int recharge_amount;  
    };
    extern EnergySystem player_energy_system;

    void initialize();
    void create(std::shared_ptr<GameState> game_state);
    void end(std::shared_ptr<GameState> game_state);
    void update();
    void select_units(Rectangle &r);
    void set_targets(Point &p);
    void spawn_of_type(int count, FighterType fighter_type, std::vector<FighterData> &fighters, int fighters_max, int faction);
}

#endif