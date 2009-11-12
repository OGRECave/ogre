/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreLogManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreImage.h"
#include "OgreTexture.h"
#include "OgreException.h"
#include "OgreResourceManager.h"
#include "OgreTextureManager.h"

namespace Ogre {
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
		img.loadRawData(stream, uWidth, uHeight, eFormat);
		loadImage(img);
	}
	//--------------------------------------------------------------------------    
	void Texture::loadImage( const Image &img )
	{

        LoadingState old = mLoadingState.get();
        if (old!=LOADSTATE_UNLOADED && old!=LOADSTATE_PREPARED) return;

        if (!mLoadingState.cas(old,LOADSTATE_LOADING)) return;

		// Scope lock for actual loading
		try
		{
			OGRE_LOCK_AUTO_MUTEX
			vector<const Image*>::type imagePtrs;
			imagePtrs.push_back(&img);
			_loadImages( imagePtrs );

		}
		catch (...)
		{
			// Reset loading in-progress flag in case failed for some reason
			mLoadingState.set(old);
			// Re-throw
			throw;
		}

        mLoadingState.set(LOADSTATE_LOADED);

		// Notify manager
		if(mCreator)
			mCreator->_notifyResourceLoaded(this);

		// No deferred loading events since this method is not called in background


	}
    //--------------------------------------------------------------------------
    void Texture::setFormat(PixelFormat pf)
    {
        mFormat = pf;
        mDesiredFormat = pf;
        mSrcFormat = pf;
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
		if(images.size() < 1)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot load empty vector of images",
			 "Texture::loadImages");
        
		// Set desired texture size and properties from images[0]
		mSrcWidth = mWidth = images[0]->getWidth();
		mSrcHeight = mHeight = images[0]->getHeight();
		mSrcDepth = mDepth = images[0]->getDepth();

        // Get source image format and adjust if required
        mSrcFormat = images[0]->getFormat();
        if (mTreatLuminanceAsAlpha && mSrcFormat == PF_L8)
        {
            mSrcFormat = PF_A8;
        }

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
        size_t imageMips = images[0]->getNumMipmaps();

		if(imageMips > 0) {
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
		
		// Check wether number of faces in images exceeds number of faces
		// in this texture. If so, clamp it.
		if(faces > getNumFaces())
			faces = getNumFaces();
		
        if (TextureManager::getSingleton().getVerbose()) {
            // Say what we're doing
            StringUtil::StrStreamType str;
            str << "Texture: " << mName << ": Loading " << faces << " faces"
                << "(" << PixelUtil::getFormatName(images[0]->getFormat()) << "," <<
                images[0]->getWidth() << "x" << images[0]->getHeight() << "x" << images[0]->getDepth() <<
                ") with ";
            if (!(mMipmapsHardwareGenerated && mNumMipmaps == 0))
                str << mNumMipmaps;
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
            // Scoped
            {
                // Print data about first destination surface
                HardwarePixelBufferSharedPtr buf = getBuffer(0, 0); 
                str << " Internal format is " << PixelUtil::getFormatName(buf->getFormat()) << 
                "," << buf->getWidth() << "x" << buf->getHeight() << "x" << buf->getDepth() << ".";
            }
            LogManager::getSingleton().logMessage( 
                    LML_NORMAL, str.str());
        }
		
		// Main loading loop
        // imageMips == 0 if the image has no custom mipmaps, otherwise contains the number of custom mips
        for(size_t mip = 0; mip<=imageMips; ++mip)
        {
            for(size_t i = 0; i < faces; ++i)
            {
                PixelBox src;
                if(multiImage)
                {
                    // Load from multiple images
                    src = images[i]->getPixelBox(0, mip);
                }
                else
                {
                    // Load from faces of images[0]
                    src = images[0]->getPixelBox(i, mip);
                }
    
                // Sets to treated format in case is difference
                src.format = mSrcFormat;

                if(mGamma != 1.0f) {
                    // Apply gamma correction
                    // Do not overwrite original image but do gamma correction in temporary buffer
                    MemoryDataStreamPtr buf; // for scoped deletion of conversion buffer
                    buf.bind(OGRE_NEW MemoryDataStream(
                        PixelUtil::getMemorySize(
                            src.getWidth(), src.getHeight(), src.getDepth(), src.format)));
                    
                    PixelBox corrected = PixelBox(src.getWidth(), src.getHeight(), src.getDepth(), src.format, buf->getPtr());
                    PixelUtil::bulkPixelConversion(src, corrected);
                    
                    Image::applyGamma(static_cast<uint8*>(corrected.data), mGamma, corrected.getConsecutiveSize(), 
                        static_cast<uchar>(PixelUtil::getNumElemBits(src.format)));
    
                    // Destination: entire texture. blitFromMemory does the scaling to
                    // a power of two for us when needed
                    getBuffer(i, mip)->blitFromMemory(corrected);
                }
                else 
                {
                    // Destination: entire texture. blitFromMemory does the scaling to
                    // a power of two for us when needed
                    getBuffer(i, mip)->blitFromMemory(src);
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
			return StringUtil::BLANK;

		String::size_type pos = mName.find_last_of(".");
		if (pos != String::npos && pos < (mName.length() - 1))
		{
			String ext = mName.substr(pos+1);
			StringUtil::toLowerCase(ext);
			return ext;
		}
		else
		{
			// No extension
			DataStreamPtr dstream;
			try
			{
				dstream = ResourceGroupManager::getSingleton().openResource(
						mName, mGroup, true, 0);
			}
			catch (Exception&)
			{
			}
			if (dstream.isNull() && getTextureType() == TEX_TYPE_CUBE_MAP)
			{
				// try again with one of the faces (non-dds)
				try
				{
					dstream = ResourceGroupManager::getSingleton().openResource(
						mName + "_rt", mGroup, true, 0);
				}
				catch (Exception&)
				{
				}
			}

			if (!dstream.isNull())
			{
				return Image::getFileExtFromMagic(dstream);
			}
		}

		return StringUtil::BLANK;

	}
	//---------------------------------------------------------------------
	void Texture::convertToImage(Image& destImage, bool includeMipMaps)
	{

		size_t numMips = includeMipMaps? getNumMipmaps() + 1 : 1;
		size_t dataSize = Image::calculateSize(numMips,
			getNumFaces(), getWidth(), getHeight(), getDepth(), getFormat());

		void* pixData = OGRE_MALLOC(dataSize, Ogre::MEMCATEGORY_GENERAL);
		// if there are multiple faces and mipmaps we must pack them into the data
		// faces, then mips
		void* currentPixData = pixData;
		for (size_t face = 0; face < getNumFaces(); ++face)
		{
			for (size_t mip = 0; mip < numMips; ++mip)
			{
				size_t mipDataSize = PixelUtil::getMemorySize(getWidth(), getHeight(), getDepth(), getFormat());

				Ogre::PixelBox pixBox(getWidth(), getHeight(), getDepth(), getFormat(), currentPixData);
				getBuffer(face, mip)->blitToMemory(pixBox);

				currentPixData = (void*)((char*)currentPixData + mipDataSize);

			}
		}


		// load, and tell Image to delete the memory when it's done.
		destImage.loadDynamicImage((Ogre::uchar*)pixData, getWidth(), getHeight(), getDepth(), getFormat(), true, 
			getNumFaces(), numMips - 1);

	}


}
