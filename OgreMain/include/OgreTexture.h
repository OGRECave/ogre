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
#ifndef _Texture_H__
#define _Texture_H__

#include "OgrePrerequisites.h"
#include "OgreHardwareBuffer.h"
#include "OgreResource.h"
#include "OgreImage.h"
#include "OgreHeaderPrefix.h"
#include "OgreSharedPtr.h"

namespace Ogre {

    /** \addtogroup Core
     *  @{
     */
    /** \addtogroup Resources
     *  @{
     */
    /** Enum identifying the texture usage
     */
    enum TextureUsage
    {
        /// same as #HBU_GPU_TO_CPU
        TU_STATIC = HBU_GPU_TO_CPU,
        /// same as #HBU_CPU_ONLY
        TU_DYNAMIC = HBU_CPU_ONLY,
        /// same as #HBU_DETAIL_WRITE_ONLY
        TU_WRITE_ONLY = HBU_DETAIL_WRITE_ONLY,
        /// same as #HBU_GPU_ONLY
        TU_STATIC_WRITE_ONLY = HBU_GPU_ONLY,
        /// same as #HBU_CPU_TO_GPU
        TU_DYNAMIC_WRITE_ONLY = HBU_CPU_TO_GPU,
        /// @deprecated do not use
        TU_DYNAMIC_WRITE_ONLY_DISCARDABLE = HBU_CPU_TO_GPU,
        /// Mipmaps will be automatically generated for this texture
        TU_AUTOMIPMAP = 0x10,
        /** This texture will be a render target, i.e. used as a target for render to texture
            setting this flag will ignore all other texture usages except TU_AUTOMIPMAP, TU_UNORDERED_ACCESS, TU_NOT_SAMPLED */
        TU_RENDERTARGET = 0x20,
        /// Texture will not be sampled inside a shader i.e. used as regular texture.
        /// When used with TU_RENDERTARGET this can improve performance and compatibility with FL9.1 hardware
        TU_NOT_SAMPLED = 0x40,
        /// Texture can be bound as an Unordered Access View
        /// (imageStore/imageRead/glBindImageTexture in GL jargon)
        TU_UNORDERED_ACCESS = 0x80,
        /// Default to automatic mipmap generation static textures
        TU_DEFAULT = TU_AUTOMIPMAP | HBU_GPU_ONLY,

        /// @deprecated
        TU_UAV_NOT_SRV = TU_UNORDERED_ACCESS | TU_NOT_SAMPLED,
        /// @deprecated
        TU_NOT_SRV = TU_NOT_SAMPLED,
        /// @deprecated
        TU_NOTSHADERRESOURCE = TU_NOT_SAMPLED,
        /// @deprecated
        TU_UAV = TU_UNORDERED_ACCESS
    };

    /** Enum identifying the texture access privilege
     */
    enum TextureAccess
    {
        TA_READ = 0x01,
        TA_WRITE = 0x10,
        TA_READ_WRITE = TA_READ | TA_WRITE
    };

    /** Enum identifying the texture type
    */
    enum TextureType : uint8
    {
        /// 1D texture, used in combination with 1D texture coordinates
        TEX_TYPE_1D = 1,
        /// 2D texture, used in combination with 2D texture coordinates (default)
        TEX_TYPE_2D = 2,
        /// 3D volume texture, used in combination with 3D texture coordinates
        TEX_TYPE_3D = 3,
        /// cube map (six two dimensional textures, one for each cube face), used in combination with 3D
        /// texture coordinates
        TEX_TYPE_CUBE_MAP = 4,
        /// 2D texture array
        TEX_TYPE_2D_ARRAY = 5,
        /// GLES2 only OES texture type
        TEX_TYPE_EXTERNAL_OES = 6
    };

    /** Enum identifying special mipmap numbers
    */
    enum TextureMipmap
    {
        /// Generate mipmaps up to 1x1
        MIP_UNLIMITED = 0x7FFFFFFF,
        /// Use TextureManager default
        MIP_DEFAULT = -1
    };

    /** Abstract class representing a Texture resource.

        The actual concrete subclass which will exist for a texture
        is dependent on the rendering system in use (Direct3D, OpenGL etc).
        This class represents the commonalities, and is the one 'used'
        by programmers even though the real implementation could be
        different in reality. Texture objects are created through
        the 'create' method of the TextureManager concrete subclass.
     */
    class _OgreExport Texture : public Resource
    {
    public:
        Texture(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual = false, ManualResourceLoader* loader = 0);

