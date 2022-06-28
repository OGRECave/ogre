/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org/

  Copyright (c) 2000-2014 Torus Knot Software Ltd

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  -----------------------------------------------------------------------------
*/

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

#include "OgreGLXGLSupport.h"
#include "OgreGLXWindow.h"
#include "OgreGLXRenderTexture.h"

#include "OgreGLUtil.h"

#include "OgreX11.h"

#include <X11/extensions/Xrandr.h>

static bool ctxErrorOccurred = false;
static Ogre::String ctxErrorMessage;
static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    char buffer[512] = {0};
    ctxErrorOccurred = true;

    XGetErrorText(dpy, ev->error_code, buffer, 512);
    ctxErrorMessage = Ogre::String(buffer);
    return 0;
}

namespace Ogre
{
    struct GLXVideoMode
    {
        typedef std::pair<uint, uint>      ScreenSize;
        typedef short                      Rate;
        ScreenSize first;
        Rate second;

        GLXVideoMode() {}
        GLXVideoMode(const VideoMode& m) : first(m.width, m.height), second(m.refreshRate) {}

        bool operator!=(const GLXVideoMode& o) const
        {
            return first != o.first || second != o.second;
        }
    };
    typedef std::vector<GLXVideoMode>    GLXVideoModes;

    GLNativeSupport* getGLSupport(int profile)
    {
        return new GLXGLSupport(profile);
    }

    //-------------------------------------------------------------------------------------------------//
    GLXGLSupport::GLXGLSupport(int profile) : GLNativeSupport(profile), mGLDisplay(0), mXDisplay(0)
    {
        // A connection that might be shared with the application for GL rendering:
        mGLDisplay = getGLDisplay();

        // A connection that is NOT shared to enable independent event processing:
        mXDisplay  = getXDisplay();

        getXVideoModes(mXDisplay, mCurrentMode, mVideoModes);

        if(mVideoModes.empty())
        {
            mCurrentMode.width = DisplayWidth(mXDisplay, DefaultScreen(mXDisplay));
            mCurrentMode.height = DisplayHeight(mXDisplay, DefaultScreen(mXDisplay));
            mCurrentMode.refreshRate = 0;

            mVideoModes.push_back(mCurrentMode);
        }

        mOriginalMode = mCurrentMode;

        GLXFBConfig *fbConfigs;
        int config, nConfigs = 0;

        fbConfigs = chooseFBConfig(NULL, &nConfigs);

        for (config = 0; config < nConfigs; config++)
        {
            int caveat, samples;

            getFBConfigAttrib (fbConfigs[config], GLX_CONFIG_CAVEAT, &caveat);

            if (caveat != GLX_SLOW_CONFIG)
            {
                getFBConfigAttrib (fbConfigs[config], GLX_SAMPLES, &samples);
                mFSAALevels.push_back(samples);
            }
        }

        XFree (fbConfigs);
    }

    //-------------------------------------------------------------------------------------------------//
    GLXGLSupport::~GLXGLSupport()
    {
        if (mXDisplay)
            XCloseDisplay(mXDisplay);

        if (! mIsExternalDisplay && mGLDisplay)
            XCloseDisplay(mGLDisplay);
    }

    //-------------------------------------------------------------------------------------------------//
    RenderWindow* GLXGLSupport::newWindow(const String &name, unsigned int width, unsigned int height, bool fullScreen, const NameValuePairList *miscParams)
    {
        GLXWindow* window = new GLXWindow(this);

        window->create(name, width, height, fullScreen, miscParams);

        return window;
    }

    //-------------------------------------------------------------------------------------------------//
    GLPBuffer *GLXGLSupport::createPBuffer(PixelComponentType format, size_t width, size_t height)
    {
        return new GLXPBuffer(this, format, width, height);
    }

    //-------------------------------------------------------------------------------------------------//
    void GLXGLSupport::start() 
    {
        LogManager::getSingleton().logMessage(
            "******************************\n"
            "*** Starting GLX Subsystem ***\n"
            "******************************");
        initialiseExtensions();
    }

