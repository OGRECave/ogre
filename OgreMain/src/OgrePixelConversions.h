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
/** Internal include file -- do not use externally */
using namespace Ogre;

// NB VC6 can't handle these templates
#if OGRE_COMPILER != OGRE_COMPILER_MSVC || OGRE_COMP_VER >= 1300

#define FMTCONVERTERID(from,to) (((from)<<8)|(to))

/**
 * Convert a box of pixel from one type to another. Who needs automatic code 
 * generation when we have C++ templates and the policy design pattern.
 * 
 * @param   U       Policy class to facilitate pixel-to-pixel conversion. This class
 *    has at least two typedefs: SrcType and DstType. SrcType is the source element type,
 *    dstType is the destination element type. It also has a static method, pixelConvert, that
 *    converts a srcType into a dstType.
 */

template <class U> struct PixelBoxConverter 
{
    static const int ID = U::ID;
    static void conversion(const PixelBox &src, const PixelBox &dst)
    {
        typename U::SrcType *srcptr = static_cast<typename U::SrcType*>(src.data)
			+ (src.left + src.top * src.rowPitch + src.front * src.slicePitch);
        typename U::DstType *dstptr = static_cast<typename U::DstType*>(dst.data)
			+ (dst.left + dst.top * dst.rowPitch + dst.front * dst.slicePitch);
        const size_t srcSliceSkip = src.getSliceSkip();
        const size_t dstSliceSkip = dst.getSliceSkip();
        const size_t k = src.right - src.left;
        for(size_t z=src.front; z<src.back; z++) 
        {
            for(size_t y=src.top; y<src.bottom; y++)
            {
                for(size_t x=0; x<k; x++)
                {
                    dstptr[x] = U::pixelConvert(srcptr[x]);
                }
                srcptr += src.rowPitch;
                dstptr += dst.rowPitch;
            }
            srcptr += srcSliceSkip;
            dstptr += dstSliceSkip;
        }    
    }
};

template <typename T, typename U, int id> struct PixelConverter {
    static const int ID = id;
    typedef T SrcType;
    typedef U DstType;    
    
    //inline static DstType pixelConvert(const SrcType &inp);
};


/** Type for PF_R8G8B8/PF_B8G8R8 */
struct Col3b {
    Col3b(unsigned int a, unsigned int b, unsigned int c): 
        x((Ogre::uint8)a), y((Ogre::uint8)b), z((Ogre::uint8)c) { }
    Ogre::uint8 x,y,z;
};
/** Type for PF_FLOAT32_RGB */
struct Col3f {
	Col3f(float r, float g, float b):
		r(r), g(g), b(b) { }
	float r,g,b;
};
/** Type for PF_FLOAT32_RGBA */
struct Col4f {
	Col4f(float r, float g, float b, float a):
		r(r), g(g), b(b), a(a) { }
	float r,g,b,a;
};

struct A8R8G8B8toA8B8G8R8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_A8R8G8B8, PF_A8B8G8R8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0x000000FF)<<16)|(inp&0xFF00FF00)|((inp&0x00FF0000)>>16);
    }
};

struct A8R8G8B8toB8G8R8A8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_A8R8G8B8, PF_B8G8R8A8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0x000000FF)<<24)|((inp&0x0000FF00)<<8)|((inp&0x00FF0000)>>8)|((inp&0xFF000000)>>24);
    }
};

struct A8R8G8B8toR8G8B8A8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_A8R8G8B8, PF_R8G8B8A8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0x00FFFFFF)<<8)|((inp&0xFF000000)>>24);
    }
};

struct A8B8G8R8toA8R8G8B8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_A8B8G8R8, PF_A8R8G8B8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0x000000FF)<<16)|(inp&0xFF00FF00)|((inp&0x00FF0000)>>16);
    }
};

struct A8B8G8R8toB8G8R8A8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_A8B8G8R8, PF_B8G8R8A8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0x00FFFFFF)<<8)|((inp&0xFF000000)>>24);
    }
};

