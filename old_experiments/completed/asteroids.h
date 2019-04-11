#include "engine.h"
#include "renderer.h"

struct GameState {
	bool inactive = false;
	float inactive_timer = 0.0f;
	float pause_time = 2.0f;
	int level = 1;
	SDL_Color text_color = { 220, 220, 220, 255 };
	SDL_Color asteroid_color = { 240, 240, 240, 255 };
} game_state;

void game_state_inactivate();
void game_state_reset();

struct InputMapping {
	SDL_Scancode up;
	SDL_Scancode down;
	SDL_Scancode left;
	SDL_Scancode right;
	SDL_Scancode fire;
	SDL_Scancode shield;
};

InputMapping input_maps[2] = {
	{ SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D, SDL_SCANCODE_SPACE, SDL_SCANCODE_LSHIFT },
	{ SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_KP_ENTER, SDL_SCANCODE_RSHIFT }
};

struct AsteroidsConfig {
	float rotation_speed = 5.0f; 
	float acceleration = 0.2f;
	float brake_speed = -0.05f;
	float drag = 0.02f;
	float fire_cooldown = 0.25f; // s
	float player_bullet_speed = 5;
	float player_bullet_size = 1;
	int player_faction_1 = 0;
	int player_faction_2 = 1;
	int enemy_faction = 2;
	float bullet_time_to_live = 20.5f; // really high because we don't care
	float player_death_inactive_time = 1.0f;
	float player_shield_time = 2.0f;
	float player_shield_inactive_time = 6.0f;
	int asteroid_count_increase_per_level = 2;
} config;

struct Position {
	// Position
	float x, y;
};

struct Velocity {
	// Position
	float x, y;
};

struct Rotation {
	float x, y;
};

struct PlayerInput {
	// Input
	float move_x;
	float move_y;
	float fire_x;
	float fire_y;
	float fire_cooldown;
	bool shield;
};

struct Shield {
	float active_timer = 0;
	float inactive_timer = 0;
	bool is_active() {
		return active_timer > 0.0f;
	}
};

struct Ship {
	// Angle
	float angle = 0;
	float radius = 7;
	int health = 3;
	int faction = 0;
	int score = 0;
	float inactive_timer = 0;
	Shield shield;
	PlayerInput input;
	Position position;
	Velocity velocity;
};

struct Asteroid {
	Position position;
	Velocity velocity;
	int size;

	float radius() {
		if(size == 1)
			return 8.0f;
		else if(size == 2) 
			return 6.0f;
		else 
			return 2.0f;
	}
};

struct Bullet {
	Position position;
	Velocity velocity;
	float time_to_live;
	float radius;
	int faction;
};

struct ShotSpawnData {
	Position position;
	Rotation rotation;
	float time_to_live;
	int faction;
};

struct AsteroidSpawnData {
	Position position;
	Velocity velocity;
	int size;
};

struct AsteroidDestroyedData {
	int size;
	int faction;
};

struct ShipHitData {
	int faction;
};

struct Event {
	enum EventType {
		FireBullet,
		SpawnAsteroid,
		AsteroidDestroyed,
		ShipHit
	} type;
	void *data;
};
void queue_event(Event e);

unsigned ship_n = 0;
std::vector<Ship> ships(100);
unsigned asteroid_n = 0;
std::vector<Asteroid> asteroids(100);
unsigned bullets_n = 0;
std::vector<Bullet> bullets(1000);
unsigned event_n;
std::vector<Event> event_queue(100);

void spawn_player(int faction) {
	Ship player;
	player.faction = faction;
	player.score = 0;
	player.position.x = gw / 2.0f;
	player.position.y = gh / 2.0f;
	player.velocity.x = 0;
	player.velocity.y = 0;
	player.angle = 0;
	player.input.move_x = 0;
	player.input.move_y = 0;
	player.input.fire_x = 0;
	player.input.fire_y = 0;
	player.input.fire_cooldown = 0;
	player.input.shield = false;
	player.shield.inactive_timer = 0;
	player.shield.active_timer = 0;
	ships[ship_n++] = player;
}

void spawn_bullet(Position position, Rotation direction, int faction, float time_to_live) {
	Bullet b = { position };
	b.time_to_live = time_to_live;
	b.faction = faction;
	if(faction == config.player_faction_1 || faction == config.player_faction_2) {
		b.velocity.x = direction.x * config.player_bullet_speed;
		b.velocity.y = direction.y * config.player_bullet_speed;
		b.radius = config.player_bullet_size;
	} 
	bullets[bullets_n++] = b;
}

