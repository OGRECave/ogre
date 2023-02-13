// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#include <OgreImGuiInputListener.h>

#include <OgreMath.h>

#include <imgui.h>

// map sdl2 mouse buttons to imgui
static int sdl2imgui(int b)
{
    switch (b)
    {
    case 2:
        return 2;
    case 3:
        return 1;
    default:
        return b - 1;
    }
}

namespace OgreBites
{
static ImGuiKey ImGui_ImplSDL2_KeycodeToImGuiKey(int keycode)
{
    switch (keycode)
    {
        case '\t': return ImGuiKey_Tab;
        case SDLK_LEFT: return ImGuiKey_LeftArrow;
        case SDLK_RIGHT: return ImGuiKey_RightArrow;
        case SDLK_UP: return ImGuiKey_UpArrow;
        case SDLK_DOWN: return ImGuiKey_DownArrow;
        case SDLK_PAGEUP: return ImGuiKey_PageUp;
        case SDLK_PAGEDOWN: return ImGuiKey_PageDown;
        case SDLK_HOME: return ImGuiKey_Home;
        case SDLK_END: return ImGuiKey_End;
        case SDLK_INSERT: return ImGuiKey_Insert;
        case SDLK_DELETE: return ImGuiKey_Delete;
        case '\b': return ImGuiKey_Backspace;
        case SDLK_SPACE: return ImGuiKey_Space;
        case SDLK_RETURN: return ImGuiKey_Enter;
        case SDLK_ESCAPE: return ImGuiKey_Escape;
        case '\'': return ImGuiKey_Apostrophe;
        case ',': return ImGuiKey_Comma;
        case '-': return ImGuiKey_Minus;
        case '.': return ImGuiKey_Period;
        case '/': return ImGuiKey_Slash;
        case ';': return ImGuiKey_Semicolon;
        case '=': return ImGuiKey_Equal;
        case '[': return ImGuiKey_LeftBracket;
        case '\\': return ImGuiKey_Backslash;
        case ']': return ImGuiKey_RightBracket;
        case '`': return ImGuiKey_GraveAccent;
        //case SDLK_CAPSLOCK: return ImGuiKey_CapsLock;
        case SDLK_SCROLLLOCK: return ImGuiKey_ScrollLock;
        case SDLK_NUMLOCKCLEAR: return ImGuiKey_NumLock;
        case SDLK_PRINTSCREEN: return ImGuiKey_PrintScreen;
        case SDLK_PAUSE: return ImGuiKey_Pause;
        case SDLK_KP_0: return ImGuiKey_Keypad0;
        case SDLK_KP_1: return ImGuiKey_Keypad1;
        case SDLK_KP_2: return ImGuiKey_Keypad2;
        case SDLK_KP_3: return ImGuiKey_Keypad3;
        case SDLK_KP_4: return ImGuiKey_Keypad4;
        case SDLK_KP_5: return ImGuiKey_Keypad5;
        case SDLK_KP_6: return ImGuiKey_Keypad6;
        case SDLK_KP_7: return ImGuiKey_Keypad7;
        case SDLK_KP_8: return ImGuiKey_Keypad8;
        case SDLK_KP_9: return ImGuiKey_Keypad9;
        case SDLK_KP_PERIOD: return ImGuiKey_KeypadDecimal;
        case SDLK_KP_DIVIDE: return ImGuiKey_KeypadDivide;
        case SDLK_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case SDLK_KP_MINUS: return ImGuiKey_KeypadSubtract;
        case SDLK_KP_PLUS: return ImGuiKey_KeypadAdd;
        case SDLK_KP_ENTER: return ImGuiKey_KeypadEnter;
        //case SDLK_KP_EQUALS: return ImGuiKey_KeypadEqual;
        //case SDLK_LCTRL: return ImGuiKey_LeftCtrl;
        case SDLK_LSHIFT: return ImGuiKey_LeftShift;
        /*case SDLK_LALT: return ImGuiKey_LeftAlt;
        case SDLK_LGUI: return ImGuiKey_LeftSuper;
        case SDLK_RCTRL: return ImGuiKey_RightCtrl;
        case SDLK_RSHIFT: return ImGuiKey_RightShift;
        case SDLK_RALT: return ImGuiKey_RightAlt;
        case SDLK_RGUI: return ImGuiKey_RightSuper;
        case SDLK_APPLICATION: return ImGuiKey_Menu;*/
    }
    return ImGuiKey_None;
}

static bool keyEvent(const KeyboardEvent& arg)
{
    ImGuiIO& io = ImGui::GetIO();

    int key = arg.keysym.sym;

    if(key >= '0' && key <= '9')
        key = key - '0' + ImGuiKey_0;
    else if(key >= 'a' && key <= 'z')
        key = key - 'a' + ImGuiKey_A;
    else if(key >= SDLK_F1 && key <= SDLK_F12)
        key = key - SDLK_F1 + ImGuiKey_F1;
    else
        key = ImGui_ImplSDL2_KeycodeToImGuiKey(key);

    io.AddKeyEvent(ImGuiMod_Shift, arg.keysym.mod & KMOD_SHIFT);
    io.AddKeyEvent(ImGuiMod_Ctrl, arg.keysym.mod & KMOD_CTRL);
    io.AddKeyEvent(ImGuiMod_Alt, arg.keysym.mod & KMOD_ALT);
    io.AddKeyEvent(ImGuiMod_Super, arg.keysym.mod & KMOD_GUI);

    if (key == ImGuiKey_None)
        return io.WantCaptureKeyboard;

    io.AddKeyEvent(ImGuiKey(key), arg.type == OgreBites::KEYDOWN);
    return io.WantCaptureKeyboard;
}

static bool buttonEvent(const ButtonEvent& evt)
{
    ImGuiIO& io = ImGui::GetIO();
    bool down = evt.type == OgreBites::CONTROLLERBUTTONDOWN;
    switch(evt.button)
    {
    case 0:
        io.AddKeyEvent(ImGuiKey_GamepadFaceDown, down);
        break;
    case 1:
        io.AddKeyEvent(ImGuiKey_GamepadFaceRight, down);
        break;
    case 2:
        io.AddKeyEvent(ImGuiKey_GamepadFaceLeft, down);
        break;
    case 11:
        io.AddKeyEvent(ImGuiKey_GamepadDpadUp, down);
        break;
    case 12:
        io.AddKeyEvent(ImGuiKey_GamepadDpadDown, down);
        break;
    case 13:
        io.AddKeyEvent(ImGuiKey_GamepadDpadLeft, down);
        break;
    case 14:
        io.AddKeyEvent(ImGuiKey_GamepadDpadRight, down);
        break;
    }
    return true;
}

ImGuiInputListener::ImGuiInputListener()
{
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
}

bool ImGuiInputListener::mouseWheelRolled(const MouseWheelEvent& arg)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseWheelEvent(0.f, Ogre::Math::Sign(arg.y));
    return io.WantCaptureMouse;
}

