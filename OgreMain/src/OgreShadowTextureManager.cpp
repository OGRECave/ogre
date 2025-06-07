/*-------------------------------------------------------------------------
This source file is a part of OGRE
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
THE SOFTWARE

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-------------------------------------------------------------------------*/
#include "OgreStableHeaders.h"
#include "OgreHardwarePixelBuffer.h"

namespace Ogre
{
    //-----------------------------------------------------------------------
    bool operator== ( const ShadowTextureConfig& lhs, const ShadowTextureConfig& rhs )
    {
        if ( lhs.width != rhs.width ||
            lhs.height != rhs.height ||
            lhs.format != rhs.format )
        {
            return false;
        }

        return true;
    }
    //-----------------------------------------------------------------------
    bool operator!= ( const ShadowTextureConfig& lhs, const ShadowTextureConfig& rhs )
    {
        return !( lhs == rhs );
    }
    //-----------------------------------------------------------------------
    template<> ShadowTextureManager* Singleton<ShadowTextureManager>::msSingleton = 0;
    ShadowTextureManager* ShadowTextureManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    ShadowTextureManager& ShadowTextureManager::getSingleton(void)
    {
        assert( msSingleton );  return ( *msSingleton );
    }
    //---------------------------------------------------------------------
    ShadowTextureManager::ShadowTextureManager()
        : mCount(0)
    {

    }
    //---------------------------------------------------------------------
    ShadowTextureManager::~ShadowTextureManager()
    {
        clear();
    }
    //---------------------------------------------------------------------
    void ShadowTextureManager::getShadowTextures(ShadowTextureConfigList& configList,
        ShadowTextureList& listToPopulate)
    {
        listToPopulate.clear();

        std::set<Texture*> usedTextures;

        for (ShadowTextureConfig& config : configList)
        {
            bool found = false;
            for (auto & tex : mTextureList)
            {
                // Skip if already used this one
                if (usedTextures.find(tex.get()) != usedTextures.end())
                    continue;

                if (config.width == tex->getWidth() && config.height == tex->getHeight() &&
                    config.depth == tex->getDepth() && config.type == tex->getTextureType() &&
                    config.format == tex->getFormat() && config.fsaa == tex->getFSAA())
                {
                    // Ok, a match
                    listToPopulate.push_back(tex);
                    usedTextures.insert(tex.get());
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                // Create a new texture
                String targName = StringUtil::format("Ogre/ShadowTexture%zu", mCount++);
                TexturePtr shadowTex = TextureManager::getSingleton().createManual(
                    targName, RGN_INTERNAL, config.type, config.width, config.height, config.depth, 0, config.format,
                    TU_RENDERTARGET, NULL, false, config.fsaa);
                OgreAssert(shadowTex, "Unsupported shadow texture configuration");
                // Ensure texture loaded
                shadowTex->load();

                // update with actual format, if the requested format is not supported
                config.format = shadowTex->getFormat();
                listToPopulate.push_back(shadowTex);
                usedTextures.insert(shadowTex.get());
                mTextureList.push_back(shadowTex);
            }
        }

    }
    //---------------------------------------------------------------------
    TexturePtr ShadowTextureManager::getNullShadowTexture(PixelFormat format)
    {
        for (auto & tex : mNullTextureList)
        {
            if (format == tex->getFormat())
            {
                // Ok, a match
                return tex;
            }
        }

        // not found, create a new one
        // A 1x1 texture of the correct format, not a render target
        static const String baseName = "Ogre/ShadowTextureNull";
        String targName = baseName + StringConverter::toString(mCount++);
        TexturePtr shadowTex = TextureManager::getSingleton().createManual(
            targName, 
            ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, 
            TEX_TYPE_2D, 1, 1, 0, format, TU_STATIC_WRITE_ONLY);
        mNullTextureList.push_back(shadowTex);

        // lock & populate the texture based on format
        if(PixelUtil::isDepth(format))
            return shadowTex;

        HardwareBufferLockGuard shadowTexLock(shadowTex->getBuffer(), HardwareBuffer::HBL_DISCARD);
        const PixelBox& box = shadowTex->getBuffer()->getCurrentLock();

        // set high-values across all bytes of the format 
        PixelUtil::packColour( 1.0f, 1.0f, 1.0f, 1.0f, shadowTex->getFormat(), box.data );

        return shadowTex;
    
    }
    //---------------------------------------------------------------------
    void ShadowTextureManager::clearUnused()
    {
        for (ShadowTextureList::iterator i = mTextureList.begin(); i != mTextureList.end(); )
        {
            // Unreferenced if only this reference and the resource system
            // Any cached shadow textures should be re-bound each frame dropping
            // any old references
            if ((*i).use_count() == ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS + 1)
            {
                TextureManager::getSingleton().remove((*i)->getHandle());
                i = mTextureList.erase(i);
            }
            else
            {
                ++i;
            }
        }
        for (ShadowTextureList::iterator i = mNullTextureList.begin(); i != mNullTextureList.end(); )
        {
            // Unreferenced if only this reference and the resource system
            // Any cached shadow textures should be re-bound each frame dropping
            // any old references
            if ((*i).use_count() == ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS + 1)
            {
                TextureManager::getSingleton().remove((*i)->getHandle());
                i = mNullTextureList.erase(i);
            }
            else
            {
                ++i;
            }
        }

    }
    //---------------------------------------------------------------------
    void ShadowTextureManager::clear()
    {
        for (auto & i : mTextureList)
        {
            TextureManager::getSingleton().remove(i->getHandle());
        }
        mTextureList.clear();

    }
    //---------------------------------------------------------------------

}

