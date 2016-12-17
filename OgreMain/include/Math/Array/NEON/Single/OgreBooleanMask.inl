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

namespace Ogre
{
    inline ArrayMaskR BooleanMask4::getMask( bool x, bool y, bool z, bool w )
    {
        size_t idx = (size_t)x | ( (size_t)y << 1 ) | ( (size_t)z << 2 ) | ( (size_t)w << 3 );
        return mMasks[idx];
    }

    inline ArrayMaskR BooleanMask4::getMask( bool b[4] )
    {
        size_t idx = (size_t)b[0] | ( (size_t)b[1] << 1 ) | ( (size_t)b[2] << 2 ) | ( (size_t)b[3] << 3 );
        return mMasks[idx];
    }
	//--------------------------------------------------------------------------------------
	inline ArrayMaskR BooleanMask4::getAllSetMask(void)
	{
		return mMasks[MASK_XYZW];
	}
    //--------------------------------------------------------------------------------------
    inline bool BooleanMask4::allBitsSet( bool mask0[4], bool mask1[4] )
    {
#if __cplusplus > 199711L //C++11
        static_assert( sizeof(bool) == 1 && sizeof(uint32) == 4,
                      "This code relies on correct casting!" );
#else
        assert( sizeof(bool) == 1 && sizeof(uint32) == 4 && "This code relies on correct casting!" );
#endif
        return ( *reinterpret_cast<uint32*>(mask0) & *reinterpret_cast<uint32*>(mask1) ) == 0x01010101;
    }
    //--------------------------------------------------------------------------------------
    inline uint32 BooleanMask4::getScalarMask( ArrayMaskR mask )
    {
        return vmovemaskq_u32( mask );
    }
    //--------------------------------------------------------------------------------------
    inline uint32 BooleanMask4::getScalarMask( ArrayInt mask )
    {
        return vmovemaskq_u32( vreinterpretq_u32_s32( mask ) );
    }

    #define IS_SET_MASK_X( intMask ) ((intMask & MASK_W) != 0)
    #define IS_SET_MASK_Y( intMask ) ((intMask & MASK_Z) != 0)
    #define IS_SET_MASK_Z( intMask ) ((intMask & MASK_Y) != 0)
    #define IS_SET_MASK_W( intMask ) ((intMask & MASK_X) != 0)

    #define IS_BIT_SET( bit, intMask ) ( (intMask & (1 << bit) ) != 0)
}
