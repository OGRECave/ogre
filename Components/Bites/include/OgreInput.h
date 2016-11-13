/*
 * Input.h
 *
 *  Created on: 05.12.2015
 *      Author: pavel
 */

#ifndef SAMPLES_COMMON_INCLUDE_INPUT_H_
#define SAMPLES_COMMON_INCLUDE_INPUT_H_

#include <OgreBuildSettings.h>

#if OGRE_BITES_HAVE_SDL
#include <SDL.h>
#endif

/** \addtogroup Optional
*  @{
*/
/** \addtogroup Bites
*  @{
*/
#if OGRE_BITES_HAVE_SDL
namespace OgreBites {
enum {
    BUTTON_LEFT = SDL_BUTTON_LEFT,
    BUTTON_MIDDLE = SDL_BUTTON_MIDDLE,
    BUTTON_RIGHT = SDL_BUTTON_RIGHT,
};

typedef SDL_Event Event;
typedef SDL_KeyboardEvent KeyboardEvent;
typedef SDL_MouseMotionEvent MouseMotionEvent;
typedef SDL_MouseButtonEvent MouseButtonEvent;
typedef SDL_MouseWheelEvent MouseWheelEvent;
typedef SDL_TouchFingerEvent TouchFingerEvent;
typedef SDL_Keycode Keycode;
}
#else
namespace OgreBites {
typedef int Keycode;
enum {
    BUTTON_LEFT,
    BUTTON_MIDDLE,
    BUTTON_RIGHT,
};
struct KeyboardEvent {
    int type;
    union {
        Keycode sym;
    } keysym;
    int repeat;
};
struct MouseMotionEvent {
    int type;
    int x, y;
    int xrel, yrel;
    int windowID;
};
struct MouseButtonEvent {
    int type;
    int x, y;
    int button;
};
struct MouseWheelEvent {
    int type;
    int y;
};
struct TouchFingerEvent {
    int type;
    int fingerId;
    float x, y;
    float dx, dy;
};

union Event
{
    int type;
    KeyboardEvent key;
    MouseButtonEvent button;
    MouseWheelEvent wheel;
    MouseMotionEvent motion;
    TouchFingerEvent tfinger;
};
}
/** @} */
/** @} */

// SDL compat
enum {
    SDLK_RETURN = '\r',
    SDLK_ESCAPE = '\033',
    SDLK_SPACE = ' ',
    SDLK_DOWN,
    SDLK_UP,
    SDLK_LEFT,
    SDLK_RIGHT,
    SDLK_PAGEUP,
    SDLK_PAGEDOWN,
    SDLK_LSHIFT,
    SDLK_F1,
    SDLK_F2,
    SDLK_F3,
    SDLK_F4,
    SDLK_F5,
    SDLK_F6,
    SDLK_F9,
    SDLK_F10,
    SDLK_F11,
    SDLK_F12,
    SDLK_KP_PLUS,
    SDLK_KP_MINUS,
    SDL_NUM_KEYCODES = 512
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
    static unsigned char state[SDL_NUM_KEYCODES] = {0};
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
