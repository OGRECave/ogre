/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreGLES2Texture.h"
#include "OgreGLES2PixelFormat.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreGLES2HardwarePixelBuffer.h"
#include "OgreGLES2HlmsSamplerblock.h"
#include "OgreGLES2Support.h"
#include "OgreGLES2StateCacheManager.h"
#include "OgreRoot.h"
#include "OgreBitwise.h"
#include "OgreTextureManager.h"

namespace Ogre {
    static inline void doImageIO(const String &name, const String &group,
                                 const String &ext,
                                 vector<Image>::type &images,
                                 Texture *tex)
    {
        size_t imgIdx = images.size();
        images.push_back(Image());

        DataStreamPtr dstream =
            ResourceGroupManager::getSingleton().openResource(
                name, group, true, tex);

        images[imgIdx].load(dstream, ext);

        if(!Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_NON_POWER_OF_2_TEXTURES) ||
           (Root::getSingleton().getRenderSystem()->getCapabilities()->getNonPOW2TexturesLimited() && tex->getNumMipmaps() > 0))
        {
            size_t w = 0, h = 0;
            
            // Scale to nearest power of 2
            w = GLES2PixelUtil::optionalPO2(images[imgIdx].getWidth());
            h = GLES2PixelUtil::optionalPO2(images[imgIdx].getHeight());
            if((images[imgIdx].getWidth() != w) || (images[imgIdx].getHeight() != h))
                images[imgIdx].resize(w, h);
        }
    }

    GLES2Texture::GLES2Texture(ResourceManager* creator, const String& name,
                             ResourceHandle handle, const String& group, bool isManual,
                             ManualResourceLoader* loader, GLES2Support& support)
        : Texture(creator, name, handle, group, isManual, loader),
          mTextureID(0),
#if OGRE_NO_GLES3_SUPPORT != 0
          mLastBoundSamplerblockRsId(0),
#endif
          mGLSupport(support)
    {
    }

    GLES2Texture::~GLES2Texture()
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

    GLenum GLES2Texture::getGLES2TextureTarget(void) const
    {
        switch(mTextureType)
        {
            case TEX_TYPE_1D:
            case TEX_TYPE_2D:
                return GL_TEXTURE_2D;
            case TEX_TYPE_CUBE_MAP:
                return GL_TEXTURE_CUBE_MAP;
#if OGRE_NO_GLES3_SUPPORT == 0
            case TEX_TYPE_3D:
                return GL_TEXTURE_3D;
            case TEX_TYPE_2D_ARRAY:
                return GL_TEXTURE_2D_ARRAY;
#endif
            default:
                return 0;
        };
    }

    void GLES2Texture::_createGLTexResource()
    {
        if(!Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_NON_POWER_OF_2_TEXTURES) ||
           (Root::getSingleton().getRenderSystem()->getCapabilities()->getNonPOW2TexturesLimited() && mNumRequestedMipmaps > 0))
        {
            // Convert to nearest power-of-two size if required
            mWidth = GLES2PixelUtil::optionalPO2(mWidth);
            mHeight = GLES2PixelUtil::optionalPO2(mHeight);
            mDepth = GLES2PixelUtil::optionalPO2(mDepth);
        }

        // Adjust format if required
        mFormat = TextureManager::getSingleton().getNativeFormat(mTextureType, mFormat, mUsage);
        GLenum texTarget = getGLES2TextureTarget();

        // Check requested number of mipmaps
        size_t maxMips = GLES2PixelUtil::getMaxMipmaps(mWidth, mHeight, mDepth, mFormat);
        
        if(PixelUtil::isCompressed(mFormat) && (mNumMipmaps == 0))
            mNumRequestedMipmaps = 0;
        
        mNumMipmaps = mNumRequestedMipmaps;
        if (mNumMipmaps > maxMips)
            mNumMipmaps = maxMips;

        // Generate texture name
        OGRE_CHECK_GL_ERROR(glGenTextures(1, &mTextureID));
           
        // Set texture type
        mGLSupport.getStateCacheManager()->bindGLTexture(texTarget, mTextureID);
        
        // If we can do automip generation and the user desires this, do so
        mMipmapsHardwareGenerated =
            Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_AUTOMIPMAP) && !PixelUtil::isCompressed(mFormat);

        if(!Bitwise::isPO2(mWidth) || !Bitwise::isPO2(mHeight))
            mMipmapsHardwareGenerated = false;

        // glGenerateMipmap require all mip levels to be prepared. So override how many this texture has.
        if((mUsage & TU_AUTOMIPMAP) && mMipmapsHardwareGenerated && mNumRequestedMipmaps)
            mNumMipmaps = maxMips;

        if(getGLES2SupportRef()->checkExtension("GL_APPLE_texture_max_level") || gleswIsSupported(3, 0))
            mGLSupport.getStateCacheManager()->setTexParameteri(texTarget, GL_TEXTURE_MAX_LEVEL_APPLE, mNumRequestedMipmaps ? mNumMipmaps + 1 : 0);

        // Set some misc default parameters, these can of course be changed later
        mGLSupport.getStateCacheManager()->setTexParameteri(texTarget,
                                                            GL_TEXTURE_MIN_FILTER, ((mUsage & TU_AUTOMIPMAP) && mNumRequestedMipmaps) ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
        mGLSupport.getStateCacheManager()->setTexParameteri(texTarget,
                                                            GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        mGLSupport.getStateCacheManager()->setTexParameteri(texTarget,
                                                            GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        mGLSupport.getStateCacheManager()->setTexParameteri(texTarget,
                                                            GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Set up texture swizzling
#if OGRE_NO_GLES3_SUPPORT == 0
        if(gleswIsSupported(3, 0))
        {
            OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_R, GL_RED));
            OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_G, GL_GREEN));
            OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_B, GL_BLUE));
            OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_A, GL_ALPHA));

            if(mFormat == PF_L8 || mFormat == PF_L16)
            {
                OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_R, GL_RED));
                OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_G, GL_RED));
                OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_B, GL_RED));
                OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_A, GL_RED));
            }
        }
