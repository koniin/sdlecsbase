#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "engine.h"
#include "components.h"
#include "level_bootstrap.h"

struct PlayerInputSystem {
    void update(ECS::ArchetypeManager &arch_manager) {
        arch_manager.iterate<PlayerInput>([](auto c, auto i) { // [this](auto c, auto i) {
            auto &pi = c->index<PlayerInput>(i);
            pi.fire = Input::key_pressed(SDLK_1) ? 1 : 0;
            pi.fire_cooldown = Math::max_f(0.0f, pi.fire_cooldown - Time::delta_time);
            //     PlayerInput &pi = players.input[i];
            //     pi.move.x = pi.move.y = 0;
            //     pi.fire_x = 0;
            //     pi.fire_y = 0;

            //     GInput::direction(pi.move);

            //     pi.fire_cooldown = Math::max_f(0.0f, pi.fire_cooldown - Time::delta_time);

            //     if(GInput::down(GInput::Fire)) {
            //         pi.fire_x = pi.fire_y = 1;
            //     }

            //     if( GInput::pressed(GInput::Pause)) {
            //         Engine::pause(1.0f);
            //     }
        });
    }
};

struct PlayerHandleInputSystem {
    std::vector<std::function<void(void)>> post_update_commands;
    void post_update() {
        for(auto pc : post_update_commands) {
            pc();
        }
        post_update_commands.clear();
    }

    void update(ECS::ArchetypeManager &arch_manager) {
        arch_manager.iterate<PlayerInput, Position>([&](auto c, auto i) { // [this](auto c, auto i) {
            auto &pi = c->index<PlayerInput>(i);
            auto &p = c->index<Position>(i);
            if(pi.fire_cooldown <= 0.0f && pi.fire > 0) {
                
                pi.fire_cooldown = 2.0f; // fire_result.fire_cooldown;

                post_update_commands.push_back([&]() {
                    auto ent = arch_manager.create_entity(LevelBootstrap::player_bullet);
                    arch_manager.set_component(ent, Position(p.value));
                    auto sc = SpriteComponent("combat_sprites", "bullet_1");
                    sc.layer = 12;
                    arch_manager.set_component(ent, sc);

                    auto a = Math::direction_from_angle(0) * 500;
                    arch_manager.set_component(ent, Velocity(a));

                    float distance = p.value.x + (float)gw;
                    if(RNG::range_i(0, 4) > 1) { // 40% chance to miss?
                        distance = (LevelBootstrap::player_pos - LevelBootstrap::enemy_pos).length();
                        Engine::logn("HIT!");
                    } else {
                        Engine::logn("MISS!");
                    }
                    arch_manager.set_component(ent, TravelDistance(distance));
                });


                // auto ent = arch_manager.create_entity(enemy_ship);
                
                // Position pos = Position((float)gw - 100, 150);
                // arch_manager.set_component(enemy_ship, ent, pos);
                // arch_manager.set_component(enemy_ship, ent, Hull(100));
                // SpriteComponent s = SpriteComponent("combat_sprites", "ship2");
                // s.layer = 10;
                // arch_manager.set_component(enemy_ship, ent, s);
                

                // From config (depends on rendering size)
                // const float gun_barrel_distance = player_config.gun_barrel_distance;
                // float original_angle = direction.angle;
                // // set the projectile start position to be gun_barrel_distance infront of the ship
                // auto gun_exit_position = players.position[i].value + Math::direction_from_angle(original_angle) * gun_barrel_distance;
                
                // Ammunition &ammo = players.ammo[i];
                // auto fire_result = game_ctrl->player_projectile_fire(ammo.value, original_angle, gun_exit_position);

                // pi.fire_cooldown = fire_result.fire_cooldown;

                // if(!fire_result.did_fire) {
                //     Engine::logn("Implement some kind of out of ammo sound etc");
                //     return;
                // }

                // // this is for all projectiles
                // // ---------------------------------
                // ammo.value = Math::max_i(ammo.value - fire_result.ammo_used, 0);
                
                // // Muzzle flash
                // game_ctrl->spawn_muzzle_flash_effect(gun_exit_position, Vector2(gun_barrel_distance, gun_barrel_distance), players.entity[i]);
                // // Camera
                // auto projectile_direction = Math::direction_from_angle(original_angle);
                // camera_shake(0.1f);
                // camera_displace(projectile_direction * player_config.fire_knockback_camera);
                // // Player knockback
                // players.position[i].value -= projectile_direction * fire_result.knockback;
                // // Sound
                // Sound::queue(game_ctrl->sound_map[fire_result.sound_name], 2);
                // // ---------------------------------
            }
        });
    }
};

struct MoveForwardSystem {
    void update(ECS::ArchetypeManager &arch_manager) {
        arch_manager.iterate<Position, Velocity>([](auto c, auto i) { // [this](auto c, auto i) {
            auto &p = c->index<Position>(i);
            p.last = p.value;

            auto &v = c->index<Velocity>(i);

            p.value += v.value * Time::delta_time;
        });
    }
};

struct TravelDistanceSystem {
    void update(ECS::ArchetypeManager &arch_manager) {
        arch_manager.iterate<Position, TravelDistance, LifeTime>([](auto c, auto i) { // [this](auto c, auto i) {
            auto &p = c->index<Position>(i);
            auto &t = c->index<TravelDistance>(i);
            auto &l = c->index<LifeTime>(i);

            auto diff = p.value - p.last;
            t.amount = t.amount + diff.length();
            Engine::logn("%f", t.amount);

            if(t.amount > t.target) {
                l.marked_for_deletion = true;
            }
        });
    }
};

struct LifeTimeSystem {
    std::vector<ECS::Entity> deleted;

    void update(ECS::ArchetypeManager &arch_manager) {
        arch_manager.iterate<LifeTime>([this](auto c, auto i) { // [this](auto c, auto i) {
            auto &l = c->index<LifeTime>(i);
            if(l.marked_for_deletion) {
                deleted.push_back(c->entity[i]);
                l.marked_for_deletion = false;
            }
        });

        for(auto e : deleted) {
            arch_manager.remove_entity(e);
        }
        deleted.clear();
    }
};

/*

template<typename T>
void system_update_life_time(T &entity_data) {
    for(int i = 0; i < entity_data.length; i++) {
        if(entity_data.life_time[i].ttl > 0.0f) {
            entity_data.life_time[i].time += Time::delta_time;
            if(!entity_data.life_time[i].marked_for_deletion && entity_data.life_time[i].time >= entity_data.life_time[i].ttl) {
                entity_data.life_time[i].marked_for_deletion = true;
            }
        }
    }
}

template<typename T>
void system_remove_deleted(T &entity_data) {
    for(int i = 0; i < entity_data.length; i++) {
        if(entity_data.life_time[i].marked_for_deletion) {
            entity_data.life_time[i].marked_for_deletion = false;
            entity_data.remove(entity_data.entity[i]);
        }
    }
}
*/

#endif