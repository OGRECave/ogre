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

#ifndef __GLES2RenderTexture_H__
#define __GLES2RenderTexture_H__

#include "OgreGLES2Prerequisites.h"
#include "OgreGLRenderTexture.h"

namespace Ogre {
    typedef GLSurfaceDesc GLES2SurfaceDesc;
    typedef GLRenderTexture GLES2RenderTexture;
    typedef GLRTTManager GLES2RTTManager;

    /** RenderTexture for simple copying from frame buffer
    */
    class GLES2CopyingRTTManager;
    class _OgreGLES2Export GLES2CopyingRenderTexture : public GLES2RenderTexture
    {
        public:
            GLES2CopyingRenderTexture(GLES2CopyingRTTManager *manager,
                                   const String &name,
                                   const GLES2SurfaceDesc &target,
                                   bool writeGamma, uint fsaa);

            virtual void getCustomAttribute(const String& name, void* pData);
    };

    /** Simple, copying manager/factory for RenderTextures. This is only used as the last fallback if
        FBOs aren't supported.
    */
    class _OgreGLES2Export GLES2CopyingRTTManager : public GLRTTManager
    {
        public:
            RenderTexture *createRenderTexture(const String &name, const GLES2SurfaceDesc &target, bool writeGamma, uint fsaa) {
                return OGRE_NEW GLES2CopyingRenderTexture(this, name, target, writeGamma, fsaa);
            }
            
            bool checkFormat(PixelFormat format) {
                return true;
            }

            void bind(RenderTarget *target) {
                // Nothing to do here
            }

            void unbind(RenderTarget *target);
    };
}

#endif
