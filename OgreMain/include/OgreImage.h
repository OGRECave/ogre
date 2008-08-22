/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef _Image_H__
#define _Image_H__

#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgrePixelFormat.h"
#include "OgreDataStream.h"

namespace Ogre {

    enum ImageFlags
    {
        IF_COMPRESSED = 0x00000001,
        IF_CUBEMAP    = 0x00000002,
        IF_3D_TEXTURE = 0x00000004
    };
    /** Class representing an image file.
        @remarks
            The Image class usually holds uncompressed image data and is the
            only object that can be loaded in a texture. Image  objects handle 
            image data decoding themselves by the means of locating the correct 
            Codec object for each data type.
        @par
            Typically, you would want to use an Image object to load a texture
            when extra processing needs to be done on an image before it is
            loaded or when you want to blit to an existing texture.
    */
	class _OgreExport Image : public ImageAlloc
    {
    public:
    	typedef Ogre::Box Box;
    	typedef Ogre::Rect Rect;
    public:
        /** Standard constructor.
        */
        Image();
        /** Copy-constructor - copies all the data from the target image.
        */
        Image( const Image &img );

        /** Standard destructor.
        */
        virtual ~Image();

        /** Assignment operator - copies all the data from the target image.
        */
        Image & operator = ( const Image & img );

        /** Flips (mirrors) the image around the Y-axis. 
            @remarks
                An example of an original and flipped image:
                <pre>                
                originalimg
                00000000000
                00000000000
                00000000000
                00000000000
                00000000000
                ------------> flip axis
                00000000000
                00000000000
                00000000000
                00000000000
                00000000000
                originalimg
                </pre>
        */
        Image & flipAroundY();

        /** Flips (mirrors) the image around the X-axis.
            @remarks
                An example of an original and flipped image:
                <pre>
                        flip axis
                            |
                originalimg|gmilanigiro
                00000000000|00000000000
                00000000000|00000000000
                00000000000|00000000000
                00000000000|00000000000
                00000000000|00000000000
                </pre>
        */                 
        Image & flipAroundX();

        /** Stores a pointer to raw data in memory. The pixel format has to be specified.
            @remarks
                This method loads an image into memory held in the object. The 
                pixel format will be either greyscale or RGB with an optional
                Alpha component.
                The type can be determined by calling getFormat().             
            @note
				Whilst typically your image is likely to be a simple 2D image,
				you can define complex images including cube maps, volume maps,
				and images including custom mip levels. The layout of the 
				internal memory should be:
				<ul><li>face 0, mip 0 (top), width x height (x depth)</li>
				<li>face 0, mip 1, width/2 x height/2 (x depth/2)</li>
				<li>face 0, mip 2, width/4 x height/4 (x depth/4)</li>
				<li>.. remaining mips for face 0 .. </li>
				<li>face 1, mip 0 (top), width x height (x depth)</li
				<li>.. and so on. </li>
				</ul>
				Of course, you will never have multiple faces (cube map) and
				depth too.
			@param
                The data pointer
            @param
				Width of image
            @param
				Height of image
			@param
                Image Depth (in 3d images, numbers of layers, otherwhise 1)
            @param
				Pixel Format
            @param
                if memory associated with this buffer is to be destroyed
				with the Image object. Note: it's important that if you set
				this option to true, that you allocated the memory using OGRE_ALLOC_T
				with a category of MEMCATEGORY_GENERAL ensure the freeing of memory 
				matches up.
			@param
				the number of faces the image data has inside (6 for cubemaps, 1 otherwise)
            @param
                the number of mipmaps the image data has inside
            @note
                 The memory associated with this buffer is NOT destroyed with the
                 Image object, unless autoDelete is set to true.
			@remarks 
				The size of the buffer must be numFaces*PixelUtil::getMemorySize(width, height, depth, format)
         */
		Image& loadDynamicImage( uchar* pData, size_t uWidth, size_t uHeight, 
							size_t depth,
							 PixelFormat eFormat, bool autoDelete = false, 
							 size_t numFaces = 1, size_t numMipMaps = 0);
		
