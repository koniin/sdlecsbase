#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "engine.h"
#include "weapons.h"

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
    float change = 0.f;
    float max = 0.f;

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

    bool line = false;
    int w = 0;
    int h = 0;

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
        if(animations[current_animation]._identifier == animation) {
            return;
        }

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
    Rectangle aabb;

    CollisionData() {}
    CollisionData(int r) {
        radius = r;
    }
    CollisionData(int w, int h) {
        aabb = Rectangle(0, 0, w, h);
    }
};

// ==============================================================


// ========
// Game specific components
// ========

struct AutomaticFireComponent {
    float fire_cooldown = 2.0f;
};

struct FactionComponent {
    int faction = -1;
};

struct HomingComponent {
    ECS::Entity target;
    Vector2 target_position;
    bool enabled = false;
};

struct Projectile {
    ECS::Entity entity;
    Position position;
    SpriteComponent sprite;
    LifeTime life_time;
    Projectile(ECS::Entity e) : entity(e) {}

    CollisionData collision;
    Velocity velocity;
    ProjectilePayLoad payload;
    FactionComponent faction;
    HomingComponent homing;
};

struct ProjectileMiss {
    ECS::Entity entity;    
    Position position;
    SpriteComponent sprite;
    LifeTime life_time;
    ProjectileMiss(ECS::Entity e) : entity(e) {}
    ProjectileMiss(ECS::Entity e, Projectile &p) : entity(e) {
        position = p.position;
        sprite = p.sprite;
        velocity = p.velocity;
        homing = p.homing;
    }

    Velocity velocity;
    HomingComponent homing;
};

struct MotherShip {
    ECS::Entity entity;
    Position position;
    SpriteComponent sprite;
    LifeTime life_time;
    MotherShip(ECS::Entity e) : entity(e) {}

    CollisionData collision;
    FactionComponent faction;
    MultiWeaponComponent weapons;
    DefenseComponent defense;
};

struct FighterShip {
    ECS::Entity entity;
    Position position;
    SpriteComponent sprite;
    LifeTime life_time;
    FighterShip(ECS::Entity e) : entity(e) {}

    CollisionData collision;
    FactionComponent faction;
    AutomaticFireComponent automatic_fire;
    MultiWeaponComponent weapons;
    DefenseComponent defense;
};

#endif