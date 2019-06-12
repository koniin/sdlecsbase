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
};

#endif