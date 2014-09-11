/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#include "OgreGLESTexture.h"
#include "OgreGLESSupport.h"
#include "OgreGLESPixelFormat.h"
#include "OgreGLESRenderSystem.h"
#include "OgreGLESHardwarePixelBuffer.h"
#include "OgreGLESStateCacheManager.h"
#include "OgreRoot.h"

namespace Ogre {
    static inline void doImageIO(const String &name, const String &group,
                                 const String &ext,
                                 std::vector<Image> &images,
                                 Resource *r)
    {
        size_t imgIdx = images.size();
        images.push_back(Image());

        DataStreamPtr dstream =
            ResourceGroupManager::getSingleton().openResource(
                name, group, true, r);

        images[imgIdx].load(dstream, ext);

        size_t w = 0, h = 0;
        
        // Scale to nearest power of 2
        w = GLESPixelUtil::optionalPO2(images[imgIdx].getWidth());
        h = GLESPixelUtil::optionalPO2(images[imgIdx].getHeight());
        if((images[imgIdx].getWidth() != w) || (images[imgIdx].getHeight() != h))
            images[imgIdx].resize(w, h);
    }

    GLESTexture::GLESTexture(ResourceManager* creator, const String& name,
                             ResourceHandle handle, const String& group, bool isManual,
                             ManualResourceLoader* loader, GLESSupport& support)
        : Texture(creator, name, handle, group, isManual, loader),
          mTextureID(0), mGLSupport(support)
    {
    }

    GLESTexture::~GLESTexture()
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

    GLenum GLESTexture::getGLESTextureTarget(void) const
    {
        switch(mTextureType)
        {
            case TEX_TYPE_1D:
            case TEX_TYPE_2D:
                return GL_TEXTURE_2D;
#ifdef GL_OES_texture_cube_map
            case TEX_TYPE_CUBE_MAP:
                return GL_TEXTURE_CUBE_MAP_OES;
#endif
            default:
                return 0;
        };
    }

