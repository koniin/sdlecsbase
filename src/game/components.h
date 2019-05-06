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

struct SpriteRender {
    float scale;
    float rotation;
    int w, h;
    int16_t radius;
    int16_t color_r;
    int16_t color_g;
    int16_t color_b;
    int16_t color_a;
    size_t sprite_sheet_index;
    std::string sprite_name;
    int layer;
    bool line;
    Vector2 position;
    short flip = 0; // 1 = Horizontal, 2 = Vertical

    SpriteRender() {}

    SpriteRender(const std::string &sprite_sheet_name, const std::string &sprite_name) : sprite_name(sprite_name) {
        sprite_sheet_index = Resources::sprite_sheet_index(sprite_sheet_name);
        auto sprite = Resources::sprite_get_from_sheet(sprite_sheet_index, sprite_name);
        w = sprite.w;
        h = sprite.h;
        scale = 1.0f;
        rotation = 0.0f;
        color_r = color_g = color_b = color_a = 255;
        layer = 0;
        line = false;
        flip = 0;
    }

    SpriteRender(const std::string &sprite_sheet_name, const std::string &sprite_name, const SpriteRender &render) : sprite_name(sprite_name) {
        sprite_sheet_index = Resources::sprite_sheet_index(sprite_sheet_name);
        auto sprite = Resources::sprite_get_from_sheet(sprite_sheet_index, sprite_name);
        w = sprite.w;
        h = sprite.h;
        scale = render.scale;
        rotation = render.rotation;
        color_r = render.color_r;
        color_g = render.color_g;
        color_b = render.color_b;
        color_a = render.color_a;
        layer = render.layer;
        line = render.line;
        flip = render.flip;
    }
};

struct Animation {
    std::string identifier;
	int frame = 0;
	float timer = 0;
	float fps = 3;
	float duration = 1;
	bool loop = false;

	SpriteRender render_data;
};

struct SpriteComponent {
    private:
    std::vector<Animation> animations;
    size_t current_animation = 0;

    public:
    SpriteComponent() {}

    SpriteComponent(const std::string &sprite_sheet_name, std::string name) {
        add_animation("__default__", name, sprite_sheet_name, 0, false);
    }

    const SpriteRender &get(){
        return animations[current_animation].render_data;
    }

    void set_layer(int layer) {
        animations[0].render_data.layer = layer;
    }

    void set_rotation(float rotation) {
        animations[0].render_data.rotation = rotation;
    }

    void set_flip(short flip) {
        animations[0].render_data.flip = flip;
    }

    void set_current_animation(std::string animation) {
        for(size_t i = 0; i < animations.size(); i++) {
            if(animations[i].identifier == animation) {
                animations[i].timer = 0.0f;
                current_animation = i; 
                return;
            }
        }
    }

	void add_animation(std::string identifier, std::string sprite_name, std::string sprite_sheet_name, float fps, bool loop = false) {
        Animation a;
        if(animations.size() > 0) {
            // Copy all properties from default
            a.render_data = SpriteRender(sprite_sheet_name, sprite_name, animations[0].render_data);
        } else {
            a.render_data = SpriteRender(sprite_sheet_name, sprite_name);
        }
        a.identifier = identifier;
		a.fps = fps;
		a.duration = 1.0f / fps;
		a.loop = loop;
		animations.push_back(a);
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