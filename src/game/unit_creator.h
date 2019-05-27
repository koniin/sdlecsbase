#ifndef UNIT_CREATOR_H
#define UNIT_CREATOR_H

#include "engine.h"
#include "renderer.h"
#include "components.h"

namespace UnitCreator {
    const int MOTHERSHIP_LAYER = 100;
    const int FIGHTER_LAYER = 50;
    const int PROJECTILE_LAYER = 30;

    struct OneRandomTargeter : Targeting {
        std::function<bool(const int &exclude_faction, Targeting::Target &target)> get_one_target;
        OneRandomTargeter(std::function<bool(const int &exclude_faction, Targeting::Target &target)> targeter) : get_one_target(targeter) {}

        bool get_targets(const int &exclude_faction, const size_t &max_count, Targeting::Targets &targets) override {
            Targeting::Target target;
            if(get_one_target(exclude_faction, target)) {
                for(size_t i = 0; i < max_count; i++) {
                    targets.targets.push_back(target);
                }
                return true;
            }
            return false;
        }
    };

    struct MultiRandomTargeter : Targeting {
        std::function<bool(const int &exclude_faction, Targeting::Target &target)> get_one_target;
        MultiRandomTargeter(std::function<bool(const int &exclude_faction, Targeting::Target &target)> targeter) : get_one_target(targeter) {}

        bool get_targets(const int &exclude_faction, const size_t &max_count, Targeting::Targets &targets) override {
            Targeting::Target target;
            bool has_any_target = false;
            for(size_t i = 0; i < max_count; i++) {
                if(get_one_target(exclude_faction, target)) {
                    targets.targets.push_back(target);
                    has_any_target = true;
                }
            }
            return has_any_target;
        }
    };

    std::shared_ptr<Targeting> _random_targeter;
    std::shared_ptr<Targeting> _random_multi_targeter;

    void init_targeters(std::function<bool(const int &exclude_faction, Targeting::Target &target)> target_function) {
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
    
    void create_player_mothership(ECS::EntityManager &entity_manager, std::vector<MotherShip> &motherships) {
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

        WeaponComponent weaponComponent = WeaponComponent(GLOBAL_BASE_WEAPON, "Mothership blast cannon", _random_multi_targeter, ProjectileType::Missile);
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

        motherships.push_back(ship);
    }

    void create_enemy_mothership(ECS::EntityManager &entity_manager, std::vector<MotherShip> &motherships) {
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

    void create_player_fighters(ECS::EntityManager &entity_manager, std::vector<FighterShip> &fighters) {
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
                weaponComponent = WeaponComponent(GLOBAL_BASE_WEAPON, "Missiles", _random_targeter, ProjectileType::Missile);
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::Accuracy, 0.4f));
                weaponComponent.add(ValueModifier<int>::make("temp", WeaponProperty::Damage, 1));
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ProjectileSpeed, -400.0f));
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ProjectileSpeedIncrease, 1.051f));
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ProjectileSpeedMax, 300.5f));
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ReloadTime, 2.0f));
            } else if(w_choice == 1) {
                weaponComponent = WeaponComponent(GLOBAL_BASE_WEAPON, "Lazer Beam", _random_targeter, ProjectileType::GreenLazerBeam);
                // Beams dont miss
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::Accuracy, 0.5f));
                weaponComponent.add(ValueModifier<int>::make("temp", WeaponProperty::Damage, 2));
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ProjectileSpeed, -500.0f));
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::ReloadTime, 4.0f));
            } else {
                weaponComponent = WeaponComponent(GLOBAL_BASE_WEAPON, "Lazer Gun", _random_targeter, ProjectileType::RedLazerBullet);
                weaponComponent.add(ValueModifier<float>::make("temp", WeaponProperty::Accuracy, 0.3f));        
            }

            ship.automatic_fire = AutomaticFireComponent { weaponComponent.get_weapon().reload_time };
            ship.weapons.add(weaponComponent);

            auto sprite_sheet_index = Resources::sprite_sheet_index("combat_sprites");
            auto rect = Resources::sprite_get_from_sheet(sprite_sheet_index, "cs1");
            ship.collision = CollisionData(rect.w, rect.h);

            fighters.push_back(ship);
        }
    }

    void create_enemy_fighters(ECS::EntityManager &entity_manager, std::vector<FighterShip> &fighters) {
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

            WeaponComponent weaponComponent = WeaponComponent(GLOBAL_BASE_WEAPON, "Enemy Gun", _random_targeter, ProjectileType::RedLazerBullet);
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