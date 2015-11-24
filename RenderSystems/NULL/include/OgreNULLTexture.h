/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

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
#ifndef _OgreNULLTexture_H_
#define _OgreNULLTexture_H_

#include "OgreNULLPrerequisites.h"
#include "OgreNULLHardwarePixelBuffer.h"
#include "OgreTexture.h"
#include "OgreRenderTexture.h"
#include "OgreImage.h"
#include "OgreException.h"
#include "OgreHardwarePixelBuffer.h"

namespace Ogre
{
    class NULLTexture : public Texture
    {
    protected:
        virtual void createInternalResourcesImpl(void) {}
        virtual void freeInternalResourcesImpl(void) {}

        /// Resource overloads
        virtual void loadImpl() {}

    public:
        NULLTexture( ResourceManager* creator, const String& name, ResourceHandle handle,
                     const String& group, bool isManual, ManualResourceLoader* loader ) :
            Texture(creator, name, handle, group, isManual, loader)
        {
        }

        virtual v1::HardwarePixelBufferSharedPtr getBuffer(size_t face, size_t mipmap)
        {
            uint32 width = mWidth;
            uint32 height = mHeight;
            uint32 depth = mDepth;
            for( size_t i=0; i<mipmap; ++i )
            {
                width >>= 1;
                width = std::max<uint32>( 1, width );
                height >>= 1;
                height = std::max<uint32>( 1, height );
                depth >>= 1;
                depth = std::max<uint32>( 1, depth );
            }

            v1::NULLHardwarePixelBuffer *buf = new v1::NULLHardwarePixelBuffer(width, height, depth,
                    mFormat, isHardwareGammaEnabled(), static_cast<v1::HardwareBuffer::Usage>(mUsage));
            return v1::HardwarePixelBufferSharedPtr(buf);
        }

        virtual void _autogenerateMipmaps(void) {}
    };


    class NULLRenderTexture : public RenderTexture
    {
    public:
        NULLRenderTexture( v1::HardwarePixelBuffer *buffer, size_t zoffset ) :
            RenderTexture(buffer, zoffset)
        {
        }

        virtual bool requiresTextureFlipping(void) const { return true; }
    };
}

#endif
