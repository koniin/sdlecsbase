#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "engine.h"
#include "services.h"
#include "components.h"

#include <unordered_set>

struct Entity {
    unsigned id;
};

class EntityManager {
    std::unordered_set<unsigned> _entities;
    Entity _next;
 
    public:
    Entity create() {
        ++_next.id;
        while (alive(_next)) {
            ++_next.id;
        }
        _entities.insert(_next.id);
        return _next;
    }

    bool alive(Entity e) {
        return _entities.find(e.id) != _entities.end();
    }

    void destroy(Entity e) {
        _entities.erase(e.id);
    }
};

struct PlayerShip {
    Entity entity;
    Position position;
    SpriteComponent sprite;
    LifeTime life_time;
    PlayerShip(Entity e) : entity(e) {}

    PlayerInput input;  
    InputTriggerComponent trigger;
    WeaponConfigurationComponent weapon_config;
    Hull hull;
};

struct EnemyShip {
    Entity entity;
    Position position;
    SpriteComponent sprite;
    LifeTime life_time;
    EnemyShip(Entity e) : entity(e) {}

    AIComponent ai;
    WeaponConfigurationComponent weapon_config;
    Hull hull;
};

struct TargetComponent {
    Entity entity;
};

struct Projectile {
    Entity entity;
    Position position;
    SpriteComponent sprite;
    LifeTime life_time;
    Projectile(Entity e) : entity(e) {}

    Velocity velocity;
    TravelDistance travel;
    ProjectileDamageDistance damage;
    TargetComponent target;
};

namespace GameController {
    Vector2 player_pos;
    Vector2 enemy_pos;

    const int kObjectCount = 300;
    
    EntityManager entity_manager;
    std::vector<PlayerShip> _player_ships;
    std::vector<EnemyShip> _enemy_ships;
    std::vector<Projectile> _projectiles;

    void initialize() {
        player_pos = Vector2(100, 150);
        enemy_pos = Vector2((float)gw - 100, 150);

        _player_ships.reserve(kObjectCount);
        _enemy_ships.reserve(kObjectCount);
        _projectiles.reserve(kObjectCount);

        // Services::events().listen<EntityDestroyedEvent>(&entity_destroyed);
    }

    // template<class Input>
    // void UpdateInput(Input& go) {
    //     //go.input.set();
    // }
    
