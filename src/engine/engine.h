#ifndef ENGINE_H
#define ENGINE_H

#include "SDL.h"
#include <algorithm>
#include <vector>
#include <random>
#include <map>
#include <functional>
#include <iostream>
#include <sstream>
#include <queue>
#include <memory>
#include <unordered_map>
#include <stack>
#include <bitset>

#ifdef _DEBUG
#define ASSERT_WITH_MSG(cond, msg) do \
{ if (!(cond)) { std::ostringstream str; str << msg; std::cerr << str.str(); std::abort(); } \
} while(0)
#else 
#define ASSERT_WITH_MSG(cond, msg) ;
#endif

// timer
typedef struct {
    Uint64 now;
    Uint64 last;
    double dt;
    double fixed_dt;
    double accumulator;
} gameTimer;


// Find functions here if you need them
// https://github.com/nicolausYes/easing-functions/blob/master/src/easing.cpp

inline float easing_linear(float t) {
    return t;
}

inline float easing_InSine(float t) {
	return sinf( 1.5707963f * t );
}

inline float easing_OutSine(float t) {
	return 1 + sinf( 1.5707963f * (--t) );
}

inline float easing_sine_in_out(float t) {
	return 0.5f * (1 + sinf( 3.1415926f * (t - 0.5f) ) );
}

typedef float (*easing_t)(float);

namespace Engine {
	extern int32_t current_fps;
	static bool _is_running = true;

	void init();

	inline void exit() {
		_is_running = false;
	}
	inline bool is_running() {
		return _is_running;
	}
	bool is_paused();
	void toggle_logging();
	void log(const char* fmt, ...);
	void logn(const char* fmt, ...);

	void set_base_data_folder(const std::string &name);
	inline std::string get_base_data_folder();

	void pause(float time);

	void update();
	void render();

	void cleanup();
}

struct Point;
namespace FrameLog {
	const bool &is_enabled();
	void log(const std::string &message);
    void enable_at(const int x, const int y);
    void disable();
	const std::vector<std::string> &get_messages();
	const Point &get_position();
}

namespace Text {
	inline std::string format(const std::string format, ...) {
        va_list args;
        va_start (args, format);
        size_t len = std::vsnprintf(NULL, 0, format.c_str(), args);
        va_end (args);
        std::vector<char> vec(len + 1);
        va_start (args, format);
        std::vsnprintf(&vec[0], len + 1, format.c_str(), args);
        va_end (args);
        return &vec[0];
    }
};

struct Vector2;
struct Point {
	int x;
	int y;
	Point() { x = 0; y = 0; };
	Point(int xPos, int yPos) {
		x = xPos;
		y = yPos;
	}
	
	template<class T>
	inline Point operator=(const T &v) const {
		return Point((int)v.x, (int)v.y);
	}
	inline Point operator-(Point const& point) const {
		return Point(x - point.x, y - point.y);
	}
	inline Point Point::operator*(int const &num) const {
		return Point(x * num, y * num);
	}
	
	Vector2 to_vector2() const;
};
inline Point operator+( Point const& lhs, Point const& rhs ) {
	return Point(lhs.x + rhs.x, lhs.y + rhs.y);
}
inline bool operator==( Point const& lhs, Point const& rhs ) {
	return lhs.x == rhs.x && lhs.y == rhs.y;
}
inline Point operator+=(Point const& lhs, int const& rhs){
	return Point(lhs.x + rhs, lhs.y + rhs);
}
inline Point operator-=(Point const& lhs, int const& rhs){
	return Point(lhs.x - rhs, lhs.y - rhs);
}
inline const Point operator*(int lhs, Point const &rhs) {
	Point result;
	result.x=rhs.x * lhs;
	result.y=rhs.y * lhs;
	return result;
}

struct Vector2 {
	float x;
	float y;
	Vector2() { x = 0; y = 0; };
	explicit Vector2(float xPos, float yPos) {
		x = xPos;
		y = yPos;
	}
	
	static const Vector2 Zero;
	static const Vector2 One;
	inline Vector2 Vector2::operator-() const {
		return Vector2(-x, -y);
   	}
	inline Vector2& Vector2::operator+=(const Vector2& vector) {
		x += vector.x;
		y += vector.y;
		return *this;
	}
	inline Vector2& Vector2::operator-=(const Vector2& vector) {
		x -= vector.x;
		y -= vector.y;
		return *this;
	}
	inline Vector2& Vector2::operator*=(const Vector2& vector) {
		x *= vector.x;
		y *= vector.y;
		return *this;
	}
	inline Vector2& Vector2::operator/=(const Vector2& vector) {
		x /= vector.x;
		y /= vector.y;
		return *this;
	}
	inline Vector2 Vector2::operator-(const Vector2& vector) const {
			return Vector2(x - vector.x, y - vector.y);
	}
	inline Vector2 Vector2::operator*(const Vector2& vector) const {
			return Vector2(x * vector.x, y * vector.y);
	}
	inline Vector2 Vector2::operator-(float const &num) const {
		return Vector2(x - num, y - num);
	}
	inline Vector2 Vector2::operator+(float const &num) const {
		return Vector2(x + num, y + num);
	}
	inline Vector2 Vector2::operator*(float const &num) const {
		return Vector2(x * num, y * num);
	}
	inline Vector2 Vector2::operator/(float const &num) const {
		return Vector2(x / num, y / num);
	}
	inline Vector2& Vector2::operator+=(const float &num) {
		x += num;
		y += num;
		return *this;
	}
	inline Vector2& Vector2::operator-=(const float &num) {
		x -= num;
		y -= num;
		return *this;
	}
	inline Vector2& Vector2::operator*=(const float &num) {
		x *= num;
		y *= num;
		return *this;
	}
	inline Vector2& Vector2::operator/=(const float &num) {
		x /= num;
		y /= num;
		return *this;
	}
	inline bool Vector2::operator==(const Vector2& vector) const {
		return x == vector.x && y == vector.y;
	}
	inline bool Vector2::operator!=(const Vector2& vector) const {
		return x != vector.x || y != vector.y;
	}