    void GLESTexture::_createGLTexResource()
    {
        // Convert to nearest power-of-two size if required
        mWidth = GLESPixelUtil::optionalPO2(mWidth);
        mHeight = GLESPixelUtil::optionalPO2(mHeight);
        mDepth = GLESPixelUtil::optionalPO2(mDepth);
        
		// Adjust format if required
        mFormat = TextureManager::getSingleton().getNativeFormat(mTextureType, mFormat, mUsage);
        
		// Check requested number of mipmaps
        size_t maxMips = GLESPixelUtil::getMaxMipmaps(mWidth, mHeight, mDepth, mFormat);
        
        if(PixelUtil::isCompressed(mFormat) && (mNumMipmaps == 0))
            mNumRequestedMipmaps = 0;
        
        mNumMipmaps = mNumRequestedMipmaps;
        if (mNumMipmaps > maxMips)
            mNumMipmaps = maxMips;
        
		// Generate texture name
        glGenTextures(1, &mTextureID);
        GL_CHECK_ERROR;
        
		// Set texture type
        mGLSupport.getStateCacheManager()->bindGLTexture(getGLESTextureTarget(), mTextureID);
        
        // Set some misc default parameters, these can of course be changed later
        mGLSupport.getStateCacheManager()->setTexParameteri(getGLESTextureTarget(),
                                                             GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

        mGLSupport.getStateCacheManager()->setTexParameteri(getGLESTextureTarget(),
                                                             GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        mGLSupport.getStateCacheManager()->setTexParameteri(getGLESTextureTarget(),
                                                             GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        
        mGLSupport.getStateCacheManager()->setTexParameteri(getGLESTextureTarget(),
                                                             GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
		// If we can do automip generation and the user desires this, do so
        mMipmapsHardwareGenerated =
        Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_AUTOMIPMAP) && !PixelUtil::isCompressed(mFormat);
        
        if ((mUsage & TU_AUTOMIPMAP) &&
            mNumRequestedMipmaps && mMipmapsHardwareGenerated)
        {
            glTexParameteri(getGLESTextureTarget(), GL_GENERATE_MIPMAP, GL_TRUE);
            GL_CHECK_ERROR;
        }
        
        // Allocate internal buffer so that glTexSubImageXD can be used
        // Internal format
        GLenum format = GLESPixelUtil::getGLOriginFormat(mFormat);
        GLenum internalformat = GLESPixelUtil::getClosestGLInternalFormat(mFormat, mHwGamma);
        GLenum datatype = GLESPixelUtil::getGLOriginDataType(mFormat);
        size_t width = mWidth;
        size_t height = mHeight;
        size_t depth = mDepth;
        
        if (PixelUtil::isCompressed(mFormat))
        {
            //            LogManager::getSingleton().logMessage("GLESTexture::create - Mip: " + StringConverter::toString(mip) +
            //                                                  " Width: " + StringConverter::toString(width) +
            //                                                  " Height: " + StringConverter::toString(height) +
            //                                                  " Internal Format: " + StringConverter::toString(internalformat, 0, ' ', std::ios::hex) +
            //                                                  " Format: " + StringConverter::toString(format, 0, ' ', std::ios::hex) +
            //                                                  " Datatype: " + StringConverter::toString(datatype, 0, ' ', std::ios::hex)
            //                                                  );
            
            // Compressed formats
            size_t size = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);
            
            // Provide temporary buffer filled with zeroes as glCompressedTexImageXD does not
            // accept a 0 pointer like normal glTexImageXD
            // Run through this process for every mipmap to pregenerate mipmap pyramid
            
            uint8* tmpdata = new uint8[size];
            memset(tmpdata, 0, size);
            for (size_t mip = 0; mip <= mNumMipmaps; mip++)
            {
                size = PixelUtil::getMemorySize(width, height, depth, mFormat);
                
                switch(mTextureType)
				{
					case TEX_TYPE_1D:
					case TEX_TYPE_2D:
                        glCompressedTexImage2D(GL_TEXTURE_2D,
                                               mip,
                                               internalformat,
                                               width, height,
                                               0,
                                               size,
                                               tmpdata);
                        GL_CHECK_ERROR;
                        break;
#ifdef GL_OES_texture_cube_map
                    case TEX_TYPE_CUBE_MAP:
						for(int face = 0; face < 6; face++) {
							glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_OES + face, mip, internalformat,
                                                   width, height, 0,
                                                   size, tmpdata);
                            GL_CHECK_ERROR;
						}
						break;
#endif
                    default:
                        break;
                };
                
                //                LogManager::getSingleton().logMessage("GLESTexture::create - Mip: " + StringConverter::toString(mip) +
                //                                                      " Width: " + StringConverter::toString(width) +
                //                                                      " Height: " + StringConverter::toString(height) +
                //                                                      " Internal Format: " + StringConverter::toString(format)
                //                                                      );
                
                if(width > 1)
                {
                    width = width / 2;
                }
                if(height > 1)
                {
                    height = height / 2;
                }
                if(depth > 1)
                {
                    depth = depth / 2;
                }
            }
            delete [] tmpdata;
        }
        else
        {
            // Run through this process to pregenerate mipmap pyramid
            for(size_t mip = 0; mip <= mNumMipmaps; mip++)
            {
                switch(mTextureType)
				{
					case TEX_TYPE_1D:
					case TEX_TYPE_2D:
                        glTexImage2D(GL_TEXTURE_2D,
                                     mip,
                                     internalformat,
                                     width, height,
                                     0,
                                     format,
                                     datatype, 0);
                        GL_CHECK_ERROR;
                        break;
#ifdef GL_OES_texture_cube_map
					case TEX_TYPE_CUBE_MAP:
						for(int face = 0; face < 6; face++) {
							glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_OES + face, mip, internalformat,
                                         width, height, 0,
                                         format, datatype, 0);
						}
						break;
#endif
                    default:
                        break;
                };
                
                if (width > 1)
                {
                    width = width / 2;
                }
                if (height > 1)
                {
                    height = height / 2;
                }
            }
        }
    }
    
    // Creation / loading methods
    void GLESTexture::createInternalResourcesImpl(void)
    {
		_createGLTexResource();
        
        _createSurfaceList();

        // Get final internal format
        mFormat = getBuffer(0,0)->getFormat();
    }

    void GLESTexture::createRenderTexture(void)
    {
        // Create the GL texture
        // This already does everything necessary
        createInternalResources();
    }

    void GLESTexture::prepareImpl()
    {
        if (mUsage & TU_RENDERTARGET) return;

        String baseName, ext;
        size_t pos = mName.find_last_of(".");
        baseName = mName.substr(0, pos);

        if (pos != String::npos)
        {
            ext = mName.substr(pos+1);
        }

        LoadedImages loadedImages = LoadedImages(new std::vector<Image>());

        if (mTextureType == TEX_TYPE_1D || mTextureType == TEX_TYPE_2D)
        {
            doImageIO(mName, mGroup, ext, *loadedImages, this);
            
            // If this is a volumetric texture set the texture type flag accordingly.
            // If this is a cube map, set the texture type flag accordingly.
#ifdef GL_OES_texture_cube_map
            if ((*loadedImages)[0].hasFlag(IF_CUBEMAP))
                mTextureType = TEX_TYPE_CUBE_MAP;
#else
            if ((*loadedImages)[0].getDepth() > 1)
            {
                OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                            "**** Unsupported 3D texture type ****",
                            "GLESTexture::prepare" );
            }
#endif
            
			// If PVRTC and 0 custom mipmap disable auto mip generation and disable software mipmap creation
            PixelFormat imageFormat = (*loadedImages)[0].getFormat();
			if (imageFormat == PF_PVRTC_RGB2 || imageFormat == PF_PVRTC_RGBA2 ||
                imageFormat == PF_PVRTC_RGB4 || imageFormat == PF_PVRTC_RGBA4)
			{
                size_t imageMips = (*loadedImages)[0].getNumMipmaps();
                if (imageMips == 0)
                {
                    mNumMipmaps = mNumRequestedMipmaps = imageMips;
                    // Disable flag for auto mip generation
                    mUsage &= ~TU_AUTOMIPMAP;
                }
			}
        }
#ifdef GL_OES_texture_cube_map
        else if (mTextureType == TEX_TYPE_CUBE_MAP)
        {
            if(getSourceFileType() == "dds")
            {
                // XX HACK there should be a better way to specify whether
                // all faces are in the same file or not
                doImageIO(mName, mGroup, ext, *loadedImages, this);
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
                    doImageIO(fullName, mGroup, ext, *loadedImages, this);
                }
            }
        }
