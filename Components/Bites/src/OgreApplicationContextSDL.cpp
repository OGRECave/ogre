// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#include "OgreApplicationContext.h"

#include "OgreRoot.h"
#include "OgreRenderWindow.h"

#include <SDL.h>
#include <SDL_video.h>
#include <SDL_syswm.h>

#include "SDLInputMapping.h"

#include <algorithm>

namespace OgreBites {

static void initSDL()
{
    if (SDL_WasInit(SDL_INIT_VIDEO))
        return;

    if (SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt") > 0)
        Ogre::LogManager::getSingleton().logMessage("[SDL] gamecontrollerdb.txt loaded");

    SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
}

static void normalizeVideoMode(Ogre::ConfigOption& opt)
{
    if (std::find(opt.possibleValues.begin(), opt.possibleValues.end(), opt.currentValue) != opt.possibleValues.end())
        return;

    auto pos = opt.currentValue.find(" x ");
    if (pos == Ogre::String::npos)
        return;

    auto prefix = opt.currentValue.substr(0, pos + 3);
    for (const auto& mode : opt.possibleValues)
    {
        if (mode.compare(0, prefix.size(), prefix) == 0)
        {
            opt.currentValue = mode;
            return;
        }
    }
}

static void addVideoMode(const SDL_DisplayMode& mode,
                         std::vector<Ogre::String>& modes,
                         std::vector<Ogre::String>& frequencies)
{
    int bpp = SDL_BITSPERPIXEL(mode.format);
    if (bpp <= 0)
        bpp = 32;
    else if (bpp > 16)
        bpp = 32;

    auto desc = Ogre::StringUtil::format("%d x %d @ %d-bit colour", mode.w, mode.h, bpp);
    if (std::find(modes.begin(), modes.end(), desc) == modes.end())
        modes.push_back(desc);

    if (mode.refresh_rate > 0)
    {
        auto frequency = Ogre::StringConverter::toString(mode.refresh_rate);
        if (std::find(frequencies.begin(), frequencies.end(), frequency) == frequencies.end())
            frequencies.push_back(frequency);
    }
}

ApplicationContextSDL::ApplicationContextSDL(const Ogre::String& appName) : ApplicationContextBase(appName)
{
}

bool ApplicationContextSDL::oneTimeConfig()
{
    initSDL();

    std::vector<Ogre::String> modes;
    std::vector<Ogre::String> frequencies;

    for (int display = 0; display < SDL_GetNumVideoDisplays(); ++display)
    {
        SDL_DisplayMode desktop;
        if (SDL_GetDesktopDisplayMode(display, &desktop) == 0)
            addVideoMode(desktop, modes, frequencies);

        for (int i = 0; i < SDL_GetNumDisplayModes(display); ++i)
        {
            SDL_DisplayMode mode;
            if (SDL_GetDisplayMode(display, i, &mode) == 0)
                addVideoMode(mode, modes, frequencies);
        }
    }

    if (modes.empty())
    {
        Ogre::LogManager::getSingleton().logWarning("[SDL] no display modes found");
        return ApplicationContextBase::oneTimeConfig();
    }

    Ogre::LogManager::getSingleton().stream()
        << "[SDL] found " << modes.size() << " display modes across "
        << SDL_GetNumVideoDisplays() << " display(s)";

    for (auto rs : mRoot->getAvailableRenderers())
    {
        auto& options = const_cast<Ogre::ConfigOptionMap&>(rs->getConfigOptions());

        auto opt = options.find("Video Mode");
        if (opt == options.end())
            continue;

        opt->second.possibleValues = modes;
        if (std::find(modes.begin(), modes.end(), opt->second.currentValue) == modes.end())
            opt->second.currentValue = modes.front();

        opt = options.find("Display Frequency");
        if (opt != options.end())
        {
            opt->second.possibleValues = frequencies;
            if (!frequencies.empty() &&
                std::find(frequencies.begin(), frequencies.end(), opt->second.currentValue) == frequencies.end())
                opt->second.currentValue = frequencies.front();
        }
    }

    if (!ApplicationContextBase::oneTimeConfig())
        return false;

    // upgrade legacy config files that have no colour depth suffix
    for (auto rs : mRoot->getAvailableRenderers())
    {
        auto& options = const_cast<Ogre::ConfigOptionMap&>(rs->getConfigOptions());
        auto opt = options.find("Video Mode");
        if (opt != options.end())
            normalizeVideoMode(opt->second);
    }

    return true;
}

void ApplicationContextSDL::addInputListener(NativeWindowType* win, InputListener* lis)
{
    mInputListeners.insert(std::make_pair(SDL_GetWindowID(win), lis));
}


void ApplicationContextSDL::removeInputListener(NativeWindowType* win, InputListener* lis)
{
    mInputListeners.erase(std::make_pair(SDL_GetWindowID(win), lis));
}

NativeWindowPair ApplicationContextSDL::createWindow(const Ogre::String& name, Ogre::uint32 w, Ogre::uint32 h, Ogre::NameValuePairList miscParams)
{
    NativeWindowPair ret = {NULL, NULL};

    initSDL();

    auto p = mRoot->getRenderSystem()->getRenderWindowDescription();
    miscParams.insert(p.miscParams.begin(), p.miscParams.end());
    p.miscParams = miscParams;
    p.name = name;

    if(w > 0 && h > 0)
    {
        p.width = w;
        p.height= h;
    }

    int flags = p.useFullScreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE;
    int d = Ogre::StringConverter::parseInt(miscParams["monitorIndex"], 1) - 1;
    ret.native =
        SDL_CreateWindow(p.name.c_str(), SDL_WINDOWPOS_UNDEFINED_DISPLAY(d),
                         SDL_WINDOWPOS_UNDEFINED_DISPLAY(d), p.width, p.height, flags);

#if OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(ret.native, &wmInfo);
#endif

    // for tiny rendersystem
    p.miscParams["sdlwin"] = Ogre::StringConverter::toString(size_t(ret.native));

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
    if (wmInfo.subsystem == SDL_SYSWM_WAYLAND)
    {
#ifdef SDL_VIDEO_DRIVER_WAYLAND
        Ogre::LogManager::getSingleton().logMessage("[SDL] Creating Wayland window");
        p.miscParams["externalWlDisplay"] = Ogre::StringConverter::toString(size_t(wmInfo.info.wl.display));
        p.miscParams["externalWlSurface"] = Ogre::StringConverter::toString(size_t(wmInfo.info.wl.surface));
#endif
    }
    else if (wmInfo.subsystem == SDL_SYSWM_X11)
    {
#ifdef SDL_VIDEO_DRIVER_X11
        Ogre::LogManager::getSingleton().logMessage("[SDL] Creating X11 window");
        p.miscParams["externalWindowHandle"] = Ogre::StringConverter::toString(size_t(wmInfo.info.x11.window));
#endif
    }
#elif OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    p.miscParams["externalWindowHandle"] = Ogre::StringConverter::toString(size_t(wmInfo.info.win.window));
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE
    assert(wmInfo.subsystem == SDL_SYSWM_COCOA);
    p.miscParams["externalWindowHandle"] = Ogre::StringConverter::toString(size_t(wmInfo.info.cocoa.window));
#endif

    if(!mWindows.empty())
    {
        // additional windows should reuse the context
        p.miscParams["currentGLContext"] = "true";
    }

    ret.render = mRoot->createRenderWindow(p);
    mWindows.push_back(ret);
    return ret;
}

void ApplicationContextSDL::_destroyWindow(const NativeWindowPair& win)
{
    ApplicationContextBase::_destroyWindow(win);
    if(win.native)
        SDL_DestroyWindow(win.native);
}

void ApplicationContextSDL::setWindowGrab(NativeWindowType* win, bool _grab)
{
    SDL_bool grab = SDL_bool(_grab);

    SDL_SetWindowGrab(win, grab);
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE
    // osx workaround: mouse motion events are gone otherwise
    SDL_SetRelativeMouseMode(grab);
#else
    SDL_ShowCursor(!grab);
#endif
}

float ApplicationContextSDL::getDisplayDPI() const
{
    OgreAssert(!mWindows.empty(), "create a window first");
    float vdpi = -1;
    if(SDL_GetDisplayDPI(0, NULL, NULL, &vdpi) == 0 && vdpi > 0)
        return vdpi;

    return ApplicationContextBase::getDisplayDPI();
}

void ApplicationContextSDL::shutdown()
{
    ApplicationContextBase::shutdown();

    // the render system may still use the native display (e.g. EGL on Wayland),
    // so it has to be shut down before SDL closes the video subsystem
    if(mRoot)
    {
        mRoot->saveConfig();
        OGRE_DELETE mRoot;
        mRoot = NULL;
    }

    if(SDL_WasInit(SDL_INIT_VIDEO)) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
}

void ApplicationContextSDL::pollEvents()
{
    if(mWindows.empty())
    {
        // SDL events not initialized
        return;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            mRoot->queueEndRendering();
            break;
        case SDL_WINDOWEVENT:
            if(event.window.event != SDL_WINDOWEVENT_RESIZED)
                continue;

            for(auto & window : mWindows)
            {
                if(event.window.windowID != SDL_GetWindowID(window.native))
                    continue;

                Ogre::RenderWindow* win = window.render;
                win->resize(event.window.data1, event.window.data2);
                windowResized(win);
            }
            break;
        case SDL_JOYDEVICEADDED:
            if(!SDL_IsGameController(event.cdevice.which))
            {
                SDL_JoystickOpen(event.cdevice.which);
                Ogre::LogManager::getSingleton().logMessage("Opened Joystick");
            }
            break;
        case SDL_CONTROLLERDEVICEADDED:
            if(auto c = SDL_GameControllerOpen(event.cdevice.which))
            {
                const char* name = SDL_GameControllerName(c);
                Ogre::LogManager::getSingleton().stream() << "Opened Gamepad: " << (name ? name : "unnamed");
            }
            break;
        default:
            _fireInputEvent(convert(event), event.window.windowID);
            break;
        }
    }
}

}
