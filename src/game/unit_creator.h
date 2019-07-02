#ifndef UNIT_CREATOR_H
#define UNIT_CREATOR_H

#include "engine.h"
#include "renderer.h"
#include "components.h"
#include "game_state.h"

namespace UnitCreator {
    const int MOTHERSHIP_LAYER = 100;
    const int FIGHTER_LAYER = 50;
    const int PROJECTILE_LAYER = 30;

    struct OneRandomTargeter : Targeting {
        std::function<bool(const int &exclude_faction, std::vector<ECS::EntityId> &target_overrides, Targeting::Target &target)> get_one_target;

        OneRandomTargeter(std::function<bool(const int &exclude_faction, std::vector<ECS::EntityId> &target_overrides, Targeting::Target &target)> targeter) : get_one_target(targeter) {}

        bool get_targets(const int &exclude_faction, const size_t &max_count, std::vector<ECS::EntityId> &target_overrides, Targeting::Targets &targets) override {
            Targeting::Target target;
            if(get_one_target(exclude_faction, target_overrides, target)) {
                for(size_t i = 0; i < max_count; i++) {
                    targets.targets.push_back(target);
                }
                return true;
            }
            return false;
        }
    };

    struct MultiRandomTargeter : Targeting {
        std::function<bool(const int &exclude_faction, std::vector<ECS::EntityId> &target_overrides, Targeting::Target &target)> get_one_target;

        MultiRandomTargeter(std::function<bool(const int &exclude_faction, std::vector<ECS::EntityId> &target_overrides, Targeting::Target &target)> targeter) : get_one_target(targeter) {}

        bool get_targets(const int &exclude_faction, const size_t &max_count, std::vector<ECS::EntityId> &target_overrides, Targeting::Targets &targets) override {
            Targeting::Target target;
            bool has_any_target = false;
            for(size_t i = 0; i < max_count; i++) {
                if(get_one_target(exclude_faction, target_overrides, target)) {
                    targets.targets.push_back(target);
                    has_any_target = true;
                }
            }
            return has_any_target;
        }
    };

    std::shared_ptr<Targeting> _random_targeter;
    std::shared_ptr<Targeting> _random_multi_targeter;

    void init_targeters(std::function<bool(const int &exclude_faction, std::vector<ECS::EntityId> &target_overrides, Targeting::Target &target)> target_function) {
        _random_targeter = std::make_shared<OneRandomTargeter>(OneRandomTargeter(target_function));
        _random_multi_targeter = std::make_shared<MultiRandomTargeter>(MultiRandomTargeter(target_function));
    }

    static void calc_lazer(SDL_Rect &lazer_rect, const Vector2 &start, const Vector2 &end, const int &height) {
        float distance = Math::distance_v(start, end);
        Vector2 difference = end - start;
        lazer_rect.x = (int)(start.x + (difference.x / 2) - (distance / 2));
        lazer_rect.y = (int)(start.y + (difference.y / 2) - (height / 2));
        lazer_rect.w = (int)distance;
        lazer_rect.h = height;
    }
    
    void create_player_mothership(const MothershipConfig &mothership, ECS::EntityManager &entity_manager, std::vector<MotherShip> &motherships) {
        Vector2 position = Vector2(70, (float)gh / 2);

        MotherShip ship(entity_manager.create());
        ship.faction = FactionComponent { PLAYER_FACTION };

        std::string mothership_white_sprite = mothership.sprite_base + "_w";

        SpriteComponent s = SpriteComponent({ 
                Animation("idle", { { "combat_sprites", mothership.sprite_base } }, 0, false),
                Animation("hit", { 
                    { "combat_sprites", mothership_white_sprite },
                    { "combat_sprites", mothership.sprite_base },
                    { "combat_sprites", mothership_white_sprite },
                    { "combat_sprites", mothership.sprite_base },
                    { "combat_sprites", mothership_white_sprite },
                    { "combat_sprites", mothership.sprite_base },
                    { "combat_sprites", mothership_white_sprite }
                },  8, false)
            });
        s.layer = MOTHERSHIP_LAYER;
        s.flip = 0;
        ship.sprite = s;
        ship.position = position;
        ship.defense = DefenseComponent(mothership.defense.hp, mothership.defense.shield);

        for(auto &a : mothership.abilities) {
            if(a.type == AbilityConfig::IsAbility) {
                AbilityComponent ac;
                ac.ability.name = "Ability " + std::to_string(a.abilityTest);
                ac.ability.faction = PLAYER_FACTION;
                ac.ability.reload_time = 6.0f;
                ship.abilities.add(ac, true);
            } else {
                WeaponComponent weaponComponent = WeaponComponent(a.weapon,
                    a.weapon.name, 
                    a.targeting == 1 ? _random_multi_targeter : _random_targeter, 
                    a.weapon.projectile_type);
                
                ship.abilities.add(weaponComponent, true);
            }
        }

        auto sprite_sheet_index = Resources::sprite_sheet_index("combat_sprites");
        auto rect = Resources::sprite_get_from_sheet(sprite_sheet_index, mothership.sprite_base);
        ship.collision = CollisionData(rect.w, rect.h);

        motherships.push_back(ship);
    }

