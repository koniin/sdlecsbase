#include "map_scene.h"
#include "game_input_wrapper.h"
#include "services.h"
#include "components.h"
#include "engine.h"
#include "renderer.h"
#include "map_node.h"
#include "fleet_ui.h"

#include <chrono>

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

    node.type = id;

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

struct MapNavigation {
    Vector2 camera_pos;
    const int distance_to_next_node = 128;
    const float camera_gutter = 128.0f;
    float camera_y_speed = 0;
    float camera_x_speed = 0;
    std::vector<Node> _nodes;

    bool _navigation_enabled = true;

    void init() {
        _nodes.reserve(200);
    }

    void begin() {
        Maze *maze = &Services::game_state()->maze;
        camera_set_clamp_area(
            -camera_gutter, 
            (float)(maze->cols * distance_to_next_node - gw + camera_gutter), 
            -camera_gutter, 
            (float)(maze->rows * distance_to_next_node - gh + camera_gutter)
        );
        
        camera_pos = Vector2::from_i(Services::game_state()->current_node.x * distance_to_next_node, Services::game_state()->current_node.y * distance_to_next_node);
        camera_lookat(camera_pos);
        camera_set_speed(0.8f);
    }

    void disable_navigation() {
        _navigation_enabled = false;
    }

    void enable_navigation() {
        _navigation_enabled = true;
    }

    void node_interact_handler(Node &n) {
        if(Services::node_event_manager()->screens.size() > 0)
            return;

        Point p;
        Input::mouse_current(p);
        if(Intersects::circle_contains_point(n.render_position.to_vector2(), (float)n.radius, p.to_vector2())) {
            auto next_node = Point(n.maze_pos.x, n.maze_pos.y);
            Maze *maze = &Services::game_state()->maze;
            auto &current_node = Services::game_state()->current_node;
            if(maze_connection_is_open(maze, current_node, next_node)) {
                n.color = Colors::white;
                n.radius = 16;
                if(Input::mouse_left_down) {
                    camera_pos = n.maze_pos.to_vector2() * (float)distance_to_next_node;

                    if(!Services::game_state()->is_visited(next_node)) {
                        Services::game_state()->set_current_node(next_node);
                        
                        Services::node_event_manager()->start_event(n);
                    }
                }
            } else {
                n.color = Colors::white;
                if(Input::mouse_left_down) {
                    Engine::logn("Show node stats or sumtin");
                }
            }
        }
    }

    void update() {
        if(!_navigation_enabled) {
            return;
        }

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

        static const float max_camera_speed = 30;

        camera_y_speed = Math::clamp_f(camera_y_speed, -max_camera_speed, max_camera_speed);
        camera_x_speed = Math::clamp_f(camera_x_speed, -max_camera_speed, max_camera_speed);
        
        camera_pos.y += camera_y_speed;
        camera_pos.x += camera_x_speed;

        Maze *maze = &Services::game_state()->maze;
        camera_pos.y = Math::clamp_f(camera_pos.y, -camera_gutter, (float)((maze->cols) * distance_to_next_node) + camera_gutter);
        camera_pos.x = Math::clamp_f(camera_pos.x, -camera_gutter, (float)((maze->rows) * distance_to_next_node) + camera_gutter);
        
        camera_follow(camera_pos);

        camera_y_speed *= 0.5f;
        camera_x_speed *= 0.5f;

        int seed = Services::game_state()->seed;
        auto camera = get_camera();
        
        int startCol = (int)Math::floor_f(camera.x / distance_to_next_node);
        int endCol = startCol + (gw / distance_to_next_node) + 1;
        int startRow = (int)Math::floor_f(camera.y / distance_to_next_node);
        int endRow = startRow + (gh / distance_to_next_node) + 2;

        startRow = Math::clamp_i(startRow, 0, maze->rows - 1);
        startCol = Math::clamp_i(startCol, 0, maze->cols - 1);
        endRow = Math::clamp_i(endRow, 0, maze->rows - 1);
        endCol = Math::clamp_i(endCol, 0, maze->cols - 1);

        auto offsetX = -camera.x + startCol * distance_to_next_node;
        auto offsetY = -camera.y + startRow * distance_to_next_node;
        
        auto &current_node = Services::game_state()->current_node;

        _nodes.clear();
        for (auto c = startCol; c <= endCol; c++) {
            for (auto r = startRow; r <= endRow; r++) {
                auto x = (c - startCol) * distance_to_next_node + offsetX;
                auto y = (r - startRow) * distance_to_next_node + offsetY;

                Point d = get_node_displacement(c, r, seed);
                x += d.x;
                y += d.y;

                Node n = get_node(c, r, seed);
                n.render_position.x = (int)x;
                n.render_position.y = (int)y;
                n.radius = 8;
                n.current = c == current_node.x && r == current_node.y;
                n.maze_pos.x = c;
                n.maze_pos.y = r;
                
                node_interact_handler(n);

                auto cell = maze->cell(c, r);
                
                if ((cell.Openings & Directions::West) == Directions::West) {
                    auto x_left = (c - 1 - startCol) * distance_to_next_node + offsetX;
                    Point d_left = get_node_displacement(c - 1, r, seed);
                    x_left += d_left.x;
                    float y_left = y - d.y + d_left.y;
                    n.neighbour_left.x = (int)x_left;
                    n.neighbour_left.y = (int)y_left;

                    n.connections.left = true;
                }

                if ((cell.Openings & Directions::East) == Directions::East) {
                //     auto x_right = (c + 1 - startCol) * distance_to_next_node + offsetX;
                //     Point d_right = get_node_displacement(c + 1, r, seed);
                //     x_right += d_right.x;
                //     int y_right = y - d.y + d_right.y;
                //     n.neighbour_right.x = x_right;
                //     n.neighbour_right.y = y_right;

                    n.connections.right = true;
                }
                
                if ((cell.Openings & Directions::North) == Directions::North) {
                    auto y_top = (r - 1 - startRow) * distance_to_next_node + offsetY;
                    Point d_top = get_node_displacement(c, r - 1, seed);
                    y_top += d_top.y;
                    float x_top = x - d.x + d_top.x;
                    n.neighbour_top.x = (int)x_top;
                    n.neighbour_top.y = (int)y_top;

                    n.connections.top = true;
                }

                if ((cell.Openings & Directions::South) == Directions::South) {
                //     auto y_bottom = (r + 1 - startRow) * distance_to_next_node + offsetY;
                //     Point d_bottom = get_node_displacement(c, r + 1, seed);
                //     y_bottom += d_bottom.y;
                //     int x_bottom = x - d.x + d_bottom.x;
                //     n.neighbour_bottom.x = x_bottom;
                //     n.neighbour_bottom.y = y_bottom;

                    n.connections.bottom = true;
                }


                _nodes.push_back(n);
            }
        }
    }

