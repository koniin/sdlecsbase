#ifndef LEVEL_SCENE_H
#define LEVEL_SCENE_H

#include "engine.h"
#include "renderer.h"

class LevelScene : public Scene {
        public:
                void initialize() override;
                void begin() override;    
                void end() override;
                void update() override;
                void render() override;
                void unload() override;
        private:
                ECS::ArchetypeManager arch_manager;
                RenderBuffer render_buffer;
                
                void render_export();
};

#endif