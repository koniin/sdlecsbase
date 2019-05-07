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

    std::vector<AnimationFrame> _frames;

    Animation(const std::string &identifier, const std::vector<AnimationFrame> &frames, const float fps, const bool loop) {
        _identifier = identifier;
        _frame = 0;
        _timer = 0.0f;
        _fps = fps;
        if(fps > 0) {
            _duration = 1.0f / fps;
        }
        _loop = loop;
        
        _frames = frames;
    }

    void update(float dt) {
        _timer += dt;
		if(_timer >= _duration) {
			_frame++;
			if(_frame >= _frames.size()) {
				if(_loop) {
					_frame = 0;
				} else {
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

    public:
    int layer = 0;
    float scale = 1.0f;
    float rotation = 0.0f;
    short flip = 0; // 1 = Horizontal, 2 = Vertical

    SpriteComponent() {}

    SpriteComponent(const std::string &sprite_sheet_name, std::string sprite_name) {
        animations.push_back(Animation("_____ONE_____", { { sprite_sheet_name, sprite_name } }, 0, false));
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
                animations[i]._timer = 0.0f;
                current_animation = i;
                return;
            }
        }
    }

	// void add_animation(std::string identifier, std::string sprite_name, std::string sprite_sheet_name, float fps, bool loop = false) {
    //     Animation a;
    //     if(animations.size() > 0) {
    //         // Copy all properties from default
    //         a.frames.push_back(SpriteRender(sprite_sheet_name, sprite_name, animations[0].frames[0].render_data));
    //     } else {
    //         a.frames.push_back(SpriteRender(sprite_sheet_name, sprite_name));
    //     }
    //     a.identifier = identifier;
	// 	a.fps = fps;
	// 	a.duration = 1.0f / fps;
	// 	a.loop = loop;
	// 	animations.push_back(a);
    // }

    void update_animation(float dt) {
        auto &animation = animations[current_animation];
		animation.update(dt);
    }
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

struct TravelDistance {
    float amount = 0;
    float target = 0;

    TravelDistance() {}
    TravelDistance(float t) : target(t) {}
};

struct ProjectileDamageDistance {
    float distance = 0;
    short hit = 0;
    int damage = 0;
    ECS::Entity target;
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

struct WeaponConfigurationComponent {
    std::string name; // (Blaster MK2 etc)
    float reload_time; // in seconds (0.2f)
    float damage; 
    float accuracy;
    std::string projectile_type;
};

struct FactionComponent {
    int faction = -1;
};

struct TargetComponent {
    ECS::Entity entity;
};

#endif