#endif

        // Allocate internal buffer so that glTexSubImageXD can be used
        // Internal format
        GLenum format = GLES2PixelUtil::getGLOriginFormat(mFormat, mHwGamma);
        GLenum internalformat = GLES2PixelUtil::getClosestGLInternalFormat(mFormat, mHwGamma);
        uint32 width = mWidth;
        uint32 height = mHeight;
        uint32 depth = mDepth;
        
        if (PixelUtil::isCompressed(mFormat))
        {
            // Compressed formats
            GLsizei size = static_cast<GLsizei>(PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat));
            
            // Provide temporary buffer filled with zeroes as glCompressedTexImageXD does not
            // accept a 0 pointer like normal glTexImageXD
            // Run through this process for every mipmap to pregenerate mipmap pyramid
            
            uint8* tmpdata = new uint8[size];
            memset(tmpdata, 0, size);
            for (GLint mip = 0; mip <= mNumMipmaps; mip++)
            {
#if OGRE_DEBUG_MODE
                LogManager::getSingleton().logMessage("GLES2Texture::create - Mip: " + StringConverter::toString(mip) +
                                                      " Width: " + StringConverter::toString(width) +
                                                      " Height: " + StringConverter::toString(height) +
                                                      " Internal Format: " + StringConverter::toString(internalformat, 0, ' ', std::ios::hex) +
                                                      " Format: " + StringConverter::toString(format, 0, ' ', std::ios::hex)
                                                      );
#endif
                size = static_cast<GLsizei>(PixelUtil::getMemorySize(width, height, depth, mFormat));
                
                switch(mTextureType)
                {
                    case TEX_TYPE_1D:
                    case TEX_TYPE_2D:
                        OGRE_CHECK_GL_ERROR(glCompressedTexImage2D(GL_TEXTURE_2D,
                                               mip,
                                               internalformat,
                                               width, height,
                                               0,
                                               size,
                                               tmpdata));
                        break;
                    case TEX_TYPE_CUBE_MAP:
                        for(int face = 0; face < 6; face++) {
                            OGRE_CHECK_GL_ERROR(glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, internalformat,
                                width, height, 0, 
                                size, tmpdata));
                        }
                        break;
#if OGRE_NO_GLES3_SUPPORT == 0
                    case TEX_TYPE_2D_ARRAY:
                    case TEX_TYPE_3D:
                        glCompressedTexImage3D(getGLES2TextureTarget(), mip, format,
                            width, height, depth, 0, 
                            size, tmpdata);
                        break;
#endif
                    default:
                        break;
                };
                
                if(width > 1)
                {
                    width = width / 2;
                }
                if(height > 1)
                {
                    height = height / 2;
                }
                if(depth > 1 && mTextureType != TEX_TYPE_2D_ARRAY)
                {
                    depth = depth / 2;
                }
            }
            delete [] tmpdata;
        }
        else
        {
#if OGRE_NO_GLES3_SUPPORT == 0
#if OGRE_DEBUG_MODE
            LogManager::getSingleton().logMessage("GLES2Texture::create - Name: " + mName +
                                                      " ID: " + StringConverter::toString(mTextureID) +
                                                      " Width: " + StringConverter::toString(width) +
                                                      " Height: " + StringConverter::toString(height) +
                                                      " Internal Format: " + StringConverter::toString(internalformat, 0, ' ', std::ios::hex));
#endif
            switch(mTextureType)
            {
                case TEX_TYPE_1D:
                case TEX_TYPE_2D:
                case TEX_TYPE_2D_RECT:
                    OGRE_CHECK_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, GLsizei(mNumMipmaps+1), internalformat, GLsizei(width), GLsizei(height)));
                    break;
                case TEX_TYPE_CUBE_MAP:
                    OGRE_CHECK_GL_ERROR(glTexStorage2D(GL_TEXTURE_CUBE_MAP, GLsizei(mNumMipmaps+1), internalformat, GLsizei(width), GLsizei(height)));
                    break;
                case TEX_TYPE_2D_ARRAY:
                    OGRE_CHECK_GL_ERROR(glTexStorage3D(GL_TEXTURE_2D_ARRAY, GLsizei(mNumMipmaps+1), internalformat, GLsizei(width), GLsizei(height), GLsizei(depth)));
                    break;
                case TEX_TYPE_3D:
                    OGRE_CHECK_GL_ERROR(glTexStorage3D(GL_TEXTURE_3D, GLsizei(mNumMipmaps+1), internalformat, GLsizei(width), GLsizei(height), GLsizei(depth)));
                    break;
            }