void spawn_asteroid(Position position, Velocity velocity, int size) {
	asteroids[asteroid_n].position = position;
	asteroids[asteroid_n].velocity = velocity;
	asteroids[asteroid_n].size = size;
	asteroid_n++;
}

void spawn_asteroid_wave() {
	for(int i = 0; i < game_state.level + config.asteroid_count_increase_per_level; ++i) {
		Position position;
		Velocity velocity;
		position.x = RNG::range_f(0, (float)gw);
		position.y = RNG::range_f(0, (float)gh);
		velocity.x = RNG::range_f(0, 100) / 100.0f - 0.5f;
		velocity.y = RNG::range_f(0, 100) / 100.0f - 0.5f;
		int size = 1;
		spawn_asteroid(position, velocity, size);
	}
}

inline void update_player_input(unsigned id, PlayerInput &pi) {
	pi.move_x = 0;
	pi.move_y = 0;
	pi.fire_x = 0;
	pi.fire_y = 0;
	pi.shield = false;

	InputMapping key_map = input_maps[id];

	if(Input::key_down(key_map.up)) {
		pi.move_y = 1;
	} else if(Input::key_down(key_map.down)) {
		pi.move_y = -1;
	} 
	
	if(Input::key_down(key_map.left)) {
		pi.move_x = -1;
	} else if(Input::key_down(key_map.right)) {
		pi.move_x = 1;
	}

	pi.fire_cooldown = Math::max(0.0f, pi.fire_cooldown - Time::deltaTime);
	if(Input::key_down(key_map.fire)) {
		pi.fire_x = pi.fire_y = 1;
	}

	if(Input::key_down(key_map.shield)) {
		pi.shield = true;
	}
}

inline void update_position(Position &position, Velocity &velocity) {
	position.x += velocity.x;
	position.y += velocity.y;
}

inline void keep_in_bounds(Position &p) {
	if(p.x < 0) p.x = (float)gw;
	if(p.x > gw) p.x = 0.0f;
	if(p.y < 0) p.y = (float)gh;
	if(p.y > gh) p.y = 0.0f;
}

inline void update_player_movement(Ship &sdata) {
	PlayerInput &pi = sdata.input;
	Velocity &velocity = sdata.velocity;
	Position &position = sdata.position;

	// Update rotation based on rotational speed
	// for other objects than player input once
	sdata.angle += pi.move_x * config.rotation_speed;
	float rotation = sdata.angle / Math::RAD_TO_DEGREE;

	float direction_x = cos(rotation);
	float direction_y = sin(rotation);
	velocity.x += direction_x * pi.move_y * config.acceleration;
	velocity.y += direction_y * pi.move_y * config.acceleration;
	
	position.x += velocity.x;
	position.y += velocity.y;

	// Use Stokes' law to apply drag to the object
	velocity.x = velocity.x - velocity.x * config.drag;
	velocity.y = velocity.y - velocity.y * config.drag;

	if(pi.fire_cooldown <= 0.0f && Math::length_vector_f(pi.fire_x, pi.fire_y) > 0.5f) {
		Event e = { Event::FireBullet };
		ShotSpawnData *d = new ShotSpawnData;
		d->position = position;
		d->rotation.x = direction_x;
		d->rotation.y = direction_y;
		d->time_to_live = config.bullet_time_to_live;
		d->faction = sdata.faction;
		e.data = d;
		queue_event(e);
		pi.fire_cooldown = config.fire_cooldown;
	}
}

void system_asteroid_spawn() {
	if(asteroid_n == 0) {
		game_state.level++;
		spawn_asteroid_wave();
	}
}

void system_shield() {
	for(unsigned i = 0; i < ship_n; ++i) {
		Shield &s = ships[i].shield;
		s.active_timer = Math::max(0.0f, s.active_timer - Time::deltaTime);
		s.inactive_timer = Math::max(0.0f, s.inactive_timer - Time::deltaTime);
		PlayerInput &pi = ships[i].input;
		if(pi.shield && s.inactive_timer <= 0.0f) {
			s.active_timer = config.player_shield_time;
			s.inactive_timer = config.player_shield_time + config.player_shield_inactive_time;
		}
	}
}

void system_player_input() {
	for(unsigned i = 0; i < ship_n; ++i) {
		// TODO: this should be another system or something 
			// and when it is activated it should get a input component
			// and a collision component or something like that 
		ships[i].inactive_timer = Math::max(0.0f, ships[i].inactive_timer - Time::deltaTime);
		if(ships[i].inactive_timer <= 0) {
			update_player_input(ships[i].faction, ships[i].input);
		}
	}
}

