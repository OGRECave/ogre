// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreTinyTexture.h"
#include "OgreTinyRenderSystem.h"
#include "OgreHardwareBufferManager.h"
#include "OgreTinyHardwarePixelBuffer.h"
#include "OgreBitwise.h"
#include "OgreTextureManager.h"

namespace Ogre {
    TinyTexture::TinyTexture(ResourceManager* creator, const String& name,
                                   ResourceHandle handle, const String& group, bool isManual,
                                   ManualResourceLoader* loader)
        : Texture(creator, name, handle, group, isManual, loader)
    {
        mMipmapsHardwareGenerated = false;
    }

    TinyTexture::~TinyTexture()
    {
        // have to call this here rather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        if (isLoaded())
        {
            unload();
        }
        else
        {
            freeInternalResources();
        }
    }

    // Creation / loading methods
    void TinyTexture::createInternalResourcesImpl(void)
    {
        // set HardwareBuffer::Usage for TU_RENDERTARGET if nothing else specified
        if((mUsage & TU_RENDERTARGET) && (mUsage & ~TU_RENDERTARGET) == 0)
            mUsage |= HardwareBuffer::HBU_DYNAMIC;

        // Adjust format if required.
        mFormat = TextureManager::getSingleton().getNativeFormat(mTextureType, mFormat, mUsage);

        // Check requested number of mipmaps.
        uint32 maxMips = 0;

        if (PixelUtil::isCompressed(mFormat) && (mNumMipmaps == 0))
            mNumRequestedMipmaps = 0;

        mNumMipmaps = mNumRequestedMipmaps;
        if (mNumMipmaps > maxMips)
            mNumMipmaps = maxMips;

        mBuffer.create(mFormat, mWidth, mHeight, mDepth, getNumFaces(), mNumMipmaps);

        mSurfaceList.clear();

        for (uint8 face = 0; face < getNumFaces(); face++)
        {
            for (uint32 mip = 0; mip <= getNumMipmaps(); mip++)
            {
                TinyHardwarePixelBuffer* buf =
                    new TinyHardwarePixelBuffer(mBuffer.getPixelBox(face, mip), mUsage);
                mSurfaceList.push_back(HardwarePixelBufferSharedPtr(buf));
            }
        }

        // Generate mipmaps after all texture levels have been loaded
        // This is required for compressed formats such as DXT
        if (mUsage & TU_AUTOMIPMAP)
        {

        }
    }
}
