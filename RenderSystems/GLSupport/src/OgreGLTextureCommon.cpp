/*
 * OgreGLTextureCommon.cpp
 *
 *  Created on: 24.01.2017
 *      Author: pavel
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

    DataStreamPtr dstream = ResourceGroupManager::getSingleton().openResource(name, mGroup, true, this);
    img.load(dstream, ext);

    if( haveNPOT )
        return;

    // Scale to nearest power of 2
    uint32 w = Bitwise::firstPO2From(img.getWidth());
    uint32 h = Bitwise::firstPO2From(img.getHeight());
    if((img.getWidth() != w) || (img.getHeight() != h))
        img.resize(w, h);
}

HardwarePixelBufferSharedPtr GLTextureCommon::getBuffer(size_t face, size_t mipmap)
{
    if (face >= getNumFaces())
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Face index out of range",
                    "GLTextureCommon::getBuffer");
    }

    if (mipmap > mNumMipmaps)
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Mipmap index out of range",
                    "GLTextureCommon::getBuffer");
    }

    unsigned long idx = face * (mNumMipmaps + 1) + mipmap;
    assert(idx < mSurfaceList.size());
    return mSurfaceList[idx];
}

void GLTextureCommon::getCustomAttribute(const String& name, void* pData)
{
    if (name == "GLID")
        *static_cast<uint*>(pData) = mTextureID;
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

        // If PVRTC and 0 custom mipmap disable auto mip generation and disable software mipmap
        // creation
        if (loadedImages[0].getNumMipmaps() == 0 &&
            PixelUtil::getFormatName(loadedImages[0].getFormat()).find("PVRTC") != String::npos)
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
            static const char* suffixes[6] = {"_rt", "_lf", "_up", "_dn", "_fr", "_bk"};

            for (size_t i = 0; i < 6; i++)
            {
                String fullName = baseName + suffixes[i];
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

} /* namespace Ogre */