void system_player_movement() {
	for(unsigned i = 0; i < ship_n; ++i) {
		// TODO: this should be another system or something 
			// and when it is activated it should get a input component
			// and a collision component or something like that 
		if(ships[i].inactive_timer <= 0) {
			update_player_movement(ships[i]);
		}
	}
}

inline void system_forward_movement() {
	for(unsigned i = 0; i < asteroid_n; ++i) {
		Asteroid &s = asteroids[i];
		update_position(s.position, s.velocity);
	}
	for(unsigned i = 0; i < bullets_n; ++i) {
		Bullet &s = bullets[i];
		update_position(s.position, s.velocity);
	}
}

void system_keep_in_bounds() {
	for(unsigned i = 0; i < asteroid_n; ++i) {
		keep_in_bounds(asteroids[i].position);
	}
	for(unsigned i = 0; i < ship_n; ++i) {
		keep_in_bounds(ships[i].position);
	}
}

void system_collisions() {
	for(unsigned ai = 0; ai < asteroid_n; ++ai) {
		for(unsigned si = 0; si < ship_n; ++si) {
			Position &pp = ships[si].position;
			float pr = ships[si].radius;
			Position &ap = asteroids[ai].position;
			float ar = asteroids[ai].radius();
			if(Math::intersect_circles(pp.x, pp.y, pr, ap.x, ap.y, ar)) {
				queue_event({ Event::ShipHit, new ShipHitData { ships[si].faction }});
			}
		}
	}

	for(unsigned bi = 0; bi < bullets_n; ++bi) {
		for(unsigned ai = 0; ai < asteroid_n; ++ai) {
			Position &bp = bullets[bi].position;
			float br = bullets[bi].radius;
			Position &ap = asteroids[ai].position;
			float ar = asteroids[ai].radius();
			if(Math::intersect_circles(bp.x, bp.y, br, ap.x, ap.y, ar)) {
				queue_event({ Event::AsteroidDestroyed, new AsteroidDestroyedData { 
					asteroids[ai].size,
					bullets[bi].faction
				}});
				
				Velocity v = { asteroids[ai].velocity.x * 3, asteroids[ai].velocity.y * 3 };
				int size = asteroids[ai].size + 1;
				queue_event({ Event::SpawnAsteroid, new AsteroidSpawnData { ap, v, size } });
				v.x = -v.x;
				v.y = -v.y;
				queue_event({ Event::SpawnAsteroid, new AsteroidSpawnData { ap, v, size } });
				
				// TODO: This should be an destroy entity event and just send the ID
				bullets[bi].time_to_live = 0.0f;

				// TODO: This should be an destroy entity event and just send the ID
				// then some system could watch for destroyed asteroids and spawn new ones if needed
				// probably a part of the Event::AsteroidDestroyed
				asteroids[ai] = asteroids[asteroid_n - 1];
				asteroid_n--;
			}
		}	
	}
}

inline void bullet_cleanup() {
	for(unsigned i = 0; i < bullets_n; ++i) {
		Bullet &b = bullets[i];
		Position &p = bullets[i].position;
		b.time_to_live -= Time::deltaTime;

		if(p.x < 0 || p.y < 0 || p.x > gw || p.y > gh 
			|| b.time_to_live <= 0.0f 
			|| ship_n == 0) {
            
			// TODO: This should be an destroy entity event and just send the ID
			bullets[i] = bullets[bullets_n - 1];
			bullets_n--;
		}
	}
}

void queue_event(Event e) {
	ASSERT_WITH_MSG(event_n < event_queue.size(), "Too many events!");
	event_queue[event_n++] = e;
}

