/*
 * Input.h
 *
 *  Created on: 05.12.2015
 *      Author: pavel
 */

#ifndef SAMPLES_COMMON_INCLUDE_INPUT_H_
#define SAMPLES_COMMON_INCLUDE_INPUT_H_

#include <OgreBitesPrerequisites.h>

namespace Ogre {
    struct FrameEvent;
}

/** \addtogroup Optional
*  @{
*/
/** \addtogroup Bites
*  @{
*/
namespace OgreBites {

enum ButtonType {
    BUTTON_LEFT = 1,
    BUTTON_MIDDLE,
    BUTTON_RIGHT,
};

enum EventType {
    KEYDOWN = 1,
    KEYUP,
    MOUSEBUTTONDOWN,
    MOUSEBUTTONUP,
    MOUSEWHEEL,
    MOUSEMOTION,
    FINGERDOWN,
    FINGERUP,
    FINGERMOTION,
};

typedef int Keycode;

struct Keysym {
    Keycode sym;
    unsigned short mod;
};

struct KeyboardEvent {
    int type;
    Keysym keysym;
    unsigned char repeat;
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
    unsigned char button;
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

// SDL compat
enum {
    SDLK_RETURN = int('\r'),
    SDLK_ESCAPE = int('\033'),
    SDLK_SPACE = int(' '),
    SDLK_F1 = (1 << 30) | 58,
    SDLK_F2,
    SDLK_F3,
    SDLK_F4,
    SDLK_F5,
    SDLK_F6,
    SDLK_F7,
    SDLK_F8,
    SDLK_F9,
    SDLK_F10,
    SDLK_F11,
    SDLK_F12,
    SDLK_PAGEUP = (1 << 30) | 75,
    SDLK_PAGEDOWN = (1 << 30) | 77,
    SDLK_RIGHT = (1 << 30) | 79,
    SDLK_LEFT,
    SDLK_DOWN,
    SDLK_UP,
    SDLK_KP_MINUS = (1 << 30) | 86,
    SDLK_KP_PLUS,
    SDLK_LSHIFT = (1 << 30) | 225,
    KMOD_CTRL = 0x0040 | 0x0080,
};

/**
the return values of the callbacks are ignored by ApplicationContext
however they can be used to control event propagation in a hierarchy
*/
struct _OgreBitesExport InputListener {
    virtual ~InputListener() {}
    virtual void frameRendered(const Ogre::FrameEvent& evt) { }
    virtual bool keyPressed(const KeyboardEvent& evt) { return true;}
    virtual bool keyReleased(const KeyboardEvent& evt) { return true; }
    virtual bool touchMoved(const TouchFingerEvent& evt) { return true; }
    virtual bool touchPressed(const TouchFingerEvent& evt) { return true; }
    virtual bool touchReleased(const TouchFingerEvent& evt) { return true; }
    virtual bool mouseMoved(const MouseMotionEvent& evt) { return true; }
    virtual bool mouseWheelRolled(const MouseWheelEvent& evt) { return true; }
    virtual bool mousePressed(const MouseButtonEvent& evt) { return true; }
    virtual bool mouseReleased(const MouseButtonEvent& evt) { return true; }
};
}
/** @} */
/** @} */

#endif /* SAMPLES_COMMON_INCLUDE_INPUT_H_ */
