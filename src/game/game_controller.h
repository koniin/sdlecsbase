#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "engine.h"
#include "services.h"
#include "components.h"

namespace GameController {
    void create_weapon(ECS::ArcheType &archetype, Vector2 pos, ECS::Entity &parent, int id);

    ECS::ArcheType player_ship;
    ECS::ArcheType enemy_ship;
    ECS::ArcheType player_weapon_archetype;
    ECS::ArcheType enemy_weapon_archetype;
    ECS::ArcheType projectile;

    ECS::Entity player;
    ECS::Entity enemy;

    Vector2 player_pos;
    Vector2 enemy_pos;

    void entity_destroyed(EntityDestroyedEvent ev) {
        Engine::logn("recieved entity destroyed event");
        auto target = ev.entity;
        if(target.equals(player)) {
            Engine::logn("player destroyed!");
            Services::ui().game_over();
        } else if(target.equals(enemy)) {
            Engine::logn("enemy destroyed!");
            Services::ui().battle_win();
        }
    }

    void initialise() {
        ECS::ArchetypeManager &arch_manager = Services::arch_manager();

        player_pos = Vector2(100, 150);
        enemy_pos = Vector2((float)gw - 100, 150);

        player_ship = arch_manager.create_archetype<Position, SpriteComponent, Hull, LifeTime>(2);
        enemy_ship = arch_manager.create_archetype<Position, SpriteComponent, Hull, LifeTime>(2);
        player_weapon_archetype = arch_manager.create_archetype<PlayerInput, InputTriggerComponent, Position, SpriteComponent, Damage, ParentComponent, LifeTime>(20);
        enemy_weapon_archetype = arch_manager.create_archetype<AIComponent, InputTriggerComponent, Position, SpriteComponent, Damage, ParentComponent, LifeTime>(20);

        projectile = arch_manager.create_archetype<Position, SpriteComponent, Velocity, ProjectileDamageDistance, TravelDistance, LifeTime>(200);

        Services::events().listen<EntityDestroyedEvent>(&entity_destroyed);
    }
    
    void update() {
        
    }

    void create_player() {
        ECS::ArchetypeManager &arch_manager = Services::arch_manager();

        auto ent = arch_manager.create_entity(player_ship);
        if(arch_manager.is_alive(player_ship, ent)) {
            Position pos = Position(player_pos);
            arch_manager.set_component(ent, pos);
            arch_manager.set_component(ent, Hull(100));
            SpriteComponent s = SpriteComponent("combat_sprites", "ship1");
            s.layer = 10;
            s.flip = 0;
            arch_manager.set_component(ent, s);

            create_weapon(player_weapon_archetype, Vector2(120, 100), ent, 0);
            create_weapon(player_weapon_archetype, Vector2(120, 200), ent, 1);
        }
        player = ent;
    }

    void create_enemy() {
        ECS::ArchetypeManager &arch_manager = Services::arch_manager();

        auto ent = arch_manager.create_entity(enemy_ship);
        if(arch_manager.is_alive(enemy_ship, ent)) {
            Position pos = Position(enemy_pos);
            arch_manager.set_component(ent, pos);
            arch_manager.set_component(ent, Hull(100));
            SpriteComponent s = SpriteComponent("combat_sprites", "ship2");
            s.layer = 10;
            arch_manager.set_component(ent, s);

            auto weapon = arch_manager.create_entity(enemy_weapon_archetype);
            Position p_weap = Position((float)gw - 90, 150);
            arch_manager.set_component(weapon, p_weap);

            SpriteComponent s_weap = SpriteComponent("combat_sprites", "gun2");
            s_weap.layer = 15;
            s_weap.flip = 1;
            arch_manager.set_component(weapon, s_weap);
            arch_manager.set_component(weapon, ParentComponent { ent });
            arch_manager.set_component(weapon, AIComponent { 2.2f });
        }
        enemy = ent;
    }

    void create_weapon(ECS::ArcheType &archetype, Vector2 pos, ECS::Entity &parent, int trigger) {
        ECS::ArchetypeManager &arch_manager = Services::arch_manager();
        auto weapon = arch_manager.create_entity(archetype);
        Position p_weap = Position(pos);
        arch_manager.set_component(weapon, p_weap);
        SpriteComponent s_weap = SpriteComponent("combat_sprites", "gun1");
        s_weap.layer = 15;
        arch_manager.set_component(weapon, s_weap);
        arch_manager.set_component(weapon, ParentComponent { parent });
        arch_manager.set_component(weapon, InputTriggerComponent { trigger });
    }

    void player_projectile_fire(Vector2 position) {
        ECS::ArchetypeManager &arch_manager = Services::arch_manager();
        ECS::Entity target = enemy;
        Vector2 my_position = player_pos;

        if(!arch_manager.is_alive(target)) {
            return;
        }

        auto ent = arch_manager.create_entity(projectile);
        arch_manager.set_component(ent, Position(position));
        auto sc = SpriteComponent("combat_sprites", "bullet_1");
        sc.layer = 12;
        arch_manager.set_component(ent, sc);

        auto a = Math::direction_from_angle(0) * 500;
        arch_manager.set_component(ent, Velocity(a));

        auto pos = arch_manager.get_component<Position>(target);
        auto distance_to_target = (my_position - pos.value).length();

        ProjectileDamageDistance pdd;
        pdd.distance = distance_to_target;
        pdd.target = target;
        pdd.damage = 20;

        float distance = position.x + (float)gw;
        if(RNG::range_i(0, 4) > 1) { // 40% chance to miss?
            pdd.hit = 1; 
            distance = distance_to_target;
        } else {
            pdd.hit = 0;
        }

        arch_manager.set_component(ent, pdd);
        arch_manager.set_component(ent, TravelDistance(distance));
    }

    void enemy_projectile_fire(Vector2 position) {
        ECS::ArchetypeManager &arch_manager = Services::arch_manager();
        ECS::Entity target = player;
        Vector2 my_position = enemy_pos;

        if(!arch_manager.is_alive(target)) {
            return;
        }
        
        auto ent = arch_manager.create_entity(projectile);
        arch_manager.set_component(ent, Position(position));
        auto sc = SpriteComponent("combat_sprites", "bullet_2");
        sc.layer = 12;
        arch_manager.set_component(ent, sc);

        auto pos = arch_manager.get_component<Position>(target);
        auto a = Math::direction(pos.value, position) * 500;
        arch_manager.set_component(ent, Velocity(a));

        auto distance_to_target = (my_position - pos.value).length();

        ProjectileDamageDistance pdd;
        pdd.distance = distance_to_target;
        pdd.target = target;
        pdd.damage = 20;

        float distance = position.x + (float)gw;
        if(RNG::range_i(0, 4) > 1) { // 40% chance to miss?
            pdd.hit = 1; 
            distance = distance_to_target;
        } else {
            pdd.hit = 0;
        }

        arch_manager.set_component(ent, pdd);
        arch_manager.set_component(ent, TravelDistance(distance));
    }
}

#endif