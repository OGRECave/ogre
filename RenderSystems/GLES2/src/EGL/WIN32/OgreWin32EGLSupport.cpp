/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#include "OgreGLES2Prerequisites.h"
#include "OgreGLES2RenderSystem.h"

#include "OgreWin32EGLSupport.h"
#include "OgreWin32EGLWindow.h"
#include "OgreWin32EGLRenderTexture.h"
#include "OgreWin32EGLContext.h"



namespace Ogre {


    Win32EGLSupport::Win32EGLSupport()
    {
        //RECT windowRect;
        //GetClientRect(mNativeDisplay, &windowRect);
        mNativeDisplay = getNativeDisplay();
        mGLDisplay = getGLDisplay();

        // Video mode possibilities
        DEVMODE DevMode;
        DevMode.dmSize = sizeof(DEVMODE);
        for (DWORD i = 0; EnumDisplaySettings(NULL, i, &DevMode); ++i)
        {
            if (DevMode.dmBitsPerPel < 16)
                continue;

            mCurrentMode.first.first = DevMode.dmPelsWidth; 
            mCurrentMode.first.second = DevMode.dmPelsHeight; 
            mCurrentMode.second = 0;
            mOriginalMode = mCurrentMode;
            mVideoModes.push_back(mCurrentMode);
        }

        EGLConfig *glConfigs;
        int config, nConfigs = 0;

        EGLint const attrib_list[] =  {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_BUFFER_SIZE, 32,
            EGL_DEPTH_SIZE, 24,
            EGL_STENCIL_SIZE, 8,
            EGL_NONE
        };

        glConfigs = chooseGLConfig(attrib_list, &nConfigs);

        for (config = 0; config < nConfigs; config++)
        {
            int caveat, samples;

            getGLConfigAttrib(glConfigs[config], EGL_CONFIG_CAVEAT, &caveat);

            if (caveat != EGL_SLOW_CONFIG)
            {
                getGLConfigAttrib(glConfigs[config], EGL_SAMPLES, &samples);
                mSampleLevels.push_back(StringConverter::toString(samples));
            }
        }

        free(glConfigs);

        removeDuplicates(mSampleLevels);


    }

    Win32EGLSupport::~Win32EGLSupport()
    {

    }

    //Removed createEGLWindow because it was easier to call new Win32EGLWindow
    //directly to get the native version.
//  EGLWindow* Win32EGLSupport::createEGLWindow(  EGLSupport * support )
//  {
//      return new Win32EGLWindow(support);
//  }

    /*GLESPBuffer* Win32EGLSupport::createPBuffer( PixelComponentType format, size_t width, size_t height )
    {
        return new Win32EGLPBuffer(this, format, width, height);
    }*/

    void Win32EGLSupport::switchMode( uint& width, uint& height, short& frequency )
    {
        if (!mRandr)
            return;

        int size = 0;
        int newSize = -1;

        VideoModes::iterator mode;
        VideoModes::iterator end = mVideoModes.end();
        VideoMode *newMode = 0;

        for(mode = mVideoModes.begin(); mode != end; size++)
        {
            if (mode->first.first >= static_cast<int>(width) &&
                mode->first.second >= static_cast<int>(height))
            {
                if (!newMode ||
                    mode->first.first < newMode->first.first ||
                    mode->first.second < newMode->first.second)
                {
                    newSize = size;
                    newMode = &(*mode);
                }
            }

            VideoMode* lastMode = &(*mode);

            while (++mode != end && mode->first == lastMode->first)
            {
                if (lastMode == newMode && mode->second == frequency)
                {
                    newMode = &(*mode);
                }
            }
        }

        //todo
    }

    //Moved to native from EGLSupport 
   RenderWindow* Win32EGLSupport::newWindow(const String &name,
                                        unsigned int width, unsigned int height,
                                        bool fullScreen,
                                        const NameValuePairList *miscParams)
    {
//        EGLWindow* window = createEGLWindow(this);

    Win32EGLWindow* window = new Win32EGLWindow(this);
        window->create(name, width, height, fullScreen, miscParams);

        return window;
    }

    //Moved to native from EGLSupport
    NativeDisplayType Win32EGLSupport::getNativeDisplay()
    {
        return EGL_DEFAULT_DISPLAY; // TODO
    }

    //Win32EGLSupport::getGLDisplay sets up the native variable
    //then calls EGLSupport::getGLDisplay
    EGLDisplay Win32EGLSupport::getGLDisplay()
    {
        if (!mGLDisplay)
        {
            mNativeDisplay = getNativeDisplay();
            return EGLSupport::getGLDisplay();
        }
        return mGLDisplay;
    }


}
