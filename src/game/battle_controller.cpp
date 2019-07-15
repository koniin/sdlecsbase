#include "battle_controller.h"

#include "engine.h"
#include "services.h"
#include "game_input_wrapper.h"
#include "particle_config.h"
#include "systems.h"
#include "unit_creator.h"

#include <unordered_set>

namespace BattleController {
    const int c_mothership_count = 2;
    const int c_fighter_count = 40;
    const int c_projectile_count = 300;
    const int c_projectile_spawn_count = 300;
    
    int _player_count_last = 0;
    int _enemy_count_last = 0;

    Rectangle world_bounds;

    ECS::EntityManager entity_manager;
    std::vector<MotherShip> _motherships;
    std::vector<FighterShip> _fighter_ships;
    std::vector<Projectile> _projectiles;
    std::vector<ProjectileMiss> _projectile_missed;
    std::vector<Effect> _effects;

    Particles::ParticleContainer particles;
    struct ParticleConfiguration {
        Particles::Emitter explosion_emitter;
        Particles::Emitter hit_emitter;
        Particles::Emitter exhaust_emitter;
        Particles::Emitter smoke_emitter;
    } particle_config;
    
    std::vector<ProjectileSpawn> _projectile_spawns;

    CollisionPairs collision_pairs;

    bool get_one_target(const int &exclude_faction, std::vector<ECS::EntityId> &target_overrides, Targeting::Target &target) {
        int target_faction = exclude_faction == PLAYER_FACTION ? ENEMY_FACTION : PLAYER_FACTION;
        
        /// Find override matches
        std::vector<Targeting::Target> matches;
        if(target_overrides.size() > 0) {
            for(auto &f : _fighter_ships) {
                for(auto &t : target_overrides) {
                    if(f.entity.id == t) {
                        matches.push_back({ f.entity, f.position.value });
                        break;
                    }
                }
            }
            for(auto &f : _motherships) {
                for(auto &t : target_overrides) {
                    if(f.entity.id == t) {
                        matches.push_back({ f.entity, f.position.value });
                        break;
                    }
                }
            }
        }

        if(matches.size() > 0) {
            int target_index = RNG::range_i(0, matches.size() - 1);
            target = matches[target_index];
            return true;
        }

        // No targets found that are in overrides so dont use those anymore
        target_overrides.clear();
        /// End override matches

        /// find fighter matches
        for(auto &f : _fighter_ships) {
            if(f.faction.faction == target_faction) {
                matches.push_back({ f.entity, f.position.value });
            }
        }

        if(matches.size() > 0) {
            int target_index = RNG::range_i(0, matches.size() - 1);
            target = matches[target_index];
            return true;
        }
        
        /// End fighter matches

        // find mothership matches
        for(auto &f : _motherships) {
            if(f.faction.faction == target_faction) {
                matches.push_back({ f.entity, f.position.value });
            }
        }

        if(matches.size() > 0) {
            int target_index = RNG::range_i(0, matches.size() - 1);
            target = matches[target_index];
            return true;
        }

        return false;
    }
    
    struct LaneData {
        int forward_count = 0; // closest to enemy
        int middle_count = 0;
        int back_count = 0;
    };

    LaneData player_lanes;
    LaneData enemy_lanes;

    void initialize() {
        world_bounds = Rectangle(0, 0, (int)gw, (int)gh);
        _motherships.reserve(2);
        _fighter_ships.reserve(c_fighter_count);
        _projectiles.reserve(c_projectile_count);
        _projectile_missed.reserve(c_projectile_count);
        _projectile_spawns.reserve(c_projectile_spawn_count);
        collision_pairs.allocate(c_fighter_count * c_projectile_count);
        particles = Particles::make(4096);
        particle_emitters_configure(&particle_config);
        UnitCreator::init_targeters(get_one_target);
        // Services::events().listen<EntityDestroyedEvent>(&entity_destroyed);
    }

