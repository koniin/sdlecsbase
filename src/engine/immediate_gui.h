#ifndef IMMEDIATE_GUI_H
#define IMMEDIATE_GUI_H

#include <string>

namespace IGUI {
    void frame();
    void number_edit_i(std::string label, int *i, int min, int max, int step);
    void render();
}

#endif