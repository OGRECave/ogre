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

#include "OgreAndroidEGLSupport.h"
#include "OgreAndroidEGLWindow.h"
#include "OgreGLUtil.h"

namespace Ogre {

    GLNativeSupport* getGLSupport(int)
    {
        return new AndroidEGLSupport();
    }

    AndroidEGLSupport::AndroidEGLSupport() : EGLSupport(CONTEXT_ES)
    {        
        mNativeDisplay = EGL_DEFAULT_DISPLAY;
        mGLDisplay = getGLDisplay();
        
        mCurrentMode.width = 1280;
        mCurrentMode.height = 800;
        mCurrentMode.refreshRate = 0;
        mOriginalMode = mCurrentMode;
        mVideoModes.push_back(mCurrentMode);
    }

    ConfigOptionMap AndroidEGLSupport::getConfigOptions()
    {
        ConfigOptionMap mOptions = EGLSupport::getConfigOptions();
        ConfigOption optOrientation;
        optOrientation.name = "Orientation";
        optOrientation.immutable = false;
        optOrientation.possibleValues.push_back("Landscape");
        optOrientation.possibleValues.push_back("Portrait");
        optOrientation.currentValue = optOrientation.possibleValues[0];
        mOptions[optOrientation.name] = optOrientation;

        ConfigOption optScaling;
        optScaling.name = "Content Scaling Factor";
        optScaling.immutable = false;
        optScaling.possibleValues.push_back("1");
        optScaling.currentValue = optScaling.possibleValues[0];
        mOptions[optScaling.name] = optScaling;

        return mOptions;
    }

    AndroidEGLSupport::~AndroidEGLSupport()
    {
        
    }

    RenderWindow* AndroidEGLSupport::newWindow( const String &name, unsigned int width, unsigned int height, bool fullScreen, const NameValuePairList *miscParams)
    {
        AndroidEGLWindow* window = new AndroidEGLWindow(this);
        window->create(name, width, height, fullScreen, miscParams);
        
        return window;
    }
}