	Point to_point() const;
	Vector2 normal() const;
	float length() const;
	float length2() const;
	float dot(const Vector2 &v) const;
};

inline Vector2 operator+( Vector2 const& lhs, Vector2 const& rhs ) {
	return Vector2(lhs.x + rhs.x, lhs.y + rhs.y);
}
inline const Vector2 operator*(float lhs, Vector2 const &rhs) {
	Vector2 result;
	result.x=rhs.x * lhs;
	result.y=rhs.y * lhs;
	return result;
}

struct Rectangle {
	int x;
	int y;
	int w;
	int h;

	Rectangle() {
		x = y = w = h = 0;
	}

	Rectangle(int xx, int yy, int ww, int hh) {
		x = xx;
		y = yy;
		w = ww;
		h = hh;
	}

	bool contains(int xi, int yi) {
		return ((((x <= xi) && (xi < (x + w))) && (y <= yi)) && (yi < (y + h)));
	}

	bool contains(const Point &p) {
		return contains(p.x, p.y);
	}

	int left() {
        return x;
    }

	int right() {
        return x + w;
    }

	int top() {
        return y;
    }

	int bottom() {
        return y + h;
    }

	bool intersects(Rectangle &r2) {
        return !(r2.left() > right()
                 || r2.right() < left()
                 || r2.top() > bottom()
                 || r2.bottom() < top()
                );
    }

	Vector2 center() {
		return Vector2((float)x + w/2, (float)y + h/2);
	}
};

namespace Time {
	extern float delta_time;
	extern float delta_time_fixed;
	extern float delta_time_raw;
}

namespace Input {
    extern int mousex;
	extern int mousey;
	extern bool mouse_left_down;

    void init();
    void update_states();
    void map(const SDL_Event *event);
    bool key_down(const SDL_Scancode &scanCode);
	bool key_down_k(const SDL_Keycode &keyCode);
    bool key_released(const SDL_Keycode &keyCode);
    bool key_pressed(const SDL_Keycode &keyCode);
}

struct Scene {
	// called when the scene is added to the engine, this is where to load resources etc
	virtual void initialize() = 0;
	// this is when the scene is activated (either first time or again)
	virtual void begin() = 0;
	// this is for cleanup after the scene is done
	virtual void end() = 0;
	// this is for cleanup on exiting the game or scene gets removed entirely
	virtual void unload() = 0;
	// Update the scene
	virtual void update() = 0;
	// Render the scene
	virtual void render() = 0;
};

namespace Scenes {
	// Add the scene to the engine
	void setup_scene(std::string scene_name, Scene* scene);
	// Set the scene that should be used next update
	void set_scene(std::string scene_name);
	// Set the scene that should be used next update
	void set_scene(Scene* scene);
	// Get the scene with specified handle
	Scene *get_scene(std::string scene_name);
	// Internal for changing to next scene
	void switch_scenes();
	// Get the current running scene
	Scene* get_current();

	void update();
	void render();
	void unload();
}

namespace Math {
	static const float Pi = 3.14159265358979323846f; // 180 degrees
	static constexpr float TwoPi = 3.14159265358979323846f * 2; // 360 degrees
	static constexpr float PiOver2 = 3.14159265358979323846f / 2.0f;
	static const float RAD_TO_DEGREE = 180.0f / (float)M_PI;
	static const float DEGREE_TO_RAD = (float)M_PI / 180.0f;
	
	inline int clamp_i(int x, int a, int b) {
    	x = std::max(x, a);
    	x = std::min(x, b);
    	return x;
	}

	inline float clamp_f(float x, float a, float b) {
    	x = std::fmax(x, a);
    	x = std::fmin(x, b);
    	return x;
	}

	inline int max_i(int a, int b) {
		return std::max(a, b);
	}

	inline float max_f(float a, float b) {
		return std::fmax(a, b);
	}

	inline int min_i(int a, int b) {
		return std::min(a, b);
	}

	inline float min_f(float a, float b) {
		return std::fmin(a, b);
	}

	inline float ceiling(float f) {
		return std::ceilf(f);
	}

	inline int abs(int i) {
		return std::abs(i);
	}
	
	inline float abs_f(float f) {
		return std::abs(f);
	}

	inline float cos_f(float f) {
		return std::cosf(f);
	}

	inline float sin_f(float f) {
		return std::sinf(f);
	}
	
	inline int sqrt(int i) {
		return (int)std::sqrt(i);
	}
	
	inline float sqrt_f(float f) {
		return std::sqrt(f);
	}

	inline float round(float f) {
		return std::round(f);
	}

	inline float round_bankers(float f) {
		if(f == 0.5f)
			return 0.0f;
		return std::round(f);
	}

