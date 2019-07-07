#include "abilities.h"

ProjectileTypeData projectile_type_data[ProjectileType::COUNT] = {
    { "bullet_3", ProjectilePayLoad::DamageType::Kinetic, false, 6 },
    { "bullet_4", ProjectilePayLoad::DamageType::Kinetic, false, 5 },
    { "lazer_beam_1", ProjectilePayLoad::DamageType::Energy, true, 4 },
    { "bullet_5", ProjectilePayLoad::DamageType::Explosive, false, 5 },
    { "lazer_bullet_2", ProjectilePayLoad::DamageType::Energy, false, 5 },
    { "lazer_bullet_3", ProjectilePayLoad::DamageType::Energy, false, 5 }
};