		/** Stores a pointer to raw data in memory. The pixel format has to be specified.
            @remarks
                This method loads an image into memory held in the object. The 
                pixel format will be either greyscale or RGB with an optional
                Alpha component.
                The type can be determined by calling getFormat().             
            @note
				Whilst typically your image is likely to be a simple 2D image,
				you can define complex images including cube maps
				and images including custom mip levels. The layout of the 
				internal memory should be:
				<ul><li>face 0, mip 0 (top), width x height</li>
				<li>face 0, mip 1, width/2 x height/2 </li>
				<li>face 0, mip 2, width/4 x height/4 </li>
				<li>.. remaining mips for face 0 .. </li>
				<li>face 1, mip 0 (top), width x height (x depth)</li
				<li>.. and so on. </li>
				</ul>
				Of course, you will never have multiple faces (cube map) and
				depth too.
            @param
                The data pointer
            @param
				Width of image
            @param
				Height of image
            @param
				Pixel Format
            @note
                 The memory associated with this buffer is NOT destroyed with the
                 Image object.
			@remarks This function is deprecated; one should really use the
				Image::loadDynamicImage(pData, width, height, depth, format, ...) to be compatible
				with future Ogre versions.
         */
 		Image& loadDynamicImage( uchar* pData, size_t uWidth,
								 size_t uHeight, PixelFormat eFormat)
		{
			return loadDynamicImage(pData, uWidth, uHeight, 1, eFormat);
		}
		/** Loads raw data from a stream. See the function
			loadDynamicImage for a description of the parameters.
			@remarks 
				The size of the buffer must be numFaces*PixelUtil::getMemorySize(width, height, depth, format)
            @note
				Whilst typically your image is likely to be a simple 2D image,
				you can define complex images including cube maps
				and images including custom mip levels. The layout of the 
				internal memory should be:
				<ul><li>face 0, mip 0 (top), width x height (x depth)</li>
				<li>face 0, mip 1, width/2 x height/2 (x depth/2)</li>
				<li>face 0, mip 2, width/4 x height/4 (x depth/4)</li>
				<li>.. remaining mips for face 0 .. </li>
				<li>face 1, mip 0 (top), width x height (x depth)</li
				<li>.. and so on. </li>
				</ul>
				Of course, you will never have multiple faces (cube map) and
				depth too.
        */
        Image & loadRawData( 
            DataStreamPtr& stream, 
            size_t uWidth, size_t uHeight, size_t uDepth,
            PixelFormat eFormat,
			size_t numFaces = 1, size_t numMipMaps = 0);
        /** Loads raw data from a stream. The pixel format has to be specified. 
			@remarks This function is deprecated; one should really use the
				Image::loadRawData(stream, width, height, depth, format, ...) to be compatible
				with future Ogre versions.
            @note
				Whilst typically your image is likely to be a simple 2D image,
				you can define complex images including cube maps
				and images including custom mip levels. The layout of the 
				internal memory should be:
				<ul><li>face 0, mip 0 (top), width x height</li>
				<li>face 0, mip 1, width/2 x height/2 </li>
				<li>face 0, mip 2, width/4 x height/4 </li>
				<li>.. remaining mips for face 0 .. </li>
				<li>face 1, mip 0 (top), width x height (x depth)</li
				<li>.. and so on. </li>
				</ul>
				Of course, you will never have multiple faces (cube map) and
				depth too.
        */
        Image & loadRawData( 
            DataStreamPtr& stream, 
            size_t uWidth, size_t uHeight, 
            PixelFormat eFormat )
		{
			return loadRawData(stream, uWidth, uHeight, 1, eFormat);
		}

        /** Loads an image file.
            @remarks
                This method loads an image into memory. Any format for which 
				and associated ImageCodec is registered can be loaded. 
				This can include complex formats like DDS with embedded custom 
				mipmaps, cube faces and volume textures.
                The type can be determined by calling getFormat().             
            @param
                strFileName Name of a file file to load.
            @param
                groupName Name of the resource group to search for the image
            @note
                The memory associated with this buffer is destroyed with the
                Image object.
        */
        Image & load( const String& strFileName, const String& groupName );

        /** Loads an image file from a stream.
            @remarks
                This method works in the same way as the filename-based load 
                method except it loads the image from a DataStream object. 
				This DataStream is expected to contain the 
                encoded data as it would be held in a file. 
                Any format for which and associated ImageCodec is registered 
				can be loaded. 
				This can include complex formats like DDS with embedded custom 
				mipmaps, cube faces and volume textures.
                The type can be determined by calling getFormat().             
            @param
                stream The source data.
            @param
                type The type of the image. Used to decide what decompression
                codec to use. Can be left blank if the stream data includes
				a header to identify the data.
            @see
                Image::load( const String& strFileName )
        */
		Image & load(DataStreamPtr& stream, const String& type = StringUtil::BLANK );
        
