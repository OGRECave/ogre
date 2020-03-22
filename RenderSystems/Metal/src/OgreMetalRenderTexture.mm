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

#include "OgreMetalRenderTexture.h"

#import <Metal/MTLRenderPass.h>
#import <Metal/MTLTexture.h>

namespace Ogre
{
    MetalRenderTexture::MetalRenderTexture( MetalDevice *ownerDevice, const String &name,
                                            HardwarePixelBuffer *buffer,
                                            __unsafe_unretained id<MTLTexture> renderTexture,
                                            __unsafe_unretained id<MTLTexture> resolveTexture,
                                            PixelFormat format, uint32 depthPlane, uint32 slice,
                                            uint32 fsaa, uint32 mip, bool hwGamma ) :
        RenderTexture( buffer, depthPlane != 1 ? depthPlane : slice ),
        MetalRenderTargetCommon( ownerDevice )
    {
        mActive = false;
        mHwGamma = hwGamma;

        mName = name;
        mFormat = format;
        mWidth  = static_cast<uint32>( renderTexture.width );
        mHeight = static_cast<uint32>( renderTexture.height );
        mFSAA = fsaa;

        this->init( renderTexture, resolveTexture );

        mColourAttachmentDesc.depthPlane        = depthPlane;
        mColourAttachmentDesc.slice             = slice;
        mColourAttachmentDesc.level             = mip;
        mColourAttachmentDesc.resolveTexture    = resolveTexture;
        mColourAttachmentDesc.texture           = renderTexture;
        mColourAttachmentDesc.resolveLevel      = mip;
        mColourAttachmentDesc.resolveDepthPlane = depthPlane;
        mColourAttachmentDesc.resolveSlice      = slice;
    }
    //-------------------------------------------------------------------------
    MetalRenderTexture::~MetalRenderTexture()
    {
        mActive = false;
    }
    //-------------------------------------------------------------------------
    void MetalRenderTexture::swapBuffers(void)
    {
        RenderTexture::swapBuffers();
    }
    //-------------------------------------------------------------------------
    void MetalRenderTexture::getCustomAttribute( const String& name, void* pData )
    {
        if( name == "MetalRenderTargetCommon" )
        {
            *static_cast<MetalRenderTargetCommon**>(pData) = this;
        }
        else if( name == "mNumMRTs" )
        {
            *static_cast<uint8*>(pData) = 1u;
        }
        else if( name == "MetalDevice" )
        {
            *static_cast<MetalDevice**>(pData) = this->getOwnerDevice();
        }
        else
        {
            RenderTarget::getCustomAttribute( name, pData );
        }
    }
}
