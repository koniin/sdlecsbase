#ifndef SERVICES_H
#define SERVICES_H

#include "engine.h"
#include "ui_manager.h"

namespace Services {
    ECS::ArchetypeManager &arch_manager();
    UIManager &ui();
}

#endif