	inline float lerp(float from, float to, float t) {
    	return from + (to - from) * t;
	}

	inline float length_vector_f(float x, float y) {
		return sqrt_f(x*x + y*y);
	}

	inline float interpolate(float from, float to, float amount, easing_t easing) {
    	return from + (to - from) * (easing(amount));
	}

	inline void interpolate_point(Point *target, const Point &from, const Point &to, const float amount, easing_t easing) {
		target->x = (int)(from.x + (to.x - from.x) * (easing(amount)));
		target->y = (int)(from.y + (to.y - from.y) * (easing(amount)));
	}

	inline void interpolate_vector(Vector2 *target, const Vector2 &from, const Vector2 &to, const float amount, easing_t easing) {
		target->x = from.x + (to.x - from.x) * (easing(amount));
		target->y = from.y + (to.y - from.y) * (easing(amount));
	}

	inline float distance_f(const float &x1, const float &y1, const float &x2, const float &y2) {
		auto dx = x1 - x2;
    	auto dy = y1 - y2;
		return sqrt_f(dx * dx + dy * dy);
	}

	inline float distance_v(const Vector2 &a, const Vector2 &b) {
		auto dx = a.x - b.x;
    	auto dy = a.y - b.y;
		return sqrt_f(dx * dx + dy * dy);
	}

	//Returns dot product
	inline const float dot_product(Vector2 const &lhs, Vector2 const &rhs) {
		return lhs.x*rhs.x+lhs.y*rhs.y; 
	}
	//Returns length squared (length2)
	inline const float length_squared(Vector2 const &rhs) {
		return dot_product(rhs, rhs);
	}
	//Returns magnitude (length)
	inline const float magnitude(Vector2 const &rhs) {
		//return sqrtf(dot(rhs, rhs));
		return sqrt_f(length_squared(rhs));
	}
	//Returns normalized Vector2
	inline Vector2 normalize(Vector2 const &lhs){
		return (1.f /(magnitude(lhs))) * lhs;
	}

	inline Vector2 direction(const Vector2 &first, const Vector2 &second) {
        Vector2 direction = first - second;
        if (direction == Vector2::Zero)
            return Vector2::Zero;
        else
            return normalize(direction);
	}

	inline Vector2 direction_from_angle(float angle) {
		float rotation = angle / Math::RAD_TO_DEGREE;
		Vector2 direction;
        direction.x = cos_f(rotation);
        direction.y = sin_f(rotation);
		return direction;
	}

	inline float angle_from_direction(const Vector2 &vector) {
        return (float)atan2(vector.y, vector.x) * RAD_TO_DEGREE;
	}

	inline float degrees_between_v(const Vector2 &a, const Vector2 &b) {
    	return atan2(b.y - a.y, b.x - a.x) * RAD_TO_DEGREE;
	}

	inline float rads_between_f(const float &x1, const float &y1, const float &x2, const float &y2) {
    	return atan2(y2 - y1, x2 - x1);
	}

	inline float rads_between_v(const Vector2 &a, const Vector2 &b) {
    	return atan2(b.y - a.y, b.x - a.x);
	}

	inline float rads_to_degrees(float rads) {
		return rads * RAD_TO_DEGREE;
	}

	inline Vector2 scale_to(const Vector2 &vector, const float &length) {
        return vector * (length / vector.length());
    }

	inline float IEEERemainder(float dividend, float divisor) {
		return dividend - (divisor * Math::round(dividend / divisor));
	}

	inline float wrap_angle(float angle) {
        angle = IEEERemainder(angle, 6.2831854820251465);
	    if (angle <= -3.14159274f) {
			angle += 6.28318548f;
	    }
	    else if (angle > 3.14159274f) {
			angle -= 6.28318548f;
	    }
	    return angle;
	}

	// You can define a vector as a radius and an angle - the polar coordinates.
	inline Vector2 polar_coords_to_vector(float angle, float magnitude) {
        return magnitude * Vector2(Math::cos_f(angle), Math::sin_f(angle));
    }

	inline bool intersect_circles(float c1X, float c1Y, float c1Radius, float c2X, float c2Y, float c2Radius) {
		float distanceX = c2X - c1X;
		float distanceY = c2Y - c1Y;     
		float magnitudeSquared = distanceX * distanceX + distanceY * distanceY;
		return magnitudeSquared < (c1Radius + c2Radius) * (c1Radius + c2Radius);
	}

	struct AABB {
		int left;
		int right;
		int bottom;
		int top;
	};

	struct AABB_f {
		float left;
		float right;
		float bottom;
		float top;
	};

	inline bool intersect_AABB(const Rectangle &a, const Rectangle &b) {
		AABB aa = { a.x, a.x + a.w, a.y, a.y + a.h };
		AABB bb = { b.x, b.x + b.w, b.y, b.y + b.h };
  		return (aa.left <= bb.right && aa.right >= bb.left) 
			&& (aa.bottom <= bb.top && aa.top >= bb.bottom);
	}

	inline bool intersect_circle_AABB(const float &cx, const float &cy, const float &radius, const Rectangle &rect) {
		AABB_f aa = { (float)rect.x, (float)rect.x + rect.w, (float)rect.y, rect.y + (float)rect.h };
		float delta_x = cx - Math::max_f(aa.left, Math::min_f(cx, aa.right));
		float delta_y = cy - Math::max_f(aa.bottom, Math::min_f(cy, aa.top));
		return (delta_x * delta_x + delta_y * delta_y) < (radius * radius);
	}

