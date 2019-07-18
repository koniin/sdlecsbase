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
    
    Rectangle world_bounds;

    ECS::EntityManager entity_manager;
    std::vector<MotherShip> _motherships;
    std::vector<FighterShip> _fighter_ships;
    std::vector<Projectile> _projectiles;
    std::vector<ProjectileMiss> _projectile_missed;
    std::vector<Effect> _effects;

    EnergySystem player_energy_system;

    Particles::ParticleContainer particles;
    struct ParticleConfiguration {
        Particles::Emitter explosion_emitter;
        Particles::Emitter hit_emitter;
        Particles::Emitter exhaust_emitter;
        Particles::Emitter smoke_emitter;
    } particle_config;
    
    std::vector<ProjectileSpawn> _projectile_spawns;

    CollisionPairs collision_pairs;

    struct FleetAI {
        struct Fleet {
            int max_count = 8;
            std::vector<FighterData> fighters;
        };
        Fleet fleet;

        EnergySystem energy_system;

        int spawn_count = 1;
    };

    FleetAI fleet_ai;

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

    std::vector<std::vector<bool>> lanes;

    int next_free_in_lane(int lane) {
        for(size_t i = 0; i < lanes[lane].size(); i++) {
            if(!lanes[lane][i]) {
                lanes[lane][i] = true;
                return i;
            }
        }
        return -1;
    }

    int get_lane(FighterShip::FighterShipType type, int faction) {
        if(type == FighterShip::Interceptor) {
            return faction == PLAYER_FACTION ? 0 : 3;
        } else if(type == FighterShip::Cruiser) {
            return faction == PLAYER_FACTION ? 1 : 4;
        } else if(type == FighterShip::Destroyer) {
            return faction == PLAYER_FACTION ? 2 : 5;
        }
        return 9999;
    }

    void toggle_lane_position(FighterShip::FighterShipType type, int faction, int lane_position) {
        int lane = get_lane(type, faction);
        lanes[lane][lane_position] = !lanes[lane][lane_position];
    }

    void spawn_fighter(FighterData &f, int max_count, int faction) {
        if(f.count <= 0) {
            return;
        }

        Vector2 position = Vector2(170, 0);
        float y_start = 50;
        int i = 9000;
        int lane = 0;
        switch(f.fighter_type) {
            case FighterData::Type::Interceptor: {
                position.x = faction == PLAYER_FACTION ? 280 : 360;
                i = std::count_if(_fighter_ships.begin(), _fighter_ships.end(), [=](FighterShip &ship) {
                    return ship.faction.faction == faction && ship.type == FighterShip::Interceptor;
                });
                lane = faction == PLAYER_FACTION ? 0 : 3;
                break;
            }
            case FighterData::Type::Cruiser: {
                position.x = faction == PLAYER_FACTION ? 200 : 440;
                i = std::count_if(_fighter_ships.begin(), _fighter_ships.end(), [=](FighterShip &ship) {
                    return ship.faction.faction == faction && ship.type == FighterShip::Cruiser;
                });
                lane = faction == PLAYER_FACTION ? 1 : 4;
                break;
            }
            case FighterData::Type::Destroyer: {
                position.x = faction == PLAYER_FACTION ? 140 : 500;
                i = std::count_if(_fighter_ships.begin(), _fighter_ships.end(), [=](FighterShip &ship) {
                    return ship.faction.faction == faction && ship.type == FighterShip::Destroyer;
                });
                lane = faction == PLAYER_FACTION ? 2 : 5;
                break;
            }
        }

        if(i+1 > max_count) {
            Engine::logn("max reached: %d", i);
            return;
        }

        f.count--;

        int lane_pos = next_free_in_lane(lane);
        position.y = y_start + (float)lane_pos * 30;
        auto fighter = UnitCreator::create_fighter(f, faction, position, entity_manager);
        fighter.lane_position = lane_pos;
        _fighter_ships.push_back(fighter);
    }

    int get_energy_cost(FighterData::Type fighter_type) {
        switch(fighter_type) {
            case FighterData::Type::Interceptor: {
                return 20;
            }
            case FighterData::Type::Cruiser: {
                return 40;
            }
            case FighterData::Type::Destroyer: {
                return 60;
            }
        }
    }

    void spawn_of_type(int count, FighterData::Type fighter_type, std::vector<FighterData> &fighters, int fighters_max, int faction) {
        for(int i = 0; i < count; i++) {
            int &energy = faction == PLAYER_FACTION ? player_energy_system.current : fleet_ai.energy_system.current;
            int energy_cost = get_energy_cost(fighter_type);
            if(energy < energy_cost) {
                return;
            }

            energy -= energy_cost;

            for(auto &f : fighters) {
                if(f.fighter_type == fighter_type) {
                    spawn_fighter(f, fighters_max, faction);

                }
            }
        }
    }

    void spawn_enemies() {
        spawn_of_type(fleet_ai.spawn_count, fleet_ai.fleet.fighters[0].fighter_type, fleet_ai.fleet.fighters, fleet_ai.fleet.max_count, ENEMY_FACTION);
        
        // spawn_fighter(enemy_fleet.fighters[0], enemy_fleet.max_count, ENEMY_FACTION);

        // for(auto &f : enemy_fleet.fighters) {
        //     spawn_fighter(f, enemy_fleet.max_count, ENEMY_FACTION);
        // }


        // // spawn if there are slots left
        // int i, c, d;
        // for(auto &f : _fighter_ships) {
        //     if(f.faction.faction == PLAYER_FACTION) {
        //         continue;
        //     }

        //     if(f.type == FighterShip::Interceptor) {
        //         i++;
        //     }
        //     if(f.type == FighterShip::Cruiser) {
        //         c++;
        //     }
        //     if(f.type == FighterShip::Destroyer) {
        //         d++;
        //     }
        // }

        // if(i < enemy_fleet.max_count) {
        //     for(int i = enemy_fleet.max_count - i; i <= enemy_fleet.max_count; i++) {
        //         spawn_fighter(enemy_fleet.fighters[0], enemy_fleet.max_count, ENEMY_FACTION);
        //     }
        // }
        // if(c < enemy_fleet.max_count) {
            
        // }
        // if(d < enemy_fleet.max_count) {
            
        // }
    }

    void create(std::shared_ptr<GameState> game_state) {
        player_energy_system.current = 0;
        player_energy_system.max = 100;
        player_energy_system.recharge_amount = 2;
        player_energy_system.recharge_rate = 0.2f;

        UnitCreator::create_player_mothership(game_state->mothership, entity_manager, _motherships);
        
        lanes.clear();
        for(int i = 0; i < 3; i++) {
            lanes.push_back(std::vector<bool>(game_state->fighters_max));
        }

        // for(auto &f : game_state->fighters) {
        //     spawn_fighter(f, game_state->fighters_max, PLAYER_FACTION);
        // }
        
        int ENEMY_MAX_LANE_SIZE = 8;
        for(int i = 3; i < 6; i++) {
            lanes.push_back(std::vector<bool>(ENEMY_MAX_LANE_SIZE));
        }

        fleet_ai.fleet = {
            8, {
                { 0, 8, FighterData::Type::Interceptor },
                { 1, 1, FighterData::Type::Cruiser }
            }
        };
        
        fleet_ai.energy_system.current = 0;
        fleet_ai.energy_system.max = 100;
        fleet_ai.energy_system.recharge_amount = 2;
        fleet_ai.energy_system.recharge_rate = 0.3f;

        UnitCreator::create_enemy_mothership(game_state->seed, game_state->difficulty, game_state->node_distance, entity_manager, _motherships);
        //UnitCreator::create_enemy_fighters(game_state->seed, game_state->difficulty, game_state->node_distance, entity_manager, _fighter_ships);
    }

    void end(std::shared_ptr<GameState> game_state) {

        // UPDATE game state with outcome?

        _motherships.clear();
        _fighter_ships.clear();
        _projectiles.clear();
        _projectile_missed.clear();
        _projectile_spawns.clear();
        Particles::clear(particles);
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

    template<typename EntityVector>
    void system_update_lanes(EntityVector& entities) {
        for(auto &e : entities) {
            if(e.life_time.marked_for_deletion) { 
                toggle_lane_position(e.type, e.faction.faction, e.lane_position);
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
                int &energy = effect.user_faction == PLAYER_FACTION ? player_energy_system.current : fleet_ai.energy_system.current;
                if(energy > effect.tick_energy_cost) {
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

                    int energy_before = energy;
                    energy -= effect.tick_energy_cost;
                    Engine::logn("before: %d, after: %d", energy_before, energy);
                }

                effect.tick_timer = 0.0f;
            }    
        }
    }

    void system_energy_recharge(EnergySystem &e, const float dt) {
        e.recharge_timer += dt;
        if(e.recharge_timer >= e.recharge_rate) {
            e.current = Math::clamp_i(e.current + e.recharge_amount, 0, e.max);
            e.recharge_timer = 0.0f;
        }
    }
    
    void update() {
        Particles::update(particles, Time::delta_time);

        system_abilities(_motherships);
        system_abilities(_fighter_ships);

        system_energy_recharge(player_energy_system, Time::delta_time);
        system_energy_recharge(fleet_ai.energy_system, Time::delta_time);
        
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

        system_update_lanes(_fighter_ships);

        // Remove entities with no lifetime left
        _fighter_ships.erase(std::remove_if(_fighter_ships.begin(), _fighter_ships.end(), entity_remove<FighterShip>), _fighter_ships.end());
        _motherships.erase(std::remove_if(_motherships.begin(), _motherships.end(), entity_remove<MotherShip>), _motherships.end());
        _projectiles.erase(std::remove_if(_projectiles.begin(), _projectiles.end(), entity_remove<Projectile>), _projectiles.end());
        _projectile_missed.erase(std::remove_if(_projectile_missed.begin(), _projectile_missed.end(), entity_remove<ProjectileMiss>), _projectile_missed.end());

        spawn_enemies();

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
        for(auto &f : _motherships) {
            if(f.faction.faction == ENEMY_FACTION) {
                enemy_count = 1;
            } else if(f.faction.faction == PLAYER_FACTION) {
                player_count = 1;
            }
        }
        
        if(player_count == 0) {
            Services::events()->push(BattleOverEvent { ENEMY_FACTION });
        } else if(enemy_count == 0) {
            Services::events()->push(BattleOverEvent { PLAYER_FACTION });
        }
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