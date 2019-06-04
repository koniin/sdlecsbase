#ifndef BATTLE_CONTROLLER_H
#define BATTLE_CONTROLLER_H

#include "engine.h"
#include "components.h"
#include "game_state.h"
#include "particles.h"

namespace BattleController {
    extern std::vector<MotherShip> _motherships;
    extern std::vector<FighterShip> _fighter_ships;
    extern std::vector<Projectile> _projectiles;
    extern std::vector<ProjectileMiss> _projectile_missed;
    extern Particles::ParticleContainer particles;

    void initialize();
    void create(std::shared_ptr<GameState> game_state);
    void end(std::shared_ptr<GameState> game_state);
    void update();
}

#endif