#ifndef MAP_NODE_H
#define MAP_NODE_H

#include "engine.h"

struct Node {
    Point maze_pos;
    Point render_position;
    SDL_Color color;
    int radius;
    int type;
    bool current = false;
    struct Connections {
        bool top = false;
        bool bottom = false;
        bool left = false;
        bool right = false;
    } connections;

    Point neighbour_left;
    Point neighbour_right;
    Point neighbour_top;
    Point neighbour_bottom;
};

#endif