        /** Save the image as a file. */
        void save(const String& filename);

		/** Encode the image and return a stream to the data. 
			@param formatextension An extension to identify the image format
				to encode into, e.g. "jpg" or "png"
		*/
		DataStreamPtr encode(const String& formatextension);

		/** Returns a pointer to the internal image buffer.
		@remarks
			Be careful with this method. You will almost certainly
			prefer to use getPixelBox, especially with complex images
			which include many faces or custom mipmaps.
        */
        uchar* getData(void);

        /** Returns a const pointer to the internal image buffer.
		@remarks
			Be careful with this method. You will almost certainly
			prefer to use getPixelBox, especially with complex images
			which include many faces or custom mipmaps.
        */
        const uchar * getData() const;       

        /** Returns the size of the data buffer.
        */
        size_t getSize() const;

        /** Returns the number of mipmaps contained in the image.
        */
        size_t getNumMipmaps() const;

        /** Returns true if the image has the appropriate flag set.
        */
        bool hasFlag(const ImageFlags imgFlag) const;

        /** Gets the width of the image in pixels.
        */
        size_t getWidth(void) const;

        /** Gets the height of the image in pixels.
        */
        size_t getHeight(void) const;

        /** Gets the depth of the image.
        */
        size_t getDepth(void) const;
		
		/** Get the number of faces of the image. This is usually 6 for a cubemap, and
		    1 for a normal image.
		*/
		size_t getNumFaces(void) const;

        /** Gets the physical width in bytes of each row of pixels.
        */
        size_t getRowSpan(void) const;

        /** Returns the image format.
        */
        PixelFormat getFormat() const;

        /** Returns the number of bits per pixel.
        */
        uchar getBPP() const;

        /** Returns true if the image has an alpha component.
        */
        bool getHasAlpha() const;
		
		/** Does gamma adjustment.
            @note
                Basic algo taken from Titan Engine, copyright (c) 2000 Ignacio 
                Castano Iguado
        */
        static void applyGamma( uchar *buffer, Real gamma, size_t size, uchar bpp );

        /**
         * Get colour value from a certain location in the image. The z coordinate
         * is only valid for cubemaps and volume textures. This uses the first (largest)
         * mipmap.
         */
        ColourValue getColourAt(int x, int y, int z) const;
        
        /**
         * Get a PixelBox encapsulating the image data of a mipmap
         */
        PixelBox getPixelBox(size_t face = 0, size_t mipmap = 0) const;

		enum Filter
		{
			FILTER_NEAREST,
			FILTER_LINEAR,
			FILTER_BILINEAR,
			FILTER_BOX,
			FILTER_TRIANGLE,
			FILTER_BICUBIC
		};
		/** Scale a 1D, 2D or 3D image volume. 
			@param 	src			PixelBox containing the source pointer, dimensions and format
			@param 	dst			PixelBox containing the destination pointer, dimensions and format
			@param 	filter		Which filter to use
			@remarks 	This function can do pixel format conversion in the process.
			@note	dst and src can point to the same PixelBox object without any problem
		*/
		static void scale(const PixelBox &src, const PixelBox &dst, Filter filter = FILTER_BILINEAR);
		
		/** Resize a 2D image, applying the appropriate filter. */
		void resize(ushort width, ushort height, Filter filter = FILTER_BILINEAR);
		
        // Static function to calculate size in bytes from the number of mipmaps, faces and the dimensions
        static size_t calculateSize(size_t mipmaps, size_t faces, size_t width, size_t height, size_t depth, PixelFormat format);

		/// Static function to get an image type string from a stream via magic numbers
		static String getFileExtFromMagic(DataStreamPtr stream);

    protected:
        // The width of the image in pixels
        size_t m_uWidth;
        // The height of the image in pixels
        size_t m_uHeight;
        // The depth of the image
        size_t m_uDepth;
        // The size of the image buffer
        size_t m_uSize;
        // The number of mipmaps the image contains
        size_t m_uNumMipmaps;
        // Image specific flags.
        int m_uFlags;

        // The pixel format of the image
        PixelFormat m_eFormat;

        // The number of bytes per pixel
        uchar m_ucPixelSize;
        uchar* m_pBuffer;

		// A bool to determine if we delete the buffer or the calling app does
		bool m_bAutoDelete;
    };

	typedef std::vector<Image*> ImagePtrList;
	typedef std::vector<const Image*> ConstImagePtrList;


} // namespace

#endif
