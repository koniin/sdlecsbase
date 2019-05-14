#include "map_scene.h"
#include "game_input_wrapper.h"
#include "services.h"
#include "components.h"
#include "engine.h"
#include "renderer.h"

#include <chrono>

struct Node {
    Position position;
    SDL_Color color;
};

std::vector<Node> _nodes;
std::mt19937 not_random_generator;

// gör en tabell över alla noder som kan förekomma
// sen skala bara x och så du kan kolla i tabellen vad det ska vara för Node
// behövs inget random då

float scaleRange(float number, float high, float low)
{
  return (high + low) * 0.5f;
}

Node get_node(int x, int y, int seed) {
    Node node;

    float n = Noise::perlin((float)x * 10, (float)y * 10);
    n = RNG::zero_to_one(not_random_generator);
    if(n < 0) {
        n = -n;
    }

    if(n <= 0.1f) {
        node.color = { 65, 120, 200, 255 };
    } else if(n <= 0.2f) {
        node.color = { 255, 0, 0, 255 };
    } else if(n <= 0.3f) {
        node.color = { 255, 255, 0, 255 };
    } else if(n <= 0.4f) {
        node.color = { 255, 0, 255, 255 };
    } else if(n <= 0.5f) {
        node.color = { 0, 255, 0, 255 };
    } else if(n <= 0.6f) {
        node.color = { 0, 255, 255, 255 };
    } else if(n <= 0.7f) {
        node.color = { 0, 0, 255, 255 };
    } else if(n <= 0.8f) {
        node.color = { 125, 125, 255, 255 };
    } else if(n <= 0.9f) {
        node.color = { 125, 255, 125, 255 };
    } else {
        node.color = { 255, 125, 125, 255 };
    }

    return node;
}

void make_nodes() {
    int seed = Services::game_state()->seed;
    int global_x = 0;
    int global_y = 0;

    int visual_x = 20;
    int visual_y = 20;
    for(int y = global_y; y < 10; y++) {
        for(int x = global_x; x < 10; x++) {
            Node n = get_node(x, y, seed);
            n.position = Vector2(visual_x + (x * 30), visual_y + (y * 20));
            _nodes.push_back(n);
        }   
    }
}

void MapScene::initialize() {
    Engine::logn("[MAP] Init");
 	render_buffer.init(2048);
    // Resources::sprite_sheet_load("combat_sprites", "combat_sprites.data");
    
    Resources::sprite_load("background", "bkg1.png");

    _nodes.reserve(100);
}

void MapScene::begin() {
	Engine::logn("[MAP] Begin");
    Engine::logn("seed: %d",  Services::game_state()->seed);
    Noise::set_seed(Services::game_state()->seed);
    not_random_generator = std::mt19937(Services::game_state()->seed);

    make_nodes();

}

void MapScene::end() {
    Engine::logn("[MAP] End");
	render_buffer.clear();
}

void MapScene::update() {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	
    if(GInput::pressed(GInput::Action::Start)) {
	    Scenes::set_scene("level");
	}
    
    // Particles::update(GameController::particles, Time::delta_time);
    Services::events().emit();
    Services::ui().update();
    
    // render_export(render_buffer);

	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	auto diff = t2 - t1;
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( diff ).count();
	auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>( diff ).count();
	std::string frame_duration_mu = "update time mu: " + std::to_string(duration);
	std::string frame_duration_ms = "update time ms: " + std::to_string(duration_ms);
	FrameLog::log(frame_duration_mu);
	FrameLog::log(frame_duration_ms);
}

void MapScene::render() {
	renderer_clear();
    // Render Background
    draw_sprite(Resources::sprite_get("background"), 0, 0);
    
    SDL_Color color = Colors::green;
    for(auto &node: _nodes) {
        draw_g_circle_filled_color(node.position.value.x, node.position.value.y, 8, node.color);
    }

    //draw_buffer(render_buffer);

    draw_text_str(10, (int)(gh - 10), Colors::white, "Select a node to continue.. (just press start.)");
    //Particles::render_circles_filled(GameController::particles);
	Services::ui().render();
    renderer_draw_render_target_camera();
	renderer_flip();
}

void MapScene::unload() {
	Engine::logn("[MAP] Unload");
}