struct A8B8G8R8toR8G8B8A8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_A8B8G8R8, PF_R8G8B8A8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0x000000FF)<<24)|((inp&0x0000FF00)<<8)|((inp&0x00FF0000)>>8)|((inp&0xFF000000)>>24);
    }
};

struct B8G8R8A8toA8R8G8B8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_B8G8R8A8, PF_A8R8G8B8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0x000000FF)<<24)|((inp&0x0000FF00)<<8)|((inp&0x00FF0000)>>8)|((inp&0xFF000000)>>24);
    }
};

struct B8G8R8A8toA8B8G8R8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_B8G8R8A8, PF_A8B8G8R8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0x000000FF)<<24)|((inp&0xFFFFFF00)>>8);
    }
};

struct B8G8R8A8toR8G8B8A8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_B8G8R8A8, PF_R8G8B8A8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0x0000FF00)<<16)|(inp&0x00FF00FF)|((inp&0xFF000000)>>16);
    }
};

struct R8G8B8A8toA8R8G8B8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_R8G8B8A8, PF_A8R8G8B8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0x000000FF)<<24)|((inp&0xFFFFFF00)>>8);
    }
};

struct R8G8B8A8toA8B8G8R8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_R8G8B8A8, PF_A8B8G8R8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0x000000FF)<<24)|((inp&0x0000FF00)<<8)|((inp&0x00FF0000)>>8)|((inp&0xFF000000)>>24);
    }
};

struct R8G8B8A8toB8G8R8A8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_R8G8B8A8, PF_B8G8R8A8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0x0000FF00)<<16)|(inp&0x00FF00FF)|((inp&0xFF000000)>>16);
    }
};

struct A8B8G8R8toL8: public PixelConverter <Ogre::uint32, Ogre::uint8, FMTCONVERTERID(PF_A8B8G8R8, PF_L8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return (Ogre::uint8)(inp&0x000000FF);
    }
};

struct L8toA8B8G8R8: public PixelConverter <Ogre::uint8, Ogre::uint32, FMTCONVERTERID(PF_L8, PF_A8B8G8R8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return 0xFF000000|(((unsigned int)inp)<<0)|(((unsigned int)inp)<<8)|(((unsigned int)inp)<<16);
    }
};

struct A8R8G8B8toL8: public PixelConverter <Ogre::uint32, Ogre::uint8, FMTCONVERTERID(PF_A8R8G8B8, PF_L8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return (Ogre::uint8)((inp&0x00FF0000)>>16);
    }
};

struct L8toA8R8G8B8: public PixelConverter <Ogre::uint8, Ogre::uint32, FMTCONVERTERID(PF_L8, PF_A8R8G8B8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return 0xFF000000|(((unsigned int)inp)<<0)|(((unsigned int)inp)<<8)|(((unsigned int)inp)<<16);
    }
};

struct B8G8R8A8toL8: public PixelConverter <Ogre::uint32, Ogre::uint8, FMTCONVERTERID(PF_B8G8R8A8, PF_L8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return (Ogre::uint8)((inp&0x0000FF00)>>8);
    }
};

struct L8toB8G8R8A8: public PixelConverter <Ogre::uint8, Ogre::uint32, FMTCONVERTERID(PF_L8, PF_B8G8R8A8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return 0x000000FF|(((unsigned int)inp)<<8)|(((unsigned int)inp)<<16)|(((unsigned int)inp)<<24);
    }
};

struct L8toL16: public PixelConverter <Ogre::uint8, Ogre::uint16, FMTCONVERTERID(PF_L8, PF_L16)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return (Ogre::uint16)((((unsigned int)inp)<<8)|(((unsigned int)inp)));
    }
};

struct L16toL8: public PixelConverter <Ogre::uint16, Ogre::uint8, FMTCONVERTERID(PF_L16, PF_L8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return (Ogre::uint8)(inp>>8);
    }
};

struct R8G8B8toB8G8R8: public PixelConverter <Col3b, Col3b, FMTCONVERTERID(PF_R8G8B8, PF_B8G8R8)>
{
    inline static DstType pixelConvert(const SrcType &inp)
    {
        return Col3b(inp.z, inp.y, inp.x);
    }  
};

