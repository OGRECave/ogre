/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "..\..\..\..\OgreMain\include\OgreException.h"

namespace {
    // This is called by the browser when the 3D context has been flushed to the
    // browser window.
    void FlushCallback(void* data, int32_t result) 
    {
        static_cast<Ogre::NaClGLContext*>(data)->setFlushPending(false);
    }

}  // namespace

namespace Ogre {
    NaClGLContext::NaClGLContext(const NaClGLSupport *glsupport, pp::Instance* instance)
		: pp::Graphics3DClient_Dev(instance)
        , mGLSupport(glsupport)
        , mInstance(instance)
        , mFlushPending(false)
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
        if (mContext.is_null()) {
            mContext = pp::Context3D_Dev(*mInstance, 0, pp::Context3D_Dev(), NULL);
            if (mContext.is_null()) {
                glSetCurrentContextPPAPI(0);
                return;
            }
            mSurface = pp::Surface3D_Dev(*mInstance, 0, NULL);
            mContext.BindSurfaces(mSurface, mSurface);
            mInstance->BindGraphics(mSurface);
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
        NaClGLContext* res = new NaClGLContext(mGLSupport, mInstance);
        res->mInstance = this->mInstance;
        res->mSurface = this->mSurface;
        res->mFlushPending = this->mFlushPending;
        return res;
    }

    void NaClGLContext::swapBuffers( bool waitForVSync )
    {
        if (mFlushPending) 
        {
            // A flush is pending so do nothing; just drop this flush on the floor.
            return;
        }
        mFlushPending = true;

        mSurface.SwapBuffers(pp::CompletionCallback(&FlushCallback, this));
    }

    void NaClGLContext::setFlushPending( const bool flag )
    {
        mFlushPending = flag;
    }

    // overrides pp::Graphics3DClient_Dev
    void NaClGLContext::Graphics3DContextLost()
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
            "Unexpectedly lost graphics context.",
            "NaClGLContext::Graphics3DContextLost" );
    }

    void NaClGLContext::invalidate()
    {
        // Unbind the existing surface and re-bind to null surfaces.
        mInstance->BindGraphics(pp::Surface3D_Dev());
        mContext.BindSurfaces(pp::Surface3D_Dev(), pp::Surface3D_Dev());
        glSetCurrentContextPPAPI(0);

    }



} // namespace
