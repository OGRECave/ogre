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

#include "OgreNaClGLContext.h"
#include "OgreLog.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreWindowEventUtilities.h"

#include "OgreGLES2Prerequisites.h"
#include "OgreGLES2RenderSystem.h"

#include "OgreNaClGLSupport.h"

#include <stdint.h>

namespace Ogre {
    NaClGLContext::NaClGLContext(const NaClWindow * window, const NaClGLSupport *glsupport, pp::Instance* instance, pp::CompletionCallback* swapCallback)
		: pp::Graphics3DClient(instance)
        , mWindow(window)
        , mGLSupport(glsupport)
        , mInstance(instance)
        , mSwapCallback(swapCallback)
        , mWidth(mWindow->getWidth())
        , mHeight(mWindow->getHeight())
    {
    }

    NaClGLContext::~NaClGLContext()
    {
        glSetCurrentContextPPAPI(0);
    }
	
	void NaClGLContext::setCurrent()
    {
        if (mInstance == NULL) {
            glSetCurrentContextPPAPI(0);
            return;
        }
        // Lazily create the Pepper context.
        if (mContext.is_null()) 
        {
            LogManager::getSingleton().logMessage("\tcreate context started");

            int32_t attribs[] = {
                PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 8,
                PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 24,
                PP_GRAPHICS3DATTRIB_STENCIL_SIZE, 8,
                PP_GRAPHICS3DATTRIB_SAMPLES, 0,
                PP_GRAPHICS3DATTRIB_SAMPLE_BUFFERS, 0,
                PP_GRAPHICS3DATTRIB_WIDTH, mWindow->getWidth(),
                PP_GRAPHICS3DATTRIB_HEIGHT, mWindow->getHeight(),
                PP_GRAPHICS3DATTRIB_NONE
            };

            mContext = pp::Graphics3D(mInstance, pp::Graphics3D(), attribs);
            if (mContext.is_null()) 
            {
                glSetCurrentContextPPAPI(0);
                return;
            }
            mInstance->BindGraphics(mContext);
        }

        glSetCurrentContextPPAPI(mContext.pp_resource());
        return;
    }

    void NaClGLContext::endCurrent()
    {
        glSetCurrentContextPPAPI(0);
    }

    GLES2Context* NaClGLContext::clone() const
    {
        NaClGLContext* res = new NaClGLContext(mWindow, mGLSupport, mInstance, mSwapCallback);
        res->mInstance = this->mInstance;
        return res;
    }

    void NaClGLContext::swapBuffers( )
    {
        mContext.SwapBuffers(*mSwapCallback);
    }

    // overrides pp::Graphics3DClient
    void NaClGLContext::Graphics3DContextLost()
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
            "Unexpectedly lost graphics context.",
            "NaClGLContext::Graphics3DContextLost" );
    }

    void NaClGLContext::resize()
    {
        if(mWidth != mWindow->getWidth() || mHeight != mWindow->getHeight() )
        {
            LogManager::getSingleton().logMessage("\tresizing");
            mWidth = mWindow->getWidth();
            mHeight = mWindow->getHeight();
            glSetCurrentContextPPAPI(0);
            mContext.ResizeBuffers( mWidth, mHeight );
            glSetCurrentContextPPAPI(mContext.pp_resource());
            LogManager::getSingleton().logMessage("\tdone resizing");
        }            

    }




} // namespace
