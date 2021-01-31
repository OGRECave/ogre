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
#include "OgreGL3PlusStateCacheManager.h"
#include "OgreGLUtil.h"
#include "OgreRoot.h"
#include "OgreBitwise.h"
#include "OgreTextureManager.h"

namespace Ogre {
    GL3PlusTexture::GL3PlusTexture(ResourceManager* creator, const String& name,
                                   ResourceHandle handle, const String& group, bool isManual,
                                   ManualResourceLoader* loader, GL3PlusRenderSystem* renderSystem)
        : GLTextureCommon(creator, name, handle, group, isManual, loader),
          mRenderSystem(renderSystem)
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
        // set HardwareBuffer::Usage for TU_RENDERTARGET if nothing else specified
        if((mUsage & TU_RENDERTARGET) && (mUsage & ~TU_RENDERTARGET) == 0)
            mUsage |= HardwareBuffer::HBU_DYNAMIC;

        // Adjust format if required.
        mFormat = TextureManager::getSingleton().getNativeFormat(mTextureType, mFormat, mUsage);

        // Check requested number of mipmaps.
        uint32 maxMips = getMaxMipmaps();

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
        mRenderSystem->_getStateCacheManager()->bindGLTexture( texTarget, mTextureID );

        mRenderSystem->_getStateCacheManager()->setTexParameteri(texTarget, GL_TEXTURE_BASE_LEVEL, 0);
        mRenderSystem->_getStateCacheManager()->setTexParameteri(texTarget, GL_TEXTURE_MAX_LEVEL, mNumMipmaps);

        // Set up texture swizzling.
        typedef std::array<GLint, 4> SwizzleMask;
        SwizzleMask swizzleMask;

