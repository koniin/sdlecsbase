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
    struct Connections {
        bool top = false;
        bool bottom = false;
        bool left = false;
        bool right = false;
    } connections;
};

std::mt19937 not_random_generator;

float pseudo_rand_zero_to_one(uint32_t x, uint32_t y) {
	/* mix around the bits in x: */
	x = x * 3266489917 + 374761393;
	x = (x << 17) | (x >> 15);

	/* mix around the bits in y and mix those into x: */
	x += y * 3266489917;

	/* Give x a good stir: */
	x *= 668265263;
	x ^= x >> 15;
	x *= 2246822519;
	x ^= x >> 13;
	x *= 3266489917;
	x ^= x >> 16;

	/* trim the result and scale it to a float in [0,1): */
	return (x & 0x00ffffff) * (1.0f / 0x1000000);
}

const int node_distribution_count = 20;
const int nodes_distribution[node_distribution_count] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3 };

int get_node_type(uint32_t x, uint32_t y) {
    return nodes_distribution[(int)(pseudo_rand_zero_to_one(x, y) * node_distribution_count)];
}

const float max_distance = 15.0f;
int distance_to(int nodeid_a, int nodeid_b) {
    return (int)(pseudo_rand_zero_to_one(nodeid_a, nodeid_b) * max_distance);
}

const int arbitray_length = 14;
int get_node_id(int x, int y) {
    return x * arbitray_length + y;
}

Node get_node(int x, int y, int seed) {
    Node node;
    
    int id = get_node_type(x + seed, y + seed);
    //int n = pseudo_rand_zero_to_one(x, y);

    if(id == 1) {
        node.color = { 65, 120, 200, 255 };
    } else if(id == 2) {
        node.color = { 255, 0, 0, 255 };
    } else if(id == 3) {
        node.color = { 255, 255, 0, 255 };
    } else {
        ASSERT_WITH_MSG(false, "get_node: returned non specified node");
    }

    return node;
}

int global_x = 500000;
int global_y = 500000;

void MapScene::initialize() {
    Engine::logn("[MAP] Init");
 	render_buffer.init(2048);
    // Resources::sprite_sheet_load("combat_sprites", "combat_sprites.data");
    
    Resources::sprite_load("background", "bkg1.png");
}

void MapScene::begin() {
	Engine::logn("[MAP] Begin");
    Engine::logn("seed: %d",  Services::game_state()->seed);
    Noise::set_seed(Services::game_state()->seed);
    not_random_generator = std::mt19937(Services::game_state()->seed);
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

std::mt19937 _id_displacement;
Point get_node_displacement(int node_x, int node_y, int seed) {
    uint32_t r_seed = (uint32_t)(pseudo_rand_zero_to_one(node_x + seed, node_y + seed) * 3450971324.f);
    _id_displacement = std::mt19937(r_seed);

    Point p(RNG::range_i(-20, 20, _id_displacement), RNG::range_i(-20, 20, _id_displacement));
    return p;
}

void MapScene::render() {
	renderer_clear();
    // Render Background
    draw_sprite(Resources::sprite_get("background"), 0, 0);
    
    SDL_Color color = Colors::green;
    
    int visible_nodes_x = 6;
    int visible_nodes_y = 4;
    
    int x_spacing = gw / visible_nodes_x;
    int y_spacing = gh / visible_nodes_y;

    int x_width = x_spacing * (visible_nodes_x - 1);
    int y_height = y_spacing * (visible_nodes_y - 1);
    int visual_x = (gw / 2) - x_width / 2;
    int visual_y = (gh / 2) - y_height / 2;
    
    int seed = Services::game_state()->seed;

    Point d1 = get_node_displacement(100, 100, seed);
    Point d2 = get_node_displacement(101, 100, seed);
    Point d3 = get_node_displacement(100, 100, seed);

    // y,x = base coordinates
    // yi, xi = counting variables
    for(int y = global_y, yi = 0; yi < visible_nodes_y; y++, yi++) {
        for(int x = global_x, xi = 0; xi < visible_nodes_x; x++, xi++) {
            Node n = get_node(x, y, seed);

            Point d = get_node_displacement(x, y, seed);
            
            int x_pos = visual_x + (xi * x_spacing) + d.x;
            int y_pos = visual_y + (yi * y_spacing) + d.y;

            Point d_left = get_node_displacement(x - 1, y, seed);
            int x_left = x_pos - x_spacing - d.x + d_left.x;
            int y_left = y_pos - d.y + d_left.y;
            draw_g_line_RGBA(x_pos, y_pos, x_left, y_left, 255, 255, 255, 255);

            // Point d_left = get_node_displacement(x - 1, y, seed);
            // int x_left = x_pos - x_spacing - d.x + d_left.x;
            // int y_left = y_pos - d.y + d_left.y;
            // draw_g_line_RGBA(x_pos, y_pos, x_left, y_left, 255, 255, 255, 255);

            // Point d_right = get_node_displacement(x + 1, y, seed);
            // int x_right = x_pos + x_spacing + d.x + d_right.x;
            // draw_g_line_RGBA(x_pos, y_pos, x_right, y_pos, 255, 255, 255, 255);
            
            Point d_top = get_node_displacement(x, y - 1, seed);
            int x_top = x_pos - d.x + d_top.x;
            int y_top = y_pos - y_spacing - d.y + d_top.y;
            draw_g_line_RGBA(x_pos, y_pos, x_top, y_top, 255, 255, 255, 255);

            // Point d_top = get_node_displacement(x, y - 1, seed);
            // int y_top = y_pos + y_spacing + d.y + d_top.y;
            // draw_g_line_RGBA(x_pos, y_pos, x_pos, y_top, 255, 255, 255, 255);
            
            // Point d_bottom = get_node_displacement(x, y - 1, seed);
            // int y_bottom = y_pos + y_spacing + d.y + d_bottom.y;
            // draw_g_line_RGBA(x_pos, y_pos, x_pos, y_bottom, 255, 255, 255, 255);

            // Draws centered on x,y
            draw_g_circle_filled_color(x_pos, y_pos, 8, n.color);
        }   
    }

    // for(auto &node: _nodes) {
        

    //     if(node.connections.right) {
            

    //         int right_index = vis_y * 10 + (vis_x + 1);
    //         if(right_index >= 0 && right_index < _nodes.size()) {
    //             auto &right_node = _nodes[right_index];

    //             int r_x = (int)right_node.position.value.x;
    //             int r_y = (int)right_node.position.value.y;
    //             int r_vis_x = (r_x % 10);
    //             int r_vis_y = (r_y % 10);
    //             int r_x_pos = visual_x + (r_vis_x * 50);
    //             int r_y_pos = visual_y + (r_vis_y * 30);


    //             draw_g_line_RGBA(x_pos, y_pos, r_x_pos, r_y_pos, 255, 0, 0, 255);
    //         }
    //     }

    //}

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