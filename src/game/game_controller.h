#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "engine.h"
#include "services.h"
#include "components.h"

struct PlayerShip {
    Position position;
    SpriteComponent sprite;
    LifeTime life_time;

    PlayerInput input;  
    InputTriggerComponent trigger;
    WeaponConfigurationComponent weapon_config;
    Hull hull;
};

struct EnemyShip {
    Position position;
    SpriteComponent sprite;
    LifeTime life_time;

    AIComponent ai;
    InputTriggerComponent trigger;
    WeaponConfigurationComponent weapon_config;
    Hull hull;
};

struct Projectile {
    Position position;
    SpriteComponent sprite;
    LifeTime life_time;

    Velocity velocity;
    TravelDistance travel;
    ProjectileDamageDistance damage;
};

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

    const int kObjectCount = 300;
    
    std::vector<PlayerShip> _player_ships;
    std::vector<EnemyShip> _enemy_ships;
    std::vector<Projectile> _projectiles;

    void entity_destroyed(EntityDestroyedEvent ev) {
        Engine::logn("recieved entity destroyed event");
        auto target = ev.entity;
        if(target.equals(player)) {
            Engine::logn("player destroyed!");
            Services::ui().game_over();
        } else if(target.equals(enemy)) {
            Engine::logn("enemy destroyed! - CONSIDER PAUSING THE ENTITIES/REMOVING PROJECTILES");
            Services::ui().battle_win();
        }
    }

    void initialize() {
        //ECS::ArchetypeManager &arch_manager = Services::arch_manager();

        player_pos = Vector2(100, 150);
        enemy_pos = Vector2((float)gw - 100, 150);

        _player_ships.reserve(kObjectCount);
        _enemy_ships.reserve(kObjectCount);
        _projectiles.reserve(kObjectCount);
        
        // player_ship = arch_manager.create_archetype<Position, SpriteComponent, Hull, LifeTime>(2);
        // enemy_ship = arch_manager.create_archetype<Position, SpriteComponent, Hull, LifeTime>(2);
        // player_weapon_archetype = arch_manager.create_archetype<PlayerInput, InputTriggerComponent, Position, SpriteComponent, WeaponConfigurationComponent, ParentComponent, LifeTime>(20);
        // enemy_weapon_archetype = arch_manager.create_archetype<AIComponent, InputTriggerComponent, Position, SpriteComponent, WeaponConfigurationComponent, ParentComponent, LifeTime>(20);

        // projectile = arch_manager.create_archetype<Position, SpriteComponent, Velocity, ProjectileDamageDistance, TravelDistance, LifeTime>(200);

        // Services::events().listen<EntityDestroyedEvent>(&entity_destroyed);
    }

    template<class Input>
    void UpdateInput(Input& go) {
        //go.input.set();
    }
    
    void player_projectile_fire(Vector2 position, WeaponConfigurationComponent wc) {
        Vector2 target_position = _enemy_ships[0].position.value;

        Projectile p;
        p.position = Position(position);
        auto sc = SpriteComponent("combat_sprites", "bullet_1");
        sc.layer = 12;
        p.sprite = sc;
        auto a = Math::direction_from_angle(0) * 500;
        p.velocity = Velocity(a);

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
        
        
        

        // arch_manager.set_component(ent, pdd);
        // arch_manager.set_component(ent, TravelDistance(distance));
    
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

                auto &p = pr.position;
                if(pdd.hit == 1) {
                    // auto &hull = arch_manager.get_component<Hull>(pdd.target);
                    // hull.amount = hull.amount - pdd.damage;

                    // auto &position = arch_manager.get_component<Position>(pdd.target);
                    // An event ?
                    // Send that something took damage?
                    Services::ui().show_text_toast(p.value, "HIT!", 1.0f);

                } else {
                    
                    // Maybe better as an event and anyone can react
                    Services::ui().show_text_toast(p.value, "MISS!", 1.0f);
                }
            }
        }
        
        size_t c = _projectiles.size();

        _projectiles.erase(std::remove_if(_projectiles.begin(), _projectiles.end(), [](const Projectile& p) { 
            return p.life_time.marked_for_deletion;
        }), _projectiles.end());

        size_t c2 = _projectiles.size();

        if(c != c2) {
            Engine::logn("%d -> %d projectiles", c, c2);
        }

        for(auto pc : post_update_commands) {
            pc();
        }
        post_update_commands.clear();
    }

    void create_player() {
        PlayerShip ship;
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
        EnemyShip ship;
        ship.position = enemy_pos;
        ship.hull = Hull(100);
        SpriteComponent s = SpriteComponent("combat_sprites", "cs2");
        s.layer = 10;
        s.flip = 1;
        ship.sprite = s;
        _enemy_ships.push_back(ship);
    }

    // void create_player() {
    //     ECS::ArchetypeManager &arch_manager = Services::arch_manager();

    //     auto ent = arch_manager.create_entity(player_ship);
    //     if(arch_manager.is_alive(player_ship, ent)) {
    //         Position pos = Position(player_pos);
    //         arch_manager.set_component(ent, pos);
    //         arch_manager.set_component(ent, Hull(100));
    //         SpriteComponent s = SpriteComponent("combat_sprites", "ship1");
    //         s.layer = 10;
    //         s.flip = 0;
    //         arch_manager.set_component(ent, s);

    //         create_weapon(player_weapon_archetype, Vector2(120, 100), ent, 0);
    //         create_weapon(player_weapon_archetype, Vector2(120, 200), ent, 1);
    //     }
    //     player = ent;
    // }

    // void create_enemy() {
    //     ECS::ArchetypeManager &arch_manager = Services::arch_manager();

    //     auto ent = arch_manager.create_entity(enemy_ship);
    //     if(arch_manager.is_alive(enemy_ship, ent)) {
    //         Position pos = Position(enemy_pos);
    //         arch_manager.set_component(ent, pos);
    //         arch_manager.set_component(ent, Hull(100));
    //         SpriteComponent s = SpriteComponent("combat_sprites", "ship2");
    //         s.layer = 10;
    //         arch_manager.set_component(ent, s);

    //         auto weapon = arch_manager.create_entity(enemy_weapon_archetype);
    //         Position p_weap = Position((float)gw - 90, 150);
    //         arch_manager.set_component(weapon, p_weap);

    //         SpriteComponent s_weap = SpriteComponent("combat_sprites", "gun2");
    //         s_weap.layer = 15;
    //         s_weap.flip = 1;
    //         arch_manager.set_component(weapon, s_weap);
    //         arch_manager.set_component(weapon, ParentComponent { ent });
    //         WeaponConfigurationComponent w_config;
    //         w_config.accuracy = 0.8f;
    //         w_config.damage = 20;
    //         w_config.name = "Enemy Gun";
    //         w_config.reload_time = 2.0;
    //         arch_manager.set_component(weapon, w_config);
    //         arch_manager.set_component(weapon, AIComponent { w_config.reload_time });
    //     }
    //     enemy = ent;
    // }

    // void create_weapon(ECS::ArcheType &archetype, Vector2 pos, ECS::Entity &parent, int trigger) {
    //     ECS::ArchetypeManager &arch_manager = Services::arch_manager();
    //     auto weapon = arch_manager.create_entity(archetype);
    //     Position p_weap = Position(pos);
    //     arch_manager.set_component(weapon, p_weap);
    //     SpriteComponent s_weap = SpriteComponent("combat_sprites", "gun1");
    //     s_weap.layer = 15;
    //     arch_manager.set_component(weapon, s_weap);
    //     arch_manager.set_component(weapon, ParentComponent { parent });
    //     arch_manager.set_component(weapon, InputTriggerComponent { trigger });

    //     WeaponConfigurationComponent w_config;
    //     w_config.accuracy = 0.8f;
    //     w_config.damage = 10;
    //     w_config.name = "Weapon " + std::to_string(trigger);
    //     w_config.reload_time = 1.8f;
    //     arch_manager.set_component(weapon, w_config);
    //     PlayerInput pinput;
    //     pinput.fire_cooldown = w_config.reload_time;
    //     arch_manager.set_component(weapon, pinput);
    // }

    // void player_projectile_fire(Vector2 position, WeaponConfigurationComponent wc) {
    //     ECS::ArchetypeManager &arch_manager = Services::arch_manager();
    //     ECS::Entity target = enemy;
    //     Vector2 my_position = player_pos;

    //     if(!arch_manager.is_alive(target)) {
    //         return;
    //     }

    //     auto ent = arch_manager.create_entity(projectile);
    //     arch_manager.set_component(ent, Position(position));
    //     auto sc = SpriteComponent("combat_sprites", "bullet_1");
    //     sc.layer = 12;
    //     arch_manager.set_component(ent, sc);

    //     auto a = Math::direction_from_angle(0) * 500;
    //     arch_manager.set_component(ent, Velocity(a));

    //     auto pos = arch_manager.get_component<Position>(target);
    //     auto distance_to_target = (my_position - pos.value).length();

    //     ProjectileDamageDistance pdd;
    //     pdd.distance = distance_to_target;
    //     pdd.target = target;
    //     pdd.damage = (int)wc.damage;

    //     float distance = position.x + (float)gw;
    //     if(wc.accuracy >= RNG::zero_to_one()) {
    //         pdd.hit = 1; 
    //         distance = distance_to_target;
    //     } else {
    //         pdd.hit = 0;
    //     }

    //     arch_manager.set_component(ent, pdd);
    //     arch_manager.set_component(ent, TravelDistance(distance));
    // }

    // void enemy_projectile_fire(Vector2 position) {
    //     ECS::ArchetypeManager &arch_manager = Services::arch_manager();
    //     ECS::Entity target = player;
    //     Vector2 my_position = enemy_pos;

    //     if(!arch_manager.is_alive(target)) {
    //         return;
    //     }
        
    //     auto ent = arch_manager.create_entity(projectile);
    //     arch_manager.set_component(ent, Position(position));
    //     auto sc = SpriteComponent("combat_sprites", "bullet_2");
    //     sc.layer = 12;
    //     arch_manager.set_component(ent, sc);

    //     auto pos = arch_manager.get_component<Position>(target);
    //     auto a = Math::direction(pos.value, position) * 500;
    //     arch_manager.set_component(ent, Velocity(a));

    //     auto distance_to_target = (my_position - pos.value).length();

    //     ProjectileDamageDistance pdd;
    //     pdd.distance = distance_to_target;
    //     pdd.target = target;
    //     pdd.damage = 20;

    //     float distance = position.x + (float)gw;
    //     if(RNG::range_i(0, 4) > 1) { // 40% chance to miss?
    //         pdd.hit = 1; 
    //         distance = distance_to_target;
    //     } else {
    //         pdd.hit = 0;
    //     }

    //     arch_manager.set_component(ent, pdd);
    //     arch_manager.set_component(ent, TravelDistance(distance));
    // }
}

#endif