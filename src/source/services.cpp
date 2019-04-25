#include "services.h"

namespace Services {
    ECS::ArchetypeManager archetype_manager;
    UIManager ui_manager;

    ECS::ArchetypeManager &arch_manager() {
        return archetype_manager;
    }

    UIManager &ui() {
        return ui_manager;
    }
}
