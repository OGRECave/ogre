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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif
#include "OgreWin32Context.h"
#include "OgreException.h"
#include "OgreGL3PlusRenderSystem.h"
#include "OgreRoot.h"
#include "windowing/win32/OgreWin32GLSupport.h"

namespace Ogre {

    Win32Context::Win32Context(HDC HDC, HGLRC Glrc):
        mHDC(HDC),
        mGlrc(Glrc)
    {
    }
    
    Win32Context::~Win32Context()
    {
        // NB have to do this is subclass to ensure any methods called back
        // are on this subclass and not half-destructed superclass
        GL3PlusRenderSystem *rs = static_cast<GL3PlusRenderSystem*>(Root::getSingleton().getRenderSystem());
        rs->_unregisterContext(this);
    }
        
    void Win32Context::setCurrent()
    {
         wglMakeCurrent(mHDC, mGlrc);      
    }
    void Win32Context::endCurrent()
    {
        wglMakeCurrent(NULL, NULL);
    }

    GL3PlusContext* Win32Context::clone() const
    {
        const int attribList[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        #if OGRE_DEBUG_MODE
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
        #endif
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0, 0
        };

        // Create new context based on own HDC (shared with ours)
        HGLRC newCtx = wglCreateContextAttribsARB( mHDC, mGlrc, attribList );
        
        if (!newCtx)
        {
            OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR,
                         "Error calling wglCreateContextAttribsARB: " + translateWGLError(),
                         "Win32Context::clone" );
        }

        return new Win32Context(mHDC, newCtx);
    }

    void Win32Context::releaseContext()
    {
        if (mGlrc != NULL)
        {
            wglDeleteContext(mGlrc);
            mGlrc = NULL;
            mHDC  = NULL;
        }       
    }
}
