#include "map_scene.h"
#include "game_input_wrapper.h"
#include "services.h"
#include "components.h"
#include "engine.h"
#include "renderer.h"

#include <chrono>

struct Node {
    Vector2 position;
    SDL_Color color;
    int radius;
    struct Connections {
        bool top = false;
        bool bottom = false;
        bool left = false;
        bool right = false;
    } connections;
};

std::vector<Node> _nodes;

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

std::mt19937 _id_displacement;
Point get_node_displacement(int node_x, int node_y, int seed) {
    uint32_t r_seed = (uint32_t)(pseudo_rand_zero_to_one(node_x + seed, node_y + seed) * 3450971324.f);
    _id_displacement = std::mt19937(r_seed);

    Point p(RNG::range_i(-20, 20, _id_displacement), RNG::range_i(-20, 20, _id_displacement));
    return p;
}

Vector2 camera_pos(0, 0);

void MapScene::initialize() {
    Engine::logn("[MAP] Init");
 	render_buffer.init(2048);
    // Resources::sprite_sheet_load("combat_sprites", "combat_sprites.data");
    
    Resources::sprite_load("background", "bkg1.png");

    _nodes.reserve(200);
}

const int distance_to_next_node = 128;
const float camera_gutter = 32.0f;

void MapScene::begin() {
	Engine::logn("[MAP] Begin");
    Engine::logn("seed: %d",  Services::game_state()->seed);
    Noise::set_seed(Services::game_state()->seed);
    not_random_generator = std::mt19937(Services::game_state()->seed);

    Maze *maze = &Services::game_state()->maze;
    
    camera_set_clamp_area(
        -camera_gutter, 
        (float)(maze->cols * distance_to_next_node - gw + camera_gutter), 
        -camera_gutter, 
        (float)(maze->rows * distance_to_next_node - gh + camera_gutter)
    );
    
    camera_pos = Vector2(maze->cols * distance_to_next_node / 2, maze->rows * distance_to_next_node / 2);
    camera_lookat(camera_pos);
    camera_set_speed(0.8f);
}

void MapScene::end() {
    camera_reset_clamp_area();
    Engine::logn("[MAP] End");
	render_buffer.clear();
    camera_lookat(Vector2((gw / 2), (gh / 2)));
}

float camera_y_speed = 0;
float camera_x_speed = 0;