        virtual ~Texture() {}
        
        /** Sets the type of texture; can only be changed before load() 
        */
        void setTextureType(TextureType ttype ) { mTextureType = ttype; }

        /** Gets the type of texture 
        */
        TextureType getTextureType(void) const { return mTextureType; }

        /** Gets the number of mipmaps to be used for this texture.
        */
        uint32 getNumMipmaps(void) const {return mNumMipmaps;}

        /** Sets the number of mipmaps to be used for this texture.
            @note
                Must be set before calling any 'load' method.
        */
        void setNumMipmaps(uint32 num)
        {
            mNumRequestedMipmaps = mNumMipmaps = num;
            if (!num)
                mUsage &= ~TU_AUTOMIPMAP;
        }

        /** Are mipmaps hardware generated?

            Will only be accurate after texture load, or createInternalResources
        */
        bool getMipmapsHardwareGenerated(void) const { return mMipmapsHardwareGenerated; }

        /** Returns the gamma adjustment factor applied to this texture on loading.
        */
        float getGamma(void) const { return mGamma; }

        /** Sets the gamma adjustment factor applied to this texture on loading the
            data.
            @note
                Must be called before any 'load' method. This gamma factor will
                be premultiplied in and may reduce the precision of your textures.
                You can use setHardwareGamma if supported to apply gamma on 
                sampling the texture instead.
        */
        void setGamma(float g) { mGamma = g; }

        /** Sets whether this texture will be set up so that on sampling it, 
            hardware gamma correction is applied.

            24-bit textures are often saved in gamma colour space; this preserves
            precision in the 'darks'. However, if you're performing blending on 
            the sampled colours, you really want to be doing it in linear space. 
            One way is to apply a gamma correction value on loading (see setGamma),
            but this means you lose precision in those dark colours. An alternative
            is to get the hardware to do the gamma correction when reading the 
            texture and converting it to a floating point value for the rest of
            the pipeline. This option allows you to do that; it's only supported
            in relatively recent hardware (others will ignore it) but can improve
            the quality of colour reproduction.
        @note
            Must be called before any 'load' method since it may affect the
            construction of the underlying hardware resources.
            Also note this only useful on textures using 8-bit colour channels.
        */
        void setHardwareGammaEnabled(bool enabled) { mHwGamma = enabled; }

        /** Gets whether this texture will be set up so that on sampling it, 
        hardware gamma correction is applied.
        */
        bool isHardwareGammaEnabled() const { return mHwGamma; }

        /** Set the level of multisample AA to be used if this texture is a 
            rendertarget.
        @note This option will be ignored if TU_RENDERTARGET is not part of the
            usage options on this texture, or if the hardware does not support it. 
        @param fsaa The number of samples
        @param fsaaHint Any hinting text (see Root::createRenderWindow)
        */
        void setFSAA(uint fsaa, const String& fsaaHint) { mFSAA = fsaa; mFSAAHint = fsaaHint; }

        /** Get the level of multisample AA to be used if this texture is a 
        rendertarget.
        */
        uint getFSAA() const { return mFSAA; }

        /** Get the multisample AA hint if this texture is a rendertarget.
        */
        const String& getFSAAHint() const { return mFSAAHint; }

        /** Returns the height of the texture.
        */
        uint32 getHeight(void) const { return mHeight; }

        /** Returns the width of the texture.
        */
        uint32 getWidth(void) const { return mWidth; }

        /** Returns the depth of the texture (only applicable for 3D textures).
        */
        uint32 getDepth(void) const { return mDepth; }

        /** Returns the height of the original input texture (may differ due to hardware requirements).
        */
        uint32 getSrcHeight(void) const { return mSrcHeight; }

        /** Returns the width of the original input texture (may differ due to hardware requirements).
        */
        uint32 getSrcWidth(void) const { return mSrcWidth; }

        /** Returns the original depth of the input texture (only applicable for 3D textures).
        */
        uint32 getSrcDepth(void) const { return mSrcDepth; }

        /** Set the height of the texture; can only do this before load();
        */
        void setHeight(uint32 h) { mHeight = mSrcHeight = h; }

        /** Set the width of the texture; can only do this before load();
        */
        void setWidth(uint32 w) { mWidth = mSrcWidth = w; }

        /** Set the depth of the texture (only applicable for 3D textures);
            can only do this before load();
        */
        void setDepth(uint32 d)  { mDepth = mSrcDepth = d; }

