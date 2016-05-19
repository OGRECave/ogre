/*
 * Input.h
 *
 *  Created on: 05.12.2015
 *      Author: pavel
 */

#ifndef SAMPLES_COMMON_INCLUDE_INPUT_H_
#define SAMPLES_COMMON_INCLUDE_INPUT_H_

#ifdef HAVE_SDL
#include <SDL.h>

namespace OgreBites {
enum {
    BUTTON_LEFT = SDL_BUTTON_LEFT,
    BUTTON_MIDDLE = SDL_BUTTON_MIDDLE,
    BUTTON_RIGHT = SDL_BUTTON_RIGHT,
};

typedef SDL_KeyboardEvent KeyboardEvent;
typedef SDL_MouseMotionEvent MouseMotionEvent;
typedef SDL_MouseButtonEvent MouseButtonEvent;
typedef SDL_MouseWheelEvent MouseWheelEvent;
typedef SDL_TouchFingerEvent TouchFingerEvent;
typedef SDL_Scancode Keycode;
}
#else
namespace OgreBites {
typedef char Keycode;
enum {
    BUTTON_LEFT,
    BUTTON_MIDDLE,
    BUTTON_RIGHT,
};
struct KeyboardEvent {
    union {
        Keycode scancode;
    } keysym;
    int repeat;
};
struct MouseMotionEvent {
    int x, y;
    int xrel, yrel;
    int windowID;
};
struct MouseButtonEvent {
    int x, y;
    int button;
};
struct MouseWheelEvent {
    int y;
};

struct TouchFingerEvent {
    int type;
    int fingerId;
    float x, y;
    float dx, dy;
};
}

// SDL compat
enum {
    SDL_SCANCODE_A,
    SDL_SCANCODE_B,
    SDL_SCANCODE_C,
    SDL_SCANCODE_D,
    SDL_SCANCODE_E,
    SDL_SCANCODE_F,
    SDL_SCANCODE_G,
    SDL_SCANCODE_H,
    SDL_SCANCODE_Q,
    SDL_SCANCODE_R,
    SDL_SCANCODE_S,
    SDL_SCANCODE_T,
    SDL_SCANCODE_V,
    SDL_SCANCODE_W,
    SDL_SCANCODE_DOWN,
    SDL_SCANCODE_UP,
    SDL_SCANCODE_LEFT,
    SDL_SCANCODE_RIGHT,
    SDL_SCANCODE_SPACE,
    SDL_SCANCODE_PAGEUP,
    SDL_SCANCODE_PAGEDOWN,
    SDL_SCANCODE_LSHIFT,
    SDL_SCANCODE_F1,
    SDL_SCANCODE_F2,
    SDL_SCANCODE_F3,
    SDL_SCANCODE_F4,
    SDL_SCANCODE_F5,
    SDL_SCANCODE_F6,
    SDL_SCANCODE_F9,
    SDL_SCANCODE_F10,
    SDL_SCANCODE_F11,
    SDL_SCANCODE_F12,
    SDL_SCANCODE_EQUALS,
    SDL_SCANCODE_KP_PLUS,
    SDL_SCANCODE_KP_MINUS,
    SDL_SCANCODE_MINUS,
    SDL_SCANCODE_RETURN,
    SDL_SCANCODE_ESCAPE,
    SDL_NUM_SCANCODES = 512
};

enum {
    SDL_KEYDOWN,
    SDL_KEYUP,
    SDL_MOUSEBUTTONDOWN,
    SDL_MOUSEBUTTONUP,
    SDL_MOUSEWHEEL,
    SDL_MOUSEMOTION,
    SDL_FINGERDOWN,
    SDL_FINGERUP,
    SDL_FINGERMOTION,
    KMOD_CTRL,
};

typedef int SDL_Keymod;

inline unsigned char* SDL_GetKeyboardState(void*) {
    static unsigned char state[SDL_NUM_SCANCODES] = {0};
    return state;
}
inline int SDL_GetMouseState(int* x, int* y) {
    return 0;
}
inline SDL_Keymod SDL_GetModState() {
    return SDL_Keymod();
}
#endif

#endif /* SAMPLES_COMMON_INCLUDE_INPUT_H_ */
