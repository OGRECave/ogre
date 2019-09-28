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
    io.KeyMap[ImGuiKey_Home] = -1;
    io.KeyMap[ImGuiKey_End] = -1;
    io.KeyMap[ImGuiKey_Delete] = -1;
    io.KeyMap[ImGuiKey_Backspace] = '\b';
    io.KeyMap[ImGuiKey_Enter] = '\r';
    io.KeyMap[ImGuiKey_Escape] = '\033';
    io.KeyMap[ImGuiKey_Space] = ' ';
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
bool ImGuiInputListener::keyPressed(const KeyboardEvent& arg)
{
    ImGuiIO& io = ImGui::GetIO();

    // ignore
    if (arg.keysym.sym == SDLK_LSHIFT)
        return io.WantCaptureKeyboard;

    io.KeyCtrl = arg.keysym.mod & KMOD_CTRL;
    io.KeyShift = arg.keysym.mod & SDLK_LSHIFT;

    int key = kc2sc(arg.keysym.sym);

    if (key > 0 && key < 512)
    {
        io.KeysDown[key] = true;

        uint16_t sym = arg.keysym.sym;
        if (io.KeyShift)
            sym -= 32;
        io.AddInputCharacter(sym);
    }

    return io.WantCaptureKeyboard;
}
bool ImGuiInputListener::keyReleased(const KeyboardEvent& arg)
{
    int key = kc2sc(arg.keysym.sym);
    if (key < 0 || key >= 512)
        return true;

    ImGuiIO& io = ImGui::GetIO();

    io.KeyCtrl = arg.keysym.mod & KMOD_CTRL;
    io.KeyShift = arg.keysym.mod & SDLK_LSHIFT;

    io.KeysDown[key] = false;
    return io.WantCaptureKeyboard;
}
} // namespace OgreBites