    void create_enemy_mothership(const int &seed, const int &difficulty, const int &node_distance, ECS::EntityManager &entity_manager, std::vector<MotherShip> &motherships) {
        if(node_distance < 3) {
            return;
        }

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

        motherships.push_back(ship);
    }

    void create_player_fighters(const std::vector<FighterData> &fighter_ids, ECS::EntityManager &entity_manager, std::vector<FighterShip> &fighters) {
        Vector2 position = Vector2(170, 50);

        float i = 0.0f;
        for(auto &f : fighter_ids) {
            for(int j = 0; j < f.count; j++) {
                auto &f_cfg = Services::db()->get_fighter_config(f.id);

                FighterShip ship(entity_manager.create());
                ship.faction = FactionComponent { PLAYER_FACTION };
                float y = position.y + i * 30.f;
                ship.position = RNG::vector2(position.x - 10, position.x + 10, y - 8, y + 8);
                ship.defense = DefenseComponent(f_cfg.defense.hp, f_cfg.defense.shield);

                std::string fighter_white_sprite = f_cfg.sprite_base + "_w";

                SpriteComponent s = SpriteComponent({ 
                    Animation("idle", { { "combat_sprites", f_cfg.sprite_base } }, 0, false),
                    Animation("hit", { 
                        { "combat_sprites", fighter_white_sprite },
                        { "combat_sprites", f_cfg.sprite_base },
                        { "combat_sprites", fighter_white_sprite },
                        { "combat_sprites", f_cfg.sprite_base },
                        { "combat_sprites", fighter_white_sprite },
                        { "combat_sprites", f_cfg.sprite_base },
                        { "combat_sprites", fighter_white_sprite }
                    },  6, false)
                });
                s.layer = FIGHTER_LAYER;
                s.flip = 0;
                ship.sprite = s;

                for(auto &w : f_cfg.weapons) {
                    WeaponComponent weaponComponent = WeaponComponent(w.weapon,
                        w.weapon.name, 
                        w.targeting == 1 ? _random_multi_targeter : _random_targeter, 
                        w.weapon.projectile_type);
                    
                    ship.abilities.add(weaponComponent);
                }
                
                auto sprite_sheet_index = Resources::sprite_sheet_index("combat_sprites");
                auto rect = Resources::sprite_get_from_sheet(sprite_sheet_index, f_cfg.sprite_base);
                ship.collision = CollisionData(rect.w, rect.h);

                fighters.push_back(ship);
                
                i++;
            }
        }
    }

    void create_enemy_fighters(const int &seed, const int &difficulty, const int &node_distance, ECS::EntityManager &entity_manager, std::vector<FighterShip> &fighters) {
        int fighters_to_generate = node_distance + 1;

        Vector2 position = Vector2((float)gw - 170, 50);

        for(int i = 0; i < fighters_to_generate; i++) {
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

            auto weap = GLOBAL_BASE_WEAPON;
            WeaponComponent weaponComponent = WeaponComponent(weap, weap.name, _random_targeter, weap.projectile_type);
            
            ship.abilities.add(weaponComponent);
            
            auto sprite_sheet_index = Resources::sprite_sheet_index("combat_sprites");
            auto rect = Resources::sprite_get_from_sheet(sprite_sheet_index, "cs2");
            ship.collision = CollisionData(rect.w, rect.h);

            fighters.push_back(ship);
        }
    }

    void create_projectile(const ProjectileSpawn &spawn, ECS::EntityManager &entity_manager, std::vector<Projectile> &projectiles) {
        const float angle = Math::angle_between_v(spawn.position, spawn.target_position);
        const ProjectilePayLoad &payload = spawn.payload;
        
        auto sc = SpriteComponent("combat_sprites", weapon_projectile_sprite(spawn.projectile_type));
        sc.layer = PROJECTILE_LAYER;
        sc.rotation = angle;

        Projectile p(entity_manager.create());

        if(spawn.projectile_type == ProjectileType::LazerBeamGreen) {
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
                projectiles.push_back(p);
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

            projectiles.push_back(p);
            
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

            projectiles.push_back(p);
            
        }
    }
}

#endif