	inline bool intersect_lines_vector(const Vector2 &lineA1, const Vector2 &lineA2, const Vector2 &lineB1, const Vector2 &lineB2, Vector2 &collision_point) {
		float denom = ((lineB2.y - lineB1.y) * (lineA2.x - lineA1.x)) -
			((lineB2.x - lineB1.x) * (lineA2.y - lineA1.y));
		
		if (denom == 0) {
			return false;
		}
		
		float ua = (((lineB2.x - lineB1.x) * (lineA1.y - lineB1.y)) -
			((lineB2.y - lineB1.y) * (lineA1.x - lineB1.x))) / denom;
		/* The following 3 lines are only necessary if we are checking line
			segments instead of infinite-length lines */
		float ub = (((lineA2.x - lineA1.x) * (lineA1.y - lineB1.y)) -
			((lineA2.y - lineA1.y) * (lineA1.x - lineB1.x))) / denom;
		if ((ua < 0) || (ua > 1) || (ub < 0) || (ub > 1)) {
			return false;
		}

		collision_point = lineA1 + ua * (lineA2 - lineA1);
		return true;
	}
	// 	inline bool intersect_AABB(Rectangle &a, Rectangle &b) {
	//   		return (a.minX <= b.maxX && a.maxX >= b.minX) 
	// 			&& (a.minY <= b.maxY && a.maxY >= b.minY);
	// 	}

	// 	public static intersect(r1:Rectangle, r2:Rectangle):boolean {
	//         return !(r2.left > r1.right || 
	//            r2.right < r1.left || 
	//            r2.top > r1.bottom ||
	//            r2.bottom < r1.top);
	//     }

	//     public static intersectXY(x1:number, y1:number, width1:number, height1:number, x2:number, y2:number, width2:number, height2:number):boolean {
	//         return !(x2 > x1 + width1 || 
	//            x2 + width2 < x1 || 
	//            y2 > y1 + height1 ||
	//            y2 + height2 < y1);
	// }

	// call repeatedly to move to target 
	// returns true when at target
	// speed_per_tick => e.g. 4 pixels per call
	inline bool move_to(float &x, float &y, float targetX, float targetY, float speed) {
		float delta_x = targetX - x;
		float delta_y = targetY - y;
		float goal_dist = Math::sqrt_f( (delta_x * delta_x) + (delta_y * delta_y) );
		if (goal_dist > speed) {
			float ratio = speed / goal_dist;
			float x_move = ratio * delta_x;  
			float y_move = ratio * delta_y;
			x = x_move + x;
			y = y_move + y;
			return false;
		}
		
		// destination reached
		return true;
	}
}

namespace RNG {
	static std::random_device RNG_seed;
	static std::mt19937 RNG_generator(RNG_seed());

    inline int next_i(int max) {
        std::uniform_int_distribution<int> range(0, max);
        return range(RNG_generator);
    }

    inline int range_i(int min, int max) {
        std::uniform_int_distribution<int> range(min, max);
        return range(RNG_generator);
    }

    inline float range_f(float min, float max) {
        std::uniform_real_distribution<float> range(min, max);
        return range(RNG_generator);
    }

	inline void random_point_i(int xMax, int yMax, int &xOut, int &yOut) {
		std::uniform_int_distribution<int> xgen(0, xMax);
		std::uniform_int_distribution<int> ygen(0, yMax);
		xOut = xgen(RNG_generator);
		yOut = ygen(RNG_generator);
	}

	inline Vector2 vector2(const float &x_min, const float &x_max, const float &y_min, const float &y_max) {
		std::uniform_real_distribution<float> xgen(x_min, x_max);
		std::uniform_real_distribution<float> ygen(y_min, y_max);
		return Vector2(xgen(RNG_generator), ygen(RNG_generator));
	}
}

namespace Localization {
    struct Text {
        Text(const char *file);
        char *getText(const std::string s);
        std::map<std::string, char *> texts;
    };

    void load_texts(const char *file);
    char *text_lookup(const std::string s);
}

namespace Serialization {	
	void write_string(std::ostream &out, const std::string &s);
	void read_string(std::istream &in, std::string &s);
}

namespace Noise {
	// All noise functions from
	// https://github.com/Auburns/FastNoise
	// Lowest amount to highest amount interpolation
	enum Interp { Linear, Hermite, Quintic };
	
	void init(int seed = 1337);
	void set_seed(int seed);	
	float perlin(float x, float y);
	float simplex(float x, float y);
}

struct TileMap {
	unsigned tile_size;
	unsigned columns;
	unsigned rows;
	unsigned layers;
	unsigned *tiles;
};

namespace Tiling {
	unsigned tilemap_index(const TileMap &tile_map, const unsigned layer, const unsigned x, const unsigned y);
	void tilemap_load(const std::string map_name, TileMap &tile_map);
	void tilemap_make(TileMap &tile_map, unsigned layers, unsigned columns, unsigned rows, unsigned tile_size, unsigned default_tile = 0);
};

namespace Sound {
	typedef size_t SoundId;
	// Load/Cache a sound from the sound folder
	SoundId load(const std::string &file);
	void init();
	void quit();
	void play_next();
	void play_all();
	void queue(SoundId id, int volume);
};