struct B8G8R8toR8G8B8: public PixelConverter <Col3b, Col3b, FMTCONVERTERID(PF_B8G8R8, PF_R8G8B8)>
{
    inline static DstType pixelConvert(const SrcType &inp)
    {
        return Col3b(inp.z, inp.y, inp.x);
    }  
};

// X8Y8Z8 ->  X8<<xshift Y8<<yshift Z8<<zshift A8<<ashift
template <int id, unsigned int xshift, unsigned int yshift, unsigned int zshift, unsigned int ashift> struct Col3btoUint32swizzler:
    public PixelConverter <Col3b, Ogre::uint32, id>
{
    inline static Ogre::uint32 pixelConvert(const Col3b &inp)
    {
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
        return (0xFF<<ashift) | (((unsigned int)inp.x)<<xshift) | (((unsigned int)inp.y)<<yshift) | (((unsigned int)inp.z)<<zshift);
#else
        return (0xFF<<ashift) | (((unsigned int)inp.x)<<zshift) | (((unsigned int)inp.y)<<yshift) | (((unsigned int)inp.z)<<xshift);
#endif
    }
};

struct R8G8B8toA8R8G8B8: public Col3btoUint32swizzler<FMTCONVERTERID(PF_R8G8B8, PF_A8R8G8B8), 16, 8, 0, 24> { };
struct B8G8R8toA8R8G8B8: public Col3btoUint32swizzler<FMTCONVERTERID(PF_B8G8R8, PF_A8R8G8B8), 0, 8, 16, 24> { };
struct R8G8B8toA8B8G8R8: public Col3btoUint32swizzler<FMTCONVERTERID(PF_R8G8B8, PF_A8B8G8R8), 0, 8, 16, 24> { };
struct B8G8R8toA8B8G8R8: public Col3btoUint32swizzler<FMTCONVERTERID(PF_B8G8R8, PF_A8B8G8R8), 16, 8, 0, 24> { };
struct R8G8B8toB8G8R8A8: public Col3btoUint32swizzler<FMTCONVERTERID(PF_R8G8B8, PF_B8G8R8A8), 8, 16, 24, 0> { };
struct B8G8R8toB8G8R8A8: public Col3btoUint32swizzler<FMTCONVERTERID(PF_B8G8R8, PF_B8G8R8A8), 24, 16, 8, 0> { };

struct A8R8G8B8toR8G8B8: public PixelConverter <Ogre::uint32, Col3b, FMTCONVERTERID(PF_A8R8G8B8, PF_BYTE_RGB)>
{
    inline static DstType pixelConvert(Ogre::uint32 inp)
    {
        return Col3b((Ogre::uint8)((inp>>16)&0xFF), (Ogre::uint8)((inp>>8)&0xFF), (Ogre::uint8)((inp>>0)&0xFF));
    }
};
struct A8R8G8B8toB8G8R8: public PixelConverter <Ogre::uint32, Col3b, FMTCONVERTERID(PF_A8R8G8B8, PF_BYTE_BGR)>
{
    inline static DstType pixelConvert(Ogre::uint32 inp)
    {
        return Col3b((Ogre::uint8)((inp>>0)&0xFF), (Ogre::uint8)((inp>>8)&0xFF), (Ogre::uint8)((inp>>16)&0xFF));
    }
};

// Only conversions from X8R8G8B8 to formats with alpha need to be defined, the rest is implicitly the same
// as A8R8G8B8
struct X8R8G8B8toA8R8G8B8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_X8R8G8B8, PF_A8R8G8B8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return inp | 0xFF000000;
    }
};
struct X8R8G8B8toA8B8G8R8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_X8R8G8B8, PF_A8B8G8R8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0x0000FF)<<16)|((inp&0xFF0000)>>16)|(inp&0x00FF00)|0xFF000000;
    }
};
struct X8R8G8B8toB8G8R8A8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_X8R8G8B8, PF_B8G8R8A8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0x0000FF)<<24)|((inp&0xFF0000)>>8)|((inp&0x00FF00)<<8)|0x000000FF;
    }
};
struct X8R8G8B8toR8G8B8A8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_X8R8G8B8, PF_R8G8B8A8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0xFFFFFF)<<8)|0x000000FF;
    }
};

