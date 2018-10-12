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

#include "OgreGLTextureCommon.h"
#include "OgreResourceGroupManager.h"
#include "OgreRoot.h"
#include "OgreBitwise.h"

namespace Ogre
{
void GLTextureCommon::readImage(LoadedImages& imgs, const String& name, const String& ext, bool haveNPOT)
{
    imgs.push_back(Image());
    Image& img = imgs.back();

    DataStreamPtr dstream = ResourceGroupManager::getSingleton().openResource(name, mGroup, this);
    img.load(dstream, ext);

    if( haveNPOT )
        return;

    // Scale to nearest power of 2
    uint32 w = Bitwise::firstPO2From(img.getWidth());
    uint32 h = Bitwise::firstPO2From(img.getHeight());
    if((img.getWidth() != w) || (img.getHeight() != h))
        img.resize(w, h);
}

void GLTextureCommon::getCustomAttribute(const String& name, void* pData)
{
    if (name == "GLID")
        *static_cast<uint*>(pData) = mTextureID;
}

uint32 GLTextureCommon::getMaxMipmaps() {
    // see ARB_texture_non_power_of_two
    return Bitwise::mostSignificantBitSet(std::max(mWidth, std::max(mHeight, mDepth)));
}

void GLTextureCommon::prepareImpl(void)
{
    if (mUsage & TU_RENDERTARGET)
        return;

    const RenderSystemCapabilities* renderCaps =
        Root::getSingleton().getRenderSystem()->getCapabilities();

    bool haveNPOT = renderCaps->hasCapability(RSC_NON_POWER_OF_2_TEXTURES) ||
                    (renderCaps->getNonPOW2TexturesLimited() && mNumMipmaps == 0);

    String baseName, ext;
    StringUtil::splitBaseFilename(mName, baseName, ext);

    LoadedImages loadedImages;

    if (mTextureType == TEX_TYPE_1D || mTextureType == TEX_TYPE_2D ||
        mTextureType == TEX_TYPE_2D_RECT || mTextureType == TEX_TYPE_2D_ARRAY ||
        mTextureType == TEX_TYPE_3D)
    {
        readImage(loadedImages, mName, ext, haveNPOT);

        // If this is a volumetric texture set the texture type flag accordingly.
        // If this is a cube map, set the texture type flag accordingly.
        if (loadedImages[0].hasFlag(IF_CUBEMAP))
            mTextureType = TEX_TYPE_CUBE_MAP;
        // If this is a volumetric texture set the texture type flag accordingly.
        if (loadedImages[0].getDepth() > 1 && mTextureType != TEX_TYPE_2D_ARRAY)
            mTextureType = TEX_TYPE_3D;

        // If compressed and 0 custom mipmap, disable auto mip generation and
        // disable software mipmap creation.
        // Not supported by GLES.
        if (PixelUtil::isCompressed(loadedImages[0].getFormat()) &&
            !renderCaps->hasCapability(RSC_AUTOMIPMAP_COMPRESSED) &&
            loadedImages[0].getNumMipmaps() == 0)
        {
            mNumMipmaps = mNumRequestedMipmaps = 0;
            // Disable flag for auto mip generation
            mUsage &= ~TU_AUTOMIPMAP;
        }
    }
    else if (mTextureType == TEX_TYPE_CUBE_MAP)
    {
        if (getSourceFileType() == "dds")
        {
            // XX HACK there should be a better way to specify whether
            // all faces are in the same file or not
            readImage(loadedImages, mName, ext, haveNPOT);
        }
        else
        {
            for (size_t i = 0; i < 6; i++)
            {
                String fullName = baseName + CUBEMAP_SUFFIXES[i];
                if (!ext.empty())
                    fullName = fullName + "." + ext;
                // find & load resource data intro stream to allow resource
                // group changes if required
                readImage(loadedImages, fullName, ext, haveNPOT);
            }
        }
    }
    else
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "**** Unknown texture type ****",
                    "GLTextureCommon::prepareImpl");
    }

    // avoid copying Image data
    std::swap(mLoadedImages, loadedImages);
}

void GLTextureCommon::loadImpl()
{
    if (mUsage & TU_RENDERTARGET)
    {
        createRenderTexture();
        return;
    }

    LoadedImages loadedImages;
    // Now the only copy is on the stack and will be cleaned in case of
    // exceptions being thrown from _loadImages
    std::swap(loadedImages, mLoadedImages);

    // Call internal _loadImages, not loadImage since that's external and
    // will determine load status etc again
    ConstImagePtrList imagePtrs;

    for (size_t i = 0; i < loadedImages.size(); ++i)
    {
        imagePtrs.push_back(&loadedImages[i]);
    }

    _loadImages(imagePtrs);
}

} /* namespace Ogre */