namespace Timing {
    typedef void (*timer_complete_func)();
    
    struct Timer {
        float elapsed = 0.0f;
        float time = 0.0f;
        timer_complete_func on_elapsed = nullptr; 
    };

    static std::vector<Timer> _timers;

    inline void init(int sz) {
        _timers.reserve(sz);
    }

    inline void reset() {
        _timers.clear();
    }
    
    inline void add_timer(float time, timer_complete_func on_elapsed) {
        _timers.push_back({ 0.0f, time, on_elapsed });
    }
    
    inline void update_timers() {
        for(size_t i = 0; i < _timers.size(); i++) {
            _timers[i].elapsed += Time::delta_time;
            if(_timers[i].elapsed > _timers[i].time) {
                _timers[i].on_elapsed();
            }
        }
        
        for(size_t i = 0; i < _timers.size(); i++) {
            if(_timers[i].elapsed > _timers[i].time) {
                _timers[i] = _timers[_timers.size() - 1];
                _timers.erase(_timers.end() - 1);
            }
        }
    }
};

namespace ECS {
    const unsigned ENTITY_INDEX_BITS = 22;
    const unsigned ENTITY_INDEX_MASK = (1<<ENTITY_INDEX_BITS)-1;

    const unsigned ENTITY_GENERATION_BITS = 8;
    const unsigned ENTITY_GENERATION_MASK = (1<<ENTITY_GENERATION_BITS)-1;

    typedef unsigned EntityId;
    struct Entity {
        EntityId id = 0;

        unsigned index() const { return id & ENTITY_INDEX_MASK; }
        unsigned generation() const { return (id >> ENTITY_INDEX_BITS) & ENTITY_GENERATION_MASK; }

        bool equals(Entity other) {
            return id == other.id;
        }
    };

    class ComponentID {
        static size_t counter;
        public:
            template<typename T>
            static size_t value() {
                static size_t id = counter++;
                return id;
            }
    };

    const unsigned MINIMUM_FREE_INDICES = 1024;

    struct EntityManager {
        std::vector<unsigned char> _generation;
        std::queue<unsigned> _free_indices;

        Entity create() {
            unsigned idx;
            if (_free_indices.size() > MINIMUM_FREE_INDICES) {
                idx = _free_indices.front();
                _free_indices.pop();
            } else {
                _generation.push_back(0);
                idx = _generation.size() - 1;
                ASSERT_WITH_MSG(idx < (1 << ENTITY_INDEX_BITS), "idx is malformed, larger than 22 bits?");
            }

            return make_entity(idx, _generation[idx]);
        }

        Entity make_entity(unsigned idx, unsigned char generation) {
            Entity e;
            auto id = generation << ENTITY_INDEX_BITS | idx;
            e.id = id;
            return e;
        }

        bool alive(Entity e) const {
            return _generation[e.index()] == e.generation();
        }

        void destroy(Entity e) {
            if(!alive(e))
                return;

            const unsigned idx = e.index();
            ++_generation[idx];
            _free_indices.push(idx);
        }
    };

    typedef std::bitset<512> ComponentMask;
        
    struct BaseContainer {
        virtual void move(int index, int last_index) = 0;
        virtual void clear(size_t sz) = 0;
        virtual BaseContainer *make_empty_copy() = 0;
        virtual void copy_to(int from_index, BaseContainer* container, int to_index) = 0;
    };

    template<typename Component>
    struct ComponentContainer : BaseContainer {
        std::vector<Component> items;

        void move(int index, int last_index) override {
            items.at(index) = items.at(last_index);
        }

        void clear(size_t sz) override {
            // This only works if you want to have components without constructors
            items.clear();
            items.reserve(sz);
            for(size_t i = 0; i < sz; ++i) {
                items.emplace_back();
            }
        }

        BaseContainer *make_empty_copy() override {
            return new ComponentContainer<Component>();
        }

        void copy_to(int from_index, BaseContainer* container, int to_index) {
            static_cast<ComponentContainer<Component>*>(container)->items[to_index] = items[from_index];
        }
    };

    class EntityData {
        public:
            ComponentMask mask;
            std::vector<Entity> entity;
            int length = 0;
            struct Handle {
                int i = -1;
            };

            EntityData *clone() {
                EntityData *d = new EntityData;
                d->size = size;
                d->init_data_structures(size);

                d->mask = mask;
                d->entity = entity;
                d->length = length;
                d->_map = _map;
                d->container_indexes = container_indexes;
                d->has_component = has_component;
                
                for(size_t &i : container_indexes) {
                    auto c = containers[i]->make_empty_copy();
                    c->clear(size);
                    d->containers[i] = c;
                }

                return d;
            }

            void copy_to(Entity e, EntityData *to) {
                auto h_from = get_handle(e);
                auto h_to = to->get_handle(e);
                // to->containers[i]->items[h_to.i] = containers[i]->items[h_from.i];
                for(size_t &i : container_indexes) {
                    containers[i]->copy_to(h_from.i, to->containers[i], h_to.i);
                }
            }

            template <typename Component>
            void add_container() {
                init<Component>(size);
            }

            template <typename ... Components>
            void allocate_entities(size_t sz) {
                init_data_structures(sz);
                init<Components...>(sz);
            }
        
