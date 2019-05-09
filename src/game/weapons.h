#ifndef WEAPONS_H
#define WEAPONS_H

#include "engine.h"


struct Targeting {
    struct Target {
        ECS::Entity entity;
        Vector2 position;
    };

    virtual bool get_one_target(const int &exclude_faction, Target &target) = 0;
    virtual bool get_targets(const int &exclude_faction, const size_t &count, std::vector<Target> &targets) = 0;
};

enum ProjectileType {
    Bullet,
    SmallBullet
};

struct Weapon {
    std::string name = "Basic weapon"; // (Blaster MK2 etc)
    float reload_time = 1.0f; // in seconds (0.2f)
    float damage = 5;
    float accuracy = 0.5f;
    ProjectileType projectile_type; // name of sprite for projectile
    int projectile_count = 1;
    float burst_delay = 0.0f;
    int radius = 8;
    float projectile_speed = 500.0f;
};

enum WeaponProperty {
    Accuracy,
    ReloadTime,
    Damage,
    Projectile_Type,
    Projectile_Count,
    BurstDelay,
    Radius,
    ProjectileSpeed
};

struct WeaponModifier {
    virtual void modify(Weapon &weapon) = 0;
};

template<typename T>
struct ValueModifier : WeaponModifier {
    std::string _name;
    T _value;
    WeaponProperty _property;
    ValueModifier(std::string name, WeaponProperty property, T value) {
        _name = name;
        _property = property;
        _value = value;
    }

    void modify(Weapon &weapon) {
        switch(_property) {
            case WeaponProperty::ReloadTime: {
                weapon.reload_time += _value;
                return;
            }
            case WeaponProperty::Damage: {
                weapon.damage += _value;
                return;
            }
            case WeaponProperty::Projectile_Type: {
                weapon.projectile_type = (ProjectileType)((int)(_value));
                return;
            }
            case WeaponProperty::Accuracy: {
                weapon.accuracy += _value;
                return;
            }
            case WeaponProperty::Projectile_Count: {
                weapon.projectile_count += (int)_value;
                return;
            }
            case WeaponProperty::BurstDelay: {
                weapon.burst_delay += _value;
                return;
            }
        }
    }
};

std::string get_projectile_sprite(ProjectileType type) {
    switch(type) {
        case ProjectileType::Bullet: {
            return "bullet_3";
        }
        case ProjectileType::SmallBullet: {
            return "bullet_4";
        }
    }
    ASSERT_WITH_MSG(false, "ProjectileType not implemented!");
    return "";
}

#endif