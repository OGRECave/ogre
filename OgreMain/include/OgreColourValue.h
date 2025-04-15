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
#ifndef _COLOURVALUE_H__
#define _COLOURVALUE_H__

#include "OgrePrerequisites.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */

    typedef uint32 RGBA;
    typedef uint32 ARGB;
    typedef uint32 ABGR;
    typedef uint32 BGRA;

    /** Class representing colour.

        Colour is represented as 4 components, each of which is a
        floating-point value from 0.0 to 1.0.
        @par
            The 3 'normal' colour components are red, green and blue, a higher
            number indicating greater amounts of that component in the colour.
            The forth component is the 'alpha' value, which represents
            transparency. In this case, 0.0 is completely transparent and 1.0 is
            fully opaque.
    */
    class _OgreExport ColourValue
    {
    public:
        static const ColourValue &ZERO;
        static const ColourValue &Black;
        static const ColourValue &White;
        static const ColourValue &Red;
        static const ColourValue &Green;
        static const ColourValue &Blue;

        explicit constexpr ColourValue( float red = 1.0f,
                    float green = 1.0f,
                    float blue = 1.0f,
                    float alpha = 1.0f ) : r(red), g(green), b(blue), a(alpha)
        { }

        explicit ColourValue(const uchar* byte) : r(byte[0]), g(byte[1]), b(byte[2]), a(byte[3])
        {
            *this /= 255;
        }

        bool operator==(const ColourValue& rhs) const
        {
            return (r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a);
        }
        bool operator!=(const ColourValue& rhs) const { return !(*this == rhs); }

        float r,g,b,a;

        /// @name conversions from/ to native-endian packed formats
        /// @{

        /// value packed as #PF_R8G8B8A8
        RGBA getAsRGBA(void) const;

        /// value packed as #PF_A8R8G8B8
        ARGB getAsARGB(void) const;

        /// value packed as #PF_B8G8R8A8
        BGRA getAsBGRA(void) const;

        /// value packed as #PF_A8B8G8R8
        ABGR getAsABGR(void) const;

        /// value packed as #PF_BYTE_RGBA
        RGBA getAsBYTE() const
        {
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
            return getAsRGBA();
#else
            return getAsABGR();
#endif
        }

        /// Set value from #PF_R8G8B8A8
        void setAsRGBA(RGBA val);

        /// Set value from #PF_A8R8G8B8
        void setAsARGB(ARGB val);

        /// Set value from #PF_B8G8R8A8
        void setAsBGRA(BGRA val);

        /// Set value from #PF_A8B8G8R8
        void setAsABGR(ABGR val);
        /// @}

        /** Clamps colour value to the range [0, 1].
        */
        void saturate(void)
        {
            if (r < 0)
                r = 0;
            else if (r > 1)
                r = 1;

            if (g < 0)
                g = 0;
            else if (g > 1)
                g = 1;

            if (b < 0)
                b = 0;
            else if (b > 1)
                b = 1;

            if (a < 0)
                a = 0;
            else if (a > 1)
                a = 1;
        }

        /** As saturate, except that this colour value is unaffected and
            the saturated colour value is returned as a copy. */
        ColourValue saturateCopy(void) const
        {
            ColourValue ret = *this;
            ret.saturate();
            return ret;
        }

        /// Array accessor operator
        float operator [] ( const size_t i ) const
        {
            assert( i < 4 );

            return *(&r+i);
        }

        /// Array accessor operator
        float& operator [] ( const size_t i )
        {
            assert( i < 4 );

            return *(&r+i);
        }

        /// Pointer accessor for direct copying
        float* ptr()
        {
            return &r;
        }
        /// Pointer accessor for direct copying
        const float* ptr() const
        {
            return &r;
        }

        
        // arithmetic operations
        ColourValue operator + ( const ColourValue& rkVector ) const
        {
            ColourValue kSum;

            kSum.r = r + rkVector.r;
            kSum.g = g + rkVector.g;
            kSum.b = b + rkVector.b;
            kSum.a = a + rkVector.a;

            return kSum;
        }

        ColourValue operator - ( const ColourValue& rkVector ) const
        {
            ColourValue kDiff;

            kDiff.r = r - rkVector.r;
            kDiff.g = g - rkVector.g;
            kDiff.b = b - rkVector.b;
            kDiff.a = a - rkVector.a;

            return kDiff;
        }

        ColourValue operator * (const float fScalar ) const
        {
            ColourValue kProd;

            kProd.r = fScalar*r;
            kProd.g = fScalar*g;
            kProd.b = fScalar*b;
            kProd.a = fScalar*a;

            return kProd;
        }

        ColourValue operator * ( const ColourValue& rhs) const
        {
            ColourValue kProd;

            kProd.r = rhs.r * r;
            kProd.g = rhs.g * g;
            kProd.b = rhs.b * b;
            kProd.a = rhs.a * a;

            return kProd;
        }

        ColourValue operator / ( const ColourValue& rhs) const
        {
            ColourValue kProd;

            kProd.r = r / rhs.r;
            kProd.g = g / rhs.g;
            kProd.b = b / rhs.b;
            kProd.a = a / rhs.a;

            return kProd;
        }

        ColourValue operator / (const float fScalar ) const
        {
            assert( fScalar != 0.0 );

            ColourValue kDiv;

            float fInv = 1.0f / fScalar;
            kDiv.r = r * fInv;
            kDiv.g = g * fInv;
            kDiv.b = b * fInv;
            kDiv.a = a * fInv;

            return kDiv;
        }

        friend ColourValue operator * (const float fScalar, const ColourValue& rkVector )
        {
            ColourValue kProd;

            kProd.r = fScalar * rkVector.r;
            kProd.g = fScalar * rkVector.g;
            kProd.b = fScalar * rkVector.b;
            kProd.a = fScalar * rkVector.a;

            return kProd;
        }

        // arithmetic updates
        ColourValue& operator += ( const ColourValue& rkVector )
        {
            r += rkVector.r;
            g += rkVector.g;
            b += rkVector.b;
            a += rkVector.a;

            return *this;
        }

        ColourValue& operator -= ( const ColourValue& rkVector )
        {
            r -= rkVector.r;
            g -= rkVector.g;
            b -= rkVector.b;
            a -= rkVector.a;

            return *this;
        }

        ColourValue& operator *= (const float fScalar )
        {
            r *= fScalar;
            g *= fScalar;
            b *= fScalar;
            a *= fScalar;
            return *this;
        }

        ColourValue& operator /= (const float fScalar )
        {
            assert( fScalar != 0.0 );

            float fInv = 1.0f / fScalar;

            r *= fInv;
            g *= fInv;
            b *= fInv;
            a *= fInv;

            return *this;
        }

        /** Set a colour value from Hue, Saturation and Brightness.
        @param hue Hue value, scaled to the [0,1] range as opposed to the 0-360
        @param saturation Saturation level, [0,1]
        @param brightness Brightness level, [0,1]
        */
        void setHSB(float hue, float saturation, float brightness);

        /** Convert the current colour to Hue, Saturation and Brightness values. 
        @param hue Output hue value, scaled to the [0,1] range as opposed to the 0-360
        @param saturation Output saturation level, [0,1]
        @param brightness Output brightness level, [0,1]
        */
        void getHSB(float& hue, float& saturation, float& brightness) const;

        /// @deprecated
        OGRE_DEPRECATED void getHSB(float* hue, float* saturation, float* brightness) const
        {
            getHSB(*hue, *saturation, *brightness);
        }

        /** Function for writing to a stream.
        */
        inline friend std::ostream& operator <<
            ( std::ostream& o, const ColourValue& c )
        {
            o << "ColourValue(" << c.r << ", " << c.g << ", " << c.b << ", " << c.a << ")";
            return o;
        }

    };
    /** @} */
    /** @} */

} // namespace

#endif
