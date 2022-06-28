/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

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
#ifndef _OgreMetalTexture_H_
#define _OgreMetalTexture_H_

#include "OgreMetalPrerequisites.h"
#include "OgreMetalHardwarePixelBuffer.h"
#include "OgreTexture.h"
#include "OgreRenderTexture.h"
#include "OgreImage.h"
#include "OgreException.h"
#include "OgreHardwarePixelBuffer.h"

#import <Metal/MTLTexture.h>

namespace Ogre
{
    class MetalTexture : public Texture
    {
    protected:
        id<MTLTexture>  mTexture;
        id<MTLTexture>  mMsaaTexture;
        MetalDevice     *mDevice;

        MTLTextureType getMetalTextureTarget(void) const;
        void createMetalTexResource(void);
        void createSurfaceList(void);

        virtual void createInternalResourcesImpl(void);
        virtual void freeInternalResourcesImpl(void);

    public:
        MetalTexture( ResourceManager* creator, const String& name, ResourceHandle handle,
                      const String& group, bool isManual, ManualResourceLoader* loader,
                      MetalDevice *device );
        virtual ~MetalTexture();

        void _autogenerateMipmaps(void);

        id<MTLTexture> getTextureForSampling( MetalRenderSystem *renderSystem );

        MetalDevice* getOwnerDevice(void) const             { return mDevice; }
    };
}

#endif