bool ImGuiInputListener::mouseMoved(const MouseMotionEvent& arg)
{

    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(arg.x, arg.y);
    return io.WantCaptureMouse;
}

bool ImGuiInputListener::mousePressed(const MouseButtonEvent& arg)
{
    ImGuiIO& io = ImGui::GetIO();
    int b = sdl2imgui(arg.button);
    if (b < 5)
    {
        io.AddMouseButtonEvent(b, true);
    }
    return io.WantCaptureMouse;
}
bool ImGuiInputListener::mouseReleased(const MouseButtonEvent& arg)
{
    ImGuiIO& io = ImGui::GetIO();
    int b = sdl2imgui(arg.button);
    if (b < 5)
    {
        io.AddMouseButtonEvent(b, false);
    }
    return io.WantCaptureMouse;
}
bool ImGuiInputListener::textInput (const TextInputEvent& evt)
{
    ImGuiIO& io = ImGui::GetIO ();
    io.AddInputCharactersUTF8 (evt.chars);
    return true;
}
bool ImGuiInputListener::keyPressed(const KeyboardEvent& arg) { return keyEvent(arg); }
bool ImGuiInputListener::keyReleased(const KeyboardEvent& arg) { return keyEvent(arg); }
bool ImGuiInputListener::buttonPressed(const ButtonEvent& evt) { return buttonEvent(evt); }
bool ImGuiInputListener::buttonReleased(const ButtonEvent& evt) { return buttonEvent(evt); }
} // namespace OgreBites
