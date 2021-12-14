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

// SDL2 keycode to scancode
static int kc2sc(int kc)
{
    return kc & ~(1 << 30);
}

namespace OgreBites
{

static int keypad2kc(int sym, int mod)
{
    if (sym < SDLK_KP_1 || sym > SDLK_KP_PERIOD)
        return sym;
    bool numlock = (mod & KMOD_NUM) != 0;
    if (numlock)
        return sym;
    if (sym == SDLK_KP_1)
        sym = SDLK_END;
    else if (sym == SDLK_KP_2)
        sym = SDLK_DOWN;
    else if (sym == SDLK_KP_3)
        sym = SDLK_PAGEDOWN;
    else if (sym == SDLK_KP_4)
        sym = SDLK_LEFT;
    else if (sym == SDLK_KP_6)
        sym = SDLK_RIGHT;
    else if (sym == SDLK_KP_7)
        sym = SDLK_HOME;
    else if (sym == SDLK_KP_8)
        sym = SDLK_UP;
    else if (sym == SDLK_KP_9)
        sym = SDLK_PAGEUP;
    else if (sym == SDLK_KP_0)
        sym = SDLK_INSERT;
    else if (sym == SDLK_KP_PERIOD)
        sym = SDLK_DELETE;
    return sym;
}

ImGuiInputListener::ImGuiInputListener()
{
    ImGuiIO& io = ImGui::GetIO();
    // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will
    // update during the application lifetime.
    io.KeyMap[ImGuiKey_Tab] = '\t';
    io.KeyMap[ImGuiKey_LeftArrow] = kc2sc(SDLK_LEFT);
    io.KeyMap[ImGuiKey_RightArrow] = kc2sc(SDLK_RIGHT);
    io.KeyMap[ImGuiKey_UpArrow] = kc2sc(SDLK_UP);
    io.KeyMap[ImGuiKey_DownArrow] = kc2sc(SDLK_DOWN);
    io.KeyMap[ImGuiKey_PageUp] = kc2sc(SDLK_PAGEUP);
    io.KeyMap[ImGuiKey_PageDown] = kc2sc(SDLK_PAGEDOWN);
    io.KeyMap[ImGuiKey_Home] = kc2sc(SDLK_HOME);
    io.KeyMap[ImGuiKey_End] = kc2sc(SDLK_END);
    io.KeyMap[ImGuiKey_Insert] = kc2sc(SDLK_INSERT);
    io.KeyMap[ImGuiKey_Delete] = kc2sc(SDLK_DELETE);
    io.KeyMap[ImGuiKey_Backspace] = '\b';
    io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = '\033';
    io.KeyMap[ImGuiKey_Space] = ' ';
    io.KeyMap[ImGuiKey_KeyPadEnter] = kc2sc(SDLK_KP_ENTER);
    io.KeyMap[ImGuiKey_A] = 'a';
    io.KeyMap[ImGuiKey_C] = 'c';
    io.KeyMap[ImGuiKey_V] = 'v';
    io.KeyMap[ImGuiKey_X] = 'x';
    io.KeyMap[ImGuiKey_Y] = 'y';
    io.KeyMap[ImGuiKey_Z] = 'z';
}

bool ImGuiInputListener::mouseWheelRolled(const MouseWheelEvent& arg)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel = Ogre::Math::Sign(arg.y);
    return io.WantCaptureMouse;
}

bool ImGuiInputListener::mouseMoved(const MouseMotionEvent& arg)
{

    ImGuiIO& io = ImGui::GetIO();

    io.MousePos.x = arg.x;
    io.MousePos.y = arg.y;

    return io.WantCaptureMouse;
}

bool ImGuiInputListener::mousePressed(const MouseButtonEvent& arg)
{
    ImGuiIO& io = ImGui::GetIO();
    int b = sdl2imgui(arg.button);
    if (b < 5)
    {
        io.MouseDown[b] = true;
    }
    return io.WantCaptureMouse;
}
bool ImGuiInputListener::mouseReleased(const MouseButtonEvent& arg)
{
    ImGuiIO& io = ImGui::GetIO();
    int b = sdl2imgui(arg.button);
    if (b < 5)
    {
        io.MouseDown[b] = false;
    }
    return io.WantCaptureMouse;
}
bool ImGuiInputListener::keyEvent (const KeyboardEvent& arg)
{
    ImGuiIO& io = ImGui::GetIO ();
    int sym = keypad2kc (arg.keysym.sym, arg.keysym.mod);
    int key = kc2sc (sym);
    io.KeysDown[key] = (arg.type == OgreBites::KEYDOWN);
    io.KeyShift = (arg.keysym.mod & KMOD_SHIFT) != 0;
    io.KeyCtrl = (arg.keysym.mod & KMOD_CTRL) != 0;
    io.KeyAlt = (arg.keysym.mod & KMOD_ALT) != 0;
    io.KeySuper = (arg.keysym.mod & KMOD_GUI) != 0;
    return io.WantCaptureKeyboard;
}
bool ImGuiInputListener::keyPressed(const KeyboardEvent& arg)
{
    return keyEvent (arg);
}
bool ImGuiInputListener::keyReleased(const KeyboardEvent& arg)
{
    return keyEvent (arg);
}
bool ImGuiInputListener::textInput (const TextInputEvent& evt)
{
    ImGuiIO& io = ImGui::GetIO ();
    io.AddInputCharactersUTF8 (evt.chars);
    return true;
}
bool ImGuiInputListener::buttonPressed(const ButtonEvent& evt)
{
    ImGuiIO& io = ImGui::GetIO();
    switch(evt.button)
    {
    case 0:
        io.NavInputs[ImGuiNavInput_Activate] = 1;
        break;
    case 1:
        io.NavInputs[ImGuiNavInput_Cancel] = 1;
        break;
    case 2:
        io.NavInputs[ImGuiNavInput_Menu] = 1;
        break;
    case 11:
        io.NavInputs[ImGuiNavInput_DpadUp] = 1;
        break;
    case 12:
        io.NavInputs[ImGuiNavInput_DpadDown] = 1;
        break;
    case 13:
        io.NavInputs[ImGuiNavInput_DpadLeft] = 1;
        break;
    case 14:
        io.NavInputs[ImGuiNavInput_DpadRight] = 1;
        break;
    }
    return true;
}

} // namespace OgreBites