void handle_events() {
	for(unsigned i = 0; i < event_n; ++i) {
		Event &e = event_queue[i];
		switch(e.type) {
			case Event::FireBullet: {
				ShotSpawnData *d = static_cast<ShotSpawnData*>(e.data);
				spawn_bullet(d->position, d->rotation, d->faction, d->time_to_live);
				delete d;
				break;
			}
			case Event::SpawnAsteroid: {
				AsteroidSpawnData *d = static_cast<AsteroidSpawnData*>(e.data);
				if(d->size <= 3)
					spawn_asteroid(d->position, d->velocity, d->size);
				delete d;
				break;
			}
			case Event::AsteroidDestroyed: {
				AsteroidDestroyedData *d = static_cast<AsteroidDestroyedData*>(e.data);
				int score = 0;
				switch(d->size) {
					case 1: score = 10; break;
					case 2: score = 20; break;
					case 3: score = 50; break;
				}
				// TODO: I don't think we should loop here
				// should just be get the entity from id and do to that
				for(unsigned si = 0; si < ship_n; ++si) {
					if(ships[si].faction == d->faction) {
						ships[si].score += score;
					}
				}
				delete d;
				break;
			}
			case Event::ShipHit: {
				// TODO: I don't think we should loop here
				// should just be get the entity from id and do to that
				ShipHitData *d = static_cast<ShipHitData*>(e.data);
				for(unsigned si = 0; si < ship_n; ++si) {
					if(ships[si].faction != d->faction || ships[si].inactive_timer > 0)
						continue;

					if(ships[i].shield.is_active()) {
						continue;
					}

					ships[si].inactive_timer = config.player_death_inactive_time;
					ships[si].health--;
					ships[si].position.x = gw / 2.0f;
					ships[si].position.y = gh / 2.0f;
					ships[si].angle = 0;
					ships[si].velocity.x = ships[si].velocity.y = 0;
					if(ships[si].health <= 0) {
						ships[si] = ships[ship_n - 1];
						ship_n--;
						if(ship_n <= 0) {
							game_state_inactivate();
						}
					}
				}
				delete d;
			}
		}
	}

	event_n = 0;
}

void game_state_reset() {
	spawn_player(config.player_faction_1);
	spawn_player(config.player_faction_2);
	game_state.level = 1;
	spawn_asteroid_wave();
}

void game_state_inactivate() {
	game_state.inactive = true;
	game_state.inactive_timer = game_state.pause_time;
}

static SpriteSheet the_sheet;
void asteroids_load() {
    Engine::set_base_data_folder("data");
	Font *font = Resources::font_load("normal", "pixeltype.ttf", 15);
	set_default_font(font);
	Resources::font_load("gameover", "pixeltype.ttf", 85);
	Resources::sprite_sheet_load("shooter.data", the_sheet);
	game_state_reset();
}

void asteroids_update() {
    if(game_state.inactive) {
		game_state.inactive_timer -= Time::deltaTime;
		// Remove all asteroids and bullets, better do it here than special logic in event handling
		event_n = 0;
		asteroid_n = 0;
		bullets_n = 0;
		if(game_state.inactive_timer <= 0.0f) {
			game_state_reset();
			game_state.inactive = false;
		}
	}

	system_asteroid_spawn();
	system_shield();
	system_player_input();
	system_player_movement();
	system_forward_movement();
	system_keep_in_bounds();
	system_collisions();

	handle_events();

	bullet_cleanup();
}

void asteroids_render() {
	if(game_state.inactive) {
		int seconds = (int)game_state.inactive_timer;
		draw_text_font_centered(Resources::font_get("gameover"), gw / 2, gh / 2, game_state.text_color, "GAME OVER");
		draw_text_font_centered(Resources::font_get("normal"), gw / 2, gh / 2 + 100, game_state.text_color, 
			std::string("Resetting in: " + std::to_string(seconds) + " seconds..").c_str());
	} else {
	    std::string level_string = "Level: " + std::to_string(game_state.level);
	    draw_text_centered_str(gw / 2, gh - 10, game_state.text_color, level_string);
    }

	for(unsigned i = 0; i < asteroid_n; ++i) {
		Position &p = asteroids[i].position;
		draw_g_circe_color((int16_t)p.x, (int16_t)p.y, (int16_t)asteroids[i].radius(), game_state.asteroid_color);
	}
	for(unsigned i = 0; i < bullets_n; ++i) {
		Position &p = bullets[i].position;
		SDL_Color c = { 255, 0, 0, 255 };
		draw_g_circe_color((int16_t)p.x, (int16_t)p.y, (int16_t)bullets[i].radius, c);
	}

	for(unsigned i = 0; i < ship_n; ++i) {
		Ship &player = ships[i];

		draw_spritesheet_name_centered_rotated(the_sheet, "player", (int)player.position.x, (int)player.position.y, player.angle + 90);
		
		if(player.shield.is_active()) {
			draw_g_circe_RGBA((int16_t)player.position.x, (int16_t)player.position.y, 
				10, 0, 0, 255, 255);
		}
		if(player.shield.inactive_timer <= 0) {
			draw_g_rectangle_filled_RGBA(gw / 2 - 90, 11 + 10 * i, 5, 5, 0, 255, 0, 255);
		}
		
		std::string playerInfo = "Player " + std::to_string(player.faction + 1) + 
			" | Lives: " + std::to_string(player.health) + " | Score: ";
		draw_text_str(gw / 2 - 80, 10 + 10 * i, game_state.text_color, playerInfo);
		draw_text_str(gw / 2 + 60, 10 + 10 * i, game_state.text_color, std::to_string(player.score));
	}
}