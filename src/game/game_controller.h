#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "engine.h"
#include "services.h"
#include "components.h"
#include "game_input_wrapper.h"
#include "particles.h"
#include "particle_config.h"

#include <unordered_set>

struct MotherShip {
    ECS::Entity entity;
    Position position;
    SpriteComponent sprite;
    MotherShip(ECS::Entity e) : entity(e) {}

    FactionComponent faction;
};

struct FighterShip {
    ECS::Entity entity;
    Position position;
    SpriteComponent sprite;
    LifeTime life_time;
    FighterShip(ECS::Entity e) : entity(e) {}

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

    Velocity velocity;
    TravelDistance travel;
    ProjectileDamageDistance damage;
    TargetComponent target;
};

namespace GameController {
    const int c_fighter_count = 300;
    const int c_projectile_count = 300;
    const int c_projectile_spawn_count = 300;
    const int PLAYER_FACTION = 100;
    const int ENEMY_FACTION = 200;
    
    ECS::EntityManager entity_manager;
    std::vector<MotherShip> _motherships;
    std::vector<FighterShip> _fighter_ships;
    std::vector<Projectile> _projectiles;
    
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
        WeaponConfigurationComponent wc;
    };
    std::vector<ProjectileSpawn> _projectile_spawns;

    void initialize() {
        _fighter_ships.reserve(c_fighter_count);
        _projectiles.reserve(c_projectile_count);
        _projectile_spawns.reserve(c_projectile_spawn_count);
        particles = Particles::make(4096);
        particle_emitters_configure(&particle_config);
        // Services::events().listen<EntityDestroyedEvent>(&entity_destroyed);
    }

    void clear() {
        _fighter_ships.clear();
        _projectiles.clear();
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

    bool get_random_target(const int firing_faction, ECS::Entity &entity, Vector2 &target_position) {
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
        target_position = target_ship->position.value;

        return true;
    }

    void projectile_fire(const ECS::Entity &target_entity, const Vector2 &position, const Vector2 &target_position, const WeaponConfigurationComponent &wc) {
        Projectile p(entity_manager.create());
        p.position = Position(position);
        p.target = TargetComponent { target_entity };

        auto sc = SpriteComponent("combat_sprites", wc.projectile_type);
        sc.layer = 12;
        auto angle = Math::angle_between_v(p.position.value, target_position);
        sc.rotation = angle;
        auto dir = Math::direction_from_angle(angle) * 500;
        p.sprite = sc;
        p.velocity = Velocity(dir);

        auto distance_to_target = (p.position.value - target_position).length();
        ProjectileDamageDistance pdd;
        pdd.distance = distance_to_target;
        pdd.damage = (int)wc.damage;

        float distance = position.x + (float)gw;
        if(wc.accuracy >= RNG::zero_to_one()) {
            pdd.hit = 1; 
            distance = distance_to_target;
        } else {
            pdd.hit = 0;
        }
        p.damage = pdd;
        p.travel = TravelDistance(distance);

        _projectiles.push_back(p);
    }

    void create_player_mothership() {
        Vector2 position = Vector2(70, (float)gh / 2);

        MotherShip ship(entity_manager.create());
        ship.faction = FactionComponent { PLAYER_FACTION };
        SpriteComponent s = SpriteComponent("combat_sprites", "mother1");
        s.layer = 12;
        s.flip = 0;
        ship.sprite = s;
        ship.position = position;
        _motherships.push_back(ship);
    }

    void create_enemy_mothership() {
        Vector2 position = Vector2((float)gw - 70, (float)gh / 2);

        MotherShip ship(entity_manager.create());
        ship.faction = FactionComponent { ENEMY_FACTION };
        SpriteComponent s = SpriteComponent("combat_sprites", "mother2");
        s.layer = 10;
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
            s.layer = 10;
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
            s.layer = 10;
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
            _fighter_ships.push_back(ship);
        }
    }

    void update() {
        for(auto &ship : _motherships) {
            if(ship.faction.faction == PLAYER_FACTION) {
                if(GInput::pressed(GInput::Action::Fire_1)) {
                    Engine::logn("FIRE!");
                    particle_config.smoke_emitter.position = ship.position.value;
                    Particles::emit(particles, particle_config.smoke_emitter);
                    particle_config.explosion_emitter.position = ship.position.value;
                    Particles::emit(particles, particle_config.explosion_emitter);
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

            _projectile_spawns.push_back(ProjectileSpawn { 
                ship.faction.faction, p.value, wc
            });
        }
        
        for(auto &pr : _projectiles) {
            pr.position.last = pr.position.value;
            pr.position.value += pr.velocity.value * Time::delta_time;
        }

        for(auto &pr : _projectiles) {
            auto &p = pr.position;
            auto &t = pr.travel;
            auto &l = pr.life_time;
            
            auto diff = p.value - p.last;
            t.amount = t.amount + diff.length();
            if(t.amount > t.target) {
                l.marked_for_deletion = true;
            }
        }

        for(auto &pr : _projectiles) {
            auto &t = pr.travel;
            auto &pdd = pr.damage;
            
            if(t.amount >= pdd.distance) {
                pdd.distance = 999999; // we don't want to trigger this again
                 // In a normal ecs you would probably just remove the component ;D
                
                if(!entity_manager.alive(pr.target.entity)) {
                    continue;
                }

                auto &e = pr.target.entity;
                auto fighter = std::find_if(_fighter_ships.begin(), _fighter_ships.end(),
                    [=](const FighterShip &ps) -> bool { return ps.entity.id == e.id; });

                if(fighter == _fighter_ships.end()) {
                    continue;
                }    

                auto &p = pr.position;
                if(pr.damage.hit == 1) {
                    auto &hull = fighter->hull;
                    hull.amount = hull.amount - pr.damage.damage;
                    // An event ?
                    // Send that something took damage?
                    Services::ui().show_text_toast(p.value, "HIT!", 1.0f);

                    fighter->sprite.set_current_animation("hit", "idle");
                    //fighter->sprite.set_current_animation("hit");
                    //SpriteAnimation::set_current(fighter->animation, "hit");
                } else {
                    // Maybe better as an event and anyone can react
                    Services::ui().show_text_toast(p.value, "MISS!", 1.0f);
                }
            }
        }

        for (auto &ship : _fighter_ships) { 
            ship.sprite.update_animation(Time::delta_time);
            // ship.animation.update(Time::delta_time);
           //  SpriteAnimation::update(ship.animation, Time::delta_time);
        }

        // for(auto &fighter : _fighter_ships) {
        //     entity_data.blink[i].timer += Time::delta_time;
        //     entity_data.blink[i].interval_timer += Time::delta_time;

        //     if(entity_data.blink[i].timer >= entity_data.blink[i].time_to_live) {
        //         entity_data.blink[i].timer = 0.0f;
        //         if(entity_data.blink[i].time_to_live > 0) {
        //             entity_data.sprite[i].sprite_name = entity_data.blink[i].original_sprite;
        //         }
        //         entity_data.blink[i].time_to_live = 0;
        //         continue;
        //     }
            
        //     if(entity_data.blink[i].interval_timer > entity_data.blink[i].interval) {
        //         entity_data.sprite[i].sprite_name = 
        //             entity_data.sprite[i].sprite_name == entity_data.blink[i].original_sprite 
        //                 ? entity_data.blink[i].white_sprite : entity_data.blink[i].original_sprite;
        //         entity_data.blink[i].interval_timer = 0;
        //     }
        // }

        for(auto &fighter : _fighter_ships) {
            if(fighter.hull.amount <= 0) {
                fighter.life_time.marked_for_deletion = true;
            }
        }

        _fighter_ships.erase(std::remove_if(_fighter_ships.begin(), _fighter_ships.end(), entity_remove<FighterShip>), _fighter_ships.end());
        _projectiles.erase(std::remove_if(_projectiles.begin(), _projectiles.end(), entity_remove<Projectile>), _projectiles.end());

        for(auto pspawn : _projectile_spawns) {
            ASSERT_WITH_MSG(pspawn.faction == PLAYER_FACTION || pspawn.faction == ENEMY_FACTION, "undefined faction occured while spawning projectile.");

            ECS::Entity entity; 
            Vector2 target_position;
            if(get_random_target(pspawn.faction, entity, target_position)) {    
                GameController::projectile_fire(entity, pspawn.position, target_position, pspawn.wc);
            }
        }
        _projectile_spawns.clear();

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