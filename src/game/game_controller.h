#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "engine.h"
#include "services.h"
#include "components.h"
#include "game_input_wrapper.h"
#include "particles.h"
#include "particle_config.h"
#include "creator.h"

#include <unordered_set>

struct MotherShip {
    ECS::Entity entity;
    Position position;
    SpriteComponent sprite;
    MotherShip(ECS::Entity e) : entity(e) {}

    FactionComponent faction;
    MultiWeaponComponent weapons;
};

struct FighterShip {
    ECS::Entity entity;
    Position position;
    SpriteComponent sprite;
    LifeTime life_time;
    FighterShip(ECS::Entity e) : entity(e) {}

    CollisionData collision;
    FactionComponent faction;
    AIComponent ai;
    WeaponConfigurationComponent weapon_config;
    Hull hull;
};

struct Projectile {
    ECS::Entity entity;
    Position position;
    SpriteComponent sprite;
    LifeTime life_time;
    Projectile(ECS::Entity e) : entity(e) {}

    CollisionData collision;
    Velocity velocity;
    ProjectileDamage damage;
    TargetComponent target;
    FactionComponent faction;
};

struct ProjectileMiss {
    ECS::Entity entity;    
    Position position;
    SpriteComponent sprite;
    LifeTime life_time;
    ProjectileMiss(ECS::Entity e) : entity(e) {}

    Velocity velocity;
};

struct CollisionPair {
    ECS::Entity first;
    ECS::Entity second;
    float distance;
    Vector2 collision_point;
    bool operator<( const CollisionPair& rhs ) const { 
        return distance < rhs.distance; 
    }
};

struct CollisionPairs {
    std::vector<CollisionPair> collisions;
    int count = 0;
    inline CollisionPair operator [](size_t i) const { return collisions[i]; }
    inline CollisionPair & operator [](size_t i) { return collisions[i]; }

    void allocate(size_t size) {
        collisions.reserve(size);
    }

    void sort_by_distance() {
        std::sort(collisions.begin(), collisions.end());
    }

    void push(ECS::Entity first, ECS::Entity second, float distance, Vector2 collision_point) {
        collisions.push_back({ first, second, distance, collision_point });
        count++;
    }

