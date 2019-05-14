#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "engine.h"
#include "services.h"
#include "components.h"
#include "game_input_wrapper.h"
#include "particles.h"
#include "particle_config.h"
#include "systems.h"

#include <unordered_set>

namespace GameController {
    const int c_mothership_count = 2;
    const int c_fighter_count = 40;
    const int c_projectile_count = 300;
    const int c_projectile_spawn_count = 300;
    const int PLAYER_FACTION = 100;
    const int ENEMY_FACTION = 200;
    
    const int MOTHERSHIP_LAYER = 100;
    const int FIGHTER_LAYER = 50;
    const int PROJECTILE_LAYER = 30;

    Rectangle world_bounds;

    ECS::EntityManager entity_manager;
    std::vector<MotherShip> _motherships;
    std::vector<FighterShip> _fighter_ships;
    std::vector<Projectile> _projectiles;
    std::vector<ProjectileMiss> _projectile_missed;
    
    Particles::ParticleContainer particles;
    struct ParticleConfiguration {
        Particles::Emitter explosion_emitter;
        Particles::Emitter hit_emitter;
        Particles::Emitter exhaust_emitter;
        Particles::Emitter smoke_emitter;
    } particle_config;
    
    std::vector<ProjectileSpawn> _projectile_spawns;

    CollisionPairs collision_pairs;

    struct TargetFinder {
        bool get_one_target(const int &exclude_faction, Targeting::Target &target) {
            int target_faction = exclude_faction == PLAYER_FACTION ? ENEMY_FACTION : PLAYER_FACTION;

            std::vector<FighterShip*> matches;
            for(auto &f : _fighter_ships) {
                if(f.faction.faction == target_faction) {
                    matches.push_back(&f);
                }
            }
            if(matches.size() > 0) {
                int target_index = RNG::range_i(0, matches.size() - 1);
                auto target_ship = matches[target_index];
                target.entity = target_ship->entity;
                target.position = target_ship->position.value;
                return true;
            }
            
            std::vector<MotherShip*> m_matches;
            for(auto &f : _motherships) {
                if(f.faction.faction == target_faction) {
                    m_matches.push_back(&f);
                }
            }
            if(m_matches.size() > 0) {
                int target_index = RNG::range_i(0, m_matches.size() - 1);
                auto target_ship = m_matches[target_index];
                target.entity = target_ship->entity;
                target.position = target_ship->position.value;
                return true;
            }

            return false;
        }
    };

    struct OneRandomTargeter : Targeting {
        TargetFinder target_finder;
        
        bool get_targets(const int &exclude_faction, const size_t &max_count, Targeting::Targets &targets) override {
            Targeting::Target target;
            if(target_finder.get_one_target(exclude_faction, target)) {
                for(size_t i = 0; i < max_count; i++) {
                    targets.targets.push_back(target);
                }
                return true;
            }
            return false;
        }
    };

    struct MultiRandomTargeter : Targeting {
        TargetFinder target_finder;
        
        bool get_targets(const int &exclude_faction, const size_t &max_count, Targeting::Targets &targets) override {
            Targeting::Target target;
            bool has_any_target = false;
            for(size_t i = 0; i < max_count; i++) {
                if(target_finder.get_one_target(exclude_faction, target)) {
                    targets.targets.push_back(target);
                    has_any_target = true;
                }
            }
            return has_any_target;
        }
    };

