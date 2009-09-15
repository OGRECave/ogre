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
#include <OgreStableHeaders.h>
#include "OgreILUtil.h"
#include <OgrePixelFormat.h>
#include <OgreBitwise.h>
#include <OgreColourValue.h>
#include <OgreException.h>

#include <IL/il.h>
#include <IL/ilu.h>
   
namespace Ogre {
	
	/*************************************************************************
    * IL specific functions
    */
    PixelFormat ILUtil::ilFormat2OgreFormat( int ImageFormat, int ImageType ) 
	{
        PixelFormat fmt = PF_UNKNOWN;
        switch( ImageFormat )
        {
		/* Compressed formats -- ignore type */
		case IL_DXT1:   fmt = PF_DXT1; break;
		case IL_DXT2:   fmt = PF_DXT2; break;
		case IL_DXT3:   fmt = PF_DXT3; break;
		case IL_DXT4:   fmt = PF_DXT4; break;
		case IL_DXT5:   fmt = PF_DXT5; break;
		/* Normal formats */
        case IL_RGB:
			switch(ImageType)
            {
			case IL_FLOAT:  fmt = PF_FLOAT32_RGB; break;
			case IL_UNSIGNED_SHORT: 
			case IL_SHORT:  fmt = PF_SHORT_RGBA; break;
			default:       	fmt = PF_BYTE_RGB; break;
			}
            break;
        case IL_BGR:
			switch(ImageType)
            {
			case IL_FLOAT:  fmt = PF_FLOAT32_RGB; break;
			case IL_UNSIGNED_SHORT: 
			case IL_SHORT:  fmt = PF_SHORT_RGBA; break;
			default:       	fmt = PF_BYTE_BGR; break;
			}
            break;            
        case IL_RGBA:
			switch(ImageType)
            {
			case IL_FLOAT:  fmt = PF_FLOAT32_RGBA; break;
			case IL_UNSIGNED_SHORT: 
			case IL_SHORT:  fmt = PF_SHORT_RGBA; break;
			default:       	fmt = PF_BYTE_RGBA; break;
			}
            break;
        case IL_BGRA:
			switch(ImageType)
            {
			case IL_FLOAT:  fmt = PF_FLOAT32_RGBA; break;
			case IL_UNSIGNED_SHORT: 
			case IL_SHORT:  fmt = PF_SHORT_RGBA; break;
			default:       	fmt = PF_BYTE_BGRA; break;
			}
            break;
        case IL_LUMINANCE:
            switch(ImageType)
            {
            case IL_BYTE:
            case IL_UNSIGNED_BYTE:
                fmt = PF_L8;
                break;
            default:
                fmt = PF_L16;
            }
            break;            
        case IL_LUMINANCE_ALPHA:
			fmt = PF_BYTE_LA;
            break;
        }  
        return fmt;
    }

    //-----------------------------------------------------------------------

    ILUtil::ILFormat ILUtil::OgreFormat2ilFormat( PixelFormat format )
    {
		switch(format) {
            case PF_BYTE_L: return ILFormat(1, IL_LUMINANCE, IL_UNSIGNED_BYTE);
			case PF_BYTE_A: return ILFormat(1, IL_LUMINANCE, IL_UNSIGNED_BYTE);
			case PF_SHORT_L: return ILFormat(1, IL_LUMINANCE, IL_UNSIGNED_SHORT);
			case PF_BYTE_LA: return ILFormat(2, IL_LUMINANCE_ALPHA, IL_UNSIGNED_BYTE);
			case PF_BYTE_RGB: return ILFormat(3, IL_RGB, IL_UNSIGNED_BYTE);
			case PF_BYTE_RGBA: return ILFormat(4, IL_RGBA, IL_UNSIGNED_BYTE);
            case PF_BYTE_BGR: return ILFormat(3, IL_BGR, IL_UNSIGNED_BYTE);
            case PF_BYTE_BGRA: return ILFormat(4, IL_BGRA, IL_UNSIGNED_BYTE);
			case PF_SHORT_RGBA: return ILFormat(4, IL_RGBA, IL_UNSIGNED_SHORT);
			case PF_FLOAT32_RGB: return ILFormat(3, IL_RGB, IL_FLOAT);
			case PF_FLOAT32_RGBA: return ILFormat(4, IL_RGBA, IL_FLOAT);
			case PF_DXT1: return ILFormat(0, IL_DXT1);
			case PF_DXT2: return ILFormat(0, IL_DXT2);
			case PF_DXT3: return ILFormat(0, IL_DXT3);
			case PF_DXT4: return ILFormat(0, IL_DXT4);
			case PF_DXT5: return ILFormat(0, IL_DXT5);
			default: return ILFormat();
		}
	}

