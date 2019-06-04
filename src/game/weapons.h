#ifndef WEAPONS_H
#define WEAPONS_H

#include "engine.h"

enum ProjectileType {
    Bullet,
    SmallBullet,
    GreenLazerBeam,
    Missile,
    RedLazerBullet
};

struct ProjectilePayLoad {
	float accuracy;
	int radius;

    int amount;
    enum DamageType {
        Energy,
        Explosive,
        Kinetic,
        Molten
    } damage_type;
    ProjectileType projectile_type;
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
    std::string name;
    float reload_time;
    int damage;
    float accuracy;
    ProjectileType projectile_type;
    int projectile_count;
    float burst_delay;
    int radius;
    float projectile_speed;
    float projectile_speed_increase;
    float projectile_speed_max;
};

extern Weapon GLOBAL_BASE_WEAPON;

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

struct WeaponAddModifier : WeaponModifier {
    Weapon _w;

    void modify(Weapon &weapon) override {
        weapon.name += _w.name;
        weapon.projectile_type = _w.projectile_type;

        weapon.reload_time += _w.reload_time;
        weapon.damage += _w.damage;
        weapon.accuracy += _w.accuracy;
        weapon.projectile_count += _w.projectile_count;
        weapon.burst_delay += _w.burst_delay;
        weapon.radius += _w.radius;
        weapon.projectile_speed += _w.projectile_speed;
        weapon.projectile_speed_increase += _w.projectile_speed_increase;
        weapon.projectile_speed_max += _w.projectile_speed_max;
    }
};  

struct WeaponMultiModifier : WeaponModifier {
    Weapon _w;

    void modify(Weapon &weapon) override {
        weapon.name += _w.name;
        weapon.projectile_type = _w.projectile_type;

        weapon.reload_time *= _w.reload_time;
        weapon.damage *= _w.damage;
        weapon.accuracy *= _w.accuracy;
        weapon.projectile_count *= _w.projectile_count;
        weapon.burst_delay *= _w.burst_delay;
        weapon.radius *= _w.radius;
        weapon.projectile_speed *= _w.projectile_speed;
        weapon.projectile_speed_increase *= _w.projectile_speed_increase;
        weapon.projectile_speed_max *= _w.projectile_speed_max;
    }
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

    static std::shared_ptr<WeaponModifier> make(std::string name, WeaponProperty property, T value) {
        return std::make_shared<ValueModifier<T>>(ValueModifier<T>(name, property, value));
    }

