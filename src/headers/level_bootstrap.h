#ifndef LEVEL_BOOTSTRAP_H
#define LEVEL_BOOTSTRAP_H

#include "engine.h"
#include "components.h"

namespace LevelBootstrap {
    ECS::ArcheType player_ship;
    ECS::ArcheType enemy_ship;
    ECS::ArcheType weapon_archetype;
    ECS::ArcheType player_bullet;

    Vector2 player_pos;
    Vector2 enemy_pos;

    void initialise(ECS::ArchetypeManager &arch_manager) {
        player_pos = Vector2(100, 150);
        enemy_pos = Vector2((float)gw - 100, 150);

        player_ship = arch_manager.create_archetype<Position, SpriteComponent, PlayerInput, Hull>(2);
        enemy_ship = arch_manager.create_archetype<Position, SpriteComponent, Hull, ArtificalWeaponFiring>(2);
        weapon_archetype = arch_manager.create_archetype<Position, SpriteComponent, Damage>(2);

        player_bullet = arch_manager.create_archetype<Position, SpriteComponent, Velocity, TravelDistance, LifeTime>(100);
    }

    void create_player(ECS::ArchetypeManager &arch_manager) {
        auto ent = arch_manager.create_entity(player_ship);
        if(arch_manager.is_alive(player_ship, ent)) {
            Position pos = Position(player_pos);
            arch_manager.set_component(ent, pos);
            arch_manager.set_component(ent, Hull(100));
            SpriteComponent s = SpriteComponent("combat_sprites", "ship1");
            s.layer = 10;
            s.flip = 0;
            arch_manager.set_component(ent, s);

            auto weapon = arch_manager.create_entity(weapon_archetype);
            Position p_weap = Position(120, 150);
            arch_manager.set_component(weapon, p_weap);

            SpriteComponent s_weap = SpriteComponent("combat_sprites", "gun1");
            s_weap.layer = 15;
            arch_manager.set_component(weapon, s_weap);
        }
    }

    void create_enemy(ECS::ArchetypeManager &arch_manager) {
        auto ent = arch_manager.create_entity(enemy_ship);
        if(arch_manager.is_alive(enemy_ship, ent)) {
            Position pos = Position(enemy_pos);
            arch_manager.set_component(ent, pos);
            arch_manager.set_component(ent, Hull(100));
            SpriteComponent s = SpriteComponent("combat_sprites", "ship2");
            s.layer = 10;
            arch_manager.set_component(ent, s);

            auto weapon = arch_manager.create_entity(weapon_archetype);
            Position p_weap = Position((float)gw - 90, 150);
            arch_manager.set_component(weapon, p_weap);

            SpriteComponent s_weap = SpriteComponent("combat_sprites", "gun2");
            s_weap.layer = 15;
            s_weap.flip = 1;
            arch_manager.set_component(weapon, s_weap);
        }
    }
}

#endif