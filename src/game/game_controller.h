#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "engine.h"
#include "services.h"
#include "components.h"

#include <unordered_set>

// struct PlayerShip {
//     ECS::Entity entity;
//     Position position;
//     SpriteComponent sprite;
//     LifeTime life_time;
//     PlayerShip(ECS::Entity e) : entity(e) {}

//     FactionComponent faction;
//     PlayerInput input;  
//     InputTriggerComponent trigger;
//     WeaponConfigurationComponent weapon_config;
//     Hull hull;
// };

// struct EnemyShip {
//     ECS::Entity entity;
//     Position position;
//     SpriteComponent sprite;
//     LifeTime life_time;
//     EnemyShip(ECS::Entity e) : entity(e) {}

//     FactionComponent faction;
//     AIComponent ai;
//     WeaponConfigurationComponent weapon_config;
//     Hull hull;
// };

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
    std::vector<FighterShip> _fighter_ships;
    std::vector<Projectile> _projectiles;
    
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

        // Services::events().listen<EntityDestroyedEvent>(&entity_destroyed);
    }

    void clear() {
        _fighter_ships.clear();
        _projectiles.clear();
        _projectile_spawns.clear();
    }

    // template<class Input>
    // void UpdateInput(Input& go) {
    //     //go.input.set();
    // }
    
    bool get_target(const int firing_faction, ECS::Entity &entity, Vector2 &target_position) {
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

    void create_player() {
        Vector2 position = Vector2(100, 50);

        for(int i = 0; i < 10; i++) {
            FighterShip ship(entity_manager.create());
            ship.faction = FactionComponent { PLAYER_FACTION };
            ship.position = position + Vector2(0, i * 30.f);
            ship.hull = Hull(100);
            SpriteComponent s = SpriteComponent("combat_sprites", "cs1");
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

    void create_enemy() {
        Vector2 position = Vector2((float)gw - 100, 50);

        for(int i = 0; i < 10; i++) {
            FighterShip ship(entity_manager.create());
            ship.faction = FactionComponent { ENEMY_FACTION };
            ship.position = position + Vector2(0, i * 30.f);
            ship.hull = Hull(100);
            SpriteComponent s = SpriteComponent("combat_sprites", "cs2");
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
        // for (auto &ship : _player_ships) {
        //     auto &pi = ship.input;
        //     pi.controls_pressed[0] = Input::key_pressed(SDLK_1) ? 1 : 0;
        //     pi.controls_pressed[1] = Input::key_pressed(SDLK_2) ? 1 : 0;
        //     pi.controls_pressed[2] = Input::key_pressed(SDLK_3) ? 1 : 0;
        //     pi.controls_pressed[3] = Input::key_pressed(SDLK_4) ? 1 : 0;
        //     pi.controls_pressed[4] = Input::key_pressed(SDLK_5) ? 1 : 0;
        //     pi.controls_pressed[5] = Input::key_pressed(SDLK_6) ? 1 : 0;
        //     pi.controls_pressed[6] = Input::key_pressed(SDLK_7) ? 1 : 0;
        //     pi.controls_pressed[7] = Input::key_pressed(SDLK_8) ? 1 : 0;
        //     pi.controls_pressed[8] = Input::key_pressed(SDLK_9) ? 1 : 0;
        //     pi.fire_cooldown = Math::max_f(0.0f, pi.fire_cooldown - Time::delta_time);
        // }

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
        
        // for (auto &ship : _player_ships) {
        //     auto &pi = ship.input;
        //     auto &p = ship.position;
        //     auto &t = ship.trigger;

        //     if(pi.fire_cooldown > 0.0f || pi.controls_pressed[t.trigger] == 0) {
        //         continue;
        //     }

        //     auto &wc = ship.weapon_config;
        //     pi.fire_cooldown = wc.reload_time;

        //     _projectile_spawns.push_back(ProjectileSpawn { 
        //         ship.faction.faction, p.value, wc
        //     });
        // }

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
                } else {
                    // Maybe better as an event and anyone can react
                    Services::ui().show_text_toast(p.value, "MISS!", 1.0f);
                }
            }
        }
        
        _fighter_ships.erase(std::remove_if(_fighter_ships.begin(), _fighter_ships.end(), [](const FighterShip& p) { 
            return p.hull.amount <= 0;
        }), _fighter_ships.end());

        _projectiles.erase(std::remove_if(_projectiles.begin(), _projectiles.end(), [](const Projectile& p) { 
            return p.life_time.marked_for_deletion;
        }), _projectiles.end());

        for(auto pspawn : _projectile_spawns) {
            ASSERT_WITH_MSG(pspawn.faction == PLAYER_FACTION || pspawn.faction == ENEMY_FACTION, "undefined faction occured while spawning projectile.");

            ECS::Entity entity; 
            Vector2 target_position;
            if(get_target(pspawn.faction, entity, target_position)) {    
                GameController::projectile_fire(entity, pspawn.position, target_position, pspawn.wc);
            }
        }
        _projectile_spawns.clear();

        // if(_player_ships.size() == 0) {
        //     Services::ui().game_over();
        // } else if(_enemy_ships.size() == 0) {
        //     Services::ui().battle_win();
        // }
    }
}

#endif