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
        ECS::ArchetypeManager em;
        ECS::ArcheType players;
        RenderBuffer render_buffer;
	    int counter = 0;

        void render_export();
};

#endif