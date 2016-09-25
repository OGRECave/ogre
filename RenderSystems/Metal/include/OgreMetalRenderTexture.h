/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2016 Torus Knot Software Ltd

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
#ifndef _OgreMetalRenderTexture_H_
#define _OgreMetalRenderTexture_H_

#include "OgreMetalPrerequisites.h"
#include "OgreRenderTexture.h"

#include "OgreMetalRenderTargetCommon.h"

namespace Ogre
{
    class MetalRenderTexture : public RenderTexture, public MetalRenderTargetCommon
    {
        MetalRenderSystem   *mRenderSystem;

    public:
        MetalRenderTexture( MetalDevice *ownerDevice, const String &name,
                            v1::HardwarePixelBuffer *buffer,
                            __unsafe_unretained id<MTLTexture> renderTexture,
                            __unsafe_unretained id<MTLTexture> resolveTexture,
                            PixelFormat format, uint32 depthPlane, uint32 slice,
                            uint32 fsaa, uint32 mip, bool hwGamma );
        virtual ~MetalRenderTexture();

        virtual void swapBuffers(void);

        // RenderTarget overloads.
        virtual bool requiresTextureFlipping() const { return false; }
        virtual void getCustomAttribute( const String& name, void* pData );
    };
}

#endif
