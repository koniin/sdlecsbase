#include "data_repository.h"

std::vector<FighterConfig> _fighters;

void DB::load() {
    FighterConfig f;
    f.defense = { 10, 5 };
    f.sprite_base = "cs1";

    WeaponConfig wc;
    wc.targeting = 2;

    wc.weapon = GLOBAL_BASE_WEAPON;
    
    f.weapons.push_back(wc);

    f.id = _fighters.size();
    f.name = "Lazer frigate";

    _fighters.push_back(f);
}

std::vector<FighterConfig> &DB::get_fighters() {
    return _fighters;
}

const FighterConfig &DB::get_fighter_config(int id) {
    for(auto &f : _fighters) {
        if(f.id == id) {
            return f;
        }
    }
    ASSERT_WITH_MSG(false, "Fighter config not found!");
}