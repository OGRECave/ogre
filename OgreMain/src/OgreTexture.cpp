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
#include "OgreHardwarePixelBuffer.h"
#include "OgreImage.h"
#include "OgreTexture.h"

namespace Ogre {
    const char* Texture::CUBEMAP_SUFFIXES[] = {"_rt", "_lf", "_up", "_dn", "_fr", "_bk"};
    //--------------------------------------------------------------------------
    Texture::Texture(ResourceManager* creator, const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader)
        : Resource(creator, name, handle, group, isManual, loader),
            // init defaults; can be overridden before load()
            mHeight(512),
            mWidth(512),
            mDepth(1),
            mNumRequestedMipmaps(0),
            mNumMipmaps(0),
            mMipmapsHardwareGenerated(false),
            mGamma(1.0f),
            mHwGamma(false),
            mFSAA(0),
            mTextureType(TEX_TYPE_2D),            
            mFormat(PF_UNKNOWN),
            mUsage(TU_DEFAULT),
            mSrcFormat(PF_UNKNOWN),
            mSrcWidth(0),
            mSrcHeight(0), 
            mSrcDepth(0),
            mDesiredFormat(PF_UNKNOWN),
            mDesiredIntegerBitDepth(0),
            mDesiredFloatBitDepth(0),
            mTreatLuminanceAsAlpha(false),
            mInternalResourcesCreated(false)
    {
        if (createParamDictionary("Texture"))
        {
            // Define the parameters that have to be present to load
            // from a generic source; actually there are none, since when
            // predeclaring, you use a texture file which includes all the
            // information required.
        }

        // Set some defaults for default load path
        if (TextureManager::getSingletonPtr())
        {
            TextureManager& tmgr = TextureManager::getSingleton();
            setNumMipmaps(tmgr.getDefaultNumMipmaps());
            setDesiredBitDepths(tmgr.getPreferredIntegerBitDepth(), tmgr.getPreferredFloatBitDepth());
        }

        
    }
    //--------------------------------------------------------------------------
    void Texture::loadRawData( DataStreamPtr& stream, 
        ushort uWidth, ushort uHeight, PixelFormat eFormat)
    {
        Image img;
        img.loadRawData(stream, uWidth, uHeight, 1, eFormat);
        loadImage(img);
    }
    //--------------------------------------------------------------------------    
    void Texture::loadImage( const Image &img )
    {
        OgreAssert(img.getSize(), "cannot load empty image");
        LoadingState old = mLoadingState.load();
        if (old!=LOADSTATE_UNLOADED && old!=LOADSTATE_PREPARED) return;

        if (!mLoadingState.compare_exchange_strong(old,LOADSTATE_LOADING)) return;

        // Scope lock for actual loading
        try
        {
                    OGRE_LOCK_AUTO_MUTEX;
            std::vector<const Image*> imagePtrs;
            imagePtrs.push_back(&img);
            _loadImages( imagePtrs );

        }
        catch (...)
        {
            // Reset loading in-progress flag in case failed for some reason
            mLoadingState.store(old);
            // Re-throw
            throw;
        }

        mLoadingState.store(LOADSTATE_LOADED);

        // Notify manager
        if(getCreator())
            getCreator()->_notifyResourceLoaded(this);

        // No deferred loading events since this method is not called in background


    }
    //--------------------------------------------------------------------------
    void Texture::setFormat(PixelFormat pf)
    {
        mFormat = pf;
        mDesiredFormat = pf;
    }
    //--------------------------------------------------------------------------
    bool Texture::hasAlpha(void) const
    {
        return PixelUtil::hasAlpha(mFormat);
    }
    //--------------------------------------------------------------------------
    void Texture::setDesiredIntegerBitDepth(ushort bits)
    {
        mDesiredIntegerBitDepth = bits;
    }
    //--------------------------------------------------------------------------
    ushort Texture::getDesiredIntegerBitDepth(void) const
    {
        return mDesiredIntegerBitDepth;
    }
    //--------------------------------------------------------------------------
    void Texture::setDesiredFloatBitDepth(ushort bits)
    {
        mDesiredFloatBitDepth = bits;
    }
    //--------------------------------------------------------------------------
    ushort Texture::getDesiredFloatBitDepth(void) const
    {
        return mDesiredFloatBitDepth;
    }
    //--------------------------------------------------------------------------
    void Texture::setDesiredBitDepths(ushort integerBits, ushort floatBits)
    {
        mDesiredIntegerBitDepth = integerBits;
        mDesiredFloatBitDepth = floatBits;
    }
    //--------------------------------------------------------------------------
    void Texture::setTreatLuminanceAsAlpha(bool asAlpha)
    {
        mTreatLuminanceAsAlpha = asAlpha;
    }
    //--------------------------------------------------------------------------
    bool Texture::getTreatLuminanceAsAlpha(void) const
    {
        return mTreatLuminanceAsAlpha;
    }
    //--------------------------------------------------------------------------
    size_t Texture::calculateSize(void) const
    {
        return getNumFaces() * PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);
    }
    //--------------------------------------------------------------------------
    size_t Texture::getNumFaces(void) const
    {
        return getTextureType() == TEX_TYPE_CUBE_MAP ? 6 : 1;
    }
    //--------------------------------------------------------------------------
    void Texture::_loadImages( const ConstImagePtrList& images )
    {
        if(images.empty())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot load empty vector of images",
             "Texture::loadImages");
        
        // Set desired texture size and properties from images[0]
        mSrcWidth = mWidth = images[0]->getWidth();
        mSrcHeight = mHeight = images[0]->getHeight();
        mSrcDepth = mDepth = images[0]->getDepth();
        mSrcFormat = images[0]->getFormat();

        if(!mLayerNames.empty() && mTextureType != TEX_TYPE_CUBE_MAP)
            mDepth = mLayerNames.size();

        if(mTreatLuminanceAsAlpha && mSrcFormat == PF_L8)
            mDesiredFormat = PF_A8;

        if (mDesiredFormat != PF_UNKNOWN)
        {
            // If have desired format, use it
            mFormat = mDesiredFormat;
        }
        else
        {
            // Get the format according with desired bit depth
            mFormat = PixelUtil::getFormatForBitDepths(mSrcFormat, mDesiredIntegerBitDepth, mDesiredFloatBitDepth);
        }

        // The custom mipmaps in the image have priority over everything
        uint32 imageMips = images[0]->getNumMipmaps();

        if(imageMips > 0)
        {
            mNumMipmaps = mNumRequestedMipmaps = images[0]->getNumMipmaps();
            // Disable flag for auto mip generation
            mUsage &= ~TU_AUTOMIPMAP;
        }

        // Create the texture
        createInternalResources();
        // Check if we're loading one image with multiple faces
        // or a vector of images representing the faces
        size_t faces;
        bool multiImage; // Load from multiple images?
        if(images.size() > 1)
        {
            faces = images.size();
            multiImage = true;
        }
        else
        {
            faces = images[0]->getNumFaces();
            multiImage = false;
        }
        
        // Check whether number of faces in images exceeds number of faces
        // in this texture. If so, clamp it.
        if(faces > getNumFaces())
            faces = getNumFaces();
        
        if (TextureManager::getSingleton().getVerbose()) {
            // Say what we're doing
            Log::Stream str = LogManager::getSingleton().stream();
            str << "Texture '" << mName << "': Loading " << faces << " faces"
                << "(" << PixelUtil::getFormatName(images[0]->getFormat()) << ","
                << images[0]->getWidth() << "x" << images[0]->getHeight() << "x"
                << images[0]->getDepth() << ")";
            if (!(mMipmapsHardwareGenerated && mNumMipmaps == 0))
            {
                str << " with " << mNumMipmaps;
                if(mUsage & TU_AUTOMIPMAP)
                {
                    if (mMipmapsHardwareGenerated)
                        str << " hardware";

                    str << " generated mipmaps";
                }
                else
                {
                    str << " custom mipmaps";
                }
                if(multiImage)
                    str << " from multiple Images.";
                else
                    str << " from Image.";
            }

            // Print data about first destination surface
            const auto& buf = getBuffer(0, 0);
            str << " Internal format is " << PixelUtil::getFormatName(buf->getFormat()) << ","
                << buf->getWidth() << "x" << buf->getHeight() << "x" << buf->getDepth() << ".";
        }
        
        // Main loading loop
        // imageMips == 0 if the image has no custom mipmaps, otherwise contains the number of custom mips
        for(size_t mip = 0; mip <= std::min(mNumMipmaps, imageMips); ++mip)
        {
            for(size_t i = 0; i < std::max(faces, images.size()); ++i)
            {
                PixelBox src;
                size_t face = (mDepth == 1) ? i : 0; // depth = 1, then cubemap face else 3d/ array layer

                auto buffer = getBuffer(face, mip);
                Box dst(0, 0, 0, buffer->getWidth(), buffer->getHeight(), buffer->getDepth());

                if(multiImage)
                {
                    // Load from multiple images
                    src = images[i]->getPixelBox(0, mip);
                    // set dst layer
                    if(mDepth > 1)
                    {
                        dst.front = i;
                        dst.back = i + 1;
                    }
                }
                else
                {
                    // Load from faces of images[0]
                    src = images[0]->getPixelBox(i, mip);
                }

                // Allow reinterpreting luminance as alpha
                if (mDesiredFormat == PF_A8 && (src.format == PF_L8 || src.format == PF_R8))
                    src.format = PF_A8;

                if(mGamma != 1.0f) {
                    // Apply gamma correction
                    // Do not overwrite original image but do gamma correction in temporary buffer
                    Image tmp(src.format, src.getWidth(), getHeight(), src.getDepth());
                    PixelBox corrected = tmp.getPixelBox();
                    PixelUtil::bulkPixelConversion(src, corrected);

                    Image::applyGamma(corrected.data, mGamma, tmp.getSize(), tmp.getBPP());

                    // Destination: entire texture. blitFromMemory does the scaling to
                    // a power of two for us when needed
                    buffer->blitFromMemory(corrected, dst);
                }
                else 
                {
                    // Destination: entire texture. blitFromMemory does the scaling to
                    // a power of two for us when needed
                    buffer->blitFromMemory(src, dst);
                }
                
            }
        }
        // Update size (the final size, not including temp space)
        mSize = getNumFaces() * PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);

    }
    //-----------------------------------------------------------------------------
    void Texture::createInternalResources(void)
    {
        if (!mInternalResourcesCreated)
        {
            createInternalResourcesImpl();
            mInternalResourcesCreated = true;
        }
    }
    //-----------------------------------------------------------------------------
    void Texture::freeInternalResources(void)
    {
        if (mInternalResourcesCreated)
        {
            mSurfaceList.clear();
            freeInternalResourcesImpl();
            mInternalResourcesCreated = false;
        }
    }
    //-----------------------------------------------------------------------------
    void Texture::unloadImpl(void)
    {
        freeInternalResources();
    }
    //-----------------------------------------------------------------------------   
    void Texture::copyToTexture( TexturePtr& target )
    {
        if(target->getNumFaces() != getNumFaces())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Texture types must match",
                "Texture::copyToTexture");
        }
        size_t numMips = std::min(getNumMipmaps(), target->getNumMipmaps());
        if((mUsage & TU_AUTOMIPMAP) || (target->getUsage()&TU_AUTOMIPMAP))
            numMips = 0;
        for(unsigned int face=0; face<getNumFaces(); face++)
        {
            for(unsigned int mip=0; mip<=numMips; mip++)
            {
                target->getBuffer(face, mip)->blit(getBuffer(face, mip));
            }
        }
    }
    //---------------------------------------------------------------------
    String Texture::getSourceFileType() const
    {
        if (mName.empty())
            return BLANKSTRING;

        String::size_type pos = mName.find_last_of('.');
        if (pos != String::npos && pos < (mName.length() - 1))
        {
            String ext = mName.substr(pos + 1);
            StringUtil::toLowerCase(ext);
            return ext;
        }

        // No extension
        auto dstream = ResourceGroupManager::getSingleton().openResource(
            mName, mGroup, NULL, false);

        if (!dstream && getTextureType() == TEX_TYPE_CUBE_MAP)
        {
            // try again with one of the faces (non-dds)
            dstream = ResourceGroupManager::getSingleton().openResource(mName + "_rt", mGroup, NULL, false);
        }

        return dstream ? Image::getFileExtFromMagic(dstream) : BLANKSTRING;

    }
    const HardwarePixelBufferSharedPtr& Texture::getBuffer(size_t face, size_t mipmap)
    {
        if (face >= getNumFaces())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Face index out of range", "Texture::getBuffer");
        }

        if (mipmap > mNumMipmaps)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Mipmap index out of range", "Texture::getBuffer");
        }

        unsigned long idx = face * (mNumMipmaps + 1) + mipmap;
        assert(idx < mSurfaceList.size());
        return mSurfaceList[idx];
    }

    //---------------------------------------------------------------------
    void Texture::convertToImage(Image& destImage, bool includeMipMaps)
    {
        uint32 numMips = includeMipMaps? getNumMipmaps() + 1 : 1;
        destImage.create(getFormat(), getWidth(), getHeight(), getDepth(), getNumFaces(), numMips);

        for (size_t face = 0; face < getNumFaces(); ++face)
        {
            for (uint32 mip = 0; mip < numMips; ++mip)
            {
                getBuffer(face, mip)->blitToMemory(destImage.getPixelBox(face, mip));
            }
        }
    }

    //--------------------------------------------------------------------------
    void Texture::getCustomAttribute(const String&, void*)
    {
    }

    void Texture::readImage(LoadedImages& imgs, const String& name, const String& ext, bool haveNPOT)
    {
        DataStreamPtr dstream = ResourceGroupManager::getSingleton().openResource(name, mGroup, this);

        imgs.push_back(Image());
        Image& img = imgs.back();
        img.load(dstream, ext);

        if( haveNPOT )
            return;

        // Scale to nearest power of 2
        uint32 w = Bitwise::firstPO2From(img.getWidth());
        uint32 h = Bitwise::firstPO2From(img.getHeight());
        if((img.getWidth() != w) || (img.getHeight() != h))
            img.resize(w, h);
    }

    void Texture::prepareImpl(void)
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

        try
        {
            if(mLayerNames.empty())
            {
                readImage(loadedImages, mName, ext, haveNPOT);

                // If this is a volumetric texture set the texture type flag accordingly.
                // If this is a cube map, set the texture type flag accordingly.
                if (loadedImages[0].hasFlag(IF_CUBEMAP))
                    mTextureType = TEX_TYPE_CUBE_MAP;
                // If this is a volumetric texture set the texture type flag accordingly.
                if (loadedImages[0].getDepth() > 1 && mTextureType != TEX_TYPE_2D_ARRAY)
                    mTextureType = TEX_TYPE_3D;
            }
        }
        catch(const FileNotFoundException&)
        {
            if(mTextureType == TEX_TYPE_CUBE_MAP)
            {
                mLayerNames.resize(6);
                for (size_t i = 0; i < 6; i++)
                    mLayerNames[i] = StringUtil::format("%s%s.%s", baseName.c_str(), CUBEMAP_SUFFIXES[i], ext.c_str());
            }
            else if (mTextureType == TEX_TYPE_2D_ARRAY)
            { // ignore
            }
            else
                throw; // rethrow
        }

        // read sub-images
        for(const String& name : mLayerNames)
        {
            StringUtil::splitBaseFilename(name, baseName, ext);
            readImage(loadedImages, name, ext, haveNPOT);
        }

        // If compressed and 0 custom mipmap, disable auto mip generation and
        // disable software mipmap creation.
        // Not supported by GLES.
        if (PixelUtil::isCompressed(loadedImages[0].getFormat()) &&
            !renderCaps->hasCapability(RSC_AUTOMIPMAP_COMPRESSED) && loadedImages[0].getNumMipmaps() == 0)
        {
            mNumMipmaps = mNumRequestedMipmaps = 0;
            // Disable flag for auto mip generation
            mUsage &= ~TU_AUTOMIPMAP;
        }

        // avoid copying Image data
        std::swap(mLoadedImages, loadedImages);
    }

    void Texture::unprepareImpl()
    {
        mLoadedImages.clear();
    }

    void Texture::loadImpl()
    {
        if (mUsage & TU_RENDERTARGET)
        {
            createInternalResources();
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
}