    void spawn_fighter(const FighterData &f, int max_count) {
        Vector2 position = Vector2(170, 0);
        float y_start = 50;
        int *i;
        switch(f.fighter_type) {
            case FighterData::Type::Interceptor: {
                position.x = 280;
                i = &player_lanes.forward_count;
                break;
            }
            case FighterData::Type::Cruiser: {
                position.x = 200;
                i = &player_lanes.middle_count;
                break;
            }
            case FighterData::Type::Destroyer: {
                position.x = 140;
                i = &player_lanes.back_count;
                break;
            }
        }

        for(int j = (*i); j < f.count; j++) {
            (*i)++;
            if(*i >= max_count) {
                Engine::logn("max reached: %d", *i);
                return;
            }

            position.y = y_start + (float)(j) * 30;
            auto fighter = UnitCreator::create_fighter(f, PLAYER_FACTION, position, entity_manager);
            _fighter_ships.push_back(fighter);
        }
    }

    void spawn(int type, int max_spawns) {
        if(type == 0) { // interceptor 
            spawn_fighter({ 0, 1, FighterData::Type::Interceptor }, max_spawns);
        }
    }

    void create(std::shared_ptr<GameState> game_state) {
        UnitCreator::create_player_mothership(game_state->mothership, entity_manager, _motherships);
        
        for(auto &f : game_state->fighters) {
            spawn_fighter(f, game_state->fighters_max);
        }
        
        UnitCreator::create_enemy_mothership(game_state->seed, game_state->difficulty, game_state->node_distance, entity_manager, _motherships);
        UnitCreator::create_enemy_fighters(game_state->seed, game_state->difficulty, game_state->node_distance, entity_manager, _fighter_ships);
    }

    void end(std::shared_ptr<GameState> game_state) {

        // UPDATE game state with outcome?

        _motherships.clear();
        _fighter_ships.clear();
        _projectiles.clear();
        _projectile_missed.clear();
        _projectile_spawns.clear();
        Particles::clear(particles);

        _player_count_last = 0;
        _enemy_count_last = 0;
    }

    template<typename E>
    bool entity_remove(const E& e) {
        if(e.life_time.marked_for_deletion) { 
            entity_manager.destroy(e.entity);
        }
        return e.life_time.marked_for_deletion;
    }
    
    template<typename Entity>
    void system_abilities(Entity &entities) {
        for (auto &entity : entities) {
            entity.abilities.update_timer(Time::delta_time);
            
            for(auto &id : entity.abilities.ids()) {
                if(entity.abilities.is_manual(id)) {
                    int ability_id = GInput::pressed_id();
                    if(entity.abilities.can_use(ability_id)) {
                        entity.abilities.use(ability_id, entity.faction.faction, entity.position.value, _projectile_spawns, _effects);
                    }
                } else {
                    if(entity.abilities.can_use(id)) {
                        entity.abilities.use(id, entity.faction.faction, entity.position.value, _projectile_spawns, _effects);
                    }
                }
            }
        }
    }

    template<typename Entity>
    void system_destroy_explode_entities(Entity &entities) {
        for(auto &entity : entities) {
            if(entity.defense.hp <= 0) {
                particle_config.explosion_emitter.position = entity.position.value;
                Particles::emit(particles, particle_config.explosion_emitter);
                entity.life_time.marked_for_deletion = true;
            }
        }
    }

    template<typename T> 
    void collision_handle(Projectile &projectile, T &ship, const CollisionPair &entities) {
        projectile.life_time.marked_for_deletion = true;
        // Handle global reductions here like invulnerability and stuff
        // Evasion etc?
        float chance = RNG::zero_to_one();
        if(!weapon_is_beam(projectile.payload.projectile_type) && projectile.payload.accuracy <= chance) {
            Services::ui()->show_text_toast(projectile.position.value, "MISS!", 1.0f);
            _projectile_missed.push_back(ProjectileMiss(entity_manager.create(), projectile));
            return;
        }
        ship.defense.handle(projectile.payload);
        
        particle_config.smoke_emitter.position = ship.position.value;
        Particles::emit(particles, particle_config.smoke_emitter);
        ship.sprite.set_current_animation("hit", "idle");
        //fighter->sprite.set_current_animation("hit");
        //SpriteAnimation::set_current(fighter->animation, "hit");
    }