    void modify(Weapon &weapon) override {
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
                weapon.damage += (int)_value;
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

inline std::string weapon_projectile_sprite(ProjectileType type) {
    switch(type) {
        case ProjectileType::Bullet: {
            return "bullet_3";
        }
        case ProjectileType::SmallBullet: {
            return "bullet_4";
        }
        case ProjectileType::GreenLazerBeam: {
            return "lazer";
        }
        case ProjectileType::Missile: {
            return "bullet_4";
        }
        case ProjectileType::RedLazerBullet: {
            return "RedLazerBullet";
        }
    }
    ASSERT_WITH_MSG(false, "weapon_projectile_sprite: ProjectileType not implemented!");
    return "";
}

inline ProjectilePayLoad::DamageType weapon_payload_type(ProjectileType type) {
    switch(type) {
        case ProjectileType::Bullet: {
            return ProjectilePayLoad::DamageType::Kinetic;
        }
        case ProjectileType::SmallBullet: {
            return ProjectilePayLoad::DamageType::Kinetic;
        }
        case ProjectileType::GreenLazerBeam: {
            return ProjectilePayLoad::DamageType::Energy;
        }
        case ProjectileType::Missile: {
            return ProjectilePayLoad::DamageType::Explosive;
        }
        case ProjectileType::RedLazerBullet: {
            return ProjectilePayLoad::DamageType::Energy;
        }
    }
    ASSERT_WITH_MSG(false, "weapon_payload_type: ProjectileType not implemented!");
    return ProjectilePayLoad::DamageType::Kinetic;
}

inline bool weapon_is_beam(ProjectileType type) {
    switch(type) {
        case ProjectileType::Bullet:
        case ProjectileType::SmallBullet:
        case ProjectileType::RedLazerBullet:
        case ProjectileType::Missile:
            return false;
        case ProjectileType::GreenLazerBeam:
            return true;
    }
    ASSERT_WITH_MSG(false, "weapon_is_beam: ProjectileType not implemented!");
    return false;
}

inline int weapon_get_radius(ProjectileType type) {
    switch(type) {
        case ProjectileType::Bullet: {
            return 6;
        }
        case ProjectileType::SmallBullet: {
            return 5;
        }
        case ProjectileType::GreenLazerBeam: {
            return 4;
        }
        case ProjectileType::Missile: {
            return 5;
        }
        case ProjectileType::RedLazerBullet: {
            return 5;
        }
    }
    ASSERT_WITH_MSG(false, "weapon_get_radius: ProjectileType not implemented!");
    return 0;
}

struct WeaponComponent {
    private:
    Weapon _weapon;
    std::shared_ptr<Targeting> _targeting;
    std::vector<std::shared_ptr<WeaponModifier>> _weaponModifiers;

    public:
    WeaponComponent() {}
    WeaponComponent(Weapon base_weapon, std::string name, std::shared_ptr<Targeting> targeting, ProjectileType type) : _targeting(targeting) {
        _weapon = base_weapon;
        _weapon.name = name;
        _weapon.projectile_type = type;

        _weapon.radius = weapon_get_radius(type);
    }
    
    // void add(std::shared_ptr<WeaponModifier> modifier) {
    //     _weaponModifiers.push_back(modifier);
    // }

    template<typename T>
    void add(T modifier) {
        _weaponModifiers.push_back(modifier);
    }

    template<typename T, typename... Args>
    T add(T first, Args... args) {
        add(first);
        adder(args...);
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
        payload.projectile_type = weapon.projectile_type;

        spawn.payload = payload;

        Targeting::Targets targets;
        auto targets_found = _targeting->get_targets(faction, weapon.projectile_count, targets);
        if(!targets_found) {
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

struct MultiAbilityComponent {
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

    void update_timer(float dt) {
        for(auto &timer : _reload_timer) {
            timer += Time::delta_time;
        }    
    }

    Weapon get_weapon(int id) {
        return _weapons[id].get_weapon();
    }

    float get_timer(int id) {
        return _reload_timer[id];
    }

    bool can_use(int id) {
        size_t index = id;
        if(index < 0 || index >= _weapons.size()) {
            return false;
        }
        return _reload_timer[index] > _weapons[index].get_weapon().reload_time;
    }

    void use(int id, int faction, Vector2 position, std::vector<ProjectileSpawn> &projectile_spawns) {
        size_t index = id;
         _weapons[index].make_spawns(faction, position, projectile_spawns);    
         _reload_timer[index] = 0.0f;
    }
};

struct DefenseComponent {
    int hp = 0;
    int hp_max = 0;
    int shield = 0;
    int shield_max = 0;
    float shield_recharge_rate = 2.0f;
    int shield_recharge_amount = 1;

    float shield_timer = 0.0f;

    DefenseComponent() {}
    DefenseComponent(const int hp, const int shield) : hp(hp), shield(shield) {
        hp_max = hp;
        shield_max = shield;
    }

    void handle(const ProjectilePayLoad &payload) {    
        switch(payload.damage_type) {
            case ProjectilePayLoad::DamageType::Energy: {
                int reminder = shield_damage(payload.amount);
                hp_damage(reminder);
                break;
            }
            case ProjectilePayLoad::DamageType::Explosive: {
                hp_damage(payload.amount);
                break;
            }
            case ProjectilePayLoad::DamageType::Kinetic: {
                hp_damage(payload.amount);
                break;
            }
            case ProjectilePayLoad::DamageType::Molten: {
                shield_damage(payload.amount);
                hp_damage(payload.amount);
            }
            default: {
                ASSERT_WITH_MSG(false, "ProjectilePayLoad damage type is not implemented");
            }
        }
    }
    
    void shield_recharge(float dt) {
        shield_timer += dt;
        if(shield_timer >= shield_recharge_rate) {
            shield = Math::clamp_i(shield + shield_recharge_amount, 0, shield_max);
            shield_timer = 0.0f;
        }
    }

    private:
    int hp_damage(int amount) {
        int reminder = hp - amount;
        hp = Math::clamp_i(reminder, 0, hp_max);
        return reminder < 0 ? -reminder : 0;
    }

    int shield_damage(int amount) {
        int reminder = shield - amount;
        shield = Math::clamp_i(reminder, 0, shield_max);
        return reminder < 0 ? -reminder : 0;
    }
};

#endif