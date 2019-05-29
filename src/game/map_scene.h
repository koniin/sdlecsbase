#ifndef MAP_SCENE_H
#define MAP_SCENE_H

#include "engine.h"
#include "renderer.h"

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

class MapScene : public Scene {
        public:
                void initialize() override;
                void begin() override;    
                void end() override;
                void update() override;
                void render() override;
                void unload() override;
        private:
                RenderBuffer render_buffer;
                Sprite _background;
};

#endif