            void add_entity(Entity e) {
                ASSERT_WITH_MSG(entity.size() <= size, "Component storage is full, n:" + std::to_string(entity.size()));
                ASSERT_WITH_MSG(!contains(e), "Entity already has component");
                
                unsigned int index = entity.size();
                _map[e.id] = index;
                entity.push_back(e);

                ++length;
            }

            void remove(Entity e) {
                if(!contains(e))
                    return;

                auto a = _map.find(e.id);
                const int index = a->second;
                const int lastIndex = entity.size() - 1;

                if (lastIndex >= 0) {
                    // Get the entity at the index to destroy
                    Entity entityToDestroy = entity[index];
                    // Get the entity at the end of the array
                    Entity lastEntity = entity[lastIndex];

                    // Move last entity's data
                    entity[index] = entity[lastIndex];

                    for(size_t &i : container_indexes) {
                        containers[i]->move(index, lastIndex);
                    }
                    
                    // Update map entry for the swapped entity
                    _map[lastEntity.id] = index;
                    // Remove the map entry for the destroyed entity
                    _map.erase(entityToDestroy.id);

                    // Decrease count
                    entity.pop_back();

                    --length;
                }
            }

            void remove_all() {
                for(int i = length - 1; i >= 0; i--) {
                    remove(entity[i]);
                }
            }

            Handle get_handle(Entity e) {
                auto a = _map.find(e.id);
                if(a != _map.end()) {
                    return { (int)a->second };
                }
                return { invalid_handle };
            }

            const Handle get_handle(Entity e) const {
                auto a = _map.find(e.id);
                if(a != _map.end()) {
                    return { (int)a->second };
                }
                return { invalid_handle };
            }

            bool is_valid_handle(Handle h) {
                return h.i != invalid_handle;
            }

            template <typename C>
            C &get(const Handle &handle) {
                auto container_index = ComponentID::value<C>();
                return static_cast<ComponentContainer<C>*>(containers[container_index])->items[handle.i];
            }

            template <typename C>
            void set(const Handle &handle, const C &component) {
                auto container_index = ComponentID::value<C>();
                static_cast<ComponentContainer<C>*>(containers[container_index])->items[handle.i] = component;
            }

            template <typename C>
            C &index(const int index) {
                auto container_index = ComponentID::value<C>();
                return static_cast<ComponentContainer<C>*>(containers[container_index])->items[index];
            }

            template <typename C>
            std::vector<C> &get_components_by_type() {
                auto container_index = ComponentID::value<C>();
                return static_cast<ComponentContainer<C>*>(containers[container_index])->items;
            }

            template <typename C>
            bool match() {
                auto container_index = ComponentID::value<C>();
                return has_component[container_index];
            }

            template <typename C1, typename C2, typename ... Components>
            bool match() {
                return match<C1>() && match<C2, Components ...>();
            }

        private:
            static const int invalid_handle = -1;
            std::unordered_map<EntityId, unsigned> _map;
            std::vector<size_t> container_indexes;
            std::vector<bool> has_component;
            std::vector<BaseContainer*> containers;
            size_t size = 0;
            
            void init_data_structures(size_t sz) {
                size = sz;
                entity.reserve(size);

                size_t max_components_total = 512;

                containers.reserve(max_components_total);
                for(size_t i = 0; i < max_components_total; ++i) {
                    containers.emplace_back();
                }
                has_component.reserve(max_components_total);
                for(size_t i = 0; i < max_components_total; ++i) {
                    has_component.emplace_back(false);
                }
            }

            template <typename C>
            void init(size_t sz) {
                auto c = new ComponentContainer<C>();
                auto container_index = ComponentID::value<C>();

                c->clear(sz);
                container_indexes.push_back(container_index);
                has_component[container_index] = true;
                containers[container_index] = c;
            }

            template <typename C1, typename C2, typename ... Components>
            void init(size_t sz) {
                init<C1>(sz);
                init<C2, Components ...>(sz);
            }
            
            bool contains(Entity e) {
                auto a = _map.find(e.id);
                return a != _map.end();
            }
    };

    struct ArcheType {
        ComponentMask _mask;
    };

    template <typename C>
    ComponentMask create_mask() {
        ComponentMask mask;
        mask.set(ComponentID::value<C>());
        return mask;
    }

    template <typename C1, typename C2, typename ... Components>
    ComponentMask create_mask() {
        return create_mask<C1>() | create_mask<C2, Components ...>();
    }
        
    struct ContainerIterator {
        std::vector<EntityData*> containers;
        // std::vector<ArcheType> archetypes;
    };

    struct ArchetypeManager {
        private:
        EntityManager em;
        std::vector<EntityData*> archetypes;
        std::unordered_map<ComponentMask, int> archetype_map;
        std::unordered_map<EntityId, ComponentMask> entity_to_archetype;
        
        public:
        void clear_entities() {
            for(auto a : archetypes) {
                a->remove_all();
            }

            entity_to_archetype.clear();
        }

        template <typename ... Components>
        ArcheType create_archetype(size_t sz) {
            ComponentMask mask = create_mask<Components...>();

            EntityData *container = new EntityData();
            container->allocate_entities<Components...>(sz);
            container->mask = mask;
            
            archetypes.push_back(container);
            archetype_map[mask] = archetypes.size() - 1;
            
            ArcheType a;
            a._mask = mask;
            return a;
        }

        bool archetype_exists(ComponentMask mask) {
            return archetype_map.find(mask) != archetype_map.end();
        }