        /** Returns the TextureUsage identifier for this Texture
        */
        int getUsage() const
        {
            return mUsage;
        }

        /** Sets the TextureUsage identifier for this Texture; only useful before load()
            
            @param u is a combination of TU_STATIC, TU_DYNAMIC, TU_WRITE_ONLY 
                TU_AUTOMIPMAP and TU_RENDERTARGET (see TextureUsage enum). You are
                strongly advised to use HBU_GPU_ONLY wherever possible, if you need to
                update regularly, consider HBU_CPU_TO_GPU.
        */
        void setUsage(int u) { mUsage = u; }

        /** Creates the internal texture resources for this texture. 

            This method creates the internal texture resources (pixel buffers, 
            texture surfaces etc) required to begin using this texture. You do
            not need to call this method directly unless you are manually creating
            a texture, in which case something must call it, after having set the
            size and format of the texture (e.g. the ManualResourceLoader might
            be the best one to call it). If you are not defining a manual texture,
            or if you use one of the self-contained load...() methods, then it will be
            called for you.
        */
        void createInternalResources(void);
        
        /** Copies (and maybe scales to fit) the contents of this texture to
            another texture. */
        virtual void copyToTexture( TexturePtr& target );

        /** Loads the data from an image.
        @attention only call this from outside the load() routine of a 
            Resource. Don't call it within (including ManualResourceLoader) - use
            _loadImages() instead. This method is designed to be external, 
            performs locking and checks the load status before loading.
        */
        void loadImage( const Image &img );
            
        /** Loads the data from a raw stream.
        @attention only call this from outside the load() routine of a 
            Resource. Don't call it within (including ManualResourceLoader) - use
            _loadImages() instead. This method is designed to be external, 
            performs locking and checks the load status before loading.
        @param stream Data stream containing the raw pixel data
        @param uWidth Width of the image
        @param uHeight Height of the image
        @param eFormat The format of the pixel data
        */
        void loadRawData( DataStreamPtr& stream,
            ushort uWidth, ushort uHeight, PixelFormat eFormat);

        /** Internal method to load the texture from a set of images. 
        @attention Do NOT call this method unless you are inside the load() routine
            already, e.g. a ManualResourceLoader. It is not threadsafe and does
            not check or update resource loading status.
        */
        void _loadImages( const ConstImagePtrList& images );

        /** Returns the pixel format for the texture surface. */
        PixelFormat getFormat() const
        {
            return mFormat;
        }

        /** Returns the desired pixel format for the texture surface. */
        PixelFormat getDesiredFormat(void) const
        {
            return mDesiredFormat;
        }

        /** Returns the pixel format of the original input texture (may differ due to
            hardware requirements and pixel format conversion).
        */
        PixelFormat getSrcFormat(void) const
        {
            return mSrcFormat;
        }

        /** Sets the desired pixel format for the texture surface; can only be set before load(). */
        void setFormat(PixelFormat pf);

        /** Returns true if the texture has an alpha layer. */
        bool hasAlpha(void) const;

        /** Sets desired bit depth for integer pixel format textures.

            Available values: 0, 16 and 32, where 0 (the default) means keep original format
            as it is. This value is number of bits for the pixel.
        */
        void setDesiredIntegerBitDepth(ushort bits);

        /** gets desired bit depth for integer pixel format textures.
        */
        ushort getDesiredIntegerBitDepth(void) const;

        /** Sets desired bit depth for float pixel format textures.

            Available values: 0, 16 and 32, where 0 (the default) means keep original format
            as it is. This value is number of bits for a channel of the pixel.
        */
        void setDesiredFloatBitDepth(ushort bits);

        /** gets desired bit depth for float pixel format textures.
        */
        ushort getDesiredFloatBitDepth(void) const;

        /** Sets desired bit depth for integer and float pixel format.
        */
        void setDesiredBitDepths(ushort integerBits, ushort floatBits);

        /// @deprecated use setFormat(PF_A8)
        OGRE_DEPRECATED void setTreatLuminanceAsAlpha(bool asAlpha);

        /** Return the number of faces this texture has. This will be 6 for a cubemap
            texture and 1 for a 1D, 2D or 3D one.
        */
        uint32 getNumFaces() const;

