#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "engine.h"

// Should be components for the engine/renderer really
// Could be nice to have a base component library 
// Then you can move the export function also
// ==============================================================
struct Position {
    Vector2 value;
    Vector2 last;

    Position() : value(), last() {}
    Position(Vector2 pos) : value(pos), last(pos) {}
    Position(float x, float y) : Position(Vector2(x, y)) {}
};

struct Velocity {
    Vector2 value;
    float change;

    Velocity(): value(Vector2()) {}
    Velocity(Vector2 v): value(v) {}
    Velocity(float xv, float yv): value(xv, yv) {}
};

struct LifeTime {
    bool marked_for_deletion = false;
    float ttl = 0.0f;
    float time = 0.0f;

    LifeTime() {}
};

struct AnimationFrame {
    int w, h;
    int16_t radius;
    int16_t color_r;
    int16_t color_g;
    int16_t color_b;
    int16_t color_a;
    size_t sprite_sheet_index;
    std::string sprite_name;
    
    AnimationFrame() {}

    AnimationFrame(const std::string &sprite_sheet_name, const std::string &sprite_name) : sprite_name(sprite_name) {
        sprite_sheet_index = Resources::sprite_sheet_index(sprite_sheet_name);
        auto sprite = Resources::sprite_get_from_sheet(sprite_sheet_index, sprite_name);
        w = sprite.w;
        h = sprite.h;
        color_r = color_g = color_b = color_a = 255;
    }
};

struct Animation {
    std::string _identifier;
	size_t _frame = 0;
	float _timer = 0;
	float _fps = 3;
	float _duration = 1;
	bool _loop = false;
    bool _completed = false;

    std::vector<AnimationFrame> _frames;

    Animation(const std::string &identifier, const std::vector<AnimationFrame> &frames, const float fps, const bool loop) {
        _identifier = identifier;
        _fps = fps;
        if(fps > 0) {
            _duration = 1.0f / fps;
        }
        _loop = loop;
        _frames = frames;

        restart();
    }

    void restart() {
        _frame = 0;
        _timer = 0.0f;
        _completed = false;
    }

    void update(float dt) {
        _timer += dt;
		if(_timer >= _duration) {
			_frame++;
			if(_frame >= _frames.size()) {
				if(_loop) {
					_frame = 0;
				} else {
                    _completed = true;
					_frame = _frames.size() - 1;
				}
			}
			_timer = 0;
		}
    }
};

struct SpriteComponent {
    private:
    std::vector<Animation> animations;
    size_t current_animation = 0;

    int _next_animation = -1;

    public:
    int layer = 0;
    float scale = 1.0f;
    float rotation = 0.0f;
    short flip = 0; // 1 = Horizontal, 2 = Vertical

    SpriteComponent() {}

    SpriteComponent(const std::string &sprite_sheet_name, std::string sprite_name) {
        animations.push_back(Animation("_____DEFAULT_____", { { sprite_sheet_name, sprite_name } }, 0, false));
    }

    SpriteComponent(const std::vector<Animation> anims) {
        animations = anims;
    }

    const AnimationFrame &get_current_frame() const {
        auto &animation = animations[current_animation];
        return animation._frames[animation._frame];
    }

    void set_current_animation(std::string animation) {
        for(size_t i = 0; i < animations.size(); i++) {
            if(animations[i]._identifier == animation) {
                animations[i].restart();
                current_animation = i;
                return;
            }
        }
    }

    void set_current_animation(std::string animation, std::string next_animation) {
        set_current_animation(animation);
        for(size_t i = 0; i < animations.size(); i++) {
            if(animations[i]._identifier == next_animation) {
                _next_animation = i;
            }
        }
    }

    void update_animation(float dt) {
        auto &animation = animations[current_animation];
		animation.update(dt);
        if(animation._completed && _next_animation >= 0) {
            set_current_animation(animations[_next_animation]._identifier);
            _next_animation = -1;
        }
    }
};