        if (PixelUtil::isLuminance(mFormat))
        {
            if (PixelUtil::getComponentCount(mFormat) == 2)
            {
                swizzleMask = SwizzleMask{GL_RED, GL_RED, GL_RED, GL_GREEN};
            }
            else
            {
                swizzleMask = SwizzleMask{GL_RED, GL_RED, GL_RED, GL_ONE};
            }
        }
        else if(mFormat == PF_A8)
        {
            swizzleMask = SwizzleMask{GL_ZERO, GL_ZERO, GL_ZERO, GL_RED};
        }
        else
        {
            swizzleMask = SwizzleMask{GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
        }
        OGRE_CHECK_GL_ERROR(glTexParameteriv(texTarget, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask.data()));

        GLenum format = GL3PlusPixelUtil::getGLInternalFormat(mFormat, mHwGamma);
        GLenum datatype = GL3PlusPixelUtil::getGLOriginDataType(mFormat);
        width = mWidth;
        height = mHeight;
        depth = mDepth;

        // Allocate texture storage so that glTexSubImageXD can be
        // used to upload the texture.
        if (PixelUtil::isCompressed(mFormat))
        {
            // Compressed formats
            GLsizei size;

            for (uint32 mip = 0; mip <= mNumMipmaps; mip++)
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
                                                                   size, NULL));
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
            if (mRenderSystem->hasMinGLVersion(4, 2) || mRenderSystem->checkExtension("GL_ARB_texture_storage"))
            {
                switch(mTextureType)
                {
                case TEX_TYPE_1D:
                    OGRE_CHECK_GL_ERROR(glTexStorage1D(GL_TEXTURE_1D, GLsizei(mNumMipmaps+1), format, GLsizei(width)));
                    break;
                case TEX_TYPE_2D:
                case TEX_TYPE_2D_RECT:
                case TEX_TYPE_CUBE_MAP:
                    OGRE_CHECK_GL_ERROR(glTexStorage2D(texTarget, GLsizei(mNumMipmaps+1), format, GLsizei(width), GLsizei(height)));
                    break;
                case TEX_TYPE_2D_ARRAY:
                case TEX_TYPE_3D:
                    OGRE_CHECK_GL_ERROR(glTexStorage3D(texTarget, GLsizei(mNumMipmaps+1), format, GLsizei(width), GLsizei(height), GLsizei(depth)));
                    break;
                case TEX_TYPE_EXTERNAL_OES:
                    OGRE_EXCEPT(
                        Exception::ERR_RENDERINGAPI_ERROR,
                        "Attempt to store texture for unsupported TEX_TYPE_EXTERNAL_OES, should never happen",
                        "GL3PlusTexture::createInternalResourcesImpl"
                    );
                    break;
                }
            }
            else
            {
                GLenum originFormat = GL3PlusPixelUtil::getGLOriginFormat(mFormat);

                // Run through this process to pregenerate mipmap pyramid
                for(uint32 mip = 0; mip <= mNumMipmaps; mip++)
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
                                                         originFormat, datatype, NULL));
                        break;
                    case TEX_TYPE_2D:
                    case TEX_TYPE_2D_RECT:
                        OGRE_CHECK_GL_ERROR(glTexImage2D(texTarget,
                                                         mip,
                                                         format,
                                                         width, height,
                                                         0,
                                                         originFormat,
                                                         datatype, NULL));
                        break;
                    case TEX_TYPE_3D:
                    case TEX_TYPE_2D_ARRAY:
                        OGRE_CHECK_GL_ERROR(glTexImage3D(texTarget, mip, format,
                                                         width, height, depth, 0,
                                                         originFormat, datatype, NULL));
                        break;
                    case TEX_TYPE_CUBE_MAP:
                        for(int face = 0; face < 6; face++) {
                            OGRE_CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, format,
                                                             width, height, 0,
                                                             originFormat, datatype, NULL));
                        }
                        break;
                    case TEX_TYPE_EXTERNAL_OES:
                        OGRE_EXCEPT(
                            Exception::ERR_RENDERINGAPI_ERROR,
                            "Attempt to create mipmaps for unsupported TEX_TYPE_EXTERNAL_OES, should never happen",
                            "GL3PlusTexture::createInternalResourcesImpl"
                        );
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

    void GL3PlusTexture::freeInternalResourcesImpl()
    {
        if (GL3PlusStateCacheManager* stateCacheManager = mRenderSystem->_getStateCacheManager())
        {
            OGRE_CHECK_GL_ERROR(glDeleteTextures(1, &mTextureID));
            stateCacheManager->invalidateStateForTexture(mTextureID);
        }
    }

    void GL3PlusTexture::_createSurfaceList()
    {
        mSurfaceList.clear();

        size_t depth = mDepth;
        for (uint8 face = 0; face < getNumFaces(); face++)
        {
            size_t width = mWidth;
            size_t height = mHeight;

            for (uint32 mip = 0; mip <= getNumMipmaps(); mip++)
            {
                GL3PlusHardwarePixelBuffer* buf =
                    new GL3PlusTextureBuffer(this, face, mip, width, height, depth);
                mSurfaceList.push_back(HardwarePixelBufferSharedPtr(buf));

                if (width > 1)
                    width = width / 2;
                if (height > 1)
                    height = height / 2;
                if (depth > 1 && mTextureType != TEX_TYPE_2D_ARRAY)
                    depth = depth / 2;
            }
        }
    }

    void GL3PlusTexture::createShaderAccessPoint(uint bindPoint, TextureAccess access, 
                                                 int mipmapLevel, int textureArrayIndex, 
                                                 PixelFormat format)
    {
        GLenum GlAccess = 0;

        switch (access)
        {
        case TA_READ:
            GlAccess = GL_READ_ONLY;
            break;
        case TA_WRITE:
            GlAccess = GL_WRITE_ONLY;
            break;
        case TA_READ_WRITE:
            GlAccess = GL_READ_WRITE;
            break;
        }

        if (format == PF_UNKNOWN) format = mFormat;
        GLenum GlFormat = GL3PlusPixelUtil::getClosestGLImageInternalFormat(format);
        GLboolean isArrayTexture = mTextureType == TEX_TYPE_2D_ARRAY;

        // TODO
        // * add memory barrier
        // * material script access (can have multiple instances for a single texture_unit)
        //     shader_access <binding point> [<access>] [<mipmap level>] [<texture array layer>] [<format>]
        //     shader_access 2 read_write 0 0 PF_UINT32_R
        //   binding point - location to bind for shader access; for OpenGL this must be unique and is not related to texture binding point
        //   access - give the shader read, write, or read_write privileges [default read_write]
        //   mipmap level - texture mipmap level to use [default 0]
        //   texture array layer - layer of texture array to use: 'all', or layer number (if not layered, just use 0) [default 0]
        //   format - texture format to be read in shader; for OpenGL this may be different than bound texture format - not sure about DX11 [default same format as texture]
        //   Note that for OpenGL the shader access (image) binding point 
        //   must be specified, it is NOT the same as the texture binding point,
        //   and it must be unique among textures in this pass.
        // * enforce binding point uniqueness by checking against 
        //   image binding point allocation list in GL3PlusTextureManager
        // * generalize for other render systems by introducing vitual method in Texture 
        // for (image in mImages)
        // {
        // OGRE_CHECK_GL_ERROR(
        //     glBindImageTexture(
        //         mImageBind, mTextureID, 
        //         mMipmapLevel, 
        //         mLayered.find('all') != str::npos ? GL_TRUE : GL_FALSE, mLayer,
        //         mImageAccess (READ, WRITE, READ_WRITE), 
        //         toImageFormat(mFormatInShader))); //GL_RGBA8)); //GL_R32UI)); GL_READ_WRITE
        if (mRenderSystem->hasMinGLVersion(4, 2) || mRenderSystem->checkExtension("GL_ARB_shader_image_load_store"))
        {
            OGRE_CHECK_GL_ERROR(glBindImageTexture(bindPoint, mTextureID, mipmapLevel, isArrayTexture, textureArrayIndex, GlAccess, GlFormat));
        }
    }


}