    void clear() {
        count = 0;
        collisions.clear();
    }
};

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
    
    struct ProjectileSpawn {
        int faction;
        Vector2 position;
        ECS::Entity target;
        WeaponConfigurationComponent wc;
        float delay = 0;

        // Always last
        float timer = 0;
    };
    std::vector<ProjectileSpawn> _projectile_spawns;

    CollisionPairs collision_pairs;

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

    bool get_random_target(const int firing_faction, ECS::Entity &entity) {
        int target_faction = firing_faction == PLAYER_FACTION ? ENEMY_FACTION : PLAYER_FACTION;

        std::vector<FighterShip*> matches;
        for(auto &f : _fighter_ships) {
            if(f.faction.faction == target_faction) {
                matches.push_back(&f);
            }
        }

        if(matches.size() == 0) {
            return false;
        }
        
        int target = RNG::range_i(0, matches.size() - 1);
        auto target_ship = matches[target];
        entity = target_ship->entity;

        return true;
    }

    void spawn_projectile(int faction, const float angle, const Vector2 &start_position, const WeaponConfigurationComponent &wc, ECS::Entity &target_entity) {
        auto velocity = Math::direction_from_angle(angle) * 500;
        
        auto sc = SpriteComponent("combat_sprites", wc.projectile_type);
        sc.layer = PROJECTILE_LAYER;
        sc.rotation = angle;

        float chance = RNG::zero_to_one();
        if(wc.accuracy < chance) {
            ProjectileMiss p(entity_manager.create());
            p.position = Position(start_position);
            p.velocity = Velocity(velocity);
            p.sprite = sc;
            _projectile_missed.push_back(p);
        } else {
            Projectile p(entity_manager.create());
            p.position = Position(start_position);
            p.target = TargetComponent { target_entity };
            p.faction.faction = faction;
            p.damage.damage = (int)wc.damage;
            
            p.sprite = sc;
            p.velocity = Velocity(velocity);

            p.collision.radius = wc.radius;

            _projectiles.push_back(p);
        }
    }

    void create_player_mothership() {
        Vector2 position = Vector2(70, (float)gh / 2);

        MotherShip ship(entity_manager.create());
        ship.faction = FactionComponent { PLAYER_FACTION };
        SpriteComponent s = SpriteComponent("combat_sprites", "mother1");
        s.layer = MOTHERSHIP_LAYER;
        s.flip = 0;
        ship.sprite = s;
        ship.position = position;

        WeaponConfigurationComponent w_config;
        w_config.accuracy = 0.8f;
        w_config.damage = 10;
        w_config.name = "Mothership blast cannon";
        w_config.reload_time = 3.0f;
        w_config.projectile_type = "bullet_3";
        w_config.projectile_count = 8;
        w_config.burst_delay = 0.1f;

        ship.weapons.add(w_config);

        _motherships.push_back(ship);
    }

    void create_enemy_mothership() {
        Vector2 position = Vector2((float)gw - 70, (float)gh / 2);

        MotherShip ship(entity_manager.create());
        ship.faction = FactionComponent { ENEMY_FACTION };
        SpriteComponent s = SpriteComponent("combat_sprites", "mother2");
        s.layer = MOTHERSHIP_LAYER;
        s.flip = 1;
        ship.sprite = s;
        ship.position = position;
        _motherships.push_back(ship);
    }

    void create_player_fighters() {
        Vector2 position = Vector2(170, 50);

        for(int i = 0; i < 10; i++) {
            FighterShip ship(entity_manager.create());
            ship.faction = FactionComponent { PLAYER_FACTION };
            float y = position.y + i * 30.f;
            ship.position = RNG::vector2(position.x - 10, position.x + 10, y - 8, y + 8);
            ship.hull = Hull(100);

            SpriteComponent s = SpriteComponent({ 
                Animation("idle", { { "combat_sprites", "cs1" } }, 0, false),
                Animation("hit", { 
                    { "combat_sprites", "cs1" },
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

            WeaponConfigurationComponent w_config;
            w_config.accuracy = 0.8f;
            w_config.damage = 10;
            w_config.name = "Player Gun";
            w_config.reload_time = 1.0f;
            w_config.projectile_type = "bullet_4";
            ship.weapon_config = w_config;
            ship.ai = AIComponent { w_config.reload_time };

            ship.collision.radius = 8;

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
            ship.hull = Hull(100);
            SpriteComponent s = SpriteComponent({ 
                Animation("idle", { { "combat_sprites", "cs2" } }, 0, false),
                Animation("hit", { 
                    { "combat_sprites", "cs2" },
                    { "combat_sprites", "cs2_w" },
                    { "combat_sprites", "cs2_b" },
                    { "combat_sprites", "cs2_w"}, 
                    { "combat_sprites", "cs2" },
                    { "combat_sprites", "cs2_w"}
                },  12, false)
            });
            s.layer = FIGHTER_LAYER;
            s.flip = 1;
            ship.sprite = s;

            WeaponConfigurationComponent w_config;
            w_config.accuracy = 0.8f;
            w_config.damage = 20;
            w_config.name = "Enemy Gun";
            w_config.reload_time = 2.0;
            w_config.projectile_type = "bullet_3";
            ship.weapon_config = w_config;
            ship.ai = AIComponent { w_config.reload_time };

            ship.collision.radius = 8;

            _fighter_ships.push_back(ship);
        }
    }

    template<typename entity>
    void system_remove_outside(std::vector<entity> &entities) {
        for(auto &entity : entities) {
            if(!world_bounds.contains(entity.position.value.to_point())) {
                entity.life_time.marked_for_deletion = true;
            }
        }
    }

    template<typename First, typename Second>
    void system_collisions(CollisionPairs &collision_pairs, const std::vector<First> &entity_first, const std::vector<Second> &entity_second) {
        for(auto &first : entity_first) {
            for(auto &second : entity_second) {
                if(first.faction.faction == second.faction.faction) {
                    continue;
                }

                const Vector2 p_pos = first.position.value;
                const float projectile_radius = (float)first.collision.radius;
                const Vector2 p_last = first.position.last;
                const Vector2 t_pos = second.position.value;
                const float t_radius = (float)second.collision.radius;

                float dist = Math::distance_v(p_last, t_pos);
                if(Math::intersect_circles(p_pos.x, p_pos.y, projectile_radius, t_pos.x, t_pos.y, t_radius)) {
                    // Collision point is the point on the target circle 
                    // that is on the edge in the direction of the projectiles 
                    // reverse velocity
                    //Engine::logn("circle intersect");
                    Vector2 collision_point = t_pos + (t_radius * -first.velocity.value.normal());
                    collision_pairs.push(first.entity, second.entity, dist, collision_point);
                    continue;
                }

                Vector2 entry_point;
                int result = Intersects::line_circle_entry(p_last, p_pos, t_pos, t_radius, entry_point);
                if(result == 1 || result == 2) {
                    Vector2 collision_point = t_pos + (t_radius * first.velocity.value.normal());
                    collision_pairs.push(first.entity, second.entity, dist, collision_point);
                    Engine::logn("line intersect");
                }
            }
        }
    }

    void collision_handle(Projectile &projectile, FighterShip &fighter, const CollisionPair &entities) {
        auto &p = projectile.position;
        
        projectile.life_time.marked_for_deletion = true;

        auto &hull = fighter.hull;
        hull.amount = hull.amount - projectile.damage.damage;
        // An event ?
        // Send that something took damage?
        Services::ui().show_text_toast(p.value, "HIT!", 1.0f);
        
        particle_config.smoke_emitter.position = fighter.position.value;
        Particles::emit(particles, particle_config.smoke_emitter);
                fighter.sprite.set_current_animation("hit", "idle");
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
            //Engine::logn("pair 1");
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
        for(auto &ship : _motherships) {
            for(auto &timer : ship.weapons._reload_timer) {
                timer += Time::delta_time;
            }
            
            if(ship.faction.faction == PLAYER_FACTION) {
                int weapon_id = GInput::pressed_weapon_id();
                if(ship.weapons.can_fire(weapon_id)) {
                    ECS::Entity target_entity;
                    if(get_random_target(ship.faction.faction, target_entity)) {
                        const auto &wc = ship.weapons.get_config(weapon_id);
                        
                        // fire burst at same target
                        if(wc.burst_delay > 0) {
                            for(size_t i = 0; i < wc.projectile_count; i++) {
                                _projectile_spawns.push_back(ProjectileSpawn { 
                                    ship.faction.faction, ship.position.value, target_entity, wc, i * wc.burst_delay
                                });
                            }
                        }
                    }
                }
            } else {

            }
        }

        for (auto &ship : _fighter_ships) {
            auto &ai = ship.ai;
            auto &p = ship.position;

            ai.fire_cooldown = Math::max_f(0.0f, ai.fire_cooldown - Time::delta_time);
            if(ai.fire_cooldown > 0.0f) {
                continue;
            }

            auto &wc = ship.weapon_config;
            ai.fire_cooldown = wc.reload_time;;

            ECS::Entity target_entity;
            if(get_random_target(ship.faction.faction, target_entity)) {
                _projectile_spawns.push_back(ProjectileSpawn { 
                    ship.faction.faction, p.value, target_entity, wc
                });
            }
        }
        
        for(auto &pr : _projectiles) {
            pr.position.last = pr.position.value;
            pr.position.value += pr.velocity.value * Time::delta_time;
        }

        for(auto &pr : _projectile_missed) {
            pr.position.last = pr.position.value;
            pr.position.value += pr.velocity.value * Time::delta_time;
        }

        // for(auto &pr : _projectiles) {
        //     auto &p = pr.position;
        //     auto &t = pr.travel;
        //     auto &l = pr.life_time;
            
        //     auto diff = p.value - p.last;
        //     t.amount = t.amount + diff.length();
        //     if(t.amount > t.target) {
        //         const float outside_range = (float)gw * 2;
        //         if(!entity_manager.alive(pr.target.entity) && t.target < outside_range) {
        //             t.target = outside_range;
        //         } else {
        //             l.marked_for_deletion = true;
        //         }
        //     }
        // }

        system_collisions(collision_pairs, _projectiles, _fighter_ships);
        system_collision_resolution(collision_pairs, _projectiles, _fighter_ships);
        collision_pairs.clear();

        // for(auto &pr : _projectile_missed) {
        //     auto &t = pr.travel;
            
        //     if(t.amount >= pr.travel.distance) {
        //         pdd.distance = 999999; // we don't want to trigger this again
        //          // In a normal ecs you would probably just remove the component ;D
                
        //         if(!entity_manager.alive(pr.target.entity)) {
        //             continue;
        //         }

        //         auto &e = pr.target.entity;
        //         auto fighter = std::find_if(_fighter_ships.begin(), _fighter_ships.end(),
        //             [=](const FighterShip &ps) -> bool { return ps.entity.id == e.id; });

        //         if(fighter == _fighter_ships.end()) {
        //             continue;
        //         }    

                   
        //     Maybe better as an event and anyone can react
        //     Services::ui().show_text_toast(p.value, "MISS!", 1.0f);
        //     }
        // }

        for (auto &ship : _fighter_ships) { 
            ship.sprite.update_animation(Time::delta_time);
            // ship.animation.update(Time::delta_time);
           //  SpriteAnimation::update(ship.animation, Time::delta_time);
        }

        for(auto &fighter : _fighter_ships) {
            if(fighter.hull.amount <= 0) {
                particle_config.explosion_emitter.position = fighter.position.value;
                Particles::emit(particles, particle_config.explosion_emitter);
                fighter.life_time.marked_for_deletion = true;
            }
        }

        system_remove_outside(_projectiles);
        system_remove_outside(_projectile_missed);

        _fighter_ships.erase(std::remove_if(_fighter_ships.begin(), _fighter_ships.end(), entity_remove<FighterShip>), _fighter_ships.end());
        _projectiles.erase(std::remove_if(_projectiles.begin(), _projectiles.end(), entity_remove<Projectile>), _projectiles.end());
        _projectile_missed.erase(std::remove_if(_projectile_missed.begin(), _projectile_missed.end(), entity_remove<ProjectileMiss>), _projectile_missed.end());

        for(auto &pspawn : _projectile_spawns) {
            ASSERT_WITH_MSG(pspawn.faction == PLAYER_FACTION || pspawn.faction == ENEMY_FACTION, "undefined faction occured while spawning projectile.");

            pspawn.timer += Time::delta_time;
            if(pspawn.timer >= pspawn.delay) {

                auto fighter = std::find_if(_fighter_ships.begin(), _fighter_ships.end(),
                    [=](const FighterShip &ps) -> bool { return ps.entity.id == pspawn.target.id; });
                if(fighter == _fighter_ships.end()) {
                    continue;
                }
                
                Vector2 target_position = fighter->position.value;
                auto angle = Math::angle_between_v(pspawn.position, target_position);
                GameController::spawn_projectile(pspawn.faction, angle, pspawn.position, pspawn.wc, pspawn.target);
            }
        }
        _projectile_spawns.erase(std::remove_if(_projectile_spawns.begin(), _projectile_spawns.end(), 
            [=](const ProjectileSpawn &projectile_spawn) -> bool { return projectile_spawn.timer >= projectile_spawn.delay; }), _projectile_spawns.end());
        
        auto player_fighters = std::count_if(_fighter_ships.begin(), _fighter_ships.end(),
                    [=](const FighterShip &ps) -> bool { return ps.faction.faction == PLAYER_FACTION; });
        auto enemy_fighters = std::count_if(_fighter_ships.begin(), _fighter_ships.end(),
                    [=](const FighterShip &ps) -> bool { return ps.faction.faction == ENEMY_FACTION; });
        if(player_fighters == 0) {
            Services::ui().game_over();
        } else if(enemy_fighters == 0) {
             Services::ui().battle_win();
        }
    }
}

#endif