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

#include "OgreGL3PlusTexture.h"
#include "OgreGL3PlusPixelFormat.h"
#include "OgreGL3PlusRenderSystem.h"
#include "OgreGL3PlusHardwareBufferManager.h"
#include "OgreGL3PlusHardwarePixelBuffer.h"
#include "OgreGL3PlusTextureBuffer.h"
#include "OgreGL3PlusUtil.h"
#include "OgreRoot.h"
#include "OgreBitwise.h"
#include "OgreTextureManager.h"

namespace Ogre {

    static inline void doImageIO(const String &name, const String &group,
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

    GL3PlusTexture::GL3PlusTexture(ResourceManager* creator, const String& name,
                                   ResourceHandle handle, const String& group, bool isManual,
                                   ManualResourceLoader* loader, GL3PlusSupport& support)
        : Texture(creator, name, handle, group, isManual, loader),
          mTextureID(0), mGLSupport(support)
    {
        mMipmapsHardwareGenerated = true;
    }

    GL3PlusTexture::~GL3PlusTexture()
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

    GLenum GL3PlusTexture::getGL3PlusTextureTarget(void) const
    {
        switch (mTextureType)
        {
        case TEX_TYPE_1D:
            return GL_TEXTURE_1D;
        case TEX_TYPE_2D:
            return GL_TEXTURE_2D;
        case TEX_TYPE_3D:
            return GL_TEXTURE_3D;
        case TEX_TYPE_CUBE_MAP:
            return GL_TEXTURE_CUBE_MAP;
        case TEX_TYPE_2D_ARRAY:
            return GL_TEXTURE_2D_ARRAY;
        case TEX_TYPE_2D_RECT:
            return GL_TEXTURE_RECTANGLE;
        default:
            return 0;
        };
    }

    // Creation / loading methods
    void GL3PlusTexture::createInternalResourcesImpl(void)
    {
        // Adjust format if required.
        mFormat = TextureManager::getSingleton().getNativeFormat(mTextureType, mFormat, mUsage);

        // Check requested number of mipmaps.
        size_t maxMips = GL3PlusPixelUtil::getMaxMipmaps(mWidth, mHeight, mDepth, mFormat);

        if (PixelUtil::isCompressed(mFormat) && (mNumMipmaps == 0))
            mNumRequestedMipmaps = 0;

        mNumMipmaps = mNumRequestedMipmaps;
        if (mNumMipmaps > maxMips)
            mNumMipmaps = maxMips;

        // Create a texture object and identify its GL type.
        OGRE_CHECK_GL_ERROR(glGenTextures(1, &mTextureID));
        GLenum texTarget = getGL3PlusTextureTarget();

        // Calculate size for all mip levels of the texture.
        uint32 width, height, depth;

        if ((mWidth * PixelUtil::getNumElemBytes(mFormat)) & 3) {
            // Standard alignment of 4 is not right for some formats.
            OGRE_CHECK_GL_ERROR(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        }

        // Bind texture object to its type, making it the active texture object
        // for that type.
        OGRE_CHECK_GL_ERROR(glBindTexture(texTarget, mTextureID));

        OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_BASE_LEVEL, 0));
        OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_MAX_LEVEL, mNumMipmaps));

        // Set some misc default parameters, these can of course be changed later.
        OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget,
                                            GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget,
                                            GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget,
                                            GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget,
                                            GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

        bool hasGL42 = mGLSupport.hasMinGLVersion(4, 2);

        // Set up texture swizzling.
        OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_R, GL_RED));
        OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_G, GL_GREEN));
        OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_B, GL_BLUE));
        OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_A, GL_ALPHA));

        if (mFormat == PF_BYTE_LA)
        {
            OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_R, GL_RED));
            OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_G, GL_RED));
            OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_B, GL_RED));
            OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_A, GL_GREEN));
        }
        else if (mFormat == PF_L8 || mFormat == PF_L16)
        {
            OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_R, GL_RED));
            OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_G, GL_RED));
            OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_B, GL_RED));
            OGRE_CHECK_GL_ERROR(glTexParameteri(texTarget, GL_TEXTURE_SWIZZLE_A, GL_RED));
        }

        GLenum format = GL3PlusPixelUtil::getClosestGLInternalFormat(mFormat, mHwGamma);
        GLenum datatype = GL3PlusPixelUtil::getGLOriginDataType(mFormat);
        width = mWidth;
        height = mHeight;
        depth = mDepth;

        if (hasGL42 || mGLSupport.checkExtension("GL_ARB_texture_storage"))
        {
            switch(mTextureType)
            {
            case TEX_TYPE_1D:
                OGRE_CHECK_GL_ERROR(glTexStorage1D(GL_TEXTURE_1D, GLsizei(mNumMipmaps+1), format, GLsizei(width)));
                break;
            case TEX_TYPE_2D:
            case TEX_TYPE_2D_RECT:
                OGRE_CHECK_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, GLsizei(mNumMipmaps+1), format, GLsizei(width), GLsizei(height)));
                break;
            case TEX_TYPE_CUBE_MAP:
                OGRE_CHECK_GL_ERROR(glTexStorage2D(GL_TEXTURE_CUBE_MAP, GLsizei(mNumMipmaps+1), format, GLsizei(width), GLsizei(height)));
                break;
            case TEX_TYPE_2D_ARRAY:
                OGRE_CHECK_GL_ERROR(glTexStorage3D(GL_TEXTURE_2D_ARRAY, GLsizei(mNumMipmaps+1), format, GLsizei(width), GLsizei(height), GLsizei(depth)));
                break;
            case TEX_TYPE_3D:
                OGRE_CHECK_GL_ERROR(glTexStorage3D(GL_TEXTURE_3D, GLsizei(mNumMipmaps+1), format, GLsizei(width), GLsizei(height), GLsizei(depth)));
                break;
            }
        }
        else
        {
            // Allocate texture storage so that glTexSubImageXD can be
            // used to upload the texture.
            if (PixelUtil::isCompressed(mFormat))
            {
                // Compressed formats
                GLsizei size;

                for (uint8 mip = 0; mip <= mNumMipmaps; mip++)
                {
                    size = static_cast<GLsizei>(PixelUtil::getMemorySize(width, height, depth, mFormat));
                    // std::stringstream str;
                    // str << "GL3PlusTexture::create - " << StringConverter::toString(mTextureID)
                    // << " bytes: " << StringConverter::toString(PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat))
                    // << " Mip: " + StringConverter::toString(mip)
                    // << " Width: " << StringConverter::toString(width)
                    // << " Height: " << StringConverter::toString(height)
                    // << " Format " << PixelUtil::getFormatName(mFormat)
                    // << " Internal Format: 0x" << std::hex << format
                    // << " Origin Format: 0x" << std::hex << GL3PlusPixelUtil::getGLOriginFormat(mFormat)
                    // << " Data type: 0x" << std::hex << datatype;
                    // LogManager::getSingleton().logMessage(LML_NORMAL, str.str());

                    switch(mTextureType)
                    {
                    case TEX_TYPE_1D:
                        OGRE_CHECK_GL_ERROR(glCompressedTexImage1D(GL_TEXTURE_1D, mip, format,
                                                                   width, 0,
                                                                   size, NULL));
                        break;
                    case TEX_TYPE_2D:
                        OGRE_CHECK_GL_ERROR(glCompressedTexImage2D(GL_TEXTURE_2D,
                                                                   mip,
                                                                   format,
                                                                   width, height,
                                                                   0,
                                                                   size,
                                                                   NULL));
                        break;
                    case TEX_TYPE_2D_RECT:
                        OGRE_CHECK_GL_ERROR(glCompressedTexImage2D(GL_TEXTURE_RECTANGLE,
                                                                   mip,
                                                                   format,
                                                                   width, height,
                                                                   0,
                                                                   size,
                                                                   NULL));
                        break;
                    case TEX_TYPE_2D_ARRAY:
                    case TEX_TYPE_3D:
                        OGRE_CHECK_GL_ERROR(glCompressedTexImage3D(texTarget, mip, format,
                                                                   width, height, depth, 0,
                                                                   size, NULL));
                        break;
                    case TEX_TYPE_CUBE_MAP:
                        for(int face = 0; face < 6; face++) {
                            OGRE_CHECK_GL_ERROR(glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, format,
                                                                       width, height, 0,
                                                                       size / depth, NULL));
                        }
                        break;
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
                    if (depth > 1 && mTextureType != TEX_TYPE_2D_ARRAY)
                    {
                        depth = depth / 2;
                    }
                }
            }
            else
            {
                // Run through this process to pregenerate mipmap pyramid
                for(uint8 mip = 0; mip <= mNumMipmaps; mip++)
                {
                    //                    std::stringstream str;
                    //                    str << "GL3PlusTexture::create - " << StringConverter::toString(mTextureID)
                    //                    << " bytes: " << StringConverter::toString(PixelUtil::getMemorySize(width, height, depth, mFormat))
                    //                    << " Mip: " + StringConverter::toString(mip)
                    //                    << " Width: " << StringConverter::toString(width)
                    //                    << " Height: " << StringConverter::toString(height)
                    //                    << " Format " << PixelUtil::getFormatName(mFormat)
                    //                    << " Internal Format: 0x" << std::hex << format
                    //                    << " Origin Format: 0x" << std::hex << GL3PlusPixelUtil::getGLOriginFormat(mFormat)
                    //                    << " Data type: 0x" << std::hex << datatype;
                    //                    LogManager::getSingleton().logMessage(LML_NORMAL, str.str());

                    // Normal formats
                    switch(mTextureType)
                    {
                    case TEX_TYPE_1D:
                        OGRE_CHECK_GL_ERROR(glTexImage1D(GL_TEXTURE_1D, mip, format,
                                                         width, 0,
                                                         GL3PlusPixelUtil::getGLOriginFormat(mFormat), datatype, NULL));
                        break;
                    case TEX_TYPE_2D:
                        OGRE_CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D,
                                                         mip,
                                                         format,
                                                         width, height,
                                                         0,
                                                         GL3PlusPixelUtil::getGLOriginFormat(mFormat),
                                                         datatype, NULL));
                        break;
                    case TEX_TYPE_2D_RECT:
                        OGRE_CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_RECTANGLE,
                                                         mip,
                                                         format,
                                                         width, height,
                                                         0,
                                                         GL3PlusPixelUtil::getGLOriginFormat(mFormat),
                                                         datatype, NULL));
                        break;
                    case TEX_TYPE_3D:
                    case TEX_TYPE_2D_ARRAY:
                        OGRE_CHECK_GL_ERROR(glTexImage3D(texTarget, mip, format,
                                                         width, height, depth, 0,
                                                         GL3PlusPixelUtil::getGLOriginFormat(mFormat), datatype, NULL));
                        break;
                    case TEX_TYPE_CUBE_MAP:
                        for(int face = 0; face < 6; face++) {
                            OGRE_CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, format,
                                                             width, height, 0,
                                                             GL3PlusPixelUtil::getGLOriginFormat(mFormat), datatype, NULL));
                        }
                        break;
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
                    if (depth > 1 && mTextureType != TEX_TYPE_2D_ARRAY)
                    {
                        depth = depth / 2;
                    }
                }
            }
        }

        // Reset unpack alignment to defaults
        OGRE_CHECK_GL_ERROR(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));

        //Allocate internal buffers for automipmaps before we load anything into them
        if(mUsage & TU_AUTOMIPMAP)
            OGRE_CHECK_GL_ERROR(glGenerateMipmap(getGL3PlusTextureTarget()));

        _createSurfaceList();

        // Generate mipmaps after all texture levels have been loaded
        // This is required for compressed formats such as DXT
        if (PixelUtil::isCompressed(mFormat) && mUsage & TU_AUTOMIPMAP)
        {
            OGRE_CHECK_GL_ERROR(glGenerateMipmap(getGL3PlusTextureTarget()));
        }

        // Get final internal format.
        mFormat = getBuffer(0,0)->getFormat();
    }

    void GL3PlusTexture::createRenderTexture(void)
    {
        // Create the GL texture
        // This already does everything necessary
        createInternalResources();
    }

    void GL3PlusTexture::prepareImpl()
    {
        if (mUsage & TU_RENDERTARGET)
            return;

        String baseName, ext;
        size_t pos = mName.find_last_of(".");
        baseName = mName.substr(0, pos);

        if (pos != String::npos)
        {
            ext = mName.substr(pos+1);
        }

        LoadedImages loadedImages = LoadedImages(new vector<Image>::type());

        if (mTextureType == TEX_TYPE_1D || mTextureType == TEX_TYPE_2D ||
           mTextureType == TEX_TYPE_2D_RECT || mTextureType == TEX_TYPE_2D_ARRAY || mTextureType == TEX_TYPE_3D)
        {
            doImageIO(mName, mGroup, ext, *loadedImages, this);

            // If this is a volumetric texture set the texture type flag accordingly.
            // If this is a cube map, set the texture type flag accordingly.
            if ((*loadedImages)[0].hasFlag(IF_CUBEMAP))
                mTextureType = TEX_TYPE_CUBE_MAP;
            // If this is a volumetric texture set the texture type flag accordingly.
            if ((*loadedImages)[0].getDepth() > 1 && mTextureType != TEX_TYPE_2D_ARRAY)
                mTextureType = TEX_TYPE_3D;
        }
        else if (mTextureType == TEX_TYPE_CUBE_MAP)
        {
            if (getSourceFileType() == "dds" || getSourceFileType() == "oitd")
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
                        "GL3PlusTexture::prepare");
        }

        mLoadedImages = loadedImages;
    }

    void GL3PlusTexture::unprepareImpl()
    {
        mLoadedImages.setNull();
    }

    void GL3PlusTexture::loadImpl()
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

        // Generate mipmaps after all texture levels have been loaded
        // This is required for compressed formats such as DXT
        if (mUsage & TU_AUTOMIPMAP && (mTextureType != TEX_TYPE_2D_ARRAY && mTextureType != TEX_TYPE_3D))
        {
            OGRE_CHECK_GL_ERROR(glGenerateMipmap(getGL3PlusTextureTarget()));
        }
    }

    void GL3PlusTexture::freeInternalResourcesImpl()
    {
        mSurfaceList.clear();
        OGRE_CHECK_GL_ERROR(glDeleteTextures(1, &mTextureID));
    }

    void GL3PlusTexture::_createSurfaceList()
    {
        mSurfaceList.clear();

        for (uint8 face = 0; face < getNumFaces(); face++)
        {
            for (uint8 mip = 0; mip <= getNumMipmaps(); mip++)
            {
                v1::GL3PlusHardwarePixelBuffer *buf = new v1::GL3PlusTextureBuffer(mName,
                                                                           getGL3PlusTextureTarget(),
                                                                           mTextureID,
                                                                           face,
                                                                           mip,
                                                                           static_cast<v1::HardwareBuffer::Usage>(mUsage),
                                                                           mHwGamma, mFSAA);

                mSurfaceList.push_back(v1::HardwarePixelBufferSharedPtr(buf));

                // Check for error
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
                        "GL3PlusTexture::_createSurfaceList");
                }
            }
        }
    }

    void GL3PlusTexture::_autogenerateMipmaps(void)
    {
        const GLenum texTarget = getGL3PlusTextureTarget();
        OCGE( glBindTexture( texTarget, mTextureID ) );
        OCGE( glGenerateMipmap( texTarget ) );

        mSurfaceList[0]->getRenderTarget()->_setMipmapsUpdated();
    }

    v1::HardwarePixelBufferSharedPtr GL3PlusTexture::getBuffer(size_t face, size_t mipmap)
    {
        if (face >= getNumFaces())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Face index out of range",
                        "GL3PlusTexture::getBuffer");
        }

        if (mipmap > mNumMipmaps)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Mipmap index out of range",
                        "GL3PlusTexture::getBuffer");
        }

        unsigned long idx = face * (mNumMipmaps + 1) + mipmap;
        assert(idx < mSurfaceList.size());
        return mSurfaceList[idx];
    }


    void GL3PlusTexture::getCustomAttribute(const String& name, void* pData)
    {
        if (name == "GLID")
            *static_cast<GLuint*>(pData) = mTextureID;
    }
}
