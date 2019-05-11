#ifndef WEAPONS_H
#define WEAPONS_H

#include "engine.h"

struct ProjectilePayLoad {
	float accuracy;
	int radius;

    float amount;
    enum DamageType {
        Energy,
        Explosive,
        Kinetic,
        Molten
    } damage_type;
};

enum ProjectileType {
    Bullet,
    SmallBullet,
    GreenLazer,
    Missile
};

struct ProjectileSpawn {
    int faction;
    Vector2 position;
    Vector2 target_position;
    ECS::Entity target;
    float projectile_speed;
    float projectile_speed_increase;
    float projectile_speed_max;
	ProjectileType projectile_type;
    ProjectilePayLoad payload;
    float delay = 0;
    // Always last
    float timer = 0;
};

struct Targeting {
    struct Target {
        ECS::Entity entity;
        Vector2 position;
    };

    struct Targets {
        std::vector<Target> targets;
        size_t index = 0;

        Target &next() {
            if(index >= targets.size()) {
                index = 0;
            }
            return targets[index++];
        }
    };  

    virtual bool get_targets(const int &exclude_faction, const size_t &max_count, Targets &targets) = 0;
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
    float projectile_speed_increase = 0.0f;
    float projectile_speed_max = 0.0f;
};

enum WeaponProperty {
    Accuracy,
    ReloadTime,
    Damage,
    Projectile_Type,
    Projectile_Count,
    BurstDelay,
    Radius,
    ProjectileSpeed,
    ProjectileSpeedIncrease,
    ProjectileSpeedMax,
};

struct WeaponModifier {
    virtual void modify(Weapon &weapon) = 0;
};

template<typename T>
struct ValueModifier : WeaponModifier {
    std::string _name;
    T _value;
    WeaponProperty _property;
    int type = 0;
    ValueModifier(std::string name, WeaponProperty property, T value) {
        _name = name;
        _property = property;
        _value = value;
    }

    void modify(Weapon &weapon) {
        switch(_property) {
            case WeaponProperty::Accuracy: {
                weapon.accuracy += _value;
                return;
            }
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
            case WeaponProperty::Projectile_Count: {
                weapon.projectile_count += (int)_value;
                return;
            }
            case WeaponProperty::BurstDelay: {
                weapon.burst_delay += _value;
                return;
            }
            case WeaponProperty::Radius: {
                weapon.radius += (int)_value;
                return;
            }
            case WeaponProperty::ProjectileSpeed: {
                weapon.projectile_speed += _value;
                return;
            }
            case WeaponProperty::ProjectileSpeedIncrease: {
                weapon.projectile_speed_increase += _value;
                return;
            }
            case WeaponProperty::ProjectileSpeedMax: {
                weapon.projectile_speed_max += _value;
                return;
            }
        }
    }
};

int weapon_get_radius(ProjectileType type) {
    switch(type) {
        case ProjectileType::Bullet: {
            return 6;
        }
        case ProjectileType::SmallBullet: {
            return 5;
        }
        case ProjectileType::GreenLazer: {
            return 4;
        }
        case ProjectileType::Missile: {
            return 5;
        }
    }
    ASSERT_WITH_MSG(false, "weapon_get_radius: ProjectileType not implemented!");
    return 0;
}

std::string weapon_projectile_sprite(ProjectileType type) {
    switch(type) {
        case ProjectileType::Bullet: {
            return "bullet_3";
        }
        case ProjectileType::SmallBullet: {
            return "bullet_4";
        }
        case ProjectileType::GreenLazer: {
            return "lazer";
        }
        case ProjectileType::Missile: {
            return "bullet_4";
        }
    }
    ASSERT_WITH_MSG(false, "weapon_projectile_sprite: ProjectileType not implemented!");
    return "";
}

ProjectilePayLoad::DamageType weapon_payload_type(ProjectileType type) {
    switch(type) {
        case ProjectileType::Bullet: {
            return ProjectilePayLoad::DamageType::Kinetic;
        }
        case ProjectileType::SmallBullet: {
            return ProjectilePayLoad::DamageType::Kinetic;
        }
        case ProjectileType::GreenLazer: {
            return ProjectilePayLoad::DamageType::Energy;
        }
        case ProjectileType::Missile: {
            return ProjectilePayLoad::DamageType::Explosive;
        }
    }
    ASSERT_WITH_MSG(false, "weapon_payload_type: ProjectileType not implemented!");
    return ProjectilePayLoad::DamageType::Kinetic;
}

#endif