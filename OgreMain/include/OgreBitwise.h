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
#ifndef _Bitwise_H__
#define _Bitwise_H__

#include "OgrePrerequisites.h"

#ifdef bswap16
#undef bswap16
#undef bswap32
#undef bswap64
#endif

#ifndef __has_builtin
    // Compatibility with non-clang compilers
    #define __has_builtin(x) 0
#endif

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Math
    *  @{
    */

    /** Class for manipulating bit patterns.
    */
    class Bitwise {
    public:
        /** Returns value with reversed bytes order.
        */
        static OGRE_FORCE_INLINE uint16 bswap16(uint16 arg)
        {
#if OGRE_COMPILER == OGRE_COMPILER_MSVC && OGRE_COMP_VER >= 1310
            return _byteswap_ushort(arg);
#elif (OGRE_COMPILER == OGRE_COMPILER_CLANG && __has_builtin(__builtin_bswap16)) || (OGRE_COMPILER == OGRE_COMPILER_GNUC && OGRE_COMP_VER >= 480)
            return __builtin_bswap16(arg);
#else
            return ((arg << 8) & 0xFF00) | ((arg >> 8) & 0x00FF);
#endif
        }
        /** Returns value with reversed bytes order.
        */
        static OGRE_FORCE_INLINE uint32 bswap32(uint32 arg)
        {
#if OGRE_COMPILER == OGRE_COMPILER_MSVC && OGRE_COMP_VER >= 1310
            return _byteswap_ulong(arg);
#elif (OGRE_COMPILER == OGRE_COMPILER_CLANG && __has_builtin(__builtin_bswap32)) || (OGRE_COMPILER == OGRE_COMPILER_GNUC && OGRE_COMP_VER >= 430)
            return __builtin_bswap32(arg);
#else
            return ((arg & 0x000000FF) << 24) | ((arg & 0x0000FF00) << 8) | ((arg >> 8) & 0x0000FF00) | ((arg >> 24) & 0x000000FF);
#endif
        }
        /** Returns value with reversed bytes order.
        */
        static OGRE_FORCE_INLINE uint64 bswap64(uint64 arg)
        {
#if OGRE_COMPILER == OGRE_COMPILER_MSVC && OGRE_COMP_VER >= 1310
            return _byteswap_uint64(arg);
#elif (OGRE_COMPILER == OGRE_COMPILER_CLANG && __has_builtin(__builtin_bswap64)) || (OGRE_COMPILER == OGRE_COMPILER_GNUC && OGRE_COMP_VER >= 430)
            return __builtin_bswap64(arg);
#else
            union { 
                uint64 sv;
                uint32 ul[2];
            } tmp, result;
            tmp.sv = arg;
            result.ul[0] = bswap32(tmp.ul[1]);
            result.ul[1] = bswap32(tmp.ul[0]);
            return result.sv; 
#endif
        }

        /** Reverses byte order of buffer. Use bswap16/32/64 instead if possible.
        */
        static inline void bswapBuffer(void * pData, size_t size)
        {
            char swapByte;
            for(char *p0 = (char*)pData, *p1 = p0 + size - 1; p0 < p1; ++p0, --p1)
            {
                swapByte = *p0;
                *p0 = *p1;
                *p1 = swapByte;
            }
        }
        /** Reverses byte order of chunks in buffer, where 'size' is size of one chunk.
        */
        static inline void bswapChunks(void * pData, size_t size, size_t count)
        {
            for(size_t c = 0; c < count; ++c)
            {
                char swapByte;
                for(char *p0 = (char*)pData + c * size, *p1 = p0 + size - 1; p0 < p1; ++p0, --p1)
                {
                    swapByte = *p0;
                    *p0 = *p1;
                    *p1 = swapByte;
                }
            }
        }

        /** Returns the most significant bit set in a value.
        */
        static OGRE_FORCE_INLINE unsigned int mostSignificantBitSet(unsigned int value)
        {
            //                                     0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
            static const unsigned char msb[16] = { 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };

            unsigned int result = 0;
            if(value & 0xFFFF0000) { result += 16;value >>= 16; }
            if(value & 0x0000FF00) { result += 8; value >>= 8; }
            if(value & 0x000000F0) { result += 4; value >>= 4; }
            result += msb[value];
            return result-1;
        }
        /** Returns the closest power-of-two number greater or equal to value.
            @note 0 and 1 are powers of two, so 
                firstPO2From(0)==0 and firstPO2From(1)==1.
        */
        static OGRE_FORCE_INLINE uint32 firstPO2From(uint32 n)
        {
            --n;            
            n |= n >> 16;
            n |= n >> 8;
            n |= n >> 4;
            n |= n >> 2;
            n |= n >> 1;
            ++n;
            return n;
        }
        /** Determines whether the number is power-of-two or not.
            @note 0 and 1 are treat as power of two.
        */
        template<typename T>
        static OGRE_FORCE_INLINE bool isPO2(T n)
        {
            return (n & (n-1)) == 0;
        }
        /** Returns the number of bits a pattern must be shifted right by to
            remove right-hand zeros.
        */
        template<typename T>
        static OGRE_FORCE_INLINE unsigned int getBitShift(T mask)
        {
            if (mask == 0)
                return 0;

            unsigned int result = 0;
            while ((mask & 1) == 0) {
                ++result;
                mask >>= 1;
            }
            return result;
        }

        /** Takes a value with a given src bit mask, and produces another
            value with a desired bit mask.

            This routine is useful for colour conversion.
        */
        template<typename SrcT, typename DestT>
        static inline DestT convertBitPattern(SrcT srcValue, SrcT srcBitMask, DestT destBitMask)
        {
            // Mask off irrelevant source value bits (if any)
            srcValue = srcValue & srcBitMask;

            // Shift source down to bottom of DWORD
            const unsigned int srcBitShift = getBitShift(srcBitMask);
            srcValue >>= srcBitShift;

            // Get max value possible in source from srcMask
            const SrcT srcMax = srcBitMask >> srcBitShift;

            // Get max available in dest
            const unsigned int destBitShift = getBitShift(destBitMask);
            const DestT destMax = destBitMask >> destBitShift;

            // Scale source value into destination, and shift back
            DestT destValue = (srcValue * destMax) / srcMax;
            return (destValue << destBitShift);
        }

        /**
         * Convert N bit colour channel value to P bits. It fills P bits with the
         * bit pattern repeated. (this is /((1<<n)-1) in fixed point)
         */
        static inline unsigned int fixedToFixed(uint32 value, unsigned int n, unsigned int p) 
        {
            if(n > p) 
            {
                // Less bits required than available; this is easy
                value >>= n-p;
            } 
            else if(n < p)
            {
                // More bits required than are there, do the fill
                // Use old fashioned division, probably better than a loop
                if(value == 0)
                        value = 0;
                else if(value == (static_cast<unsigned int>(1)<<n)-1)
                        value = (1<<p)-1;
                else    value = value*(1<<p)/((1<<n)-1);
            }
            return value;    
        }

        /**
         * Convert floating point colour channel value between 0.0 and 1.0 (otherwise clamped) 
         * to integer of a certain number of bits. Works for any value of bits between 0 and 31.
         */
        static inline unsigned int floatToFixed(const float value, const unsigned int bits)
        {
            if(value <= 0.0f) return 0;
            else if (value >= 1.0f) return (1<<bits)-1;
            else return (unsigned int)(value * float(1<<bits));
        }

        /**
         * Fixed point to float
         */
        static inline float fixedToFloat(unsigned value, unsigned int bits)
        {
            return (float)value/(float)((1<<bits)-1);
        }

        /**
         * Write a n*8 bits integer value to memory in native endian.
         */
        static inline void intWrite(void *dest, const int n, const unsigned int value)
        {
            switch(n) {
                case 1:
                    ((uint8*)dest)[0] = (uint8)value;
                    break;
                case 2:
                    ((uint16*)dest)[0] = (uint16)value;
                    break;
                case 3:
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG      
                    ((uint8*)dest)[0] = (uint8)((value >> 16) & 0xFF);
                    ((uint8*)dest)[1] = (uint8)((value >> 8) & 0xFF);
                    ((uint8*)dest)[2] = (uint8)(value & 0xFF);
#else
                    ((uint8*)dest)[2] = (uint8)((value >> 16) & 0xFF);
                    ((uint8*)dest)[1] = (uint8)((value >> 8) & 0xFF);
                    ((uint8*)dest)[0] = (uint8)(value & 0xFF);
#endif
                    break;
                case 4:
                    ((uint32*)dest)[0] = (uint32)value;                
                    break;                
            }        
        }
        /**
         * Read a n*8 bits integer value to memory in native endian.
         */
        static inline unsigned int intRead(const void *src, int n) {
            switch(n) {
                case 1:
                    return ((const uint8*)src)[0];
                case 2:
                    return ((const uint16*)src)[0];
                case 3:
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG      
                    return ((uint32)((const uint8*)src)[0]<<16)|
                            ((uint32)((const uint8*)src)[1]<<8)|
                            ((uint32)((const uint8*)src)[2]);
#else
                    return ((uint32)((const uint8*)src)[0])|
                            ((uint32)((const uint8*)src)[1]<<8)|
                            ((uint32)((const uint8*)src)[2]<<16);
#endif
                case 4:
                    return ((const uint32*)src)[0];
            } 
            return 0; // ?
        }

        /** Convert a float32 to a float16 (NV_half_float)
            Courtesy of meshoptimizer
        */
        static inline uint16 floatToHalf(float i)
        {
            union { float f; uint32 i; } v;
            v.f = i;
            return floatToHalfI(v.i);
        }
        /** Converts float in uint32 format to a a half in uint16 format
        */
        static inline uint16 floatToHalfI(uint32 ui)
        {
            int s = (ui >> 16) & 0x8000;
            int em = ui & 0x7fffffff;

            // bias exponent and round to nearest; 112 is relative exponent bias (127-15)
            int h = (em - (112 << 23) + (1 << 12)) >> 13;

            // underflow: flush to zero; 113 encodes exponent -14
            h = (em < (113 << 23)) ? 0 : h;

            // overflow: infinity; 143 encodes exponent 16
            h = (em >= (143 << 23)) ? 0x7c00 : h;

            // NaN; note that we convert all types of NaN to qNaN
            h = (em > (255 << 23)) ? 0x7e00 : h;

            return (unsigned short)(s | h);
        }
        
        /**
         * Convert a float16 (NV_half_float) to a float32
         * Courtesy of meshoptimizer
         */
        static inline float halfToFloat(uint16 y)
        {
            union { float f; uint32 i; } v;
            v.i = halfToFloatI(y);
            return v.f;
        }
        /** Converts a half in uint16 format to a float
            in uint32 format
         */
        static inline uint32 halfToFloatI(uint16 h)
        {
            unsigned int s = unsigned(h & 0x8000) << 16;
            int em = h & 0x7fff;

            // bias exponent and pad mantissa with 0; 112 is relative exponent bias (127-15)
            int r = (em + (112 << 10)) << 13;

            // denormal: flush to zero
            r = (em < (1 << 10)) ? 0 : r;

            // infinity/NaN; note that we preserve NaN payload as a byproduct of unifying inf/nan cases
            // 112 is an exponent bias fixup; since we already applied it once, applying it twice converts 31 to 255
            r += (em >= (31 << 10)) ? (112 << 23) : 0;

            return s | r;
        }
         

    };
    /** @} */
    /** @} */

}

#endif