#else
            GLenum datatype = GLES2PixelUtil::getGLOriginDataType(mFormat);

            // Run through this process to pregenerate mipmap pyramid
            for(GLint mip = 0; mip <= mNumMipmaps; mip++)
            {
#if OGRE_DEBUG_MODE
                LogManager::getSingleton().logMessage("GLES2Texture::create - Mip: " + StringConverter::toString(mip) +
                                                      " Name: " + mName +
                                                      " ID: " + StringConverter::toString(mTextureID) +
                                                      " Width: " + StringConverter::toString(width) +
                                                      " Height: " + StringConverter::toString(height) +
                                                      " Internal Format: " + StringConverter::toString(internalformat, 0, ' ', std::ios::hex) +
                                                      " Format: " + StringConverter::toString(format, 0, ' ', std::ios::hex) +
                                                      " Datatype: " + StringConverter::toString(datatype, 0, ' ', std::ios::hex)
                                                      );
#endif
                // Normal formats
                switch(mTextureType)
                {
                    case TEX_TYPE_1D:
                    case TEX_TYPE_2D:
#if OGRE_PLATFORM == OGRE_PLATFORM_NACL
                        if(internalformat != format)
                        {
                            LogManager::getSingleton().logMessage("glTexImage2D: format != internalFormat, "
                                "format=" + StringConverter::toString(format) + 
                                ", internalFormat=" + StringConverter::toString(internalformat));
                        }
#endif
                        OGRE_CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D,
                                     mip,
                                     internalformat,
                                     width, height,
                                     0,
                                     format,
                                     datatype, 0));
                        break;
                    case TEX_TYPE_CUBE_MAP:
                        for(int face = 0; face < 6; face++) {
                            OGRE_CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, internalformat,
                                width, height, 0, 
                                format, datatype, 0));
                        }
                        break;
                    default:
                        break;
                };
                
                if (width > 1)
                {
                    width = Bitwise::firstPO2From(width / 2);
                }
                if (height > 1)
                {
                    height = Bitwise::firstPO2From(height / 2);
                }
            }
