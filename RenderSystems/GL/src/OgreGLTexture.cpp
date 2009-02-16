/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include "OgreGLTexture.h"
#include "OgreGLSupport.h"
#include "OgreGLPixelFormat.h"
#include "OgreGLHardwarePixelBuffer.h"

#include "OgreTextureManager.h"
#include "OgreImage.h"
#include "OgreLogManager.h"
#include "OgreCamera.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreCodec.h"
#include "OgreImageCodec.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"

#include "OgreGLFBORenderTexture.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX // required to stop windows.h messing up std::min
#  include <windows.h>
#  include <wingdi.h>
#endif

namespace Ogre {



    GLTexture::GLTexture(ResourceManager* creator, const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader, GLSupport& support) 
        : Texture(creator, name, handle, group, isManual, loader),
        mTextureID(0), mGLSupport(support)
    {
    }


    GLTexture::~GLTexture()
    {
        // have to call this here reather than in Resource destructor
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

    GLenum GLTexture::getGLTextureTarget(void) const
    {
        switch(mTextureType)
        {
            case TEX_TYPE_1D:
                return GL_TEXTURE_1D;
            case TEX_TYPE_2D:
                return GL_TEXTURE_2D;
            case TEX_TYPE_3D:
                return GL_TEXTURE_3D;
            case TEX_TYPE_CUBE_MAP:
                return GL_TEXTURE_CUBE_MAP;
            default:
                return 0;
        };
    }

	//* Creation / loading methods ********************************************
	void GLTexture::createInternalResourcesImpl(void)
    {
		// Convert to nearest power-of-two size if required
        mWidth = GLPixelUtil::optionalPO2(mWidth);      
        mHeight = GLPixelUtil::optionalPO2(mHeight);
        mDepth = GLPixelUtil::optionalPO2(mDepth);
		

		// Adjust format if required
		mFormat = TextureManager::getSingleton().getNativeFormat(mTextureType, mFormat, mUsage);
		
		// Check requested number of mipmaps
		size_t maxMips = GLPixelUtil::getMaxMipmaps(mWidth, mHeight, mDepth, mFormat);
		mNumMipmaps = mNumRequestedMipmaps;
		if(mNumMipmaps>maxMips)
			mNumMipmaps = maxMips;
		
		// Generate texture name
        glGenTexturesEXT( 1, &mTextureID );
		
		// Set texture type
		glBindTextureEXT( getGLTextureTarget(), mTextureID );
        
		// This needs to be set otherwise the texture doesn't get rendered
        glTexParameteri( getGLTextureTarget(), GL_TEXTURE_MAX_LEVEL, mNumMipmaps );
        
        // Set some misc default parameters so NVidia won't complain, these can of course be changed later
        glTexParameteri(getGLTextureTarget(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(getGLTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(getGLTextureTarget(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(getGLTextureTarget(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		// If we can do automip generation and the user desires this, do so
		mMipmapsHardwareGenerated = 
			Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_AUTOMIPMAP);
		// NVIDIA 175.16 drivers break hardware mip generation for non-compressed
		// textures - disable until fixed
		// Leave hardware gen on compressed textures since that's the only way we
		// can realistically do it since GLU doesn't support DXT
		// However DON'T do this on Apple, their drivers aren't subject to this
		// problem yet and in fact software generation appears to cause a crash 
		// in some cases which I've yet to track down
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE
		if (Root::getSingleton().getRenderSystem()->getCapabilities()->getVendor() == GPU_NVIDIA
			&& !PixelUtil::isCompressed(mFormat))
		{
			mMipmapsHardwareGenerated = false;
		}
#endif
		if((mUsage & TU_AUTOMIPMAP) &&
		    mNumRequestedMipmaps && mMipmapsHardwareGenerated)
        {
            glTexParameteri( getGLTextureTarget(), GL_GENERATE_MIPMAP, GL_TRUE );
        }
		
		// Allocate internal buffer so that glTexSubImageXD can be used
		// Internal format
		GLenum format = GLPixelUtil::getClosestGLInternalFormat(mFormat, mHwGamma);
		size_t width = mWidth;
		size_t height = mHeight;
		size_t depth = mDepth;

		if(PixelUtil::isCompressed(mFormat))
		{
			// Compressed formats
			size_t size = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);
			// Provide temporary buffer filled with zeroes as glCompressedTexImageXD does not
			// accept a 0 pointer like normal glTexImageXD
			// Run through this process for every mipmap to pregenerate mipmap piramid
			uint8 *tmpdata = new uint8[size];
			memset(tmpdata, 0, size);
			
			for(size_t mip=0; mip<=mNumMipmaps; mip++)
			{
				size = PixelUtil::getMemorySize(width, height, depth, mFormat);
				switch(mTextureType)
				{
					case TEX_TYPE_1D:
						glCompressedTexImage1DARB(GL_TEXTURE_1D, mip, format, 
							width, 0, 
							size, tmpdata);
						break;
					case TEX_TYPE_2D:
						glCompressedTexImage2DARB(GL_TEXTURE_2D, mip, format,
							width, height, 0, 
							size, tmpdata);
						break;
					case TEX_TYPE_3D:
						glCompressedTexImage3DARB(GL_TEXTURE_3D, mip, format,
							width, height, depth, 0, 
							size, tmpdata);
						break;
					case TEX_TYPE_CUBE_MAP:
						for(int face=0; face<6; face++) {
							glCompressedTexImage2DARB(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, format,
								width, height, 0, 
								size, tmpdata);
						}
						break;
				};
				if(width>1)		width = width/2;
				if(height>1)	height = height/2;
				if(depth>1)		depth = depth/2;
			}
			delete [] tmpdata;
		}
		else
		{
			// Run through this process to pregenerate mipmap piramid
			for(size_t mip=0; mip<=mNumMipmaps; mip++)
			{
				// Normal formats
				switch(mTextureType)
				{
					case TEX_TYPE_1D:
						glTexImage1D(GL_TEXTURE_1D, mip, format,
							width, 0, 
							GL_RGBA, GL_UNSIGNED_BYTE, 0);
	
						break;
					case TEX_TYPE_2D:
						glTexImage2D(GL_TEXTURE_2D, mip, format,
							width, height, 0, 
							GL_RGBA, GL_UNSIGNED_BYTE, 0);
						break;
					case TEX_TYPE_3D:
						glTexImage3D(GL_TEXTURE_3D, mip, format,
							width, height, depth, 0, 
							GL_RGBA, GL_UNSIGNED_BYTE, 0);
						break;
					case TEX_TYPE_CUBE_MAP:
						for(int face=0; face<6; face++) {
							glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, format,
								width, height, 0, 
								GL_RGBA, GL_UNSIGNED_BYTE, 0);
						}
						break;
				};
				if(width>1)		width = width/2;
				if(height>1)	height = height/2;
				if(depth>1)		depth = depth/2;
			}
		}
		_createSurfaceList();
		// Get final internal format
		mFormat = getBuffer(0,0)->getFormat();
	}
	
    void GLTexture::createRenderTexture(void)
    {
        // Create the GL texture
		// This already does everything neccessary
        createInternalResources();
    }

    static inline void do_image_io(const String &name, const String &group,
                                   const String &ext,
                                   vector<Image>::type &images,
                                   Resource *r)
    {
		size_t imgIdx = images.size();
        images.push_back(Image());

        DataStreamPtr dstream = 
            ResourceGroupManager::getSingleton().openResource(
                name, group, true, r);

        images[imgIdx].load(dstream, ext);
    }

	
    void GLTexture::prepareImpl()
    {
        if( mUsage & TU_RENDERTARGET ) return;

        String baseName, ext;
        size_t pos = mName.find_last_of(".");
        baseName = mName.substr(0, pos);
        if( pos != String::npos )
            ext = mName.substr(pos+1);

        LoadedImages loadedImages = LoadedImages(new vector<Image>::type());

        if(mTextureType == TEX_TYPE_1D || mTextureType == TEX_TYPE_2D || 
            mTextureType == TEX_TYPE_3D)
        {

            do_image_io(mName, mGroup, ext, *loadedImages, this);


            // If this is a cube map, set the texture type flag accordingly.
            if ((*loadedImages)[0].hasFlag(IF_CUBEMAP))
                mTextureType = TEX_TYPE_CUBE_MAP;
            // If this is a volumetric texture set the texture type flag accordingly.
            if((*loadedImages)[0].getDepth() > 1)
                mTextureType = TEX_TYPE_3D;

        }
        else if (mTextureType == TEX_TYPE_CUBE_MAP)
        {
            if(getSourceFileType() == "dds")
            {
                // XX HACK there should be a better way to specify whether 
                // all faces are in the same file or not
                do_image_io(mName, mGroup, ext, *loadedImages, this);
            }
            else
            {
                vector<Image>::type images(6);
                ConstImagePtrList imagePtrs;
                static const String suffixes[6] = {"_rt", "_lf", "_up", "_dn", "_fr", "_bk"};

                for(size_t i = 0; i < 6; i++)
                {
                    String fullName = baseName + suffixes[i];
                    if (!ext.empty())
                        fullName = fullName + "." + ext;
                    // find & load resource data intro stream to allow resource
                    // group changes if required
                    do_image_io(fullName,mGroup,ext,*loadedImages,this);
                }
            }
        }
        else
            OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "**** Unknown texture type ****", "GLTexture::prepare" );

        mLoadedImages = loadedImages;
    }
	
    void GLTexture::unprepareImpl()
    {
        mLoadedImages.setNull();
    }

    void GLTexture::loadImpl()
    {
        if( mUsage & TU_RENDERTARGET )
        {
            createRenderTexture();
            return;
        }

        // Now the only copy is on the stack and will be cleaned in case of
        // exceptions being thrown from _loadImages
        LoadedImages loadedImages = mLoadedImages;
        mLoadedImages.setNull();

        // Call internal _loadImages, not loadImage since that's external and 
        // will determine load status etc again
        ConstImagePtrList imagePtrs;
        for (size_t i=0 ; i<loadedImages->size() ; ++i) {
            imagePtrs.push_back(&(*loadedImages)[i]);
        }

        _loadImages(imagePtrs);

    }

	//*************************************************************************
    
    void GLTexture::freeInternalResourcesImpl()
    {
		mSurfaceList.clear();
        glDeleteTextures( 1, &mTextureID );
    }

	
	//---------------------------------------------------------------------------------------------
	void GLTexture::_createSurfaceList()
	{
		mSurfaceList.clear();
		
		// For all faces and mipmaps, store surfaces as HardwarePixelBufferSharedPtr
		bool wantGeneratedMips = (mUsage & TU_AUTOMIPMAP)!=0;
		
		// Do mipmapping in software? (uses GLU) For some cards, this is still needed. Of course,
		// only when mipmap generation is desired.
		bool doSoftware = wantGeneratedMips && !mMipmapsHardwareGenerated && getNumMipmaps(); 
		
		for(size_t face=0; face<getNumFaces(); face++)
		{
			for(size_t mip=0; mip<=getNumMipmaps(); mip++)
			{
                GLHardwarePixelBuffer *buf = new GLTextureBuffer(mName, getGLTextureTarget(), mTextureID, face, mip,
						static_cast<HardwareBuffer::Usage>(mUsage), doSoftware && mip==0, mHwGamma, mFSAA);
				mSurfaceList.push_back(HardwarePixelBufferSharedPtr(buf));
                
                /// Check for error
                if(buf->getWidth()==0 || buf->getHeight()==0 || buf->getDepth()==0)
                {
                    OGRE_EXCEPT(
                        Exception::ERR_RENDERINGAPI_ERROR, 
                        "Zero sized texture surface on texture "+getName()+
                            " face "+StringConverter::toString(face)+
                            " mipmap "+StringConverter::toString(mip)+
                            ". Probably, the GL driver refused to create the texture.", 
                            "GLTexture::_createSurfaceList");
                }
			}
		}
	}
	
	//---------------------------------------------------------------------------------------------
	HardwarePixelBufferSharedPtr GLTexture::getBuffer(size_t face, size_t mipmap)
	{
		if(face >= getNumFaces())
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Face index out of range",
					"GLTexture::getBuffer");
		if(mipmap > mNumMipmaps)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Mipmap index out of range",
					"GLTexture::getBuffer");
		unsigned int idx = face*(mNumMipmaps+1) + mipmap;
		assert(idx < mSurfaceList.size());
		return mSurfaceList[idx];
	}
	
}