    //-------------------------------------------------------------------------------------------------//
    void GLXGLSupport::stop()
    {
        LogManager::getSingleton().logMessage(
            "******************************\n"
            "*** Stopping GLX Subsystem ***\n"
            "******************************");
    }

    //-------------------------------------------------------------------------------------------------//
    void* GLXGLSupport::getProcAddress(const char* procname) const {
        return (void*)glXGetProcAddressARB((const GLubyte*)procname);
    }

    //-------------------------------------------------------------------------------------------------//
    void GLXGLSupport::initialiseExtensions()
    {
        assert (mGLDisplay);

        mGLXVerMajor = mGLXVerMinor = 0;
        glXQueryVersion(mGLDisplay, &mGLXVerMajor, &mGLXVerMinor);

        const char* verStr = glXGetClientString(mGLDisplay, GLX_VERSION);
        LogManager::getSingleton().stream() << "GLX_VERSION = " << verStr;

        const char* extensionsString;

        // This is more realistic than using glXGetClientString:
        extensionsString = glXGetClientString(mGLDisplay, GLX_EXTENSIONS);

        LogManager::getSingleton().stream() << "GLX_EXTENSIONS = " << extensionsString;

        StringStream ext;
        String instr;

        ext << extensionsString;

        while(ext >> instr)
        {
            extensionList.insert(instr);
        }
    }

    //-------------------------------------------------------------------------------------------------//
    // Returns the FBConfig behind a GLXContext

    GLXFBConfig GLXGLSupport::getFBConfigFromContext(::GLXContext context)
    {
        GLXFBConfig fbConfig = 0;

        int fbConfigAttrib[] = {
            GLX_FBCONFIG_ID, 0,
            None
        };
        GLXFBConfig *fbConfigs;
        int nElements = 0;

        glXQueryContext(mGLDisplay, context, GLX_FBCONFIG_ID, &fbConfigAttrib[1]);
        fbConfigs = glXChooseFBConfig(mGLDisplay, DefaultScreen(mGLDisplay), fbConfigAttrib, &nElements);

        if (nElements)
        {
            fbConfig = fbConfigs[0];
            XFree(fbConfigs);
        }

        return fbConfig;
    }

    //-------------------------------------------------------------------------------------------------//
    // Returns the FBConfig behind a GLXDrawable, or returns 0 when
    //   missing GLX_SGIX_fbconfig and drawable is Window (unlikely), OR
    //   missing GLX_VERSION_1_3 and drawable is a GLXPixmap (possible).

    GLXFBConfig GLXGLSupport::getFBConfigFromDrawable(GLXDrawable drawable, unsigned int *width, unsigned int *height)
    {
        GLXFBConfig fbConfig = 0;

        int fbConfigAttrib[] = {
            GLX_FBCONFIG_ID, 0,
            None
        };
        GLXFBConfig *fbConfigs;
        int nElements = 0;

        glXQueryDrawable (mGLDisplay, drawable, GLX_FBCONFIG_ID, (unsigned int*)&fbConfigAttrib[1]);

        fbConfigs = glXChooseFBConfig(mGLDisplay, DefaultScreen(mGLDisplay), fbConfigAttrib, &nElements);

        if (nElements)
        {
            fbConfig = fbConfigs[0];
            XFree (fbConfigs);

            glXQueryDrawable(mGLDisplay, drawable, GLX_WIDTH, width);
            glXQueryDrawable(mGLDisplay, drawable, GLX_HEIGHT, height);
        }

        if (! fbConfig)
        {
            XWindowAttributes windowAttrib;

            if (XGetWindowAttributes(mGLDisplay, drawable, &windowAttrib))
            {
                VisualID visualid = XVisualIDFromVisual(windowAttrib.visual);

                fbConfig = getFBConfigFromVisualID(visualid);

                *width = windowAttrib.width;
                *height = windowAttrib.height;
            }
        }

        return fbConfig;
    }

    //-------------------------------------------------------------------------------------------------//
    // Finds a GLXFBConfig compatible with a given VisualID.

