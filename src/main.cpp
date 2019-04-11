#include "engine.h"
#include "renderer.h"
#include "game.h"

#include <iostream>

gameTimer timer;

void windowEvent(const SDL_Event * event);

static SDL_Event event;

void input() {
	Input::update_states();
	while (SDL_PollEvent(&event)) {
		Input::map(&event);
		
		switch(event.type) {
			case SDL_QUIT:
				Engine::exit();
				break;
			case SDL_WINDOWEVENT: {
				// windowEvent(&event);
        		break;             
			} 
			case SDL_KEYDOWN: {
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					Engine::exit();
				} else if (event.key.keysym.sym == SDLK_F1) {
					window_set_scale(1);
					window_center();
				} else if (event.key.keysym.sym == SDLK_F2) {
					window_set_scale(2);
					window_center();
				} else if (event.key.keysym.sym == SDLK_F3) {
					window_set_scale(3);
					window_center();
				} else if (event.key.keysym.sym == SDLK_F4) {
					window_set_scale(3);
					window_toggle_fullscreen(false);
				} else if (event.key.keysym.sym == SDLK_F5) {
					window_set_scale(3);
					window_toggle_fullscreen(true);
				}
				break;
			}
		}
	}
}


int main(int argc, char* argv[]) {
	if(!renderer_init("TITLE", 640, 360, 1)) {
		printf("init renderer failed");
		return 1;
	}

	Engine::init();
	
	game_load();
	
	// Initiate timer
    timer.now = SDL_GetPerformanceCounter();
    timer.last = 0;
    timer.dt = 0;
    timer.fixed_dt = 1.0/60.0;
    timer.accumulator = 0;

	// FPS timer
	int32_t fps_lasttime = SDL_GetTicks(); //the last recorded time.
	int32_t fps_current = 0; //the current FPS.
	int32_t fps_frames = 0; //frames passed since the last recorded fps.

	Time::delta_time = (float)timer.fixed_dt;
	Time::delta_time_fixed = (float)timer.fixed_dt;
	Time::delta_time_raw = (float)timer.fixed_dt;

    while (Engine::is_running()) {
		timer.last = timer.now;
        timer.now = SDL_GetPerformanceCounter();
        timer.dt = ((timer.now - timer.last)/(double)SDL_GetPerformanceFrequency());
        // This timing method is the 4th (Free the Physics) from this article: https://gafferongames.com/post/fix_your_timestep/
        timer.accumulator += timer.dt;
		
        while (timer.accumulator >= timer.fixed_dt) {	
			input();
			Engine::update();
			
            timer.accumulator -= timer.fixed_dt;
        }
		
		Engine::render();

		fps_frames++;
#define FPS_INTERVAL 1.0 //seconds.
		if (fps_lasttime < SDL_GetTicks() - FPS_INTERVAL*1000)
		{
			fps_lasttime = SDL_GetTicks();
			fps_current = fps_frames;
			fps_frames = 0;
			Engine::current_fps = fps_current;
		}
	}
	
	Engine::cleanup();
	renderer_destroy();

    return 0;
}

void windowEvent(const SDL_Event *window_event) {
    switch (window_event->window.event) {
        case SDL_WINDOWEVENT_SHOWN:
            SDL_Log("Window %d shown", window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            SDL_Log("Window %d hidden", window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            SDL_Log("Window %d exposed", window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MOVED:
            SDL_Log("Window %d moved to %d,%d",
                    window_event->window.windowID, window_event->window.data1,
                    window_event->window.data2);
            break;
        case SDL_WINDOWEVENT_RESIZED:
            SDL_Log("Window %d resized to %dx%d",
                    window_event->window.windowID, window_event->window.data1,
                    window_event->window.data2);
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            SDL_Log("Window %d size changed to %dx%d",
                    window_event->window.windowID, window_event->window.data1,
                    window_event->window.data2);
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            SDL_Log("Window %d minimized", window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            SDL_Log("Window %d maximized", window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_RESTORED:
            SDL_Log("Window %d restored", window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_ENTER:
            SDL_Log("Mouse entered window %d",
                    window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_LEAVE:
            SDL_Log("Mouse left window %d", window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            SDL_Log("Window %d gained keyboard focus",
                    window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            SDL_Log("Window %d lost keyboard focus",
                    window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_CLOSE:
            SDL_Log("Window %d closed", window_event->window.windowID);
            break;
#if SDL_VERSION_ATLEAST(2, 0, 5)
        case SDL_WINDOWEVENT_TAKE_FOCUS:
            SDL_Log("Window %d is offered a focus", window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_HIT_TEST:
            SDL_Log("Window %d has a special hit test", window_event->window.windowID);
            break;
#endif
        default:
            SDL_Log("Window %d got unknown event %d",
                    window_event->window.windowID, window_event->window.event);
            break;
        }
}