    std::shared_ptr<Targeting> _random_targeter;
    std::shared_ptr<Targeting> _random_multi_targeter;

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
        _random_targeter = std::make_shared<OneRandomTargeter>(OneRandomTargeter());
        _random_multi_targeter = std::make_shared<MultiRandomTargeter>(MultiRandomTargeter());
        // Services::events().listen<EntityDestroyedEvent>(&entity_destroyed);
    }

    void clear() {
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
    
    static void calc_lazer(SDL_Rect &lazer_rect, const Vector2 &start, const Vector2 &end, const int &height) {
        float distance = Math::distance_v(start, end);
        Vector2 difference = end - start;
        lazer_rect.x = (int)(start.x + (difference.x / 2) - (distance / 2));
        lazer_rect.y = (int)(start.y + (difference.y / 2) - (height / 2));
        lazer_rect.w = (int)distance;
        lazer_rect.h = height;
    }
    
    void spawn_projectile(ProjectileSpawn &spawn) {
        const float angle = Math::angle_between_v(spawn.position, spawn.target_position);
        const ProjectilePayLoad &payload = spawn.payload;
        
        auto sc = SpriteComponent("combat_sprites", weapon_projectile_sprite(spawn.projectile_type));
        sc.layer = PROJECTILE_LAYER;
        sc.rotation = angle;

        Projectile p(entity_manager.create());

        if(spawn.projectile_type == ProjectileType::GreenLazerBeam) {
            sc.line = true;
            auto direction = Math::direction_from_angle(angle);    
            // if(payload.accuracy < chance) {
            //     SDL_Rect lazer_rect;
            //     int height = payload.radius;
            //     auto ext = direction * 30;
            //     calc_lazer(lazer_rect, spawn.position, spawn.target_position + ext, height);

            //     sc.w = lazer_rect.w;
            //     sc.h = lazer_rect.h;

            //     ProjectileMiss p(entity_manager.create());
            //     p.position = Position(spawn.target_position);
            //     p.position.last = Vector2((float)lazer_rect.x, (float)lazer_rect.y);
            //     //p.velocity = Velocity();
            //     p.sprite = sc;
            //     p.life_time.ttl = 0.2f;
            //     _projectile_missed.push_back(p);
            // } else {
                SDL_Rect lazer_rect;
                int height = payload.radius;
                calc_lazer(lazer_rect, spawn.position, spawn.target_position, height);

                sc.w = lazer_rect.w;
                sc.h = lazer_rect.h;
                
                p.position = Position(spawn.target_position);
                p.position.last = Vector2((float)lazer_rect.x, (float)lazer_rect.y);
                p.collision = CollisionData(payload.radius);
                p.faction.faction = spawn.faction;
                p.payload = payload;
                p.sprite = sc;
                
                p.collision = CollisionData(payload.radius);
                _projectiles.push_back(p);
            //}
        } else if(spawn.projectile_type == ProjectileType::Missile) {
            auto homing_angle = angle + RNG::range_f(-40, 40);
            auto direction = Math::direction_from_angle(homing_angle);
            auto velocity = direction * spawn.projectile_speed;
        
            p.position = Position(spawn.position);
            p.faction.faction = spawn.faction;
            p.payload = payload;
            
            p.sprite = sc;
            p.velocity = Velocity(velocity);
            p.velocity.change = spawn.projectile_speed_increase;
            p.velocity.max = spawn.projectile_speed_max;

            p.collision = CollisionData(payload.radius);

            p.homing.enabled = true;
            p.homing.target = spawn.target;
            p.homing.target_position = spawn.target_position;

            _projectiles.push_back(p);
            
        } else {
            auto direction = Math::direction_from_angle(angle);
            auto velocity = direction * spawn.projectile_speed;
            
            p.position = Position(spawn.position);
            p.faction.faction = spawn.faction;
            p.payload = payload;
            
            p.sprite = sc;
            p.velocity = Velocity(velocity);
            p.velocity.change = spawn.projectile_speed_increase;

            p.collision = CollisionData(payload.radius);

            _projectiles.push_back(p);
            
        }
    }

    void create_player_mothership() {
        Vector2 position = Vector2(70, (float)gh / 2);

        MotherShip ship(entity_manager.create());
        ship.faction = FactionComponent { PLAYER_FACTION };
        SpriteComponent s = SpriteComponent({ 
                Animation("idle", { { "combat_sprites", "mother1" } }, 0, false),
                Animation("hit", { 
                    { "combat_sprites", "mother1_w" },
                    { "combat_sprites", "mother1" },
                    { "combat_sprites", "mother1_w" },
                    { "combat_sprites", "mother1" },
                    { "combat_sprites", "mother1_w" },
                    { "combat_sprites", "mother1" },
                    { "combat_sprites", "mother1_w" }
                },  8, false)
            });
        s.layer = MOTHERSHIP_LAYER;
        s.flip = 0;
        ship.sprite = s;
        ship.position = position;
        ship.defense = DefenseComponent(100, 50);

        WeaponComponent weaponComponent = WeaponComponent("Mothership blast cannon", _random_multi_targeter, ProjectileType::Missile);
        weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ProjectileSpeed, -400.0f));
        weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ProjectileSpeedIncrease, 1.031f));
        weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::Accuracy, 0.5f));
        weaponComponent.add(ValueModifier<int>::make("temp", WeaponProperty::Damage, 2));
        weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ReloadTime, 3.0f));
        weaponComponent.add(ValueModifier<int>::make("temp", WeaponProperty::Projectile_Count, 7));
        weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::BurstDelay, 0.1f));

        ship.weapons.add(weaponComponent, true);

        auto sprite_sheet_index = Resources::sprite_sheet_index("combat_sprites");
        auto rect = Resources::sprite_get_from_sheet(sprite_sheet_index, "mother1");
        ship.collision = CollisionData(rect.w, rect.h);

        _motherships.push_back(ship);
    }

    void create_enemy_mothership() {
        Vector2 position = Vector2((float)gw - 70, (float)gh / 2);

        MotherShip ship(entity_manager.create());
        ship.faction = FactionComponent { ENEMY_FACTION };
        SpriteComponent s = SpriteComponent({ 
                Animation("idle", { { "combat_sprites", "mother2" } }, 0, false),
                Animation("hit", { 
                    { "combat_sprites", "mother2_w" },
                    { "combat_sprites", "mother2" },
                    { "combat_sprites", "mother2_w" },
                    { "combat_sprites", "mother2" },
                    { "combat_sprites", "mother2_w" },
                    { "combat_sprites", "mother2" },
                    { "combat_sprites", "mother2_w" }
                },  8, false)
            });
        
        s.layer = MOTHERSHIP_LAYER;
        s.flip = 1;
        ship.sprite = s;
        ship.position = position;
        ship.defense = DefenseComponent(100, 20);

        auto sprite_sheet_index = Resources::sprite_sheet_index("combat_sprites");
        auto rect = Resources::sprite_get_from_sheet(sprite_sheet_index, "mother2");
        ship.collision = CollisionData(rect.w, rect.h);

        _motherships.push_back(ship);
    }

    void create_player_fighters() {
        Vector2 position = Vector2(170, 50);

        for(int i = 0; i < 10; i++) {
            FighterShip ship(entity_manager.create());
            ship.faction = FactionComponent { PLAYER_FACTION };
            float y = position.y + i * 30.f;
            ship.position = RNG::vector2(position.x - 10, position.x + 10, y - 8, y + 8);
            ship.defense = DefenseComponent(10, 5);

            SpriteComponent s = SpriteComponent({ 
                Animation("idle", { { "combat_sprites", "cs1" } }, 0, false),
                Animation("hit", { 
                    { "combat_sprites", "cs1_w" },
                    { "combat_sprites", "cs1" },
                    { "combat_sprites", "cs1_w" },
                    { "combat_sprites", "cs1" },
                    { "combat_sprites", "cs1_w" },
                    { "combat_sprites", "cs1" },
                    { "combat_sprites", "cs1_w" }
                },  6, false)
            });
            s.layer = FIGHTER_LAYER;
            s.flip = 0;
            ship.sprite = s;

            int w_choice = RNG::range_i(0, 2);

            WeaponComponent weaponComponent;
            if(w_choice == 0) {
                weaponComponent = WeaponComponent("Missiles", _random_targeter, ProjectileType::Missile);
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::Accuracy, 0.4f));
                weaponComponent.add(ValueModifier<int>::make("temp", WeaponProperty::Damage, 1));
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ProjectileSpeed, -400.0f));
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ProjectileSpeedIncrease, 1.051f));
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ProjectileSpeedMax, 300.5f));
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ReloadTime, 2.0f));
            } else if(w_choice == 1) {
                weaponComponent = WeaponComponent("Lazer Gun", _random_targeter, ProjectileType::GreenLazerBeam);
                // Beams dont miss
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::Accuracy, 0.5f));
                weaponComponent.add(ValueModifier<int>::make("temp", WeaponProperty::Damage, 2));
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ProjectileSpeed, -500.0f));
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ReloadTime, 4.0f));
            } else {
                weaponComponent = WeaponComponent("Player Gun", _random_targeter, ProjectileType::RedLazerBullet);
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::Accuracy, 0.3f));        
            }

            ship.automatic_fire = AutomaticFireComponent { weaponComponent.get_weapon().reload_time };
            ship.weapons.add(weaponComponent);

            auto sprite_sheet_index = Resources::sprite_sheet_index("combat_sprites");
            auto rect = Resources::sprite_get_from_sheet(sprite_sheet_index, "cs1");
            ship.collision = CollisionData(rect.w, rect.h);

            _fighter_ships.push_back(ship);
        }
    }

    void create_enemy_fighters() {
        Vector2 position = Vector2((float)gw - 170, 50);

        for(int i = 0; i < 10; i++) {
            FighterShip ship(entity_manager.create());
            ship.faction = FactionComponent { ENEMY_FACTION };
            float y = position.y + i * 30.f;
            ship.position = RNG::vector2(position.x - 10, position.x + 10, y - 8, y + 8);
            ship.defense = DefenseComponent(10, 5);
            SpriteComponent s = SpriteComponent({ 
                Animation("idle", { { "combat_sprites", "cs2" } }, 0, false),
                Animation("hit", { 
                    { "combat_sprites", "cs2_w" },
                    { "combat_sprites", "cs2" },
                    { "combat_sprites", "cs2_w"}, 
                    { "combat_sprites", "cs2" },
                    { "combat_sprites", "cs2_w"}
                },  12, false)
            });
            s.layer = FIGHTER_LAYER;
            s.flip = 1;
            ship.sprite = s;

            WeaponComponent weaponComponent = WeaponComponent("Enemy Gun", _random_targeter, ProjectileType::RedLazerBullet);
            weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::Accuracy, 0.3f));
            weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ReloadTime, 1.0f));
            // weaponComponent.add_modifier(std::make_unique<WeaponModifier>(ValueModifier<ProjectileType>("temp", WeaponProperty::Projectile_Type, )));
            // weaponComponent.add_modifier(std::make_unique<WeaponModifier>(ValueModifier<int>("temp", WeaponProperty::Projectile_Count, 0)));
            // weaponComponent.add_modifier(std::make_unique<WeaponModifier>(ValueModifier<float>("temp", WeaponProperty::BurstDelay, 0.1f)));

            ship.weapons.add(weaponComponent);

            ship.automatic_fire = AutomaticFireComponent { weaponComponent.get_weapon().reload_time };

            auto sprite_sheet_index = Resources::sprite_sheet_index("combat_sprites");
            auto rect = Resources::sprite_get_from_sheet(sprite_sheet_index, "cs2");
            ship.collision = CollisionData(rect.w, rect.h);

            _fighter_ships.push_back(ship);
        }
    }

    template<typename Entity>
    void system_weapons(Entity &entities) {
        for (auto &entity : entities) {
            entity.weapons.update_reload_timer(Time::delta_time);
            
            for(auto &id : entity.weapons.ids()) {
                if(entity.weapons.is_manual(id)) {
                    int weapon_id = GInput::pressed_weapon_id();
                    if(entity.weapons.can_fire(weapon_id)) {
                        entity.weapons.fire(weapon_id, entity.faction.faction, entity.position.value, _projectile_spawns);
                    }
                } else {
                    if(entity.weapons.can_fire(id)) {
                        entity.weapons.fire(id, entity.faction.faction, entity.position.value, _projectile_spawns);
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
            Services::ui().show_text_toast(projectile.position.value, "MISS!", 1.0f);
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

    void update() {
        system_weapons(_motherships);
        system_weapons(_fighter_ships);

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

        // Animation system
        for (auto &ship : _fighter_ships) { 
            ship.sprite.update_animation(Time::delta_time);
        }
        // Animation system
        for (auto &ship : _motherships) { 
            ship.sprite.update_animation(Time::delta_time);
        }
        
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
                spawn_projectile(pspawn);
            }
        }

        // Remove spawned projectile spawns
        _projectile_spawns.erase(std::remove_if(_projectile_spawns.begin(), _projectile_spawns.end(), 
            [=](const ProjectileSpawn &projectile_spawn) -> bool { return projectile_spawn.timer >= projectile_spawn.delay; }), _projectile_spawns.end());
        

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
        
        if(player_count == 0) {
            Services::ui().game_over();
        } else if(enemy_count == 0) {
             Services::ui().battle_win();
        }
    }
}

#endif