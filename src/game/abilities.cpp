#include "abilities.h"

Weapon GLOBAL_BASE_WEAPON = {
    "GLOBAL_BASE", // std::string name = "Basic weapon"; // (Blaster MK2 etc)
    0.5f, //float reload_time = 1.0f; // in seconds (0.2f)
    1, // int damage = 1;
    0.8f, // float accuracy = 0.5f;
    ProjectileType::LazerBulletRed, // ProjectileType projectile_type; // name of sprite for projectile
    1, // int projectile_count = 1;
    0.0f, // float burst_delay = 0.0f;
    6, // int radius = 8;
    500.0f, // float projectile_speed = 500.0f;
    0.0f, // float projectile_speed_increase = 0.0f;
    0.0f // float projectile_speed_max = 0.0f;
};

ProjectileTypeData projectile_type_data[ProjectileType::COUNT] = {
    { "bullet_3", ProjectilePayLoad::DamageType::Kinetic, false, 6 },
    { "bullet_4", ProjectilePayLoad::DamageType::Kinetic, false, 5 },
    { "lazer_beam_1", ProjectilePayLoad::DamageType::Energy, true, 4 },
    { "bullet_5", ProjectilePayLoad::DamageType::Explosive, false, 5 },
    { "lazer_bullet_2", ProjectilePayLoad::DamageType::Energy, false, 5 }
};