    GLXFBConfig GLXGLSupport::getFBConfigFromVisualID(VisualID visualid)
    {
        PFNGLXGETFBCONFIGFROMVISUALSGIXPROC glXGetFBConfigFromVisualSGIX = 
            (PFNGLXGETFBCONFIGFROMVISUALSGIXPROC)getProcAddress("glXGetFBConfigFromVisualSGIX");

        GLXFBConfig fbConfig = 0;

        XVisualInfo visualInfo;

        visualInfo.screen = DefaultScreen(mGLDisplay);
        visualInfo.depth = DefaultDepth(mGLDisplay, DefaultScreen(mGLDisplay));
        visualInfo.visualid = visualid;

        fbConfig = glXGetFBConfigFromVisualSGIX(mGLDisplay, &visualInfo);

        if (! fbConfig)
        {
            int minAttribs[] = {
                GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
                GLX_RENDER_TYPE,        GLX_RGBA_BIT,
                GLX_RED_SIZE,      1,
                GLX_BLUE_SIZE,    1,
                GLX_GREEN_SIZE,  1,
                None
            };
            int nConfigs = 0;

            GLXFBConfig *fbConfigs = chooseFBConfig(minAttribs, &nConfigs);

            for (int i = 0; i < nConfigs && ! fbConfig; i++)
            {
                XVisualInfo *vInfo = getVisualFromFBConfig(fbConfigs[i]);

                if (vInfo ->visualid == visualid)
                    fbConfig = fbConfigs[i];

                XFree(vInfo );
            }

            XFree(fbConfigs);
        }

        return fbConfig;
    }

    //-------------------------------------------------------------------------------------------------//
    // A helper class for the implementation of selectFBConfig

    class FBConfigAttribs
    {
    public:
        FBConfigAttribs(const int* attribs)
        {
            fields[GLX_CONFIG_CAVEAT] = GLX_NONE;

            for (int i = 0; attribs[2*i]; i++)
            {
                fields[attribs[2*i]] = attribs[2*i+1];
            }
        }

        void load(GLXGLSupport* const glSupport, GLXFBConfig fbConfig)
        {
            std::map<int,int>::iterator it;

            for (it = fields.begin(); it != fields.end(); it++)
            {
                it->second = 0;

                glSupport->getFBConfigAttrib(fbConfig, it->first, &it->second);
            }
        }

        bool operator>(FBConfigAttribs& alternative)
        {
            // Caveats are best avoided, but might be needed for anti-aliasing

            if (fields[GLX_CONFIG_CAVEAT] != alternative.fields[GLX_CONFIG_CAVEAT])
            {
                if (fields[GLX_CONFIG_CAVEAT] == GLX_SLOW_CONFIG)
                    return false;

                if (fields.find(GLX_SAMPLES) != fields.end() &&
                    fields[GLX_SAMPLES] < alternative.fields[GLX_SAMPLES])
                    return false;
            }

            std::map<int,int>::iterator it;

            for (it = fields.begin(); it != fields.end(); it++)
            {
                if (it->first != GLX_CONFIG_CAVEAT && fields[it->first] > alternative.fields[it->first])
                    return true;
            }

            return false;
        }

        std::map<int,int> fields;
    };

    //-------------------------------------------------------------------------------------------------//
    // Finds an FBConfig that possesses each of minAttribs and gets as close
    // as possible to each of the maxAttribs without exceeding them.
    // Resembles glXChooseFBConfig, but is forgiving to platforms
    // that do not support the attributes listed in the maxAttribs.

    GLXFBConfig GLXGLSupport::selectFBConfig (const int* minAttribs, const int *maxAttribs)
    {
        GLXFBConfig *fbConfigs;
        GLXFBConfig fbConfig = 0;
        int config, nConfigs = 0;

        fbConfigs = chooseFBConfig(minAttribs, &nConfigs);

        // this is a fix for cases where chooseFBConfig is not supported.
        // On the 10/2010 chooseFBConfig was not supported on VirtualBox
        // http://www.virtualbox.org/ticket/7195
        if (!nConfigs)
        {
            fbConfigs = glXGetFBConfigs(mGLDisplay, DefaultScreen(mGLDisplay), &nConfigs);
        }

        if (! nConfigs)
            return 0;

        fbConfig = fbConfigs[0];

        if (maxAttribs)
        {
            FBConfigAttribs maximum(maxAttribs);
            FBConfigAttribs best(maxAttribs);
            FBConfigAttribs candidate(maxAttribs);

            best.load(this, fbConfig);

            for (config = 1; config < nConfigs; config++)
            {
                candidate.load(this, fbConfigs[config]);

                if (candidate > maximum)
                    continue;

                if (candidate > best)
                {
                    fbConfig = fbConfigs[config];

                    best.load(this, fbConfig);
                }
            }
        }

        XFree (fbConfigs);
        return fbConfig;
    }

