#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "engine.h"
#include "components.h"

/// GENERAL SYSTEMS (Reusable probably)
/// ============================================

template<typename Entity>
void system_move_forward(Entity &entities) {
    for(auto &pr : entities) {
        if(pr.velocity.value.x != 0 || pr.velocity.value.y != 0) {
            pr.position.last = pr.position.value;
            pr.position.value += pr.velocity.value * Time::delta_time;
        }
    }
}

template<typename Entity>
void system_velocity_increase(Entity &entities) {
    for(auto &e : entities) {
        if(e.velocity.change != 0) {
            e.velocity.value *= e.velocity.change;
        }
        if(e.velocity.max != 0) {
            e.velocity.value = Math::clamp_max_magnitude(e.velocity.value, e.velocity.max);
        }
    }
}

template<typename Entity>
void system_remove_outside(Entity &entities, Rectangle &world_bounds) {
    for(auto &entity : entities) {
        if(!world_bounds.contains(entity.position.value.to_point())) {
            entity.life_time.marked_for_deletion = true;
        }
    }
}

template<typename Entity>
void system_homing(Entity &entities) {
    for(auto &e : entities) {
        if(!e.homing.enabled) {
            continue;
        }
        auto projectile_heading = e.velocity.value.normal();
        const Vector2 &pos = e.position.value;
        const Vector2 &target_pos = e.homing.target_position;
        float angle = Math::rads_between_v(pos, target_pos);
        auto to_target_heading = Vector2(Math::cos_f(angle), Math::sin_f(angle)).normal();
        auto final_heading = (projectile_heading + 0.1f * to_target_heading).normal();
                
        e.velocity.value = final_heading * e.velocity.value.length();
    }
    /*
    auto projectile_heading = homing_entities.velocity[i].value.normal();
                float angle = Math::rads_between_v(s, target);
                auto to_target_heading = Vector2(Math::cos_f(angle), Math::sin_f(angle)).normal();
                auto final_heading = (projectile_heading + 0.1f * to_target_heading).normal();
                
                homing_entities.velocity[i].value = final_heading * homing_entities.velocity[i].value.length();
    */
}

struct CollisionPair {
    ECS::Entity first;
    ECS::Entity second;
    float distance;
    Vector2 collision_point;
    bool operator<( const CollisionPair& rhs ) const { 
        return distance < rhs.distance; 
    }
};

struct CollisionPairs {
    std::vector<CollisionPair> collisions;
    int count = 0;
    inline CollisionPair operator [](size_t i) const { return collisions[i]; }
    inline CollisionPair & operator [](size_t i) { return collisions[i]; }

    void allocate(size_t size) {
        collisions.reserve(size);
    }

    void sort_by_distance() {
        std::sort(collisions.begin(), collisions.end());
    }

    void push(ECS::Entity first, ECS::Entity second, float distance, Vector2 collision_point) {
        collisions.push_back({ first, second, distance, collision_point });
        count++;
    }

    void clear() {
        count = 0;
        collisions.clear();
    }
};

template<typename First, typename Second>
void system_collisions(CollisionPairs &collision_pairs, const First &entity_first, const Second &entity_second) {
    for(auto &first : entity_first) {
        for(auto &second : entity_second) {
            if(first.faction.faction == second.faction.faction) {
                continue;
            }
            const Vector2 p_pos = first.position.value;
            const float first_radius = (float)first.collision.radius;
            const Vector2 p_last = first.position.last;
            const Vector2 t_pos = second.position.value;
            if(Math::intersect_circle_AABB(p_pos.x, p_pos.y, first_radius, t_pos.x, t_pos.y, (float)second.collision.aabb.w, (float)second.collision.aabb.h)) {
                float dist = Math::distance_v(p_last, t_pos);
                Vector2 collision_point;
                collision_point.x = Math::max_f(t_pos.x, Math::min_f(p_pos.x, t_pos.x + (float)second.collision.aabb.w));
                collision_point.y = Math::max_f(t_pos.y, Math::min_f(p_pos.y, t_pos.y + (float)second.collision.aabb.h));
                collision_pairs.push(first.entity, second.entity, dist, collision_point);
            }
            // const Vector2 t_pos = second.position.value;
            // const float t_radius = (float)second.collision.radius;
            // if(Math::intersect_circles(p_pos.x, p_pos.y, first_radius, t_pos.x, t_pos.y, t_radius)) {
            //     // Collision point is the point on the target circle 
            //     // that is on the edge in the direction of the projectiles 
            //     // reverse velocity
            //     //Engine::logn("circle intersect");
            //     float dist = Math::distance_v(p_last, t_pos);
            //     Vector2 collision_point = t_pos + (t_radius * -first.velocity.value.normal());
            //     collision_pairs.push(first.entity, second.entity, dist, collision_point);
            //     continue;
            // }
            // Vector2 entry_point;
            // int result = Intersects::line_circle_entry(p_last, p_pos, t_pos, t_radius, entry_point);
            // if(result == 1 || result == 2) {
            //     float dist = Math::distance_v(p_last, t_pos);
            //     Vector2 collision_point = t_pos + (t_radius * first.velocity.value.normal());
            //     collision_pairs.push(first.entity, second.entity, dist, collision_point);
            //     Engine::logn("line intersect");
            // }
        }
    }
}

template<typename Entity>
void system_shield_recharge(Entity &entities) {
    for(auto &entity : entities) {
        entity.defense.shield_recharge(Time::delta_time);
    }
}

template<typename Entity>
void system_update_ttl(Entity &entities) {
    for(auto &entity : entities) {
        if(entity.life_time.ttl > 0) {
            entity.life_time.time += Time::delta_time;
            if(entity.life_time.time >= entity.life_time.ttl) {
                entity.life_time.marked_for_deletion = true;
            }
        }
    }
}

template<typename Entity> 
void system_animation(Entity &entities)  {
    // Animation system
    for (auto &ship : entities) { 
        ship.sprite.update_animation(Time::delta_time);
    }
}
/// ============================================

#endif