void MapScene::update() {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	
    if(GInput::pressed(GInput::Action::Start)) {
	    Scenes::set_scene("level");
	}

    // if(Input::key_pressed(SDLK_LEFT)) {
    //     global_x -= 1;
    // }
    // if(Input::key_pressed(SDLK_RIGHT)) {
    //     global_x += 1;
    // }
    // if(Input::key_down(SDL_SCANCODE_RIGHT)) {
    //     // start.x += 12;
    //     global_x += 1;
    // }

    if(Input::key_down(SDL_SCANCODE_UP)) {
        camera_y_speed -= 15;
    } 
    if(Input::key_down(SDL_SCANCODE_DOWN)) {
        camera_y_speed += 15;
    } 
    if(Input::key_down(SDL_SCANCODE_LEFT)) {
        camera_x_speed -= 15;
    } 
    if(Input::key_down(SDL_SCANCODE_RIGHT)) {
        camera_x_speed += 15;
    } 

    static const int max_camera_speed = 30;

    camera_y_speed = Math::clamp_i(camera_y_speed, -max_camera_speed, max_camera_speed);
    camera_x_speed = Math::clamp_i(camera_x_speed, -max_camera_speed, max_camera_speed);
    
    camera_pos.y += camera_y_speed;
    camera_pos.x += camera_x_speed;

    Maze *maze = &Services::game_state()->maze;
    camera_pos.y = Math::clamp_f(camera_pos.y, -camera_gutter, (float)((maze->cols) * distance_to_next_node + camera_gutter));
    camera_pos.x = Math::clamp_f(camera_pos.x, -camera_gutter, (float)((maze->rows) * distance_to_next_node + camera_gutter));
    
    camera_follow(camera_pos);

    camera_y_speed *= 0.5f;
    camera_x_speed *= 0.5f;

    int seed = Services::game_state()->seed;
    auto camera = get_camera();
    
    auto startCol = Math::floor_f(camera.x / distance_to_next_node);
    auto endCol = startCol + (gw / distance_to_next_node);
    auto startRow = Math::floor_f(camera.y / distance_to_next_node);
    auto endRow = startRow + (gh / distance_to_next_node) + 1;

    auto offsetX = -camera.x + startCol * distance_to_next_node;
    auto offsetY = -camera.y + startRow * distance_to_next_node;
    
    _nodes.clear();
    for (auto c = startCol; c <= endCol; c++) {
        for (auto r = startRow; r <= endRow; r++) {
            auto x = (c - startCol) * distance_to_next_node + offsetX;
            auto y = (r - startRow) * distance_to_next_node + offsetY;

            Point d = get_node_displacement(c, r, seed);
            x += d.x;
            y += d.y;

            SDL_Color color = Colors::green;
            if(c == 0 || r == 0) {
                color = Colors::white;
            } else if(c == maze->cols || r == maze->rows) {
                color = Colors::red;
            }

            Node n = get_node(c, r, seed);
            n.position.x = x;
            n.position.y = y;
            n.radius = 8;
            _nodes.push_back(n);
        }
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

/*
static void drawOpenings(Tilemap &t, const int startX, const int startY, const Directions &dir, 
							const int roomX, const int roomY) {
	static GPU_Rect bottom { 0, 0, 32, 32 };
	static GPU_Rect right { 32, 0, 32, 32 };
	static GPU_Rect left { 64, 0, 32, 32 };
	static GPU_Rect top { 96, 0, 32, 32 };

	auto temp_connections = Resources::getSprite("connections");
	
	if ((dir & Directions::West) == Directions::West) {
		draw_sprite(t.tile_sheet, &t.sprites[19], startX + (t.sprite_size / 2), startY + (t.rows*t.sprite_size / 2));
		if(!room_is_visited(roomX - 1, roomY))
			draw_sprite(temp_connections, &left, (int)(startX - (left.w / 2)), (int)(startY + (t.rows*t.sprite_size / 2)));
	}
	if ((dir & Directions::East) == Directions::East) {
		draw_sprite(t.tile_sheet, &t.sprites[20], startX + (t.cols * t.sprite_size - (t.sprite_size / 2)), startY + (t.rows*t.sprite_size / 2));
		if(!room_is_visited(roomX + 1, roomY))
			draw_sprite(temp_connections, &right, (int)(startX + (t.cols*t.sprite_size) + (right.w / 2)), (int)(startY + (t.rows*t.sprite_size / 2)));
	}
	if ((dir & Directions::North) == Directions::North) {
		draw_sprite(t.tile_sheet, &t.sprites[18], startX + (t.cols * t.sprite_size / 2), startY + (t.sprite_size / 2));
		if(!room_is_visited(roomX, roomY - 1))
			draw_sprite(temp_connections, &top, (int)(startX + (t.cols*t.sprite_size / 2)), (int)(startY - (top.h / 2)));
	}
	if ((dir & Directions::South) == Directions::South) {
		draw_sprite(t.tile_sheet, &t.sprites[21], (int)(startX + (t.cols * t.sprite_size / 2)), (int)(startY + (t.rows * t.sprite_size - (t.sprite_size / 2))));
		if(!room_is_visited(roomX, roomY + 1))
			draw_sprite(temp_connections, &bottom, (int)(startX + (t.cols*t.sprite_size / 2)), (int)(startY + (t.rows*t.sprite_size) + (bottom.h / 2)));
	}
}

static void draw_room(const Maze* maze, const int &room_x, const int &room_y, const int &render_x, const int &render_y) {
	// auto t = getRoomTileMap(x, y);
	drawTilemap(testTileMap, render_x + 8, render_y + 8);

	if(room_is_exit(room_x, room_y)) {
		draw_sprite(Resources::getSprite("portal"), NULL, render_x + (roomSize / 2), render_y + (roomSize / 2));
		Lights::light("portallight", render_x + (roomSize / 2), render_y + (roomSize / 2));
	}
	if (room_is_exit(room_x, room_y) || room_is_visited(room_x, room_y)) {
		drawOpenings(testTileMap, render_x, render_y, maze->buffer[maze->index(room_x, room_y)].Openings, room_x, room_y);
	}
}

void MazeScene::drawMaze(Maze* maze) {
	for (int y = 0; y < maze->rows; y++) {
        for (int x = 0; x < maze->cols; x++){
			if (room_is_exit(x, y) || room_is_visited(x, y) || game_state.room_current_index == (int)room_index(x, y)) {
				int render_x = mazeStartX + x * roomCenterDistance;
				int render_y = mazeStartY + y * roomCenterDistance;
				draw_room(maze, x, y, render_x, render_y);
			}
        }
    }
}
*/

void MapScene::render() {
	renderer_clear();
    
    draw_sprite(Resources::sprite_get("background"), 0, 0);
    
    for(auto &n : _nodes) {
        draw_g_circle_filled_color(n.position.x, n.position.y, n.radius, n.color);
    }

    // for (int y = 0; y < maze->rows; y++) {
    //     for (int x = 0; x < maze->cols; x++){
    // //         auto y = (r - startRow) * x_spacing + offsetY;
    // //         Point d = get_node_displacement(c, r, seed);
    // //         x += d.x;
    // //         y += d.y;

    // //         Node n = get_node(c, r, seed);
	// 		// if (room_is_exit(x, y) || room_is_visited(x, y) || game_state.room_current_index == (int)room_index(x, y)) {
	// 		// 	int render_x = mazeStartX + x * roomCenterDistance;
	// 		// 	int render_y = mazeStartY + y * roomCenterDistance;
	// 		// 	draw_room(maze, x, y, render_x, render_y);
	// 		// }


    //     }
    // }

    // auto startCol = Math::floor_f(camera.x / x_spacing);
    // auto endCol = startCol + (gw / x_spacing) + 1;
    // auto startRow = Math::floor_f(camera.y / x_spacing);
    // auto endRow = startRow + (gh / x_spacing) + 1;

    // auto offsetX = -camera.x + startCol * x_spacing;
    // auto offsetY = -camera.y + startRow * x_spacing;

    // for (auto c = startCol; c <= endCol; c++) {
    //     for (auto r = startRow; r <= endRow; r++) {
    //         auto x = (c - startCol) * x_spacing + offsetX;
    //         auto y = (r - startRow) * x_spacing + offsetY;
    //         Point d = get_node_displacement(c, r, seed);
    //         x += d.x;
    //         y += d.y;

    //         Node n = get_node(c, r, seed);
    //         draw_g_circle_filled_color(x, y, 8, n.color);

    //         auto x_left = (c - 1 - startCol) * x_spacing + offsetX;
    //         Point d_left = get_node_displacement(c - 1, r, seed);
    //         x_left += d_left.x;
    //         int y_left = y - d.y + d_left.y;
    //         draw_g_line_RGBA(x, y, x_left, y_left, 255, 255, 255, 255);
            
    //         auto y_top = (r - 1 - startRow) * x_spacing + offsetY;
    //         Point d_top = get_node_displacement(c, r - 1, seed);
    //         y_top += d_top.y;
    //         int x_top = x - d.x + d_top.x;
    //         draw_g_line_RGBA(x, y, x_top, y_top, 255, 255, 255, 255);

    //         if(r == endRow) {
    //             auto y_bottom = (r + 1 - startRow) * x_spacing + offsetY;
    //             Point d_bottom = get_node_displacement(c, r + 1, seed);
    //             y_top += d_top.y;
    //             int x_bottom = x - d.x + d_bottom.x;
    //             draw_g_line_RGBA(x, y, x_bottom, y_bottom, 255, 255, 255, 255);
    //         }
    //     }
    // }

    //draw_buffer(render_buffer);

    int population = Services::game_state()->population;
    std::string population_text = "Population: " + std::to_string(population);
    draw_text_centered_str((int)(gw / 2), 10, Colors::white, population_text);

    draw_text_str(10, (int)(gh - 10), Colors::white, "Select a node to continue.. (just press start.)");
    //Particles::render_circles_filled(GameController::particles);
	Services::ui().render();
    renderer_draw_render_target_camera();
	renderer_flip();
}

void MapScene::unload() {
	Engine::logn("[MAP] Unload");
}