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

/*
    int[] nodeQueue = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3 };

	node = nodeQueue[(int)(rand((UInt32)i, (UInt32)j) * (float)nodeQueue.Length)]



*/

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

const int node_distribution_count = 17;
const int nodes_distribution[node_distribution_count] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3 };

int get_node_id(uint32_t x, uint32_t y) {
    return nodes_distribution[(int)(pseudo_rand_zero_to_one(x, y) * node_distribution_count)];
}

/*
void Main()
{
	Perlin p = new Perlin();
	
	List<int> nodes = new List<int>();
	
	int x_start = 100;
	int y_start = 100;
	int number_of_nodes_x = 10;
	int number_of_nodes_y = 10;
	
	int[] nodeQueue = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3 };
	
	int[] table = new int[255];
	int q_index = 0;
	for(int i = 0; i < 255; i++) {
		table[i] = nodeQueue[q_index++];
		if (q_index >= nodeQueue.Length)
		{
			q_index = 0;
		}
	}

	int a = 179426549;
	int b = 179426111;
	
	int c = 23987;
	int d = 1234;

	for (float i = x_start; i < x_start + number_of_nodes_x; i++)
	{
		for (float j = y_start; j < y_start + number_of_nodes_y; j++)
		{
			int x = (int)i;
			int z = (int)j;
			
			int val = (a * x + b * z + c) ^ d;
			int vall = Math.Abs(val);
			//Console.WriteLine(vall);
			
			
			Console.WriteLine( p.Nodes[p.p[vall % 255]]);
			
			//int val = nodeQueue.Length * (int)j + (int)i;
			//Console.WriteLine($"{val} => {val % 255} => { table[val % 255] }");
			
			
//			float x = i / 100;
//			float y = j / 100;
//			
//			int xi = (int)x & 255;                              // Calculate the "unit cube" that the point asked will be located in
//			int yi = (int)y & 255;                              // The left bound is ( |_x_|,|_y_|,|_z_| ) and the right bound is that
//
//			float xf = x - (int)x;
//			float yf = y - (int)y;
//			//Console.WriteLine($"{xi} {yi} - {xf} , {yf}   -    {(int)(xf * 255)} , {(int)(yf * 255)}");
//			
//			int x_index = Math.Abs((int)(xf * 255));
//			int y_index = Math.Abs((int)(yf * 255));
//			nodes.Add(p.Get(x_index, y_index));
		}
	}
	foreach(var node in nodes) {
		Console.WriteLine(node);
	}
}


class Perlin
{
	static readonly int[] permutation = { 151,160,137,91,90,15,                 // Hash lookup table as defined by Ken Perlin.  This is a randomly
	    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,    // arranged array of all numbers from 0-255 inclusive.
	    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
		88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
		77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
		102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
		135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
		223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};
	public readonly int[] p;                                                    // Doubled permutation to avoid overflow
	
	public int[] Nodes;
	
	public int Get(int x, int y) {
	 	//return Nodes[p[p[x] + y]];
		return Nodes[p[p[x] + p[y]]];
	}
	
	public Perlin() {
		int[] nodeQueue = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3 };
		Random rnd = new Random();
		nodeQueue = nodeQueue.OrderBy(x => rnd.Next()).ToArray();
		
		Nodes = new int[512];
		int q_index = 0;
		for (int x = 0; x < 512; x++) {
			Nodes[x] = nodeQueue[q_index++];
			if(q_index >= nodeQueue.Length) {
				q_index = 0;
			}
		}
		
		p = new int[512];
		for (int x = 0; x < 512; x++)
		{
			p[x] = permutation[x % 256];
		}
	}
}


*/

float scaleRange(float number, float high, float low)
{
  return (high + low) * 0.5f;
}

Node get_node(int x, int y, int seed) {
    Node node;
    
    int id = get_node_id(x + seed, y + seed);
    //int n = pseudo_rand_zero_to_one(x, y);

    if(id == 1) {
        node.color = { 65, 120, 200, 255 };
    } else if(id == 2) {
        node.color = { 255, 0, 0, 255 };
    } else if(id == 3) {
        node.color = { 255, 255, 0, 255 };
    } else {
        ASSERT_WITH_MSG(false, "WTF?!");
    }

    // //n = RNG::zero_to_one(not_random_generator);
    // if(n < 0) {
    //     n = -n;
    // }

    // if(n <= 0.1f) {
    //     node.color = { 65, 120, 200, 255 };
    // } else if(n <= 0.2f) {
    //     node.color = { 255, 0, 0, 255 };
    // } else if(n <= 0.3f) {
    //     node.color = { 255, 255, 0, 255 };
    // } else if(n <= 0.4f) {
    //     node.color = { 255, 0, 255, 255 };
    // } else if(n <= 0.5f) {
    //     node.color = { 0, 255, 0, 255 };
    // } else if(n <= 0.6f) {
    //     node.color = { 0, 255, 255, 255 };
    // } else if(n <= 0.7f) {
    //     node.color = { 0, 0, 255, 255 };
    // } else if(n <= 0.8f) {
    //     node.color = { 125, 125, 255, 255 };
    // } else if(n <= 0.9f) {
    //     node.color = { 125, 255, 125, 255 };
    // } else {
    //     node.color = { 255, 125, 125, 255 };
    // }

    return node;
}

void make_nodes() {
    int seed = Services::game_state()->seed;
    int global_x = 4000;
    int global_y = 4000;

    int visual_x = 20;
    int visual_y = 20;
    for(int y = global_y, vis_y = 0; vis_y < 10; y++, vis_y++) {
        for(int x = global_x, vis_x = 0; vis_x < 10; x++, vis_x++) {
            Node n = get_node(x, y, seed);
            n.position = Vector2((float)(visual_x + (vis_x * 30)), (float)(visual_y + (vis_y * 20)));
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
        draw_g_circle_filled_color((int)node.position.value.x, (int)node.position.value.y, 8, node.color);
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