// X8B8G8R8
struct X8B8G8R8toA8R8G8B8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_X8B8G8R8, PF_A8R8G8B8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0x0000FF)<<16)|((inp&0xFF0000)>>16)|(inp&0x00FF00)|0xFF000000;
    }
};
struct X8B8G8R8toA8B8G8R8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_X8B8G8R8, PF_A8B8G8R8)>
{
	inline static DstType pixelConvert(SrcType inp)
    {
        return inp | 0xFF000000;
    }
};
struct X8B8G8R8toB8G8R8A8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_X8B8G8R8, PF_B8G8R8A8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0xFFFFFF)<<8)|0x000000FF;
    }
};
struct X8B8G8R8toR8G8B8A8: public PixelConverter <Ogre::uint32, Ogre::uint32, FMTCONVERTERID(PF_X8B8G8R8, PF_R8G8B8A8)>
{
    inline static DstType pixelConvert(SrcType inp)
    {
        return ((inp&0x0000FF)<<24)|((inp&0xFF0000)>>8)|((inp&0x00FF00)<<8)|0x000000FF;
    }
};


#define CASECONVERTER(type) case type::ID : PixelBoxConverter<type>::conversion(src, dst); return 1;

inline int doOptimizedConversion(const PixelBox &src, const PixelBox &dst)
{;
    switch(FMTCONVERTERID(src.format, dst.format))
    {
        // Register converters here
		CASECONVERTER(A8R8G8B8toA8B8G8R8);
		CASECONVERTER(A8R8G8B8toB8G8R8A8);
		CASECONVERTER(A8R8G8B8toR8G8B8A8);
		CASECONVERTER(A8B8G8R8toA8R8G8B8);
		CASECONVERTER(A8B8G8R8toB8G8R8A8);
		CASECONVERTER(A8B8G8R8toR8G8B8A8);
		CASECONVERTER(B8G8R8A8toA8R8G8B8);
		CASECONVERTER(B8G8R8A8toA8B8G8R8);
		CASECONVERTER(B8G8R8A8toR8G8B8A8);
		CASECONVERTER(R8G8B8A8toA8R8G8B8);
		CASECONVERTER(R8G8B8A8toA8B8G8R8);
		CASECONVERTER(R8G8B8A8toB8G8R8A8);
        CASECONVERTER(A8B8G8R8toL8);
        CASECONVERTER(L8toA8B8G8R8);
        CASECONVERTER(A8R8G8B8toL8);
        CASECONVERTER(L8toA8R8G8B8);
        CASECONVERTER(B8G8R8A8toL8);
        CASECONVERTER(L8toB8G8R8A8);
        CASECONVERTER(L8toL16);
        CASECONVERTER(L16toL8);
        CASECONVERTER(B8G8R8toR8G8B8);
        CASECONVERTER(R8G8B8toB8G8R8);
        CASECONVERTER(R8G8B8toA8R8G8B8);
        CASECONVERTER(B8G8R8toA8R8G8B8);
        CASECONVERTER(R8G8B8toA8B8G8R8);
        CASECONVERTER(B8G8R8toA8B8G8R8);
        CASECONVERTER(R8G8B8toB8G8R8A8);
        CASECONVERTER(B8G8R8toB8G8R8A8);
		CASECONVERTER(A8R8G8B8toR8G8B8);
		CASECONVERTER(A8R8G8B8toB8G8R8);
		CASECONVERTER(X8R8G8B8toA8R8G8B8);
		CASECONVERTER(X8R8G8B8toA8B8G8R8);
		CASECONVERTER(X8R8G8B8toB8G8R8A8);
		CASECONVERTER(X8R8G8B8toR8G8B8A8);
		CASECONVERTER(X8B8G8R8toA8R8G8B8);
		CASECONVERTER(X8B8G8R8toA8B8G8R8);
		CASECONVERTER(X8B8G8R8toB8G8R8A8);
		CASECONVERTER(X8B8G8R8toR8G8B8A8);

        default:
            return 0;
    }
}
#undef CASECONVERTER

#endif // VC6 protection