#endif
        }
    }
    
    // Creation / loading methods
    void GLES2Texture::createInternalResourcesImpl(void)
    {
        _createGLTexResource();
        
        _createSurfaceList();

        // Get final internal format
        mFormat = getBuffer(0,0)->getFormat();
    }

    void GLES2Texture::createRenderTexture(void)
    {
        // Create the GL texture
        // This already does everything necessary
        createInternalResources();
    }

    void GLES2Texture::prepareImpl()
    {
        if (mUsage & TU_RENDERTARGET) return;

        String baseName, ext;
        size_t pos = mName.find_last_of(".");
        baseName = mName.substr(0, pos);

        if (pos != String::npos)
        {
            ext = mName.substr(pos+1);
        }

        LoadedImages loadedImages = LoadedImages(new vector<Image>::type());

        if (mTextureType == TEX_TYPE_1D || mTextureType == TEX_TYPE_2D ||
            mTextureType == TEX_TYPE_2D_ARRAY || mTextureType == TEX_TYPE_3D)

        {
            doImageIO(mName, mGroup, ext, *loadedImages, this);

            // If this is a volumetric texture set the texture type flag accordingly.
            // If this is a cube map, set the texture type flag accordingly.
            if ((*loadedImages)[0].hasFlag(IF_CUBEMAP))
                mTextureType = TEX_TYPE_CUBE_MAP;

            // If this is a volumetric texture set the texture type flag accordingly.
            if((*loadedImages)[0].getDepth() > 1 && mTextureType != TEX_TYPE_2D_ARRAY)
                mTextureType = TEX_TYPE_3D;
                        
            // If PVRTC and 0 custom mipmap disable auto mip generation and disable software mipmap creation
            if (PixelUtil::isCompressed((*loadedImages)[0].getFormat()))
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
        else if (mTextureType == TEX_TYPE_CUBE_MAP)
        {
            if(getSourceFileType() == "dds" || getSourceFileType() == "oitd")
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
        else
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                        "**** Unknown texture type ****",
                        "GLES2Texture::prepare");
        }

        mLoadedImages = loadedImages;
    }

    void GLES2Texture::unprepareImpl()
    {
        mLoadedImages.setNull();
    }

    void GLES2Texture::loadImpl()
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

        if((mUsage & TU_AUTOMIPMAP) &&
           mNumRequestedMipmaps && mMipmapsHardwareGenerated)
        {
            OGRE_CHECK_GL_ERROR(glGenerateMipmap(getGLES2TextureTarget()));
        }
    }

    void GLES2Texture::freeInternalResourcesImpl()
    {
        mSurfaceList.clear();
        OGRE_CHECK_GL_ERROR(glDeleteTextures(1, &mTextureID));
        mGLSupport.getStateCacheManager()->invalidateStateForTexture( mTextureID );
        mTextureID = 0;
    }
    
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
    void GLES2Texture::notifyOnContextLost()
    {
        if (!mIsManual) 
        {
            freeInternalResources();
        }
        else
        {
            OGRE_CHECK_GL_ERROR(glDeleteTextures(1, &mTextureID));
            mTextureID = 0;
        }
    }
    
    void GLES2Texture::notifyOnContextReset()
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
                static_cast<GLES2TextureBuffer*>(mSurfaceList[i].getPointer())->updateTextureId(mTextureID);
            }
            
            if (mLoader)
            {
                mLoader->loadResource(this);
            }
            
            postLoadImpl();
        }
    }