    //-------------------------------------------------------------------------------------------------//
    Display* GLXGLSupport::getGLDisplay(void)
    {
        if (! mGLDisplay)
        {
            //                  glXGetCurrentDisplay = (PFNGLXGETCURRENTDISPLAYPROC)getProcAddress("glXGetCurrentDisplay");

            mGLDisplay = glXGetCurrentDisplay();
            mIsExternalDisplay = true;

            if (! mGLDisplay)
            {
                mGLDisplay = XOpenDisplay(0);
                mIsExternalDisplay = false;
            }

            if(! mGLDisplay)
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Couldn`t open X display " + String((const char*)XDisplayName (0)), "GLXGLSupport::getGLDisplay");
            }
        }

        return mGLDisplay;
    }

    //-------------------------------------------------------------------------------------------------//
    Display* GLXGLSupport::getXDisplay(void)
    {
        if (! mXDisplay)
        {
            char* displayString = mGLDisplay ? DisplayString(mGLDisplay) : 0;

            mXDisplay = XOpenDisplay(displayString);

            if (! mXDisplay)
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Couldn`t open X display " + String((const char*)displayString), "GLXGLSupport::getXDisplay");
            }

            mAtomDeleteWindow = XInternAtom(mXDisplay, "WM_DELETE_WINDOW", True);
            mAtomFullScreen = XInternAtom(mXDisplay, "_NET_WM_STATE_FULLSCREEN", True);
            mAtomState = XInternAtom(mXDisplay, "_NET_WM_STATE", True);
        }

