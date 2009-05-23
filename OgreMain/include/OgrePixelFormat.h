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
#ifndef _PixelFormat_H__
#define _PixelFormat_H__

#include "OgrePrerequisites.h"
#include "OgreCommon.h"

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Image
	*  @{
	*/
	/** The pixel format used for images, textures, and render surfaces */
    enum PixelFormat
    {
        /// Unknown pixel format.
        PF_UNKNOWN = 0,
        /// 8-bit pixel format, all bits luminace.
        PF_L8 = 1,
		PF_BYTE_L = PF_L8,
        /// 16-bit pixel format, all bits luminace.
        PF_L16 = 2,
		PF_SHORT_L = PF_L16,
        /// 8-bit pixel format, all bits alpha.
        PF_A8 = 3,
		PF_BYTE_A = PF_A8,
        /// 8-bit pixel format, 4 bits alpha, 4 bits luminance.
        PF_A4L4 = 4,
		/// 2 byte pixel format, 1 byte luminance, 1 byte alpha
		PF_BYTE_LA = 5,
        /// 16-bit pixel format, 5 bits red, 6 bits green, 5 bits blue.
        PF_R5G6B5 = 6,
		/// 16-bit pixel format, 5 bits red, 6 bits green, 5 bits blue.
        PF_B5G6R5 = 7,
        /// 8-bit pixel format, 2 bits blue, 3 bits green, 3 bits red.
        PF_R3G3B2 = 31,
        /// 16-bit pixel format, 4 bits for alpha, red, green and blue.
        PF_A4R4G4B4 = 8,
        /// 16-bit pixel format, 5 bits for blue, green, red and 1 for alpha.
        PF_A1R5G5B5 = 9,
        /// 24-bit pixel format, 8 bits for red, green and blue.
        PF_R8G8B8 = 10,
        /// 24-bit pixel format, 8 bits for blue, green and red.
        PF_B8G8R8 = 11,
        /// 32-bit pixel format, 8 bits for alpha, red, green and blue.
        PF_A8R8G8B8 = 12,
        /// 32-bit pixel format, 8 bits for blue, green, red and alpha.
        PF_A8B8G8R8 = 13,
        /// 32-bit pixel format, 8 bits for blue, green, red and alpha.
        PF_B8G8R8A8 = 14,
		/// 32-bit pixel format, 8 bits for red, green, blue and alpha.
		PF_R8G8B8A8 = 28,
        /// 32-bit pixel format, 8 bits for red, 8 bits for green, 8 bits for blue
        /// like PF_A8R8G8B8, but alpha will get discarded
        PF_X8R8G8B8 = 26,
        /// 32-bit pixel format, 8 bits for blue, 8 bits for green, 8 bits for red
        /// like PF_A8B8G8R8, but alpha will get discarded
        PF_X8B8G8R8 = 27,
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
		/// 3 byte pixel format, 1 byte for red, 1 byte for green, 1 byte for blue
		PF_BYTE_RGB = PF_R8G8B8,
		/// 3 byte pixel format, 1 byte for blue, 1 byte for green, 1 byte for red
		PF_BYTE_BGR = PF_B8G8R8,
		/// 4 byte pixel format, 1 byte for blue, 1 byte for green, 1 byte for red and one byte for alpha
		PF_BYTE_BGRA = PF_B8G8R8A8,
		/// 4 byte pixel format, 1 byte for red, 1 byte for green, 1 byte for blue, and one byte for alpha
		PF_BYTE_RGBA = PF_R8G8B8A8,
#else
		/// 3 byte pixel format, 1 byte for red, 1 byte for green, 1 byte for blue
		PF_BYTE_RGB = PF_B8G8R8,
		/// 3 byte pixel format, 1 byte for blue, 1 byte for green, 1 byte for red
		PF_BYTE_BGR = PF_R8G8B8,
		/// 4 byte pixel format, 1 byte for blue, 1 byte for green, 1 byte for red and one byte for alpha
		PF_BYTE_BGRA = PF_A8R8G8B8,
		/// 4 byte pixel format, 1 byte for red, 1 byte for green, 1 byte for blue, and one byte for alpha
		PF_BYTE_RGBA = PF_A8B8G8R8,
#endif        
        /// 32-bit pixel format, 2 bits for alpha, 10 bits for red, green and blue.
        PF_A2R10G10B10 = 15,
        /// 32-bit pixel format, 10 bits for blue, green and red, 2 bits for alpha.
        PF_A2B10G10R10 = 16,
        /// DDS (DirectDraw Surface) DXT1 format
        PF_DXT1 = 17,
        /// DDS (DirectDraw Surface) DXT2 format
        PF_DXT2 = 18,
        /// DDS (DirectDraw Surface) DXT3 format
        PF_DXT3 = 19,
        /// DDS (DirectDraw Surface) DXT4 format
        PF_DXT4 = 20,
        /// DDS (DirectDraw Surface) DXT5 format
        PF_DXT5 = 21,
		// 16-bit pixel format, 16 bits (float) for red
        PF_FLOAT16_R = 32,
        // 48-bit pixel format, 16 bits (float) for red, 16 bits (float) for green, 16 bits (float) for blue
        PF_FLOAT16_RGB = 22,
        // 64-bit pixel format, 16 bits (float) for red, 16 bits (float) for green, 16 bits (float) for blue, 16 bits (float) for alpha
        PF_FLOAT16_RGBA = 23,
		// 16-bit pixel format, 16 bits (float) for red
        PF_FLOAT32_R = 33,
       // 96-bit pixel format, 32 bits (float) for red, 32 bits (float) for green, 32 bits (float) for blue
        PF_FLOAT32_RGB = 24,
        // 128-bit pixel format, 32 bits (float) for red, 32 bits (float) for green, 32 bits (float) for blue, 32 bits (float) for alpha
        PF_FLOAT32_RGBA = 25,
		// 32-bit, 2-channel s10e5 floating point pixel format, 16-bit green, 16-bit red
		PF_FLOAT16_GR = 35,
		// 64-bit, 2-channel floating point pixel format, 32-bit green, 32-bit red
		PF_FLOAT32_GR = 36,
		// Depth texture format
		PF_DEPTH = 29,
		// 64-bit pixel format, 16 bits for red, green, blue and alpha
		PF_SHORT_RGBA = 30,
		// 32-bit pixel format, 16-bit green, 16-bit red
		PF_SHORT_GR = 34,
		// 48-bit pixel format, 16 bits for red, green and blue
		PF_SHORT_RGB = 37,
		// Number of pixel formats currently defined
        PF_COUNT = 38
    };
	typedef vector<PixelFormat>::type PixelFormatList;

    /**
     * Flags defining some on/off properties of pixel formats
     */
    enum PixelFormatFlags {
        // This format has an alpha channel
        PFF_HASALPHA        = 0x00000001,      
        // This format is compressed. This invalidates the values in elemBytes,
        // elemBits and the bit counts as these might not be fixed in a compressed format.
        PFF_COMPRESSED    = 0x00000002,
        // This is a floating point format
        PFF_FLOAT           = 0x00000004,         
        // This is a depth format (for depth textures)
        PFF_DEPTH           = 0x00000008,
        // Format is in native endian. Generally true for the 16, 24 and 32 bits
        // formats which can be represented as machine integers.
        PFF_NATIVEENDIAN    = 0x00000010,
        // This is an intensity format instead of a RGB one. The luminance
        // replaces R,G and B. (but not A)
        PFF_LUMINANCE       = 0x00000020
    };
    
    /** Pixel component format */
    enum PixelComponentType
    {
        PCT_BYTE = 0,    /// Byte per component (8 bit fixed 0.0..1.0)
        PCT_SHORT = 1,   /// Short per component (16 bit fixed 0.0..1.0))
        PCT_FLOAT16 = 2, /// 16 bit float per component
        PCT_FLOAT32 = 3, /// 32 bit float per component
        PCT_COUNT = 4    /// Number of pixel types
    };
    
	/** A primitive describing a volume (3D), image (2D) or line (1D) of pixels in memory.
     	In case of a rectangle, depth must be 1. 
     	Pixels are stored as a succession of "depth" slices, each containing "height" rows of 
     	"width" pixels.
    */
    class _OgreExport PixelBox: public Box {
    public:
    	/// Parameter constructor for setting the members manually
    	PixelBox() {}
		/** Constructor providing extents in the form of a Box object. This constructor
    		assumes the pixel data is laid out consecutively in memory. (this
    		means row after row, slice after slice, with no space in between)
    		@param extents	    Extents of the region defined by data
    		@param pixelFormat	Format of this buffer
    		@param pixelData	Pointer to the actual data
    	*/
		PixelBox(const Box &extents, PixelFormat pixelFormat, void *pixelData=0):
			Box(extents), data(pixelData), format(pixelFormat)
		{
			setConsecutive();
		}
    	/** Constructor providing width, height and depth. This constructor
    		assumes the pixel data is laid out consecutively in memory. (this
    		means row after row, slice after slice, with no space in between)
    		@param width	    Width of the region
    		@param height	    Height of the region
    		@param depth	    Depth of the region
    		@param pixelFormat	Format of this buffer
    		@param pixelData    Pointer to the actual data
    	*/
    	PixelBox(size_t width, size_t height, size_t depth, PixelFormat pixelFormat, void *pixelData=0):
    		Box(0, 0, 0, width, height, depth),
    		data(pixelData), format(pixelFormat)
    	{
    		setConsecutive();
    	}
    	
        /// The data pointer 
        void *data;
        /// The pixel format 
        PixelFormat format;
        /** Number of elements between the leftmost pixel of one row and the left
         	pixel of the next. This value must always be equal to getWidth() (consecutive) 
			for compressed formats.
        */
        size_t rowPitch;
        /** Number of elements between the top left pixel of one (depth) slice and 
         	the top left pixel of the next. This can be a negative value. Must be a multiple of
         	rowPitch. This value must always be equal to getWidth()*getHeight() (consecutive) 
			for compressed formats.
        */
        size_t slicePitch;
        
        /** Set the rowPitch and slicePitch so that the buffer is laid out consecutive 
         	in memory.
        */        
        void setConsecutive()
        {
            rowPitch = getWidth();
            slicePitch = getWidth()*getHeight();
        }
        /**	Get the number of elements between one past the rightmost pixel of 
         	one row and the leftmost pixel of the next row. (IE this is zero if rows
         	are consecutive).
        */
        size_t getRowSkip() const { return rowPitch - getWidth(); }
        /** Get the number of elements between one past the right bottom pixel of
         	one slice and the left top pixel of the next slice. (IE this is zero if slices
         	are consecutive).
        */
        size_t getSliceSkip() const { return slicePitch - (getHeight() * rowPitch); }

        /** Return whether this buffer is laid out consecutive in memory (ie the pitches
         	are equal to the dimensions)
        */        
        bool isConsecutive() const 
		{ 
			return rowPitch == getWidth() && slicePitch == getWidth()*getHeight(); 
		}
        /** Return the size (in bytes) this image would take if it was
        	laid out consecutive in memory
      	*/
      	size_t getConsecutiveSize() const;
      	/** Return a subvolume of this PixelBox.
      		@param def	Defines the bounds of the subregion to return
      		@returns	A pixel box describing the region and the data in it
      		@remarks	This function does not copy any data, it just returns
      			a PixelBox object with a data pointer pointing somewhere inside 
      			the data of object.
      		@throws	Exception(ERR_INVALIDPARAMS) if def is not fully contained
      	*/
      	PixelBox getSubVolume(const Box &def) const;
    };
    

    /**
     * Some utility functions for packing and unpacking pixel data
     */
    class _OgreExport PixelUtil {
    public:
        /** Returns the size in bytes of an element of the given pixel format.
         @returns
               The size in bytes of an element. See Remarks.
         @remarks
               Passing PF_UNKNOWN will result in returning a size of 0 bytes.
        */
        static size_t getNumElemBytes( PixelFormat format );

        /** Returns the size in bits of an element of the given pixel format.
          @returns
               The size in bits of an element. See Remarks.
           @remarks
               Passing PF_UNKNOWN will result in returning a size of 0 bits.
        */
        static size_t getNumElemBits( PixelFormat format );

		/** Returns the size in memory of a region with the given extents and pixel
			format with consecutive memory layout.
			@param width
				The width of the area
			@param height
				The height of the area
			@param depth
				The depth of the area
			@param format
				The format of the area
		  	@returns
		  		The size in bytes
			@remarks
				In case that the format is non-compressed, this simply returns
				width*height*depth*PixelUtil::getNumElemBytes(format). In the compressed
				case, this does serious magic.
		*/
		static size_t getMemorySize(size_t width, size_t height, size_t depth, PixelFormat format);
		
        /** Returns the property flags for this pixel format
          @returns
               A bitfield combination of PFF_HASALPHA, PFF_ISCOMPRESSED,
               PFF_FLOAT, PFF_DEPTH, PFF_NATIVEENDIAN, PFF_LUMINANCE
          @remarks
               This replaces the seperate functions for formatHasAlpha, formatIsFloat, ...
        */
        static unsigned int getFlags( PixelFormat format );

        /** Shortcut method to determine if the format has an alpha component */
        static bool hasAlpha(PixelFormat format);
        /** Shortcut method to determine if the format is floating point */
        static bool isFloatingPoint(PixelFormat format);
        /** Shortcut method to determine if the format is compressed */
        static bool isCompressed(PixelFormat format);
        /** Shortcut method to determine if the format is a depth format. */
        static bool isDepth(PixelFormat format);
        /** Shortcut method to determine if the format is in native endian format. */
        static bool isNativeEndian(PixelFormat format);
        /** Shortcut method to determine if the format is a luminance format. */
        static bool isLuminance(PixelFormat format);
		
		/** Return wether a certain image extent is valid for this image format.
			@param width
				The width of the area
			@param height
				The height of the area
			@param depth
				The depth of the area
			@param format
				The format of the area
			@remarks For non-compressed formats, this is always true. For DXT formats,
			only sizes with a width and height multiple of 4 and depth 1 are allowed.
		*/
		static bool isValidExtent(size_t width, size_t height, size_t depth, PixelFormat format);

        /** Gives the number of bits (RGBA) for a format. See remarks.          
          @remarks      For non-colour formats (dxt, depth) this returns [0,0,0,0].
        */
        static void getBitDepths(PixelFormat format, int rgba[4]);

		/** Gives the masks for the R, G, B and A component
		  @note			Only valid for native endian formats
        */
        static void getBitMasks(PixelFormat format, uint32 rgba[4]);

		/** Gives the bit shifts for R, G, B and A component
		@note			Only valid for native endian formats
		*/
		static void getBitShifts(PixelFormat format, unsigned char rgba[4]);

        /** Gets the name of an image format
        */
        static String getFormatName(PixelFormat srcformat);

        /** Returns wether the format can be packed or unpacked with the packColour()
        and unpackColour() functions. This is generally not true for compressed and
        depth formats as they are special. It can only be true for formats with a
        fixed element size.
          @returns 
               true if yes, otherwise false
        */
        static bool isAccessible(PixelFormat srcformat);
        
        /** Returns the component type for a certain pixel format. Returns PCT_BYTE
            in case there is no clear component type like with compressed formats.
            This is one of PCT_BYTE, PCT_SHORT, PCT_FLOAT16, PCT_FLOAT32.
        */
        static PixelComponentType getComponentType(PixelFormat fmt);
        
        /** Returns the component count for a certain pixel format. Returns 3(no alpha) or 
            4 (has alpha) in case there is no clear component type like with compressed formats.
         */
        static size_t getComponentCount(PixelFormat fmt);

        /** Gets the format from given name.
            @param  name            The string of format name
            @param  accessibleOnly  If true, non-accessible format will treat as invalid format,
                                    otherwise, all supported format are valid.
            @param  caseSensitive   Should be set true if string match should use case sensitivity.
            @returns                The format match the format name, or PF_UNKNOWN if is invalid name.
        */
        static PixelFormat getFormatFromName(const String& name, bool accessibleOnly = false, bool caseSensitive = false);

        /** Gets the BNF expression of the pixel-formats.
            @note                   The string returned by this function is intented to use as a BNF expression
                                    to work with Compiler2Pass.
            @param  accessibleOnly  If true, only accessible pixel format will take into account, otherwise all
                                    pixel formats list in PixelFormat enumeration will being returned.
            @returns                A string contains the BNF expression.
        */
        static String getBNFExpressionOfPixelFormats(bool accessibleOnly = false);

        /** Returns the similar format but acoording with given bit depths.
            @param fmt      The original foamt.
            @param integerBits Preferred bit depth (pixel bits) for integer pixel format.
                            Available values: 0, 16 and 32, where 0 (the default) means as it is.
            @param floatBits Preferred bit depth (channel bits) for float pixel format.
                            Available values: 0, 16 and 32, where 0 (the default) means as it is.
            @returns        The format that similar original format with bit depth according
                            with preferred bit depth, or original format if no convertion occuring.
        */
        static PixelFormat getFormatForBitDepths(PixelFormat fmt, ushort integerBits, ushort floatBits);

        /** Pack a colour value to memory
        	@param colour	The colour
        	@param pf		Pixelformat in which to write the colour
        	@param dest		Destination memory location
        */
        static void packColour(const ColourValue &colour, const PixelFormat pf,  void* dest);
        /** Pack a colour value to memory
        	@param r,g,b,a	The four colour components, range 0x00 to 0xFF
        	@param pf		Pixelformat in which to write the colour
        	@param dest		Destination memory location
        */
        static void packColour(const uint8 r, const uint8 g, const uint8 b, const uint8 a, const PixelFormat pf,  void* dest);
         /** Pack a colour value to memory
        	@param r,g,b,a	The four colour components, range 0.0f to 1.0f
        					(an exception to this case exists for floating point pixel
        					formats, which don't clamp to 0.0f..1.0f)
        	@param pf		Pixelformat in which to write the colour
        	@param dest		Destination memory location
        */
        static void packColour(const float r, const float g, const float b, const float a, const PixelFormat pf,  void* dest);

        /** Unpack a colour value from memory
        	@param colour	The colour is returned here
        	@param pf		Pixelformat in which to read the colour
        	@param src		Source memory location
        */
        static void unpackColour(ColourValue *colour, PixelFormat pf,  const void* src);
        /** Unpack a colour value from memory
        	@param r,g,b,a	The colour is returned here (as byte)
        	@param pf		Pixelformat in which to read the colour
        	@param src		Source memory location
        	@remarks 	This function returns the colour components in 8 bit precision,
        		this will lose precision when coming from PF_A2R10G10B10 or floating
        		point formats.  
        */
        static void unpackColour(uint8 *r, uint8 *g, uint8 *b, uint8 *a, PixelFormat pf,  const void* src);
        /** Unpack a colour value from memory
        	@param r,g,b,a	The colour is returned here (as float)
        	@param pf		Pixelformat in which to read the colour
        	@param src		Source memory location
        */
        static void unpackColour(float *r, float *g, float *b, float *a, PixelFormat pf,  const void* src); 
        
        /** Convert consecutive pixels from one format to another. No dithering or filtering is being done. 
         	Converting from RGB to luminance takes the R channel.  In case the source and destination format match,
         	just a copy is done.
         	@param	src			Pointer to source region
         	@param	srcFormat	Pixel format of source region
         	@param   dst			Pointer to destination region
         	@param	dstFormat	Pixel format of destination region
         */
        static void bulkPixelConversion(void *src, PixelFormat srcFormat, void *dest, PixelFormat dstFormat, unsigned int count);

      	/** Convert pixels from one format to another. No dithering or filtering is being done. Converting
          	from RGB to luminance takes the R channel. 
		 	@param	src			PixelBox containing the source pixels, pitches and format
		 	@param	dst			PixelBox containing the destination pixels, pitches and format
		 	@remarks The source and destination boxes must have the same
         	dimensions. In case the source and destination format match, a plain copy is done.
        */
        static void bulkPixelConversion(const PixelBox &src, const PixelBox &dst);
    };
	/** @} */
	/** @} */

}

#endif