    void render() {
        for(auto &n : _nodes) {
            if(n.connections.left) {
                draw_g_line_RGBA(n.render_position.x, n.render_position.y, n.neighbour_left.x, n.neighbour_left.y, 255, 255, 255, 255);
            }
            if(n.connections.top) {
                draw_g_line_RGBA(n.render_position.x, n.render_position.y, n.neighbour_top.x, n.neighbour_top.y, 255, 255, 255, 255);
            }
            // if(n.connections.right) {
            //     draw_g_line_RGBA(n.position.x, n.position.y, n.neighbour_right.x, n.neighbour_right.y, 255, 255, 255, 255);
            // }
            // if(n.connections.bottom) {
            //     draw_g_line_RGBA(n.position.x, n.position.y, n.neighbour_bottom.x, n.neighbour_bottom.y, 255, 255, 255, 255);
            // }
            if(n.current) {
                static SDL_Color color = Colors::yellow;
                draw_g_circle_color(n.render_position.x, n.render_position.y, n.radius + 8, color);    
            }
            draw_g_circle_filled_color(n.render_position.x, n.render_position.y, n.radius, n.color);
        }
    }
};

MapNavigation map_navigator;

void show_research(std::function<void(void)> on_close) {
    Services::ui()->add_state("research_ui");

	Button research_btn = Button(gw / 2, gh / 2, "RESEARCH");
    research_btn.on_click = [&]() { 
        Engine::logn("RESEARCH CLICKED");
    };
	Services::ui()->add_element(research_btn, "research_ui");
    
    Button fleet_button = Button(gw / 2, gh - 20, "CLOSE");
    fleet_button.on_click = on_close;
	Services::ui()->add_element(fleet_button, "research_ui");
}

void on_close_research() {
    map_navigator.enable_navigation();
    Services::ui()->remove_state("research_ui");
}

void MapScene::initialize() {
    Engine::logn("[MAP] Init");
 	render_buffer.init(2048);
    // Resources::sprite_sheet_load("combat_sprites", "combat_sprites.data");
    
    Resources::sprite_load("background", "bkg1.png");

    map_navigator.init();
}

void on_close_fleet_ui() {
    fleet_ui_hide();
    map_navigator.enable_navigation();
}

void MapScene::begin() {
	Engine::logn("[MAP] Begin");
    Engine::logn("seed: %d",  Services::game_state()->seed);
    Noise::set_seed(Services::game_state()->seed);
    //not_random_generator = std::mt19937(Services::game_state()->seed);

    map_navigator.begin();

    Services::ui()->add_state("map_nav");

	Button fleet_button = Button(200, gh - 20, "Fleet");
    fleet_button.on_click = [&]() { 
        on_close_research();
        map_navigator.disable_navigation();
        fleet_ui_show(on_close_fleet_ui);
    };
	Services::ui()->add_element(fleet_button, "map_nav");

    Button research_button = Button(420, gh - 20, "Research");
    research_button.on_click = [&]() {
        on_close_fleet_ui();
        map_navigator.disable_navigation();
        show_research(on_close_research);
    };
	Services::ui()->add_element(research_button, "map_nav");
}

void MapScene::end() {
    Services::ui()->clear();
    camera_reset_clamp_area();
    Engine::logn("[MAP] End");
	render_buffer.clear();
    camera_lookat(Vector2::from_i(gw / 2, gh / 2));
}

void MapScene::update() {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	
    map_navigator.update();
    Services::node_event_manager()->update();

    Services::events()->emit();
    Services::ui()->update();
    
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
    
    draw_sprite(Resources::sprite_get("background"), 0, 0);
    
    map_navigator.render();
    Services::node_event_manager()->render();

    int population = Services::game_state()->population;
    int resources = Services::game_state()->resources;
    std::string info_text = "Population: " + std::to_string(population) + " |  Resources: " + std::to_string(resources);
    draw_text_centered_str((int)(gw / 2), 10, Colors::white, info_text);

	Services::ui()->render();
    renderer_draw_render_target_camera();
	renderer_flip();
}

void MapScene::unload() {
	Engine::logn("[MAP] Unload");
}