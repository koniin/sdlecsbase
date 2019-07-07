#ifndef ABILITIES_H
#define ABILITIES_H

#include "engine.h"

enum ProjectileType {
    Bullet,
    SmallBullet,
    LazerBeamGreen,
    Missile,
    LazerBulletRed,
    LazerBulletRedLarge,
    COUNT
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

    virtual bool get_targets(const int &exclude_faction, const size_t &max_count, std::vector<ECS::EntityId> &target_overrides, Targets &targets) = 0;
};

struct Effect {
    int target_faction = 0;
    float tick;
    float tick_timer;
    float ttl;
    float ttl_timer;

    Effect(int faction_to_apply, float frequency_seconds, float time_to_live) 
        : target_faction(faction_to_apply), tick(frequency_seconds), ttl(time_to_live) {
        tick_timer = 0.0f;
        ttl_timer = 0.0f;
    }

    int shield_recharge_amount = 0;
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

struct ProjectileTypeData {
    std::string sprite;
    ProjectilePayLoad::DamageType damage_type;
    bool is_beam;
    int collision_radius;
};

extern ProjectileTypeData projectile_type_data[ProjectileType::COUNT];

inline std::string weapon_projectile_sprite(ProjectileType type) {
    return projectile_type_data[type].sprite;
}

inline ProjectilePayLoad::DamageType weapon_payload_type(ProjectileType type) {
    return projectile_type_data[type].damage_type;
}

inline bool weapon_is_beam(ProjectileType type) {
    return projectile_type_data[type].is_beam;
}

inline int weapon_get_radius(ProjectileType type) {
    return projectile_type_data[type].collision_radius;
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

    void make_spawns(const int &faction, const Vector2 &position, std::vector<ProjectileSpawn> &projectile_spawns, std::vector<Effect> &effects, std::vector<ECS::EntityId> &target_override) {
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
        auto targets_found = _targeting->get_targets(faction, weapon.projectile_count, target_override, targets);
        if(!targets_found) {
            return;
        }
        for(int i = 0; i < weapon.projectile_count; i++) {
            auto next_target = targets.next();
            spawn.target = next_target.entity;
            spawn.target_position = next_target.position;
            spawn.delay = i * weapon.burst_delay;
            projectile_spawns.push_back(spawn);
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

struct Ability {
    int faction;
    float reload_time = 0.0f;
    std::string name = "Test";
};

struct AbilityComponent {
    Ability ability;

    Ability &get_ability() {
        return ability;
    }

    void use(int faction, Vector2 position, std::vector<ProjectileSpawn> &projectile_spawns, std::vector<Effect> &effects, std::vector<ECS::EntityId> &target_override) {
        Engine::logn("Ability use: %s - %d", ability.name.c_str(), faction);

        Effect e = Effect(ability.faction, 2.0f, 4.0f);
        
        e.shield_recharge_amount = 2;

        effects.push_back(e);
    }
};

struct MultiAbilityComponent {
    private:
    struct TypeWrapper {
        enum { AbilityType, WeaponType } type;
        AbilityComponent a;
        WeaponComponent w;
    };

    std::vector<TypeWrapper> _abilities;
    std::vector<bool> _manual_control;
    std::vector<float> _reload_timer;
    std::vector<int> _ids;

    std::vector<ECS::EntityId> _target_overrides;

    public:
    void add(WeaponComponent wc, bool manual = false) {
        _ids.push_back(_abilities.size());
        TypeWrapper tw;
        tw.w = wc;
        tw.type = TypeWrapper::WeaponType;
        _abilities.push_back(tw);
        _reload_timer.push_back(0.f);
        _manual_control.push_back(manual);
    }

    void add(AbilityComponent ac, bool manual = false) {
        _ids.push_back(_abilities.size());
        TypeWrapper tw;
        tw.a = ac;
        tw.type = TypeWrapper::AbilityType;
        _abilities.push_back(tw);
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

    float get_cooldown(int id) {
        auto &ability = _abilities[id];
        if(ability.type == TypeWrapper::AbilityType) {
            return _abilities[id].a.get_ability().reload_time;
        } else {
            return _abilities[id].w.get_weapon().reload_time;
        }
    }

    std::string get_name(int id) {
        auto &ability = _abilities[id];
        if(ability.type == TypeWrapper::AbilityType) {
            return _abilities[id].a.get_ability().name;
        } else {
            return _abilities[id].w.get_weapon().name;
        }
    }

    float get_timer(int id) {
        return _reload_timer[id];
    }

    bool can_use(int id) {
        size_t index = id;
        if(index < 0 || index >= _abilities.size()) {
            return false;
        }

        float reload_time = 0.0f;
        auto &ability = _abilities[id];
        if(ability.type == TypeWrapper::AbilityType) {
            reload_time = _abilities[id].a.get_ability().reload_time;
        } else {
            reload_time = _abilities[id].w.get_weapon().reload_time;
        }

        return _reload_timer[index] > reload_time;
    }

    void use(int id, int faction, Vector2 position, std::vector<ProjectileSpawn> &projectile_spawns, std::vector<Effect> &effects) {
        size_t index = id;

        auto &ability = _abilities[id];
        if(ability.type == TypeWrapper::AbilityType) {
            _abilities[id].a.use(faction, position, projectile_spawns, effects, _target_overrides);
        } else {
            _abilities[id].w.make_spawns(faction, position, projectile_spawns, effects, _target_overrides);
        }

        _reload_timer[index] = 0.0f;
    }

    void apply(const Effect &e) {
        // Engine::logn("Multicomp = Apply effect! = %d", e.shield_recharge_amount);
    }

    void set_target_override(ECS::EntityId id) {
        _target_overrides.clear();
        _target_overrides.push_back(id);
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
    
    void apply(const Effect &e) {
        Engine::logn("Apply effect! = %d", e.shield_recharge_amount);
        shield = Math::clamp_i(shield + e.shield_recharge_amount, 0, shield_max);
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