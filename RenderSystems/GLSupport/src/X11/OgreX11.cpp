#include <OgreX11.h>
#include <X11/extensions/Xrandr.h>

#include <OgreLogManager.h>

namespace {
    int safeXErrorHandler (Display *display, XErrorEvent *event)
    {
        // Ignore all XErrorEvents
        return 0;
    }
    int (*oldXErrorHandler)(Display *, XErrorEvent*);
}

namespace Ogre
{
Display* getXDisplay(Display* glDisplay, Atom& deleteWindow, Atom& fullScreen, Atom& state)
{
    char* displayString = glDisplay ? DisplayString(glDisplay) : NULL;

    auto xDisplay = XOpenDisplay(displayString);

    if (!xDisplay)
        return xDisplay;

    deleteWindow = XInternAtom(xDisplay, "WM_DELETE_WINDOW", True);
    fullScreen = XInternAtom(xDisplay, "_NET_WM_STATE_FULLSCREEN", True);
    state = XInternAtom(xDisplay, "_NET_WM_STATE", True);
    return xDisplay;
}

void validateParentWindow(Display* display, Window parentWindow)
{
    // Ignore fatal XErrorEvents during parameter validation:
    oldXErrorHandler = XSetErrorHandler(safeXErrorHandler);

    // Validate parentWindowHandle
    if (parentWindow != DefaultRootWindow(display))
    {
        XWindowAttributes windowAttrib;

        if (!XGetWindowAttributes(display, parentWindow, &windowAttrib) ||
            windowAttrib.root != DefaultRootWindow(display))
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Invalid parentWindowHandle (wrong server or screen)");
        }
    }

    XSetErrorHandler(oldXErrorHandler);
}

Window createXWindow(Display* display, Window parent, XVisualInfo* visualInfo, int& left, int& top, uint& width,
                     uint& height, Atom wmFullScreen, bool fullScreen)
{
    XSetWindowAttributes attr;
    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap(display, DefaultRootWindow(display), visualInfo->visual, AllocNone);
    attr.event_mask = StructureNotifyMask | VisibilityChangeMask | FocusChangeMask;

    ulong mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    if (fullScreen && wmFullScreen == None)
    {
        LogManager::getSingleton().logWarning("createXWindow: Your WM has no fullscreen support");

        // A second best approach for outdated window managers
        attr.backing_store = NotUseful;
        attr.save_under = False;
        attr.override_redirect = True;
        mask |= CWSaveUnder | CWBackingStore | CWOverrideRedirect;
        left = top = 0;
    }

    auto window = XCreateWindow(display, parent, left, top, width, height, 0, visualInfo->depth, InputOutput,
                                visualInfo->visual, mask, &attr);
    XFree(visualInfo);

    if (!window)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to create an X Window");
    }

    return window;
}

void destroyXWindow(Display* display, Window window)
{
    // Ignore fatal XErrorEvents from stale handles.
    oldXErrorHandler = XSetErrorHandler(safeXErrorHandler);
    XDestroyWindow(display, window);
    XSetErrorHandler(oldXErrorHandler);
}

void queryRect(Display* display, Window window, int& left, int& top, uint& width, uint& height, bool queryOffset)
{
    XWindowAttributes windowAttrib;

    Window parent, root, *children;
    uint nChildren;

    XQueryTree(display, window, &root, &parent, &children, &nChildren);

    if (children)
        XFree(children);

    XGetWindowAttributes(display, parent, &windowAttrib);

    if (queryOffset)
    {
        // offset from window decorations
        left = windowAttrib.x;
        top  = windowAttrib.y;
        // w/ h of the actual renderwindow
        XGetWindowAttributes(display, window, &windowAttrib);
    }

    width  = windowAttrib.width;
    height = windowAttrib.height;
}

void finaliseTopLevel(Display* display, Window window, int& left, int& top, uint& width, uint& height, String& title, Atom wmDelete)
{
    XWMHints* wmHints;
    XSizeHints* sizeHints;

    // Is this really necessary ? Which broken WM might need it?
    if ((wmHints = XAllocWMHints()) != NULL)
    {
        wmHints->initial_state = NormalState;
        wmHints->input = True;
        wmHints->flags = StateHint | InputHint;
    }

    // Is this really necessary ? Which broken WM might need it?
    if ((sizeHints = XAllocSizeHints()) != NULL)
    {
        sizeHints->flags = USPosition;
    }

    XTextProperty titleprop;
    char* lst = const_cast<char*>(title.c_str());
    XStringListToTextProperty(&lst, 1, &titleprop);
    XSetWMProperties(display, window, &titleprop, NULL, NULL, 0, sizeHints, wmHints, NULL);

    XFree(titleprop.value);
    XFree(wmHints);
    XFree(sizeHints);

    XSetWMProtocols(display, window, &wmDelete, 1);

    XWindowAttributes windowAttrib;

    XGetWindowAttributes(display, window, &windowAttrib);

    left = windowAttrib.x;
    top = windowAttrib.y;
    width = windowAttrib.width;
    height = windowAttrib.height;
}

bool getXVideoModes(Display* display, VideoMode& currentMode, VideoModes& videoModes)
{
    int dummy;
    if (!XQueryExtension(display, "RANDR", &dummy, &dummy, &dummy))
        return false;

    auto screenConfig = XRRGetScreenInfo(display, DefaultRootWindow(display));
    if (!screenConfig)
        return false;

    XRRScreenSize* screenSizes;
    int nSizes = 0;
    Rotation currentRotation;
    int currentSizeID = XRRConfigCurrentConfiguration(screenConfig, &currentRotation);

    screenSizes = XRRConfigSizes(screenConfig, &nSizes);

    currentMode.width = screenSizes[currentSizeID].width;
    currentMode.height = screenSizes[currentSizeID].height;
    currentMode.refreshRate = XRRConfigCurrentRate(screenConfig);

    for (int sizeID = 0; sizeID < nSizes; sizeID++)
    {
        short* rates;
        int nRates = 0;

        rates = XRRConfigRates(screenConfig, sizeID, &nRates);

        for (int rate = 0; rate < nRates; rate++)
        {
            VideoMode mode = {};

            mode.width = screenSizes[sizeID].width;
            mode.height = screenSizes[sizeID].height;
            mode.refreshRate = rates[rate];

            videoModes.push_back(mode);
        }
    }
    XRRFreeScreenConfigInfo(screenConfig);

    return true;
}
} // namespace Ogre