        return mXDisplay;
    }

    //-------------------------------------------------------------------------------------------------//
    String GLXGLSupport::getDisplayName(void)
    {
        return String((const char*)XDisplayName(DisplayString(mGLDisplay)));
    }

    //-------------------------------------------------------------------------------------------------//
    GLXFBConfig* GLXGLSupport::chooseFBConfig(const GLint *attribList, GLint *nElements)
    {
        GLXFBConfig *fbConfigs;

        fbConfigs = glXChooseFBConfig(mGLDisplay, DefaultScreen(mGLDisplay), attribList, nElements);

        return fbConfigs;
    }

    //-------------------------------------------------------------------------------------------------//
    ::GLXContext GLXGLSupport::createNewContext(GLXFBConfig fbConfig, GLint renderType, ::GLXContext shareList, GLboolean direct) const
    {
        ::GLXContext glxContext = NULL;

        int profile;
        int majorVersion;
        int minorVersion = 0;

        switch(mContextProfile) {
        case CONTEXT_COMPATIBILITY:
            profile = GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
            majorVersion = 1;
            break;
        case CONTEXT_ES:
            profile = GLX_CONTEXT_ES2_PROFILE_BIT_EXT;
            majorVersion = 2;
            break;
        default:
            profile = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
            majorVersion = 3;
            minorVersion = 3; // 3.1 would be sufficient per spec, but we need 3.3 anyway..
            break;
        }

        int context_attribs[] = {
                GLX_CONTEXT_MAJOR_VERSION_ARB, majorVersion,
                GLX_CONTEXT_MINOR_VERSION_ARB, minorVersion,
                GLX_CONTEXT_PROFILE_MASK_ARB, profile,
                None
        };

        ctxErrorOccurred = false;
        int (*oldHandler)(Display*, XErrorEvent*) =
            XSetErrorHandler(&ctxErrorHandler);

        PFNGLXCREATECONTEXTATTRIBSARBPROC _glXCreateContextAttribsARB;
        _glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)getProcAddress("glXCreateContextAttribsARB");

        if(_glXCreateContextAttribsARB)
        {
            // find maximal supported context version
            context_attribs[1] = 4;
            context_attribs[3] = 6;
            while(!glxContext &&
                  ((context_attribs[1] > majorVersion) ||
                   (context_attribs[1] == majorVersion && context_attribs[3] >= minorVersion)))
            {
                ctxErrorOccurred = false;
                glxContext = _glXCreateContextAttribsARB(mGLDisplay, fbConfig, shareList, direct, context_attribs);
                context_attribs[1] -= context_attribs[3] == 0; // only decrement if minor == 0
                context_attribs[3] = (context_attribs[3] - 1 + 7) % 7; // decrement: -1 -> 6
            }
        }
        else
        {
            // try old style context creation as a last resort
            // Needed at least by MESA 8.0.4 on Ubuntu 12.04.
            if (mContextProfile != CONTEXT_COMPATIBILITY) {
                ctxErrorMessage = "Can not set a context profile";
            } else {
                glxContext =
                    glXCreateNewContext(mGLDisplay, fbConfig, renderType, shareList, direct);
            }
        }

        // Sync to ensure any errors generated are processed.
        XSync( mGLDisplay, False );

        // Restore the original error handler
        XSetErrorHandler( oldHandler );

        if (ctxErrorOccurred || !glxContext)
        {
            LogManager::getSingleton().logError("Failed to create an OpenGL context - " + ctxErrorMessage);
        }

        return glxContext;
    }

    //-------------------------------------------------------------------------------------------------//
    GLint GLXGLSupport::getFBConfigAttrib(GLXFBConfig fbConfig, GLint attribute, GLint *value)
    {
        GLint status;

        status = glXGetFBConfigAttrib(mGLDisplay, fbConfig, attribute, value);

        return status;
    }

    //-------------------------------------------------------------------------------------------------//
    XVisualInfo* GLXGLSupport::getVisualFromFBConfig(GLXFBConfig fbConfig)
    {
        XVisualInfo *visualInfo;

        visualInfo = glXGetVisualFromFBConfig(mGLDisplay, fbConfig);

        return visualInfo;
    }

    //-------------------------------------------------------------------------------------------------//
    void GLXGLSupport::switchMode(uint& width, uint& height, short& frequency)
    {
        int size = 0;
        int newSize = -1;

        GLXVideoModes glxVideoModes(mVideoModes.begin(), mVideoModes.end());

        GLXVideoModes::iterator mode;
        GLXVideoModes::iterator end = glxVideoModes.end();
        GLXVideoMode *newMode = 0;

        for(mode = glxVideoModes.begin(); mode != end; size++)
        {
            if (mode->first.first >= width &&
                mode->first.second >= height)
            {
                if (! newMode ||
                    mode->first.first < newMode->first.first ||
                    mode->first.second < newMode->first.second)
                {
                    newSize = size;
                    newMode = &(*mode);
                }
            }

            GLXVideoMode *lastMode = &(*mode);

            while (++mode != end && mode->first == lastMode->first)
            {
                if (lastMode == newMode && mode->second == frequency)
                {
                    newMode = &(*mode);
                }
            }
        }

        if (newMode && *newMode != mCurrentMode)
        {
            XRRScreenConfiguration *screenConfig = XRRGetScreenInfo (mXDisplay, DefaultRootWindow(mXDisplay));

            if (screenConfig)
            {
                Rotation currentRotation;

                XRRConfigCurrentConfiguration (screenConfig, &currentRotation);

                XRRSetScreenConfigAndRate(mXDisplay, screenConfig, DefaultRootWindow(mXDisplay), newSize, currentRotation, newMode->second, CurrentTime);

                XRRFreeScreenConfigInfo(screenConfig);

                mCurrentMode = {newMode->first.first, newMode->first.second, newMode->second};

                LogManager::getSingleton().logMessage("Entered video mode " + mCurrentMode.getDescription() + " @ " + StringConverter::toString(mCurrentMode.refreshRate) + "Hz");
            }
        }
    }

    //-------------------------------------------------------------------------------------------------//
    void GLXGLSupport::switchMode(void)
    {
        return switchMode(mOriginalMode.width, mOriginalMode.height, mOriginalMode.refreshRate);
    }
}