    //-----------------------------------------------------------------------
	/// Helper functions for DevIL to Ogre conversion
	inline void packI(uint8 r, uint8 g, uint8 b, uint8 a, PixelFormat pf,  void* dest)
	{
		PixelUtil::packColour(r, g, b, a, pf, dest);
	}
	inline void packI(uint16 r, uint16 g, uint16 b, uint16 a, PixelFormat pf,  void* dest)
	{
		PixelUtil::packColour((float)r/65535.0f, (float)g/65535.0f, 
			(float)b/65535.0f, (float)a/65535.0f, pf, dest);
	}
	inline void packI(float r, float g, float b, float a, PixelFormat pf,  void* dest)
	{
		PixelUtil::packColour(r, g, b, a, pf, dest);
	}
    template <typename T> void ilToOgreInternal(uint8 *tar, PixelFormat ogrefmt, 
        T r, T g, T b, T a)
    {
        const int ilfmt = ilGetInteger( IL_IMAGE_FORMAT );
        T *src = (T*)ilGetData();
        T *srcend = (T*)((uint8*)ilGetData() + ilGetInteger( IL_IMAGE_SIZE_OF_DATA ));
        const size_t elemSize = PixelUtil::getNumElemBytes(ogrefmt);
        while(src < srcend) {
            switch(ilfmt) {
			case IL_RGB:
				r = src[0];	g = src[1];	b = src[2];
				src += 3;
				break;
			case IL_BGR:
				b = src[0];	g = src[1];	r = src[2];
				src += 3;
				break;
			case IL_LUMINANCE:
				r = src[0];	g = src[0];	b = src[0];
				src += 1;
				break;
			case IL_LUMINANCE_ALPHA:
				r = src[0];	g = src[0];	b = src[0];	a = src[1];
				src += 2;
				break;
			case IL_RGBA:
				r = src[0];	g = src[1];	b = src[2];	a = src[3];
				src += 4;
				break;
			case IL_BGRA:
				b = src[0];	g = src[1];	r = src[2];	a = src[3];
				src += 4;
				break;
			default:
				return;
            }
            packI(r, g, b, a, ogrefmt, tar);
            tar += elemSize;
        }

    }   
    //----------------------------------------------------------------------- 
	/// Utility function to convert IL data types to UNSIGNED_
	ILenum ILabs(ILenum in) {
		switch(in) {
		case IL_INT: return IL_UNSIGNED_INT;
		case IL_BYTE: return IL_UNSIGNED_BYTE;
		case IL_SHORT: return IL_UNSIGNED_SHORT;
		default: return in;
		}
	}
    //-----------------------------------------------------------------------
    void ILUtil::toOgre(const PixelBox &dst) 
    {
		if(!dst.isConsecutive())
			OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                "Destination must currently be consecutive",
                "ILUtil::ilToOgre" ) ;
		if(dst.getWidth() != static_cast<size_t>(ilGetInteger( IL_IMAGE_WIDTH )) ||
        	dst.getHeight() != static_cast<size_t>(ilGetInteger( IL_IMAGE_HEIGHT )) ||
        	dst.getDepth() != static_cast<size_t>(ilGetInteger( IL_IMAGE_DEPTH )))
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                "Destination dimensions must equal IL dimension",
                "ILUtil::ilToOgre" ) ;
        
        int ilfmt = ilGetInteger( IL_IMAGE_FORMAT );
        int iltp = ilGetInteger( IL_IMAGE_TYPE );

		// Check if in-memory format just matches
		// If yes, we can just copy it and save conversion
		ILFormat ifmt = OgreFormat2ilFormat( dst.format );
		if(ifmt.format == ilfmt && ILabs(ifmt.type) == ILabs(iltp)) {
            memcpy(dst.data, ilGetData(), ilGetInteger( IL_IMAGE_SIZE_OF_DATA )); 
            return;
        }
		// Try if buffer is in a known OGRE format so we can use OGRE its
		// conversion routines
		PixelFormat bufFmt = ilFormat2OgreFormat((int)ilfmt, (int)iltp);
		
		ifmt = OgreFormat2ilFormat( bufFmt );
		if(ifmt.format == ilfmt && ILabs(ifmt.type) == ILabs(iltp))
		{
			// IL format matches another OGRE format
			PixelBox src(dst.getWidth(), dst.getHeight(), dst.getDepth(), bufFmt, ilGetData());
			PixelUtil::bulkPixelConversion(src, dst);
			return;
		}
		
        // Thee extremely slow method
        if(iltp == IL_UNSIGNED_BYTE || iltp == IL_BYTE) 
        {
            ilToOgreInternal(static_cast<uint8*>(dst.data), dst.format, (uint8)0x00,(uint8)0x00,(uint8)0x00,(uint8)0xFF);
        } 
        else if(iltp == IL_FLOAT)
        {
            ilToOgreInternal(static_cast<uint8*>(dst.data), dst.format, 0.0f,0.0f,0.0f,1.0f);          
        }
		else if(iltp == IL_SHORT || iltp == IL_UNSIGNED_SHORT)
        {
			ilToOgreInternal(static_cast<uint8*>(dst.data), dst.format, 
				(uint16)0x0000,(uint16)0x0000,(uint16)0x0000,(uint16)0xFFFF); 
        }
        else 
        {
            OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                "Cannot convert this DevIL type",
                "ILUtil::ilToOgre" ) ;
        }
    }
    //-----------------------------------------------------------------------
    void ILUtil::fromOgre(const PixelBox &src)
    {
		// ilTexImage http://openil.sourceforge.net/docs/il/f00059.htm
		ILFormat ifmt = OgreFormat2ilFormat( src.format );
		if(src.isConsecutive() && ifmt.isValid()) 
		{
			// The easy case, the buffer is laid out in memory just like 
			// we want it to be and is in a format DevIL can understand directly
			// We could even save the copy if DevIL would let us
			ilTexImage(static_cast<ILuint>(src.getWidth()), 
				static_cast<ILuint>(src.getHeight()), 
				static_cast<ILuint>(src.getDepth()), ifmt.numberOfChannels,
				ifmt.format, ifmt.type, src.data);
		} 
		else if(ifmt.isValid()) 
		{
			// The format can be understood directly by DevIL. The only 
			// problem is that ilTexImage expects our image data consecutively 
			// so we cannot use that directly.
			
			// Let DevIL allocate the memory for us, and copy the data consecutively
			// to its memory
			ilTexImage(static_cast<ILuint>(src.getWidth()), 
				static_cast<ILuint>(src.getHeight()), 
				static_cast<ILuint>(src.getDepth()), ifmt.numberOfChannels,
				ifmt.format, ifmt.type, 0);
			PixelBox dst(src.getWidth(), src.getHeight(), src.getDepth(), src.format, ilGetData());
			PixelUtil::bulkPixelConversion(src, dst);
		} 
		else 
		{
			// Here it gets ugly. We're stuck with a pixel format that DevIL
			// can't do anything with. We will do a bulk pixel conversion and
			// then feed it to DevIL anyway. The problem is finding the best
			// format to convert to.
			
			// most general format supported by OGRE and DevIL
			PixelFormat fmt = PixelUtil::hasAlpha(src.format)?PF_FLOAT32_RGBA:PF_FLOAT32_RGB; 

			// Make up a pixel format
			// We don't have to consider luminance formats as they have
			// straight conversions to DevIL, just weird permutations of RGBA an LA
			int depths[4];
			PixelUtil::getBitDepths(src.format, depths);
			
			// Native endian format with all bit depths<8 can safely and quickly be 
			// converted to 24/32 bit
			if(PixelUtil::isNativeEndian(src.format) && 
				depths[0]<=8 && depths[1]<=8 && depths[2]<=8 && depths[3]<=8) {
				if(PixelUtil::hasAlpha(src.format)) {
					fmt = PF_A8R8G8B8;
				} else {
					fmt = PF_R8G8B8;
				}
			}
			
			// Let DevIL allocate the memory for us, then do the conversion ourselves
			ifmt = OgreFormat2ilFormat( fmt );
			ilTexImage(static_cast<ILuint>(src.getWidth()), 
				static_cast<ILuint>(src.getHeight()), 
				static_cast<ILuint>(src.getDepth()), ifmt.numberOfChannels,
				ifmt.format, ifmt.type, 0);
			PixelBox dst(src.getWidth(), src.getHeight(), src.getDepth(), fmt, ilGetData());
			PixelUtil::bulkPixelConversion(src, dst);
		}
    }

}