#endif

    void GLES2Texture::_createSurfaceList()
    {
        mSurfaceList.clear();

        // For all faces and mipmaps, store surfaces as HardwarePixelBufferSharedPtr
        bool wantGeneratedMips = (mUsage & TU_AUTOMIPMAP)!=0;

        // Do mipmapping in software? (uses GLU) For some cards, this is still needed. Of course,
        // only when mipmap generation is desired.
        bool doSoftware = wantGeneratedMips && !mMipmapsHardwareGenerated && getNumMipmaps();

        for (size_t face = 0; face < getNumFaces(); face++)
        {
            uint32 width = mWidth;
            uint32 height = mHeight;
            uint32 depth = mDepth;

            for (uint8 mip = 0; mip <= getNumMipmaps(); mip++)
            {
                v1::GLES2HardwarePixelBuffer *buf = OGRE_NEW v1::GLES2TextureBuffer(mName,
                                                                            getGLES2TextureTarget(),
                                                                            mTextureID,
                                                                            width, height, depth,
                                                                            GLES2PixelUtil::getClosestGLInternalFormat(mFormat, mHwGamma),
                                                                            GLES2PixelUtil::getGLOriginDataType(mFormat),
                                                                            static_cast<GLint>(face),
                                                                            mip,
                                                                            static_cast<v1::HardwareBuffer::Usage>(mUsage),
                                                                            doSoftware && mip == 0, mHwGamma, mFSAA);

                mSurfaceList.push_back(v1::HardwarePixelBufferSharedPtr(buf));

                // Check for error
                if (buf->getWidth() == 0 ||
                    buf->getHeight() == 0 ||
                    buf->getDepth() == 0)
                {
                    OGRE_EXCEPT(
                        Exception::ERR_RENDERINGAPI_ERROR,
                        "Zero sized texture surface on texture " + getName() +
                            " face " + StringConverter::toString(face) +
                            " mipmap " + StringConverter::toString(mip) +
                            ". The GL driver probably refused to create the texture.",
                            "GLES2Texture::_createSurfaceList");
                }
            }
        }
    }

    v1::HardwarePixelBufferSharedPtr GLES2Texture::getBuffer(size_t face, size_t mipmap)
    {
        if (face >= getNumFaces())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Face index out of range",
                        "GLES2Texture::getBuffer");
        }

        if (mipmap > mNumMipmaps)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Mipmap index out of range",
                        "GLES2Texture::getBuffer");
        }

        unsigned long idx = face * (mNumMipmaps + 1) + mipmap;
        assert(idx < mSurfaceList.size());
        return mSurfaceList[idx];
    }
    
    void GLES2Texture::getCustomAttribute(const String& name, void* pData)
	{
		if (name == "GLID")
			*static_cast<GLuint*>(pData) = mTextureID;
	}

#if OGRE_NO_GLES3_SUPPORT != 0
    void GLES2Texture::bindSamplerBlock( GLES2HlmsSamplerblock *samplerblock )
    {
        mLastBoundSamplerblockRsId = samplerblock->mInternalId;

        GLenum target = getGLES2TextureTarget();

        GLES2StateCacheManager *stateCacheManager = mGLSupport.getStateCacheManager();

        stateCacheManager->setTexParameteri( target, GL_TEXTURE_MIN_FILTER, samplerblock->mMinFilter );
        stateCacheManager->setTexParameteri( target, GL_TEXTURE_MIN_FILTER, samplerblock->mMinFilter );
        stateCacheManager->setTexParameteri( target, GL_TEXTURE_MAG_FILTER, samplerblock->mMagFilter );

        stateCacheManager->setTexParameteri( target, GL_TEXTURE_WRAP_S, samplerblock->mU );
        stateCacheManager->setTexParameteri( target, GL_TEXTURE_WRAP_T, samplerblock->mV );

        if( samplerblock->mAnisotropy != 0 )
        {
            stateCacheManager->setTexParameterf( target, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                                                 samplerblock->mAnisotropy );
        }

        //stateCacheManager->setTexParameteri( target, GL_TEXTURE_WRAP_R_OES, samplerblock->mW );

        /*stateCacheManager->setTexParameterf( target, GL_TEXTURE_LOD_BIAS, samplerblock->mMipLodBias );

        OCGE( glTexParameterfv( target, GL_TEXTURE_BORDER_COLOR,
                                              reinterpret_cast<GLfloat*>(
                                                    &samplerblock->mBorderColour ) ) );

        stateCacheManager->setTexParameterf( target, GL_TEXTURE_MIN_LOD, samplerblock->mMinLod );
        stateCacheManager->setTexParameterf( target, GL_TEXTURE_MAX_LOD, samplerblock->mMaxLod );*/
    }
#endif

}