struct CollisionData {
    int radius = 0;
};

// ==============================================================


// ========
// Game specific components
// ========

struct Hull {
    int amount = 0;

    Hull() {}
    Hull(const int n) : amount(n) {}
};

struct ProjectileDamage {
    int damage = 0;
};

struct PlayerInput {
    int controls_pressed[9];
    float fire_cooldown = 0.0f;
};

struct AIComponent {
    float fire_cooldown = 2.0f;
};

struct ParentComponent {
    ECS::Entity entity;
};

struct InputTriggerComponent {
    int trigger = 0;
};

struct ProjectilePayLoad {
	float accuracy;
	int radius;

    float damage;
};

struct ProjectileSpawn {
    int faction;
    Vector2 position;
    ECS::Entity target;
    float projectile_speed;
	std::string projectile_type;
    ProjectilePayLoad payload;
    float delay = 0;
    // Always last
    float timer = 0;
};

struct Targeting {
    virtual bool get_one_target(const int &exclude_faction, ECS::Entity &entity) = 0;
    virtual bool get_targets(const int &exclude_faction, const size_t &count, std::vector<ECS::Entity> &targets) = 0;
};

struct WeaponConfigurationComponent {
    std::string name; // (Blaster MK2 etc)
    float reload_time; // in seconds (0.2f)
    float damage; 
    float accuracy;
    std::string projectile_type;
    int projectile_count = 1;
    float burst_delay = 0.0f;
    int radius = 8;
    float projectile_speed = 500.0f;

    std::shared_ptr<Targeting> targeting;

    void make_spawns(const int &faction, const Vector2 &position, std::vector<ProjectileSpawn> &spawns) {
        ProjectileSpawn spawn;
        spawn.faction = faction;
        spawn.position = position;
        spawn.projectile_speed = projectile_speed;
        spawn.projectile_type = projectile_type;
        
        ProjectilePayLoad payload;
        payload.accuracy = accuracy;
        payload.radius = radius;
        payload.damage = damage;

        spawn.payload = payload;

        ECS::Entity target_entity;
        if(targeting->get_one_target(faction, target_entity)) {
            spawn.target = target_entity;
            
            for(int i = 0; i < projectile_count; i++) {
                spawn.delay = i * burst_delay;
                spawns.push_back(spawn);
            }
        }
    }
};

struct FactionComponent {
    int faction = -1;
};

struct TargetComponent {
    ECS::Entity entity;
};

struct MultiWeaponComponent {
    std::vector<WeaponConfigurationComponent> _weapons;
    std::vector<float> _reload_timer;
    
    void add(WeaponConfigurationComponent wc) {
        _weapons.push_back(wc);
        _reload_timer.push_back(0.f);
    }

    bool can_fire(int id) {
        size_t index = id - 1;
        if(index < 0 || index > _weapons.size() - 1) {
            return false;
        }
        return _reload_timer[index] > _weapons[index].reload_time;
    }

    WeaponConfigurationComponent get_config(int id) {
        size_t index = id - 1;
        _reload_timer[index] = 0.f;
        return _weapons[index];
    }
};

struct MotherShip {
    ECS::Entity entity;
    Position position;
    SpriteComponent sprite;
    MotherShip(ECS::Entity e) : entity(e) {}

    FactionComponent faction;
    MultiWeaponComponent weapons;
};

struct FighterShip {
    ECS::Entity entity;
    Position position;
    SpriteComponent sprite;
    LifeTime life_time;
    FighterShip(ECS::Entity e) : entity(e) {}

    CollisionData collision;
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

    CollisionData collision;
    Velocity velocity;
    ProjectileDamage damage;
    FactionComponent faction;
};

struct ProjectileMiss {
    ECS::Entity entity;    
    Position position;
    SpriteComponent sprite;
    LifeTime life_time;
    ProjectileMiss(ECS::Entity e) : entity(e) {}

    Velocity velocity;
};

#endif