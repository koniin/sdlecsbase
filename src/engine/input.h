#include "precompiled.h"

struct Point;

namespace Input {
    extern int mousex;
	extern int mousey;
	extern bool mouse_left_down;
    extern bool mouse_left_up;
    extern bool mouse_right_down;
    extern bool mouse_right_up;

    void init();
    void update_states();
    void map(const SDL_Event *event);
    bool key_down(const SDL_Scancode &scanCode);
	bool key_down_k(const SDL_Keycode &keyCode);
    bool key_released(const SDL_Keycode &keyCode);
    bool key_pressed(const SDL_Keycode &keyCode);

    void mouse_current(Point &p);
}