    void player_projectile_fire(Vector2 position, WeaponConfigurationComponent wc) {
        if(_enemy_ships.size() == 0) {
            return;
        }

        int target = RNG::range_i(0, _enemy_ships.size() - 1);
        auto &target_ship = _enemy_ships[target];
        Vector2 target_position = target_ship.position.value;

        Projectile p(entity_manager.create());
        p.position = Position(position);
        p.target = TargetComponent { target_ship.entity };

        auto sc = SpriteComponent("combat_sprites", "bullet_1");
        sc.layer = 12;
        p.sprite = sc;
        auto angle = Math::angle_between_v(p.position.value, target_position);
        auto dir = Math::direction_from_angle(angle) * 500;
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

    void enemy_projectile_fire(Vector2 position, WeaponConfigurationComponent wc) {
        if(_player_ships.size() == 0) {
            return;
        }

        int target = RNG::range_i(0, _player_ships.size() - 1);
        auto &target_ship = _player_ships[target];
        Vector2 target_position = target_ship.position.value;

        Projectile p(entity_manager.create());
        p.position = Position(position);
        p.target = TargetComponent { target_ship.entity };
        auto sc = SpriteComponent("combat_sprites", "bullet_2");
        sc.layer = 12;
        p.sprite = sc;
        auto angle = Math::angle_between_v(p.position.value, target_position);
        auto dir = Math::direction_from_angle(angle) * 500;
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

    std::vector<std::function<void(void)>> post_update_commands;
    void update() {
        for (auto &ship : _player_ships) {
            auto &pi = ship.input;
            pi.controls_pressed[0] = Input::key_pressed(SDLK_1) ? 1 : 0;
            pi.controls_pressed[1] = Input::key_pressed(SDLK_2) ? 1 : 0;
            pi.controls_pressed[2] = Input::key_pressed(SDLK_3) ? 1 : 0;
            pi.controls_pressed[3] = Input::key_pressed(SDLK_4) ? 1 : 0;
            pi.controls_pressed[4] = Input::key_pressed(SDLK_5) ? 1 : 0;
            pi.controls_pressed[5] = Input::key_pressed(SDLK_6) ? 1 : 0;
            pi.controls_pressed[6] = Input::key_pressed(SDLK_7) ? 1 : 0;
            pi.controls_pressed[7] = Input::key_pressed(SDLK_8) ? 1 : 0;
            pi.controls_pressed[8] = Input::key_pressed(SDLK_9) ? 1 : 0;
            pi.fire_cooldown = Math::max_f(0.0f, pi.fire_cooldown - Time::delta_time);
        }

        for (auto &ship : _enemy_ships) {
            auto &ai = ship.ai;
            auto &p = ship.position;

            ai.fire_cooldown = Math::max_f(0.0f, ai.fire_cooldown - Time::delta_time);
            if(ai.fire_cooldown > 0.0f) {
                continue;
            }

            auto &wc = ship.weapon_config;
            ai.fire_cooldown = wc.reload_time;;
            post_update_commands.push_back([=]() {
                GameController::enemy_projectile_fire(p.value, wc);
            });
        }
        
        for (auto &ship : _player_ships) {
            auto &pi = ship.input;
            auto &p = ship.position;
            auto &t = ship.trigger;

            if(pi.fire_cooldown <= 0.0f && pi.controls_pressed[t.trigger] > 0) {
                auto &wc = ship.weapon_config;
                pi.fire_cooldown = wc.reload_time;

                post_update_commands.push_back([=]() {
                    GameController::player_projectile_fire(p.value, wc);
                });
            }
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
                
                // if(!arch_manager.is_alive(pdd.target)) {
                //     t.target = t.target * 2;
                //     return;
                // }

                if(!entity_manager.alive(pr.target.entity)) {
                    continue;
                }

                Entity e = pr.target.entity;
                for(auto &ship : _player_ships) {
                    if(ship.entity.id == e.id) {
                        auto &p = pr.position;
                        if(pdd.hit == 1) {
                            auto &hull = ship.hull;
                            hull.amount = hull.amount - pdd.damage;

                            // An event ?
                            // Send that something took damage?
                            Services::ui().show_text_toast(p.value, "HIT!", 1.0f);
                            break;
                        } else {
                            // Maybe better as an event and anyone can react
                            Services::ui().show_text_toast(p.value, "MISS!", 1.0f);
                            break;
                        }
                    }
                }
                for(auto &ship : _enemy_ships) {
                    if(ship.entity.id == e.id) {
                        auto &p = pr.position;
                        if(pdd.hit == 1) {
                            auto &hull = ship.hull;
                            hull.amount = hull.amount - pdd.damage;

                            // An event ?
                            // Send that something took damage?
                            Services::ui().show_text_toast(p.value, "HIT!", 1.0f);
                            break;
                        } else {
                            // Maybe better as an event and anyone can react
                            Services::ui().show_text_toast(p.value, "MISS!", 1.0f);
                            break;
                        }
                    }
                }
            }
        }
        
        _player_ships.erase(std::remove_if(_player_ships.begin(), _player_ships.end(), [](const PlayerShip& p) { 
            return p.hull.amount <= 0;
        }), _player_ships.end());

        _enemy_ships.erase(std::remove_if(_enemy_ships.begin(), _enemy_ships.end(), [](const EnemyShip& p) { 
            return p.hull.amount <= 0;
        }), _enemy_ships.end());

        _projectiles.erase(std::remove_if(_projectiles.begin(), _projectiles.end(), [](const Projectile& p) { 
            return p.life_time.marked_for_deletion;
        }), _projectiles.end());


        for(auto pc : post_update_commands) {
            pc();
        }
        post_update_commands.clear();
    }

    void create_player() {
        PlayerShip ship(entity_manager.create());
        ship.position = player_pos;
        ship.hull = Hull(100);
        SpriteComponent s = SpriteComponent("combat_sprites", "cs1");
        s.layer = 10;
        s.flip = 0;
        ship.sprite = s;
        ship.trigger = InputTriggerComponent { 0 };
        WeaponConfigurationComponent w_config;
        w_config.accuracy = 0.8f;
        w_config.damage = 10;
        w_config.name = "Weapon " + std::to_string(ship.trigger.trigger);
        w_config.reload_time = 1.8f;
        ship.weapon_config = w_config;
        _player_ships.push_back(ship);
    }

    void create_enemy() {
        EnemyShip ship(entity_manager.create());
        ship.position = enemy_pos;
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
        ship.weapon_config = w_config;
        ship.ai = AIComponent { w_config.reload_time };
        _enemy_ships.push_back(ship);
    }
}

#endif