    template<typename First, typename Second>
    void system_collision_resolution(CollisionPairs &collision_pairs, std::vector<First> &entity_first, std::vector<Second> &entity_second) {
        collision_pairs.sort_by_distance();
        // This set will contain all collisions that we have handled
        // Since first in this instance is projectile and the list is sorted by distance
        // we only care about the collision with the shortest distance in this implementation
        std::unordered_set<ECS::EntityId> handled_collisions;
        for(int i = 0; i < collision_pairs.count; ++i) {
            if(handled_collisions.find(collision_pairs[i].first.id) != handled_collisions.end()) {
                continue;
            }
            handled_collisions.insert(collision_pairs[i].first.id);
            
            std::vector<First>::iterator first = std::find_if(entity_first.begin(), entity_first.end(), [&](First const& entity){
                return entity.entity.equals(collision_pairs[i].first);
            });
            std::vector<Second>::iterator second = std::find_if(entity_second.begin(), entity_second.end(), [&](Second const& entity){
                return entity.entity.equals(collision_pairs[i].second);
            });
            if(first == entity_first.end() || second == entity_second.end()) {
                continue;
            }
            
            collision_handle(*first, *second, collision_pairs[i]);
        }
    }

    template<typename First, typename Second>
    void system_effects(First &first, Second &second) {
        for(auto &effect : _effects) {
            effect.tick_timer += Time::delta_time;
            effect.ttl_timer += Time::delta_time;
            if(effect.tick_timer >= effect.tick) {
                for(auto &entity : first) {
                    if(effect.target_faction == ALL_FACTIONS || effect.target_faction == entity.faction.faction) {
                        entity.defense.apply(effect);
                        entity.abilities.apply(effect);
                    }     
                }
                for(auto &entity : second) {
                    if(effect.target_faction == ALL_FACTIONS || effect.target_faction == entity.faction.faction) {
                        entity.defense.apply(effect);
                        entity.abilities.apply(effect);
                    }     
                }

                effect.tick_timer = 0.0f;
            }    
        }
    }

    void update() {
        Particles::update(particles, Time::delta_time);

        system_abilities(_motherships);
        system_abilities(_fighter_ships);

        system_shield_recharge(_motherships);
        system_shield_recharge(_fighter_ships);
        
        system_homing(_projectiles);
        system_homing(_projectile_missed);
        
        // Move forward system
        system_move_forward(_projectiles);
        system_move_forward(_projectile_missed);
        
        system_velocity_increase(_projectiles);
        system_velocity_increase(_projectile_missed);

        system_collisions(collision_pairs, _projectiles, _fighter_ships);
        system_collision_resolution(collision_pairs, _projectiles, _fighter_ships);
        collision_pairs.clear();

        system_collisions(collision_pairs, _projectiles, _motherships);
        system_collision_resolution(collision_pairs, _projectiles, _motherships);
        collision_pairs.clear();

        system_animation(_fighter_ships);
        system_animation(_motherships);
        
        system_effects(_fighter_ships, _motherships);

        system_destroy_explode_entities(_fighter_ships);
        system_destroy_explode_entities(_motherships);
        
        system_update_ttl(_projectiles);
        system_update_ttl(_projectile_missed);
        
        system_remove_outside(_projectiles, world_bounds);
        system_remove_outside(_projectile_missed, world_bounds);

        // Remove entities with no lifetime left
        _fighter_ships.erase(std::remove_if(_fighter_ships.begin(), _fighter_ships.end(), entity_remove<FighterShip>), _fighter_ships.end());
        _motherships.erase(std::remove_if(_motherships.begin(), _motherships.end(), entity_remove<MotherShip>), _motherships.end());
        _projectiles.erase(std::remove_if(_projectiles.begin(), _projectiles.end(), entity_remove<Projectile>), _projectiles.end());
        _projectile_missed.erase(std::remove_if(_projectile_missed.begin(), _projectile_missed.end(), entity_remove<ProjectileMiss>), _projectile_missed.end());

        // Spawn projectiles
        for(auto &pspawn : _projectile_spawns) {
            ASSERT_WITH_MSG(pspawn.faction == PLAYER_FACTION || pspawn.faction == ENEMY_FACTION, "undefined faction occured while spawning projectile.");

            pspawn.timer += Time::delta_time;
            if(pspawn.timer >= pspawn.delay) {
                UnitCreator::create_projectile(pspawn, entity_manager, _projectiles);
            }
        }

        // Remove spawned projectile spawns
        _projectile_spawns.erase(std::remove_if(_projectile_spawns.begin(), _projectile_spawns.end(), 
            [=](const ProjectileSpawn &projectile_spawn) -> bool { return projectile_spawn.timer >= projectile_spawn.delay; }), _projectile_spawns.end());
        
        // Remove expired effects
        _effects.erase(std::remove_if(_effects.begin(), _effects.end(), 
            [=](const Effect &effect) -> bool { return effect.ttl_timer >= effect.ttl; }), _effects.end());

        // Handle game over and battle win !
        auto player_count = 0;
        auto enemy_count = 0;
        for(auto &ship : _fighter_ships) {
            if(ship.faction.faction == PLAYER_FACTION) player_count++;
            if(ship.faction.faction == ENEMY_FACTION) enemy_count++;
        }
        for(auto &ship : _motherships) {
            if(ship.faction.faction == PLAYER_FACTION) player_count++;
            if(ship.faction.faction == ENEMY_FACTION) enemy_count++;
        }
        
        if(_player_count_last > 0 && player_count == 0) {
            Services::events()->push(BattleOverEvent { ENEMY_FACTION });
        } else if(_enemy_count_last > 0 && enemy_count == 0) {
            Services::events()->push(BattleOverEvent { PLAYER_FACTION });
        }

        _player_count_last = player_count;
        _enemy_count_last = enemy_count;
    }

