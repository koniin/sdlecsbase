#ifndef MAP_SCENE_H
#define MAP_SCENE_H

#include "engine.h"
#include "renderer.h"

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
                Vector2 camera_pos;
                const int distance_to_next_node = 128;
                const float camera_gutter = 128.0f;
                float camera_y_speed = 0;
                float camera_x_speed = 0;
};

#endif