        /** Return hardware pixel buffer for a surface. This buffer can then
            be used to copy data from and to a particular level of the texture.
            @param face Face number, in case of a cubemap texture. Must be 0
            for other types of textures.
            For cubemaps, this is one of 
            +X (0), -X (1), +Y (2), -Y (3), +Z (4), -Z (5)
            @param mipmap Mipmap level. This goes from 0 for the first, largest
            mipmap level to getNumMipmaps()-1 for the smallest.
            @return A shared pointer to a hardware pixel buffer.
            @remarks The buffer is invalidated when the resource is unloaded or destroyed.
            Do not use it after the lifetime of the containing texture.
        */
        virtual const HardwarePixelBufferSharedPtr& getBuffer(size_t face=0, size_t mipmap=0);


        /** Populate an Image with the contents of this texture. 
            @param destImage The target image (contents will be overwritten)
            @param includeMipMaps Whether to embed mipmaps in the image
        */
        void convertToImage(Image& destImage, bool includeMipMaps = false);
        
        /** Retrieve a platform or API-specific piece of information from this texture.
            This method of retrieving information should only be used if you know what you're doing.

            | Name        | Description                  |
            |-------------|------------------------------|
            | GLID        | The OpenGL texture object id |

            @param name The name of the attribute to retrieve.
            @param pData Pointer to memory matching the type of data you want to retrieve.
        */
        virtual void getCustomAttribute(const String& name, void* pData);
        
        /** simplified API for bindings
         * 
         * @overload
         */
        uint getCustomAttribute(const String& name)
        {
            uint ret = 0;
            getCustomAttribute(name, &ret);
            return ret;
        }

        /** Enable read and/or write privileges to the texture from shaders.
            @param bindPoint The buffer binding location for shader access. For OpenGL this must be unique and is not related to the texture binding point.
            @param access The texture access privileges given to the shader.
            @param mipmapLevel The texture mipmap level to use.
            @param textureArrayIndex The index of the texture array to use. If texture is not a texture array, set to 0.
            @param format Texture format to be read in by shader. For OpenGL this may be different than the bound texture format.
        */
        virtual void createShaderAccessPoint(uint bindPoint, TextureAccess access = TA_READ_WRITE,
                                        int mipmapLevel = 0, int textureArrayIndex = 0,
                                        PixelFormat format = PF_UNKNOWN) {}
        /** Set image names to be loaded as layers (3d & texture array) or cubemap faces
         */
        void setLayerNames(const std::vector<String>& names)
        {
            mLayerNames = names;
        }

    protected:
        uint32 mHeight;
        uint32 mWidth;
        uint32 mDepth;

        uint32 mNumRequestedMipmaps;
        uint32 mNumMipmaps;

        float mGamma;
        uint mFSAA;

        PixelFormat mFormat;
        int mUsage; /// Bit field, so this can't be TextureUsage

        PixelFormat mSrcFormat;
        uint32 mSrcWidth, mSrcHeight, mSrcDepth;

        bool mTreatLuminanceAsAlpha;
        bool mInternalResourcesCreated;
        bool mMipmapsHardwareGenerated;
        bool mHwGamma;

        String mFSAAHint;

        /// Vector of pointers to subsurfaces
        typedef std::vector<HardwarePixelBufferSharedPtr> SurfaceList;
        SurfaceList mSurfaceList;

        TextureType mTextureType;

        void prepareImpl() override;
        void unprepareImpl() override;
        void loadImpl() override;

        /// @copydoc Resource::calculateSize
        size_t calculateSize(void) const override;
        

        /** Implementation of creating internal texture resources 
        */
        virtual void createInternalResourcesImpl(void) = 0;

        /** Implementation of freeing internal texture resources 
        */
        virtual void freeInternalResourcesImpl(void) = 0;

        /** Default implementation of unload which calls freeInternalResources */
        void unloadImpl(void) override;

        /** Returns the maximum number of Mipmaps that can be generated until we reach
        the mininum possible size. This does not count the base level.

        @return how many times we can divide this texture in 2 until we reach 1x1.
        */
        uint32 getMaxMipmaps() const;

    private:
        uchar mDesiredIntegerBitDepth;
        uchar mDesiredFloatBitDepth;
        PixelFormat mDesiredFormat;

        /// vector of images that should be loaded (cubemap/ texture array)
        std::vector<String> mLayerNames;

        /** Vector of images that were pulled from disk by
            prepareLoad but have yet to be pushed into texture memory
            by loadImpl.  Images should be deleted by loadImpl and unprepareImpl.
        */
        typedef std::vector<Image> LoadedImages;
        LoadedImages mLoadedImages;

        void readImage(LoadedImages& imgs, const String& name, const String& ext, bool haveNPOT);
        void freeInternalResources(void);
    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