        bool archetype_empty(ArcheType &a) {
            return archetypes[archetype_map[a._mask]]->length == 0;
        }

        template<typename C>
        void add_component(Entity entity, C component) {
            const ArcheType a = get_archetype(entity);
            
            ComponentMask new_mask = a._mask;
            new_mask.set(ComponentID::value<C>());
            
            EntityData *d = archetypes[archetype_map[a._mask]];
            EntityData *d_new = nullptr;

            if(!archetype_exists(new_mask)) {
                d_new = d->clone();
                d_new->remove_all();
                d_new->add_container<C>();
                d_new->mask = new_mask;
                d_new->add_entity(entity);

                // Create archetype
                archetypes.push_back(d_new);
                archetype_map[new_mask] = archetypes.size() - 1;
            } else {
                d_new = archetypes[archetype_map[new_mask]];
            }
            
            // Move entity to new archetype in mapping
            entity_to_archetype[entity.id] = new_mask;

            d->copy_to(entity, d_new);
            
            // Remove entity from old archetype
            archetypes[archetype_map[a._mask]]->remove(entity);
            
            set_component(entity, component);
        }

        template <typename ... Components>
        ContainerIterator get_iterator() {
            ContainerIterator it;
            for(auto c : archetypes) {
                if(c->match<Components...>()) {
                    it.containers.push_back(c);
                    // ArcheType a;
                    // a._mask = c->mask;
                    // it.archetypes.push_back(a);
                }
            }
            return it;
        }

        template<typename ... Components>
        void iterate(std::function<void(ECS::EntityData*, const int)> f) {
            auto ci = get_iterator<Components...>();
            for(auto c : ci.containers) {
                for(int i = 0; i < c->length; i++) {
                    f(c, i);
                }
            }
        }

        Entity create_entity(const ArcheType &a) {
            auto entity = em.create();
            archetypes[archetype_map[a._mask]]->add_entity(entity);
            entity_to_archetype[entity.id] = a._mask;
            return entity;
        }

        void remove_entity(const ArcheType &a, Entity entity) {
            archetypes[archetype_map[a._mask]]->remove(entity);
            entity_to_archetype.erase(entity.id);
        }

        void remove_entity(Entity entity) {
            const ArcheType a = get_archetype(entity);
            EntityData *d = archetypes[archetype_map[a._mask]];
            
            auto handle = d->get_handle(entity);
            ASSERT_WITH_MSG(d->is_valid_handle(handle), "remove_entity: Invalid entity handle! Entity already removed!");

            archetypes[archetype_map[a._mask]]->remove(entity);
            entity_to_archetype.erase(entity.id);
        }

        bool is_alive(Entity entity) {
            bool not_found = entity_to_archetype.find(entity.id) == entity_to_archetype.end();
            if(not_found) {
                return false;
            }
            
            ArcheType a = get_archetype(entity);
            return is_alive(a, entity);
        }

        bool is_alive(const ArcheType &a, Entity entity) {
            EntityData *data = archetypes[archetype_map[a._mask]];
            auto handle = data->get_handle(entity);
            return data->is_valid_handle(handle);
        }

        ArcheType get_archetype(Entity entity) {
            ASSERT_WITH_MSG(entity_to_archetype.find(entity.id) != entity_to_archetype.end(), "Entity is not in any archetype");
            return { entity_to_archetype[entity.id] };
        }

        template<typename T>
        bool has_component(Entity entity) {
            const ArcheType a = get_archetype(entity);
            return a._mask.test(ComponentID::value<T>());
        }

        template<typename T>
        void set_component(const ArcheType &a, Entity entity, const T &component) {
            ASSERT_WITH_MSG(a._mask.test(ComponentID::value<T>()), "set_component: component is not defined on this entity");
            EntityData *data = archetypes[archetype_map[a._mask]];
            auto handle = data->get_handle(entity);
            ASSERT_WITH_MSG(data->is_valid_handle(handle), "set_component: Invalid entity handle! Check if entity is alive first!?");
            data->set(handle, component);
        }

        template<typename T>
        void set_component(Entity entity, const T &component) {
            const ArcheType a = get_archetype(entity);
            EntityData *data = archetypes[archetype_map[a._mask]];
            ASSERT_WITH_MSG(a._mask.test(ComponentID::value<T>()), "set_component: component is not defined on this entity");
            auto handle = data->get_handle(entity);
            ASSERT_WITH_MSG(data->is_valid_handle(handle), "set_component: Invalid entity handle! Check if entity is alive first!?");
            data->set(handle, component);
        }

        template<typename T>
        T &get_component(const ArcheType &a, Entity entity) {
            ASSERT_WITH_MSG(a._mask.test(ComponentID::value<T>()), "get_component: component is not defined on this entity");
            EntityData *data = archetypes[archetype_map[a._mask]];
            auto handle = data->get_handle(entity);
            ASSERT_WITH_MSG(data->is_valid_handle(handle), "get_component: Invalid entity handle! Check if entity is alive first!?");
            return data->get<T>(handle);
        }

        template<typename T>
        T &get_component(Entity entity) {
            const ArcheType &a = get_archetype(entity);
            ASSERT_WITH_MSG(a._mask.test(ComponentID::value<T>()), "get_component: component is not defined on this entity");
            EntityData *data = archetypes[archetype_map[a._mask]];
            auto handle = data->get_handle(entity);
            ASSERT_WITH_MSG(data->is_valid_handle(handle), "get_component: Invalid entity handle! Check if entity is alive first!?");
            return data->get<T>(handle);
        }
    };
};