#endif
        else
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                        "**** Unknown texture type ****",
                        "GLESTexture::prepare");
        }

        mLoadedImages = loadedImages;
    }

    void GLESTexture::unprepareImpl()
    {
        mLoadedImages.setNull();
    }

    void GLESTexture::loadImpl()
    {
        if (mUsage & TU_RENDERTARGET)
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

        for (size_t i = 0; i < loadedImages->size(); ++i)
        {
            imagePtrs.push_back(&(*loadedImages)[i]);
        }

        _loadImages(imagePtrs);
    }

    void GLESTexture::freeInternalResourcesImpl()
    {
        mSurfaceList.clear();
        glDeleteTextures(1, &mTextureID);
        GL_CHECK_ERROR;
    }

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    void GLESTexture::notifyOnContextLost()
    {
        if (!mIsManual)
        {
            freeInternalResources();
        }
        else
        {
            glDeleteTextures(1, &mTextureID);
            GL_CHECK_ERROR;
            mTextureID = 0;
        }
    }
    
    void GLESTexture::notifyOnContextReset()
    {
        if (!mIsManual)
        {
            reload();
        }
        else
        {
            preLoadImpl();
            
            _createGLTexResource();
            
            for(size_t i = 0; i < mSurfaceList.size(); i++)
            {
                static_cast<GLESTextureBuffer*>(mSurfaceList[i].getPointer())->updateTextureId(mTextureID);
            }
            
            if (mLoader)
            {
                mLoader->loadResource(this);
            }
            
            postLoadImpl();
        }
    }
#endif

    void GLESTexture::_createSurfaceList()
    {
        mSurfaceList.clear();

        // For all faces and mipmaps, store surfaces as HardwarePixelBufferSharedPtr
        bool wantGeneratedMips = (mUsage & TU_AUTOMIPMAP)!=0;

        // Do mipmapping in software? (uses GLU) For some cards, this is still needed. Of course,
        // only when mipmap generation is desired.
        bool doSoftware = wantGeneratedMips && !mMipmapsHardwareGenerated && getNumMipmaps();

        for (size_t face = 0; face < getNumFaces(); face++)
        {
			size_t width = mWidth;
			size_t height = mHeight;
            for (size_t mip = 0; mip <= getNumMipmaps(); mip++)
            {
                GLESHardwarePixelBuffer *buf = OGRE_NEW GLESTextureBuffer(mName,
                                                                     getGLESTextureTarget(),
                                                                     mTextureID,
                                                                     width, height,
                                                                     GLESPixelUtil::getClosestGLInternalFormat(mFormat, mHwGamma),
                                                                     GLESPixelUtil::getGLOriginDataType(mFormat),
                                                                     face,
                                                                     mip,
                                                                     static_cast<HardwareBuffer::Usage>(mUsage),
                                                                     doSoftware && mip==0, mHwGamma, mFSAA);

                mSurfaceList.push_back(HardwarePixelBufferSharedPtr(buf));

                /// Check for error
                if (buf->getWidth() == 0 ||
                    buf->getHeight() == 0 ||
                    buf->getDepth() == 0)
                {
                    OGRE_EXCEPT(
                        Exception::ERR_RENDERINGAPI_ERROR,
                        "Zero sized texture surface on texture "+getName()+
                            " face "+StringConverter::toString(face)+
                            " mipmap "+StringConverter::toString(mip)+
                            ". The GL driver probably refused to create the texture.",
                            "GLESTexture::_createSurfaceList");
                }
            }
        }
    }

    HardwarePixelBufferSharedPtr GLESTexture::getBuffer(size_t face, size_t mipmap)
    {
        if (face >= getNumFaces())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Face index out of range",
                        "GLESTexture::getBuffer");
        }

        if (mipmap > mNumMipmaps)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Mipmap index out of range",
                        "GLESTexture::getBuffer");
        }

        unsigned int idx = face * (mNumMipmaps + 1) + mipmap;
        assert(idx < mSurfaceList.size());
        return mSurfaceList[idx];
    }
    
    void GLESTexture::getCustomAttribute(const String& name, void* pData)
	{
		if (name == "GLID")
			*static_cast<GLuint*>(pData) = mTextureID;
	}
}