    /////
    // UNIT SELECTION CODE

    // template<typename Entity>
    // void fill_rect(const Entity &entity, Rectangle &unit_rect) {
    //     unit_rect.x = (int)(entity.position.value.x - (entity.collision.aabb.w / 2));
    //     unit_rect.y = (int)(entity.position.value.y - (entity.collision.aabb.h / 2));
    //     unit_rect.w = entity.collision.aabb.w;
    //     unit_rect.h = entity.collision.aabb.h;
    // }

    // std::vector<ECS::EntityId> _selected_entities;
    // void select_units(Rectangle &r) {
    //     Rectangle unit_rect;
    //     _selected_entities.clear();

    //     for(auto &ship : _fighter_ships) {
    //         fill_rect(ship, unit_rect);
    //         if(ship.faction.faction == PLAYER_FACTION && r.intersects(unit_rect)) {
    //             _selected_entities.push_back(ship.entity.id);
    //         }
    //     }
    //     for(auto &ship : _motherships) {
    //         fill_rect(ship, unit_rect);
    //         if(ship.faction.faction == PLAYER_FACTION && r.intersects(unit_rect)) {
    //             _selected_entities.push_back(ship.entity.id);
    //         }
    //     }
    // }
    
    // bool get_target(Point &p, ECS::EntityId &target) {
    //     Rectangle unit_rect;
    //     for(auto &ship : _fighter_ships) {
    //         fill_rect(ship, unit_rect);
    //         if(unit_rect.contains(p)) {
    //             target = ship.entity.id;
    //             return true;
    //         }
    //     }
        
    //     for(auto &ship : _motherships) {
    //         fill_rect(ship, unit_rect);
    //         if(unit_rect.contains(p)) {
    //             target = ship.entity.id;
    //             return true;
    //         }
    //     }
    //     return false;
    // }

    // void set_targets(Point &p) {
    //     ECS::EntityId target;
    //     if(!get_target(p, target)) {
    //         return;
    //     }

    //     for(auto &ship : _fighter_ships) {
    //         for(auto selected : _selected_entities) {
    //             if(ship.entity.id == selected) {
    //                 ship.abilities.set_target_override(target);
    //                 break;
    //             }
    //         }
    //     }
    //     for(auto &ship : _motherships) {
    //         for(auto selected : _selected_entities) {
    //             if(ship.entity.id == selected) {
    //                 ship.abilities.set_target_override(target);
    //                 break;
    //             }
    //         }
    //     }
    // }
}