namespace Intersects {
    inline bool circle_contains_point(Vector2 circle, float radius, Vector2 point) {
        float circle_left = circle.x - radius;
        float circle_right = circle.x + radius;
        float circle_bottom = circle.y + radius;
        float circle_top = circle.y - radius;
        //  Check if point is inside bounds
        if (radius > 0 && point.x >= circle_left && point.x <= circle_right && point.y >= circle_top && point.y <= circle_bottom) {
            float dx = (circle.x - point.x) * (circle.x - point.x);
            float dy = (circle.y - point.y) * (circle.y - point.y);
            return (dx + dy) <= (radius * radius);
        }
        
        return false;
    }

    // From Phaser
    // Works well and can detect if a line is inside a circle also
    // Nearest is the point closest to the center
    inline bool line_circle(const Vector2 &lineP1, const Vector2 &lineP2, const Vector2 &circle_center, const float &radius, Vector2 &nearest) {
        if (circle_contains_point(circle_center, radius, lineP1)) {
            nearest.x = lineP1.x;
            nearest.y = lineP1.y;
            // furthest.x = lineP2.x;
            // furthest.y = lineP2.y;
            return true;
        }

        if (circle_contains_point(circle_center, radius, lineP2)) {
            nearest.x = lineP2.x;
            nearest.y = lineP2.y;
            // furthest.x = lineP1.x;
            // furthest.y = lineP1.y;
            return true;
        }

        float dx = lineP2.x - lineP1.x;
        float dy = lineP2.y - lineP1.y;

        float lcx = circle_center.x - lineP1.x;
        float lcy = circle_center.y - lineP1.y;

        //  project lc onto d, resulting in vector p
        float dLen2 = (dx * dx) + (dy * dy);
        float px = dx;
        float py = dy;

        if (dLen2 > 0) {
            float dp = ((lcx * dx) + (lcy * dy)) / dLen2;
            px *= dp;
            py *= dp;
        }

        nearest.x = lineP1.x + px;
        nearest.y = lineP1.y + py;
        
        //  len2 of p
        float pLen2 = (px * px) + (py * py);
        return pLen2 <= dLen2 && ((px * dx) + (py * dy)) >= 0 && circle_contains_point(circle_center, radius, nearest);
    }

    // Works good and finds the entry point of collision
    // return values:
    // 0: no collision
    // 1: collision but no entry/exit point
    // 2: collision and entry/exit point closest to segment_start
    inline int line_circle_entry(const Vector2 &segment_start, const Vector2 &segment_end, const Vector2 &center, const float &radius, Vector2 &intersection) {
        // if (circle_contains_point(center, radius, segment_start)) {
        //     return true;
        // }

        // if (circle_contains_point(center, radius, segment_end)) {
        //     return true;
        // }
        
        /*
        Taking

        E is the starting point of the ray,
        L is the end point of the ray,
        C is the center of sphere you're testing against
        r is the radius of that sphere

        Compute:
        d = L - E ( Direction vector of ray, from start to end )
        f = E - C ( Vector from center sphere to ray start ) 
        */
        Vector2 d = segment_end - segment_start;
        Vector2 f = segment_start - center;
        float r = radius;

        float a = d.dot( d ) ;
        float b = 2*f.dot( d ) ;
        float c = f.dot( f ) - r*r ;

        float discriminant = b*b-4*a*c;
        if( discriminant < 0 ) {
            // no intersection
            return 0;
        }
    
        // ray didn't totally miss sphere,
        // so there is a solution to
        // the equation.
        discriminant = Math::sqrt_f( discriminant );

        // either solution may be on or off the ray so need to test both
        // t1 is always the smaller value, because BOTH discriminant and
        // a are nonnegative.
        float t1 = (-b - discriminant)/(2*a);
        float t2 = (-b + discriminant)/(2*a);
        
        // 3x HIT cases:
        //          -o->             --|-->  |            |  --|->
        // Impale(t1 hit,t2 hit), Poke(t1 hit,t2>1), ExitWound(t1<0, t2 hit), 

        // 3x MISS cases:
        //       ->  o                     o ->              | -> |
        // FallShort (t1>1,t2>1), Past (t1<0,t2<0), CompletelyInside(t1<0, t2>1)

        if(t1 <= 0 && t2 >= 1) {
            // Completely inside
            // we consider this a hit, not a miss
            // Engine::logn("inside");
            return 1;
        }

        if(t1 >= 0 && t1 <= 1)
        {
            // t1 is the intersection, and it's closer than t2
            // (since t1 uses -b - discriminant)
            // Impale, Poke
            // Engine::logn("impale, poke");
            intersection = Vector2(segment_start.x + t1 * d.x, segment_start.y + t1 * d.y);
            return 2;
        }

        // here t1 didn't intersect so we are either started
        // inside the sphere or completely past it
        if(t2 >= 0 && t2 <= 1)
        {
            // ExitWound
            // Engine::logn("exit wound");
            intersection = Vector2(segment_start.x + t1 * d.x, segment_start.y + t1 * d.y);
            return 2;
        }

        // no intn: FallShort, Past,  // CompletelyInside
        return 0;    
    }
}

#endif