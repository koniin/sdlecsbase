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


struct WeaponComponent {
    private:
    Weapon _weapon;
    std::shared_ptr<Targeting> _targeting;
    std::vector<std::shared_ptr<WeaponModifier>> _weaponModifiers;

    public:
    WeaponComponent() {}
    WeaponComponent(std::string name, std::shared_ptr<Targeting> targeting, ProjectileType type) : _targeting(targeting) {
        _weapon.name = name;
        _weapon.projectile_type = type;

        _weapon.radius = weapon_get_radius(type);
    }
    
    void add_modifier(std::shared_ptr<WeaponModifier> modifier) {
        _weaponModifiers.push_back(modifier);
    }

    Weapon get_weapon() {
        Weapon w = _weapon; // Make a copy
        for(auto modifier : _weaponModifiers) {
            modifier->modify(w);
        }
        return w;
    }

    void make_spawns(const int &faction, const Vector2 &position, std::vector<ProjectileSpawn> &spawns) {
        Weapon weapon = get_weapon();

        ProjectileSpawn spawn;
        spawn.faction = faction;
        spawn.position = position;
        spawn.projectile_speed = weapon.projectile_speed;
        spawn.projectile_type = weapon.projectile_type;
        spawn.projectile_speed_increase = weapon.projectile_speed_increase;
        spawn.projectile_speed_max = weapon.projectile_speed_max;
        
        ProjectilePayLoad payload;
        payload.accuracy = weapon.accuracy;
        payload.radius = weapon.radius;
        payload.amount = weapon.damage;
        payload.damage_type = weapon_payload_type(weapon.projectile_type);

        spawn.payload = payload;

        Targeting::Targets targets;
        auto targets_found = _targeting->get_targets(faction, weapon.projectile_count, targets);
        if(!targets_found) {
            Engine::logn("No targets found when trying to make projectile spawns");
            return;
        }
        for(int i = 0; i < weapon.projectile_count; i++) {
            auto next_target = targets.next();
            spawn.target = next_target.entity;
            spawn.target_position = next_target.position;
            spawn.delay = i * weapon.burst_delay;
            spawns.push_back(spawn);
        }

        // if(_targeting->get_one_target(faction, target)) {
        //     for(int i = 0; i < weapon.projectile_count; i++) {
        //         spawn.target = target.entity;
        //         spawn.target_position = target.position;
        //         spawn.delay = i * weapon.burst_delay;
        //         spawns.push_back(spawn);
        //     }
        // }
    }
};

struct MultiWeaponComponent {
    private:
    std::vector<WeaponComponent> _weapons;
    std::vector<bool> _manual_control;
    std::vector<float> _reload_timer;
    std::vector<int> _ids;
    
    public:
    void add(WeaponComponent wc, bool manual = false) {
        _ids.push_back(_weapons.size());
        _weapons.push_back(wc);
        _reload_timer.push_back(0.f);
        _manual_control.push_back(manual);
    }

    std::vector<int> &ids() {
        return _ids;
    }

    bool is_manual(int id) {
        return _manual_control[id];
    }

    void update_reload_timer(float dt) {
        for(auto &timer : _reload_timer) {
            timer += Time::delta_time;
        }    
    }

    Weapon get_weapon(int id) {
        return _weapons[id].get_weapon();
    }

    float get_reload_timer(int id) {
        return _reload_timer[id];
    }

    bool can_fire(int id) {
        size_t index = id;
        if(index < 0 || index >= _weapons.size()) {
            return false;
        }
        return _reload_timer[index] > _weapons[index].get_weapon().reload_time;
    }

    void fire(int id, int faction, Vector2 position, std::vector<ProjectileSpawn> &projectile_spawns) {
        size_t index = id;
         _weapons[index].make_spawns(faction, position, projectile_spawns);    
         _reload_timer[index] = 0.0f;
    }
};

#endif