#include "weapons.h"

Weapon GLOBAL_BASE_WEAPON = {
    "GLOBAL_BASE", // std::string name = "Basic weapon"; // (Blaster MK2 etc)
    1.0f, //float reload_time = 1.0f; // in seconds (0.2f)
    1, // int damage = 1;
    0.5f, // float accuracy = 0.5f;
    ProjectileType::Bullet, // ProjectileType projectile_type; // name of sprite for projectile
    1, // int projectile_count = 1;
    0.0f, // float burst_delay = 0.0f;
    6, // int radius = 8;
    500.0f, // float projectile_speed = 500.0f;
    0.0f, // float projectile_speed_increase = 0.0f;
    0.0f // float projectile_speed_max = 0.0f;
};