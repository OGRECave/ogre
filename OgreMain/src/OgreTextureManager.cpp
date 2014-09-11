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
#include "OgreStableHeaders.h"
#include "OgreTextureManager.h"
#include "OgreException.h"
#include "OgrePixelFormat.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    template<> TextureManager* Singleton<TextureManager>::msSingleton = 0;
    TextureManager* TextureManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    TextureManager& TextureManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }
    //-----------------------------------------------------------------------
    TextureManager::TextureManager(void)
         : mPreferredIntegerBitDepth(0)
         , mPreferredFloatBitDepth(0)
         , mDefaultNumMipmaps(MIP_UNLIMITED)
    {
        mResourceType = "Texture";
        mLoadOrder = 75.0f;

        // Subclasses should register (when this is fully constructed)
    }
    //-----------------------------------------------------------------------
    TextureManager::~TextureManager()
    {
        // subclasses should unregister with resource group manager

    }
    //-----------------------------------------------------------------------
    TexturePtr TextureManager::getByName(const String& name, const String& groupName)
    {
        return getResourceByName(name, groupName).staticCast<Texture>();
    }
    //-----------------------------------------------------------------------
    TexturePtr TextureManager::create (const String& name, const String& group,
                                    bool isManual, ManualResourceLoader* loader,
                                    const NameValuePairList* createParams)
    {
        return createResource(name,group,isManual,loader,createParams).staticCast<Texture>();
    }
    //-----------------------------------------------------------------------
    TextureManager::ResourceCreateOrRetrieveResult TextureManager::createOrRetrieve(
            const String &name, const String& group, bool isManual, ManualResourceLoader* loader,
            const NameValuePairList* createParams, TextureType texType, int numMipmaps, Real gamma,
            bool isAlpha, PixelFormat desiredFormat, bool hwGamma)
    {
		ResourceCreateOrRetrieveResult res =
            Ogre::ResourceManager::createOrRetrieve(name, group, isManual, loader, createParams);
		// Was it created?
		if(res.second)
        {
            TexturePtr tex = res.first.staticCast<Texture>();
            tex->setTextureType(texType);
            tex->setNumMipmaps((numMipmaps == MIP_DEFAULT)? mDefaultNumMipmaps :
				static_cast<size_t>(numMipmaps));
            tex->setGamma(gamma);
            tex->setTreatLuminanceAsAlpha(isAlpha);
            tex->setFormat(desiredFormat);
			tex->setHardwareGammaEnabled(hwGamma);
        }
        return res;
    }
    //-----------------------------------------------------------------------
    TexturePtr TextureManager::prepare(const String &name, const String& group, TextureType texType,
                                       int numMipmaps, Real gamma, bool isAlpha,
                                       PixelFormat desiredFormat, bool hwGamma)
    {
		ResourceCreateOrRetrieveResult res =
            createOrRetrieve(name,group,false,0,0,texType,numMipmaps,gamma,isAlpha,desiredFormat,hwGamma);
        TexturePtr tex = res.first.staticCast<Texture>();
		tex->prepare();
        return tex;
    }
    //-----------------------------------------------------------------------
    TexturePtr TextureManager::load(const String &name, const String& group, TextureType texType,
                                    int numMipmaps, Real gamma, bool isAlpha, PixelFormat desiredFormat,
                                    bool hwGamma)
    {
		ResourceCreateOrRetrieveResult res =
            createOrRetrieve(name,group,false,0,0,texType,numMipmaps,gamma,isAlpha,desiredFormat,hwGamma);
        TexturePtr tex = res.first.staticCast<Texture>();
		tex->load();
        return tex;
    }

    //-----------------------------------------------------------------------
    TexturePtr TextureManager::loadImage( const String &name, const String& group,
        const Image &img, TextureType texType, int numMipmaps, Real gamma, bool isAlpha, 
		PixelFormat desiredFormat, bool hwGamma)
    {
        TexturePtr tex = createResource(name, group, true).staticCast<Texture>();

        tex->setTextureType(texType);
        tex->setNumMipmaps((numMipmaps == MIP_DEFAULT)? mDefaultNumMipmaps :
			static_cast<size_t>(numMipmaps));
        tex->setGamma(gamma);
        tex->setTreatLuminanceAsAlpha(isAlpha);
        tex->setFormat(desiredFormat);
		tex->setHardwareGammaEnabled(hwGamma);
        tex->loadImage(img);

        return tex;
    }
    //-----------------------------------------------------------------------
    TexturePtr TextureManager::loadRawData(const String &name, const String& group,
        DataStreamPtr& stream, ushort uWidth, ushort uHeight, 
        PixelFormat format, TextureType texType, 
        int numMipmaps, Real gamma, bool hwGamma)
	{
		TexturePtr tex = createResource(name, group, true).staticCast<Texture>();

        tex->setTextureType(texType);
        tex->setNumMipmaps((numMipmaps == MIP_DEFAULT)? mDefaultNumMipmaps :
			static_cast<size_t>(numMipmaps));
        tex->setGamma(gamma);
		tex->setHardwareGammaEnabled(hwGamma);
		tex->loadRawData(stream, uWidth, uHeight, format);
		
        return tex;
	}
    //-----------------------------------------------------------------------
    TexturePtr TextureManager::createManual(const String & name, const String& group,
        TextureType texType, uint width, uint height, uint depth, int numMipmaps,
        PixelFormat format, int usage, ManualResourceLoader* loader, bool hwGamma, 
		uint fsaa, const String& fsaaHint)
    {
        TexturePtr ret;
        ret.setNull();

        // Check for 3D texture support
		const RenderSystemCapabilities* caps =
            Root::getSingleton().getRenderSystem()->getCapabilities();
        if (((texType == TEX_TYPE_3D) || (texType == TEX_TYPE_2D_ARRAY)) &&
            !caps->hasCapability(RSC_TEXTURE_3D))
            return ret;

        if (((usage & (int)TU_STATIC) != 0) && (!Root::getSingleton().getRenderSystem()->isStaticBufferLockable()))
        {
            usage = (usage & ~(int)TU_STATIC) | (int)TU_DYNAMIC;
        }
        ret = createResource(name, group, true, loader).staticCast<Texture>();
        ret->setTextureType(texType);
        ret->setWidth(width);
        ret->setHeight(height);
		ret->setDepth(depth);
        ret->setNumMipmaps((numMipmaps == MIP_DEFAULT)? mDefaultNumMipmaps :
			static_cast<size_t>(numMipmaps));
        ret->setFormat(format);
        ret->setUsage(usage);
		ret->setHardwareGammaEnabled(hwGamma);
		ret->setFSAA(fsaa, fsaaHint);
		ret->createInternalResources();
		return ret;
    }
    //-----------------------------------------------------------------------
    void TextureManager::setPreferredIntegerBitDepth(ushort bits, bool reloadTextures)
    {
        mPreferredIntegerBitDepth = bits;

        if (reloadTextures)
        {
            // Iterate through all textures
            for (ResourceMap::iterator it = mResources.begin(); it != mResources.end(); ++it)
            {
                Texture* texture = static_cast<Texture*>(it->second.get());
                // Reload loaded and reloadable texture only
                if (texture->isLoaded() && texture->isReloadable())
                {
                    texture->unload();
                    texture->setDesiredIntegerBitDepth(bits);
                    texture->load();
                }
                else
                {
                    texture->setDesiredIntegerBitDepth(bits);
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    ushort TextureManager::getPreferredIntegerBitDepth(void) const
    {
        return mPreferredIntegerBitDepth;
    }
    //-----------------------------------------------------------------------
    void TextureManager::setPreferredFloatBitDepth(ushort bits, bool reloadTextures)
    {
        mPreferredFloatBitDepth = bits;

        if (reloadTextures)
        {
            // Iterate through all textures
            for (ResourceMap::iterator it = mResources.begin(); it != mResources.end(); ++it)
            {
                Texture* texture = static_cast<Texture*>(it->second.get());
                // Reload loaded and reloadable texture only
                if (texture->isLoaded() && texture->isReloadable())
                {
                    texture->unload();
                    texture->setDesiredFloatBitDepth(bits);
                    texture->load();
                }
                else
                {
                    texture->setDesiredFloatBitDepth(bits);
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    ushort TextureManager::getPreferredFloatBitDepth(void) const
    {
        return mPreferredFloatBitDepth;
    }
    //-----------------------------------------------------------------------
    void TextureManager::setPreferredBitDepths(ushort integerBits, ushort floatBits, bool reloadTextures)
    {
        mPreferredIntegerBitDepth = integerBits;
        mPreferredFloatBitDepth = floatBits;

        if (reloadTextures)
        {
            // Iterate through all textures
            for (ResourceMap::iterator it = mResources.begin(); it != mResources.end(); ++it)
            {
                Texture* texture = static_cast<Texture*>(it->second.get());
                // Reload loaded and reloadable texture only
                if (texture->isLoaded() && texture->isReloadable())
                {
                    texture->unload();
                    texture->setDesiredBitDepths(integerBits, floatBits);
                    texture->load();
                }
                else
                {
                    texture->setDesiredBitDepths(integerBits, floatBits);
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    void TextureManager::setDefaultNumMipmaps( size_t num )
    {
        mDefaultNumMipmaps = num;
    }
    //-----------------------------------------------------------------------
	bool TextureManager::isFormatSupported(TextureType ttype, PixelFormat format, int usage)
	{
		return getNativeFormat(ttype, format, usage) == format;
	}
    //-----------------------------------------------------------------------
	bool TextureManager::isEquivalentFormatSupported(TextureType ttype, PixelFormat format, int usage)
	{
		PixelFormat supportedFormat = getNativeFormat(ttype, format, usage);

		// Assume that same or greater number of bits means quality not degraded
		return PixelUtil::getNumElemBits(supportedFormat) >= PixelUtil::getNumElemBits(format);
		
	}
}
