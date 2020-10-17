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
#include "OgreStableHeaders.h"

namespace Ogre {

    const ColourValue ColourValue::ZERO = ColourValue(0.0,0.0,0.0,0.0);
    const ColourValue ColourValue::Black = ColourValue(0.0,0.0,0.0);
    const ColourValue ColourValue::White = ColourValue(1.0,1.0,1.0);
    const ColourValue ColourValue::Red = ColourValue(1.0,0.0,0.0);
    const ColourValue ColourValue::Green = ColourValue(0.0,1.0,0.0);
    const ColourValue ColourValue::Blue = ColourValue(0.0,0.0,1.0);

    //---------------------------------------------------------------------
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
    ABGR ColourValue::getAsABGR(void) const
#else
    RGBA ColourValue::getAsRGBA(void) const
#endif
    {
        uint8 val8;
        uint32 val32 = 0;

        // Convert to 32bit pattern
        // (RGBA = 8888)

        // Red
        val8 = static_cast<uint8>(r * 255);
        val32 = val8 << 24;

        // Green
        val8 = static_cast<uint8>(g * 255);
        val32 += val8 << 16;

        // Blue
        val8 = static_cast<uint8>(b * 255);
        val32 += val8 << 8;

        // Alpha
        val8 = static_cast<uint8>(a * 255);
        val32 += val8;

        return val32;
    }
    //---------------------------------------------------------------------
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
    BGRA ColourValue::getAsBGRA(void) const
#else
    ARGB ColourValue::getAsARGB(void) const
#endif
    {
        uint8 val8;
        uint32 val32 = 0;

        // Convert to 32bit pattern
        // (ARGB = 8888)

        // Alpha
        val8 = static_cast<uint8>(a * 255);
        val32 = val8 << 24;

        // Red
        val8 = static_cast<uint8>(r * 255);
        val32 += val8 << 16;

        // Green
        val8 = static_cast<uint8>(g * 255);
        val32 += val8 << 8;

        // Blue
        val8 = static_cast<uint8>(b * 255);
        val32 += val8;


        return val32;
    }
    //---------------------------------------------------------------------
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
    ARGB ColourValue::getAsARGB(void) const
#else
    BGRA ColourValue::getAsBGRA(void) const
#endif
    {
        uint8 val8;
        uint32 val32 = 0;

        // Convert to 32bit pattern
        // (ARGB = 8888)

        // Blue
        val8 = static_cast<uint8>(b * 255);
        val32 = val8 << 24;

        // Green
        val8 = static_cast<uint8>(g * 255);
        val32 += val8 << 16;

        // Red
        val8 = static_cast<uint8>(r * 255);
        val32 += val8 << 8;

        // Alpha
        val8 = static_cast<uint8>(a * 255);
        val32 += val8;


        return val32;
    }
    //---------------------------------------------------------------------
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
    RGBA ColourValue::getAsRGBA(void) const
#else
    ABGR ColourValue::getAsABGR(void) const
#endif
    {
        uint8 val8;
        uint32 val32 = 0;

        // Convert to 32bit pattern
        // (ABRG = 8888)

        // Alpha
        val8 = static_cast<uint8>(a * 255);
        val32 = val8 << 24;

        // Blue
        val8 = static_cast<uint8>(b * 255);
        val32 += val8 << 16;

        // Green
        val8 = static_cast<uint8>(g * 255);
        val32 += val8 << 8;

        // Red
        val8 = static_cast<uint8>(r * 255);
        val32 += val8;


        return val32;
    }
    //---------------------------------------------------------------------
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
    void ColourValue::setAsABGR(ABGR val32)
#else
    void ColourValue::setAsRGBA(RGBA val32)
#endif
    {
        // Convert from 32bit pattern
        // (RGBA = 8888)

        // Red
        r = float((val32 >> 24) & 0xFF) / 255.0f;

        // Green
        g = float((val32 >> 16) & 0xFF) / 255.0f;

        // Blue
        b = float((val32 >> 8) & 0xFF) / 255.0f;

        // Alpha
        a = float(val32 & 0xFF) / 255.0f;
    }
    //---------------------------------------------------------------------
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
    void ColourValue::setAsBGRA(BGRA val32)
#else
    void ColourValue::setAsARGB(ARGB val32)
#endif
    {
        // Convert from 32bit pattern
        // (ARGB = 8888)

        // Alpha
        a = float((val32 >> 24) & 0xFF) / 255.0f;

        // Red
        r = float((val32 >> 16) & 0xFF) / 255.0f;

        // Green
        g = float((val32 >> 8) & 0xFF) / 255.0f;

        // Blue
        b = float(val32 & 0xFF) / 255.0f;
    }
    //---------------------------------------------------------------------
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
    void ColourValue::setAsARGB(ARGB val32)
#else
    void ColourValue::setAsBGRA(BGRA val32)
#endif
    {
        // Convert from 32bit pattern
        // (ARGB = 8888)

        // Blue
        b = float((val32 >> 24) & 0xFF) / 255.0f;

        // Green
        g = float((val32 >> 16) & 0xFF) / 255.0f;

        // Red
        r = float((val32 >> 8) & 0xFF) / 255.0f;

        // Alpha
        a = float(val32 & 0xFF) / 255.0f;
    }
    //---------------------------------------------------------------------
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
    void ColourValue::setAsRGBA(RGBA val32)
#else
    void ColourValue::setAsABGR(ABGR val32)
#endif
    {
        // Convert from 32bit pattern
        // (ABGR = 8888)

        // Alpha
        a = float((val32 >> 24) & 0xFF) / 255.0f;

        // Blue
        b = float((val32 >> 16) & 0xFF) / 255.0f;

        // Green
        g = float((val32 >> 8) & 0xFF) / 255.0f;

        // Red
        r = float(val32 & 0xFF) / 255.0f;
    }
    //---------------------------------------------------------------------
    void ColourValue::setHSB(float hue, float saturation, float brightness)
    {
        // wrap hue
        hue = std::fmod(hue, 1.0f);

        // clamp saturation / brightness
        saturation = Math::saturate(saturation);
        brightness = Math::saturate(brightness);

        if (brightness == 0.0f)
        {   
            // early exit, this has to be black
            r = g = b = 0.0f;
            return;
        }

        if (saturation == 0.0f)
        {   
            // early exit, this has to be grey

            r = g = b = brightness;
            return;
        }


        float hueDomain  = hue * 6.0f;
        if (hueDomain >= 6.0f)
        {
            // wrap around, and allow mathematical errors
            hueDomain = 0.0f;
        }
        unsigned short domain = (unsigned short)hueDomain;
        float f1 = brightness * (1 - saturation);
        float f2 = brightness * (1 - saturation * (hueDomain - domain));
        float f3 = brightness * (1 - saturation * (1 - (hueDomain - domain)));

        switch (domain)
        {
        case 0:
            // red domain; green ascends
            r = brightness;
            g = f3;
            b = f1;
            break;
        case 1:
            // yellow domain; red descends
            r = f2;
            g = brightness;
            b = f1;
            break;
        case 2:
            // green domain; blue ascends
            r = f1;
            g = brightness;
            b = f3;
            break;
        case 3:
            // cyan domain; green descends
            r = f1;
            g = f2;
            b = brightness;
            break;
        case 4:
            // blue domain; red ascends
            r = f3;
            g = f1;
            b = brightness;
            break;
        case 5:
            // magenta domain; blue descends
            r = brightness;
            g = f1;
            b = f2;
            break;
        }


    }
    //---------------------------------------------------------------------
    void ColourValue::getHSB(float& hue, float& saturation, float& brightness) const
    {

        float vMin = std::min(r, std::min(g, b));
        float vMax = std::max(r, std::max(g, b));
        float delta = vMax - vMin;

        brightness = vMax;

        if (Math::RealEqual(delta, 0.0f, 1e-6f))
        {
            // grey
            hue = 0;
            saturation = 0;
        }
        else                                    
        {
            // a colour
            saturation = delta / vMax;

            float deltaR = (((vMax - r) / 6.0f) + (delta / 2.0f)) / delta;
            float deltaG = (((vMax - g) / 6.0f) + (delta / 2.0f)) / delta;
            float deltaB = (((vMax - b) / 6.0f) + (delta / 2.0f)) / delta;

            if (Math::RealEqual(r, vMax))
                hue = deltaB - deltaG;
            else if (Math::RealEqual(g, vMax))
                hue = 0.3333333f + deltaR - deltaB;
            else if (Math::RealEqual(b, vMax)) 
                hue = 0.6666667f + deltaG - deltaR;

            if (hue < 0.0f)
                hue += 1.0f;
            if (hue > 1.0f)
                hue -= 1.0f;
        }

        
    }

}

