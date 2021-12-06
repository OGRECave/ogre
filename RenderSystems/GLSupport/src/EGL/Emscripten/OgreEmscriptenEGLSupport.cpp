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

#include "OgreEmscriptenEGLSupport.h"
#include "OgreEmscriptenEGLWindow.h"

#include "OgreGLUtil.h"

namespace Ogre {
    GLNativeSupport* getGLSupport(int)
    {
        return new EmscriptenEGLSupport();
    }

    EmscriptenEGLSupport::EmscriptenEGLSupport() : EGLSupport(GLNativeSupport::CONTEXT_ES)
    {        
        mNativeDisplay = EGL_DEFAULT_DISPLAY;
        mGLDisplay = getGLDisplay();
        
        mCurrentMode.width = 800;
        mCurrentMode.height = 600;
        mCurrentMode.refreshRate = 0;
        mOriginalMode = mCurrentMode;
        mVideoModes.push_back(mCurrentMode);
    }

    EmscriptenEGLSupport::~EmscriptenEGLSupport()
    {        
    }
    
    RenderWindow* EmscriptenEGLSupport::newWindow( const String &name, unsigned int width, unsigned int height, bool fullScreen, const NameValuePairList *miscParams)
    {
        EmscriptenEGLWindow* window = new EmscriptenEGLWindow(this);
        window->create(name, width, height, fullScreen, miscParams);
        
        return window;
    }

    EGLConfig* EmscriptenEGLSupport::chooseGLConfig(const EGLint *attribList, EGLint *nElements)
    {
        // Emscripten eglChooseConfig: Implemented as a stub, but this function does not do searching/filtering,
        // and is at the moment identical to eglGetConfigs
        EGLConfig* configs = EGLSupport::chooseGLConfig(attribList, nElements);
        
        for(int config = 0; config < *nElements; ++config)
        {
            for(int attrib = 0; attribList[attrib] != EGL_NONE; attrib += 2)
            {
                EGLint value;
                if (eglGetConfigAttrib(mGLDisplay, configs[config], attribList[attrib], &value) == EGL_FALSE)
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                                "Failed to get config attribute");

                    *nElements = 0;
                    free(configs);
                    return 0;
                }

                bool passed = true;
                EGLint test = attribList[attrib + 1];
		    
                switch(attribList[attrib])
                {
                    case EGL_ALPHA_MASK_SIZE:
                    case EGL_ALPHA_SIZE:
                    case EGL_BLUE_SIZE:
                    case EGL_BUFFER_SIZE:
                    case EGL_DEPTH_SIZE:
                    case EGL_GREEN_SIZE:
                    case EGL_LUMINANCE_SIZE:
                    case EGL_RED_SIZE:
                    case EGL_SAMPLE_BUFFERS:
                    case EGL_SAMPLES:
                    case EGL_STENCIL_SIZE:
                        {
                            if(value < test)
                            {
                                passed = false;
                            }
                        }
                        break;
                    case EGL_COLOR_BUFFER_TYPE:
                    case EGL_LEVEL:
                    case EGL_TRANSPARENT_TYPE:
                        {
                            if(value != test)
                            {
                                passed = false;
                            }
                        }
                        break;
                    case EGL_CONFORMANT:
                    case EGL_RENDERABLE_TYPE:
                    case EGL_SURFACE_TYPE:
                        {
                            if (test & ~value)
                            {
                                passed = false;
                            }
                        }
                        break;
                    case EGL_MATCH_NATIVE_PIXMAP:
                        {
                            if (test != EGL_NONE)
                            {
                                passed = false;
                            }
                        }
                        break;
                    default:
                        break;
                }

                if (!passed)
                {
                    // Shift all later configs up one
                    --*nElements;
                    for(int i = config; i < *nElements; ++i)
                    {
                        configs[i] = configs[i + 1];
                    }
                    --config;
                    break;
                }
            }
        }
        return configs;
    }
}
