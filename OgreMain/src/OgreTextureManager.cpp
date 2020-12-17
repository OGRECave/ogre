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
#include "OgrePixelFormat.h"

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
    SamplerPtr TextureManager::createSampler(const String& name)
    {
        SamplerPtr ret = _createSamplerImpl();
        if(!name.empty())
        {
            OgreAssert(mNamedSamplers.find(name) == mNamedSamplers.end(),
                       ("Sampler '" + name + "' already exists").c_str());
            mNamedSamplers[name] = ret;
        }
        return ret;
    }

    /// retrieve an named sampler
    const SamplerPtr& TextureManager::getSampler(const String& name) const
    {
        static SamplerPtr nullPtr;
        auto it = mNamedSamplers.find(name);
        if(it == mNamedSamplers.end())
            return nullPtr;
        return it->second;
    }
    //-----------------------------------------------------------------------
    TexturePtr TextureManager::getByName(const String& name, const String& groupName)
    {
        return static_pointer_cast<Texture>(getResourceByName(name, groupName));
    }
    //-----------------------------------------------------------------------
    TexturePtr TextureManager::create (const String& name, const String& group,
                                    bool isManual, ManualResourceLoader* loader,
                                    const NameValuePairList* createParams)
    {
        return static_pointer_cast<Texture>(createResource(name,group,isManual,loader,createParams));
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
            TexturePtr tex = static_pointer_cast<Texture>(res.first);
            tex->setTextureType(texType);
            tex->setNumMipmaps((numMipmaps == MIP_DEFAULT)? mDefaultNumMipmaps :
                static_cast<uint32>(numMipmaps));
            tex->setGamma(gamma);
            OGRE_IGNORE_DEPRECATED_BEGIN
            tex->setTreatLuminanceAsAlpha(isAlpha);
            OGRE_IGNORE_DEPRECATED_END
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
        TexturePtr tex = static_pointer_cast<Texture>(res.first);
        tex->prepare();
        return tex;
    }
    //-----------------------------------------------------------------------
    TexturePtr TextureManager::load(const String& name, const String& group, TextureType texType,
                                    int numMipmaps, Real gamma, bool isAlpha, PixelFormat desiredFormat,
                                    bool hwGamma)
    {
        auto res = createOrRetrieve(name, group, false, 0, 0, texType, numMipmaps, gamma, isAlpha,
                                    desiredFormat, hwGamma);
        TexturePtr tex = static_pointer_cast<Texture>(res.first);
        tex->load();
        return tex;
    }
    TexturePtr TextureManager::load(const String& name, const String& group, TextureType texType,
                                    int numMipmaps, Real gamma, PixelFormat desiredFormat, bool hwGamma)
    {
        auto res = createOrRetrieve(name, group, false, 0, 0, texType, numMipmaps, gamma, false,
                                    desiredFormat, hwGamma);
        TexturePtr tex = static_pointer_cast<Texture>(res.first);
        tex->load();
        return tex;
    }
    //-----------------------------------------------------------------------
    TexturePtr TextureManager::loadImage( const String &name, const String& group,
        const Image &img, TextureType texType, int numMipmaps, Real gamma, bool isAlpha, 
        PixelFormat desiredFormat, bool hwGamma)
    {
        TexturePtr tex = create(name, group, true);

        tex->setTextureType(texType);
        tex->setNumMipmaps((numMipmaps == MIP_DEFAULT)? mDefaultNumMipmaps :
            static_cast<uint32>(numMipmaps));
        tex->setGamma(gamma);
        OGRE_IGNORE_DEPRECATED_BEGIN
        tex->setTreatLuminanceAsAlpha(isAlpha);
        OGRE_IGNORE_DEPRECATED_END
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
        TexturePtr tex = create(name, group, true);

        tex->setTextureType(texType);
        tex->setNumMipmaps((numMipmaps == MIP_DEFAULT)? mDefaultNumMipmaps :
            static_cast<uint32>(numMipmaps));
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

        OgreAssert(width && height && depth, "total size of texture must not be zero");

        // Check for texture support
        const auto caps = Root::getSingleton().getRenderSystem()->getCapabilities();
        if (((texType == TEX_TYPE_3D) && !caps->hasCapability(RSC_TEXTURE_3D)) ||
            ((texType == TEX_TYPE_2D_ARRAY) && !caps->hasCapability(RSC_TEXTURE_2D_ARRAY)))
            return ret;

        ret = create(name, group, true, loader);

        if(!ret)
            return ret;

        ret->setTextureType(texType);
        ret->setWidth(width);
        ret->setHeight(height);
        ret->setDepth(depth);
        ret->setNumMipmaps((numMipmaps == MIP_DEFAULT)? mDefaultNumMipmaps :
            static_cast<uint32>(numMipmaps));
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
    void TextureManager::setDefaultNumMipmaps( uint32 num )
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

    bool TextureManager::isHardwareFilteringSupported(TextureType ttype, PixelFormat format,
                                                      int usage, bool preciseFormatOnly)
    {
        if (format == PF_UNKNOWN)
            return false;

        // Check native format
        if (preciseFormatOnly && !isFormatSupported(ttype, format, usage))
            return false;

        return true;
    }

    const TexturePtr& TextureManager::_getWarningTexture()
    {
        if(mWarningTexture)
            return mWarningTexture;

        // Generate warning texture
        Image pixels(PF_R5G6B5, 8, 8);

        // Yellow/black stripes
        const ColourValue black(0, 0, 0), yellow(1, 1, 0);
        for (size_t y = 0; y < pixels.getHeight(); ++y)
        {
            for (size_t x = 0; x < pixels.getWidth(); ++x)
            {
                pixels.setColourAt((((x + y) % 8) < 4) ? black : yellow, x, y, 0);
            }
        }

        mWarningTexture = loadImage("Warning", RGN_INTERNAL, pixels);

        return mWarningTexture;
    }

    const SamplerPtr& TextureManager::getDefaultSampler()
    {
        if(!mDefaultSampler)
            mDefaultSampler = createSampler();

        return mDefaultSampler;
    }
}
