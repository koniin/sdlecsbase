#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "engine.h"
#include "components.h"
#include "game_controller.h"

/// GENERAL SYSTEMS (Reusable probably)
/// ============================================
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
/// ============================================

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

                post_update_commands.push_back([=]() {
                    GameController::player_projectile_fire(p.value);
                });


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


struct TravelDistanceSystem {
    void update(ECS::ArchetypeManager &arch_manager) {
        arch_manager.iterate<Position, TravelDistance, LifeTime>([](auto c, auto i) { // [this](auto c, auto i) {
            auto &p = c->index<Position>(i);
            auto &t = c->index<TravelDistance>(i);
            auto &l = c->index<LifeTime>(i);

            auto diff = p.value - p.last;
            t.amount = t.amount + diff.length();
            if(t.amount > t.target) {
                l.marked_for_deletion = true;
            }
        });
    }
};

struct ProjectileHitSystem {
    void update(ECS::ArchetypeManager &arch_manager) {
        arch_manager.iterate<TravelDistance, ProjectileDamageDistance>([&](auto c, auto i) { // [this](auto c, auto i) {
            auto &t = c->index<TravelDistance>(i);
            auto &pdd = c->index<ProjectileDamageDistance>(i);
            
            if(t.amount >= pdd.distance) {
                if(pdd.hit == 1) {
                    auto &hull = arch_manager.get_component<Hull>(pdd.target);
                    hull.amount = hull.amount - pdd.damage;

                    // An event ?
                    // Send that something took damage?

                } else {
                    auto &position = arch_manager.get_component<Position>(pdd.target);

                    // Maybe better as an event and anyone can react
                    Services::ui().show_text_toast(position.value, "MISS!", 2.0f);
                }
                pdd.distance = 999999; // we don't want to trigger this again
                 // In a normal ecs you would probably just remove the component ;D
            }
        });
    }
};

struct RemoveNoHullEntitiesSystem {
    void update(ECS::ArchetypeManager &arch_manager) {
        arch_manager.iterate<Hull, LifeTime>([](auto c, auto i) { // [this](auto c, auto i) {
            auto &h = c->index<Hull>(i);
            if(h.amount <= 0) {
                auto &l = c->index<LifeTime>(i);
                l.marked_for_deletion = true;

                Services::events().push(EntityDestroyedEvent { c->entity[i] });
            }
        });
    }
};

struct RemoveNoParentAliveEntitiesSystem {
    void update(ECS::ArchetypeManager &arch_manager) {
        arch_manager.iterate<ParentComponent, LifeTime>([&](auto c, auto i) { // [this](auto c, auto i) {
            auto &p = c->index<ParentComponent>(i);

            // if it's alive we check if it has a lifetime 
            // and remove it if marked for deletion
            if(arch_manager.is_alive(p.entity)) {
                if(arch_manager.has_component<LifeTime>(p.entity)) {
                    auto &pl = arch_manager.get_component<LifeTime>(p.entity);
                    if(!pl.marked_for_deletion) {
                        return;
                    }
                } else {
                    return;
                }
            } 
            
            auto &l = c->index<LifeTime>(i);
            l.marked_for_deletion = true;

            Services::events().push(EntityDestroyedEvent { c->entity[i] });
        });
    }
};

struct AIInputSystem {
    std::vector<std::function<void(void)>> post_update_commands;

    void post_update() {
        for(auto pc : post_update_commands) {
            pc();
        }
        post_update_commands.clear();
    }

    void update(ECS::ArchetypeManager &arch_manager) {
        arch_manager.iterate<AIComponent, Position>([&](auto c, auto i) { // [this](auto c, auto i) {
            auto &ai = c->index<AIComponent>(i);
            auto &p = c->index<Position>(i);
            ai.fire_cooldown = Math::max_f(0.0f, ai.fire_cooldown - Time::delta_time);

            if(ai.fire_cooldown > 0.0f) {
                return;
            }

            ai.fire_cooldown = 2.2f;

            post_update_commands.push_back([=]() {
                GameController::enemy_projectile_fire(p.value);
            });
        });
    }
};


// template<typename AI, typename Enemy>
// void system_ai_input(AI &entity_data, Enemy &entity_search_targets, Projectile &projectiles, GameAreaController *ga_ctrl) {
//     for(int i = 0; i < entity_data.length; i++) {
//         entity_data.ai[i].fire_cooldown = Math::max_f(0.0f, entity_data.ai[i].fire_cooldown - Time::delta_time);

//         if(entity_data.ai[i].fire_cooldown > 0.0f) {
//             continue;
//         }

//         for(int t_i = 0; t_i < entity_search_targets.length; t_i++) {
//             const Vector2 &ai_position = entity_data.position[i].value;
//             const Vector2 &target_position = get_position(entity_search_targets, entity_search_targets.entity[t_i]).value;
//             if(entity_data.ai[i].fire_range > Math::distance_v(ai_position, target_position)) {
//                 entity_data.ai[i].fire_cooldown = entity_data.weapon[i].fire_cooldown;;

//                 // const Vector2 direction = Math::direction(target_position, ai_position);
//                 float angle = Math::degrees_between_v(ai_position, target_position);
                
//                 //Vector2 projectile_velocity = direction * entity_data.weapon[i].projectile_speed;                

//                 ProjectileSpawn p = ProjectileSpawn(ai_position, angle, entity_data.weapon[i].projectile_speed, 1, 8, 1.0f, 0, 0);
//                 ga_ctrl->target_projectile_fire(p);
//                 continue; // only fire at one target
//             }
//         }
//     }
// }

#endif