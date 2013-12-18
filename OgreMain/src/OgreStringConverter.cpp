/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#include "OgreStringConverter.h"

namespace Ogre {

    String StringConverter::msDefaultStringLocale = OGRE_DEFAULT_LOCALE;
    std::locale StringConverter::msLocale = std::locale(msDefaultStringLocale.c_str());
    bool StringConverter::msUseLocale = false;

    //-----------------------------------------------------------------------
    String StringConverter::toString(Real val, unsigned short precision, 
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        StringStream stream;
        if (msUseLocale)
            stream.imbue(msLocale);
        stream.precision(precision);
        stream.width(width);
        stream.fill(fill);
        if (flags)
            stream.setf(flags);
        stream << val;

        return stream.str();
    }
#if OGRE_DOUBLE_PRECISION == 1
    //-----------------------------------------------------------------------
    String StringConverter::toString(float val, unsigned short precision,
                                     unsigned short width, char fill, std::ios::fmtflags flags)
    {
        StringStream stream;
        if (msUseLocale)
            stream.imbue(msLocale);
        stream.precision(precision);
        stream.width(width);
        stream.fill(fill);
        if (flags)
            stream.setf(flags);
        stream << val;
        return stream.str();
    }
#else
    //-----------------------------------------------------------------------
    String StringConverter::toString(double val, unsigned short precision,
                                     unsigned short width, char fill, std::ios::fmtflags flags)
    {
        StringStream stream;
        if (msUseLocale)
            stream.imbue(msLocale);
        stream.precision(precision);
        stream.width(width);
        stream.fill(fill);
        if (flags)
            stream.setf(flags);
        stream << val;
        return stream.str();
    }
#endif
    //-----------------------------------------------------------------------
    String StringConverter::toString(int val, 
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        StringStream stream;
        if (msUseLocale)
            stream.imbue(msLocale);
        stream.width(width);
        stream.fill(fill);
        if (flags)
            stream.setf(flags);
        stream << val;
        return stream.str();
    }
    //-----------------------------------------------------------------------
#if OGRE_PLATFORM != OGRE_PLATFORM_NACL &&  ( OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_64 || OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS )
    String StringConverter::toString(unsigned int val, 
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        StringStream stream;
        if (msUseLocale)
            stream.imbue(msLocale);
        stream.width(width);
        stream.fill(fill);
        if (flags)
            stream.setf(flags);
        stream << val;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(size_t val, 
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        StringStream stream;
        if (msUseLocale)
            stream.imbue(msLocale);
        stream.width(width);
        stream.fill(fill);
        if (flags)
            stream.setf(flags);
        stream << val;
        return stream.str();
    }
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
    //-----------------------------------------------------------------------
    String StringConverter::toString(unsigned long val, 
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        StringStream stream;
        if (msUseLocale)
            stream.imbue(msLocale);
        stream.width(width);
        stream.fill(fill);
        if (flags)
            stream.setf(flags);
        stream << val;
        return stream.str();
    }

#endif
    //-----------------------------------------------------------------------
#else
    String StringConverter::toString(size_t val, 
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        StringStream stream;
        if (msUseLocale)
            stream.imbue(msLocale);
        stream.width(width);
        stream.fill(fill);
        if (flags)
            stream.setf(flags);
        stream << val;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(unsigned long val, 
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        StringStream stream;
        if (msUseLocale)
            stream.imbue(msLocale);
        stream.width(width);
        stream.fill(fill);
        if (flags)
            stream.setf(flags);
        stream << val;
        return stream.str();
    }
    //-----------------------------------------------------------------------
#endif
    String StringConverter::toString(long val, 
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        StringStream stream;
        if (msUseLocale)
            stream.imbue(msLocale);
        stream.width(width);
        stream.fill(fill);
        if (flags)
            stream.setf(flags);
        stream << val;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const Vector2& val)
    {
        StringStream stream;
        if (msUseLocale)
            stream.imbue(msLocale);
        stream << val.x << " " << val.y;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const Vector3& val)
    {
        StringStream stream;
        if (msUseLocale)
            stream.imbue(msLocale);
        stream << val.x << " " << val.y << " " << val.z;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const Vector4& val)
    {
        StringStream stream;
        if (msUseLocale)
            stream.imbue(msLocale);
        stream << val.x << " " << val.y << " " << val.z << " " << val.w;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const Matrix3& val)
    {
        StringStream stream;
        stream.imbue(msLocale);
        stream << val[0][0] << " "
            << val[0][1] << " "             
            << val[0][2] << " "             
            << val[1][0] << " "             
            << val[1][1] << " "             
            << val[1][2] << " "             
            << val[2][0] << " "             
            << val[2][1] << " "             
            << val[2][2];
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(bool val, bool yesNo)
    {
        if (val)
        {
            if (yesNo)
            {
                return "yes";
            }
            else
            {
                return "true";
            }
        }
        else
            if (yesNo)
            {
                return "no";
            }
            else
            {
                return "false";
            }
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const Matrix4& val)
    {
        StringStream stream;
        stream.imbue(msLocale);
        stream << val[0][0] << " "
            << val[0][1] << " "             
            << val[0][2] << " "             
            << val[0][3] << " "             
            << val[1][0] << " "             
            << val[1][1] << " "             
            << val[1][2] << " "             
            << val[1][3] << " "             
            << val[2][0] << " "             
            << val[2][1] << " "             
            << val[2][2] << " "             
            << val[2][3] << " "             
            << val[3][0] << " "             
            << val[3][1] << " "             
            << val[3][2] << " "             
            << val[3][3];
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const Quaternion& val)
    {
        StringStream stream;
        if (msUseLocale)
            stream.imbue(msLocale);
        stream  << val.w << " " << val.x << " " << val.y << " " << val.z;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const ColourValue& val)
    {
        StringStream stream;
        if (msUseLocale)
            stream.imbue(msLocale);
        stream << val.r << " " << val.g << " " << val.b << " " << val.a;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const StringVector& val)
    {
        StringStream stream;
        if (msUseLocale)
            stream.imbue(msLocale);
        StringVector::const_iterator i, iend, ibegin;
        ibegin = val.begin();
        iend = val.end();
        for (i = ibegin; i != iend; ++i)
        {
            if (i != ibegin)
                stream << " ";

            stream << *i; 
        }
        return stream.str();
    }
    //-----------------------------------------------------------------------
    Real StringConverter::parseReal(const String& val, Real defaultValue)
    {
        // Use iStringStream for direct correspondence with toString
        StringStream str(val);
        if (msUseLocale)
            str.imbue(msLocale);
        Real ret = defaultValue;
        if( !(str >> ret) )
            return defaultValue;

        return ret;
    }
    //-----------------------------------------------------------------------
    int StringConverter::parseInt(const String& val, int defaultValue)
    {
        // Use iStringStream for direct correspondence with toString
        StringStream str(val);
        if (msUseLocale)
            str.imbue(msLocale);
        int ret = defaultValue;
        if( !(str >> ret) )
            return defaultValue;

        return ret;
    }
    //-----------------------------------------------------------------------
    unsigned int StringConverter::parseUnsignedInt(const String& val, unsigned int defaultValue)
    {
        // Use iStringStream for direct correspondence with toString
        StringStream str(val);
        if (msUseLocale)
            str.imbue(msLocale);
        unsigned int ret = defaultValue;
        if( !(str >> ret) )
            return defaultValue;

        return ret;
    }
    //-----------------------------------------------------------------------
    long StringConverter::parseLong(const String& val, long defaultValue)
    {
        // Use iStringStream for direct correspondence with toString
        StringStream str(val);
        if (msUseLocale)
            str.imbue(msLocale);
        long ret = defaultValue;
        if( !(str >> ret) )
            return defaultValue;

        return ret;
    }
    //-----------------------------------------------------------------------
    unsigned long StringConverter::parseUnsignedLong(const String& val, unsigned long defaultValue)
    {
        // Use iStringStream for direct correspondence with toString
        StringStream str(val);
        if (msUseLocale)
            str.imbue(msLocale);
        unsigned long ret = defaultValue;
        if( !(str >> ret) )
            return defaultValue;

        return ret;
    }
    //-----------------------------------------------------------------------
    size_t StringConverter::parseSizeT(const String& val, size_t defaultValue)
    {
        // Use iStringStream for direct correspondence with toString
        StringStream str(val);
        if (msUseLocale)
            str.imbue(msLocale);
        size_t ret = defaultValue;
        if( !(str >> ret) )
            return defaultValue;

        return ret;
    }
    //-----------------------------------------------------------------------
    bool StringConverter::parseBool(const String& val, bool defaultValue)
    {
        //FIXME Returns both parsed value and error in same value - ambiguous.
        // Suggested alternatives: implement exception handling or make either
        // error or parsed value a parameter.
        if ((StringUtil::startsWith(val, "true") || StringUtil::startsWith(val, "yes")
             || StringUtil::startsWith(val, "1") ||  StringUtil::startsWith(val, "on")))
            return true;
        else if ((StringUtil::startsWith(val, "false") || StringUtil::startsWith(val, "no")
                  || StringUtil::startsWith(val, "0") ||  StringUtil::startsWith(val, "off")))
            return false;
        else
            return defaultValue;
    }
    //-----------------------------------------------------------------------
    Vector2 StringConverter::parseVector2(const String& val, const Vector2& defaultValue)
    {
        // Split on space
        vector<String>::type vec = StringUtil::split(val);

        if (vec.size() != 2)
        {
            return defaultValue;
        }
        else
        {
            return Vector2(parseReal(vec[0], defaultValue[0]), parseReal(vec[1], defaultValue[1]));
        }
    }
    //-----------------------------------------------------------------------
    Vector3 StringConverter::parseVector3(const String& val, const Vector3& defaultValue)
    {
        // Split on space
        vector<String>::type vec = StringUtil::split(val);

        if (vec.size() != 3)
        {
            return defaultValue;
        }
        else
        {
            return Vector3(parseReal(vec[0], defaultValue[0]),
                           parseReal(vec[1], defaultValue[1]),
                           parseReal(vec[2], defaultValue[2]));
        }
    }
    //-----------------------------------------------------------------------
    Vector4 StringConverter::parseVector4(const String& val, const Vector4& defaultValue)
    {
        // Split on space
        vector<String>::type vec = StringUtil::split(val);

        if (vec.size() != 4)
        {
            return defaultValue;
        }
        else
        {
            return Vector4(parseReal(vec[0], defaultValue[0]),
                           parseReal(vec[1], defaultValue[1]),
                           parseReal(vec[2], defaultValue[2]),
                           parseReal(vec[3], defaultValue[3]));
        }
    }
    //-----------------------------------------------------------------------
    Matrix3 StringConverter::parseMatrix3(const String& val, const Matrix3& defaultValue)
    {
        // Split on space
        vector<String>::type vec = StringUtil::split(val);

        if (vec.size() != 9)
        {
            return defaultValue;
        }
        else
        {
            return Matrix3(parseReal(vec[0], defaultValue[0][0]),
                           parseReal(vec[1], defaultValue[0][1]),
                           parseReal(vec[2], defaultValue[0][2]),

                           parseReal(vec[3], defaultValue[1][0]),
                           parseReal(vec[4], defaultValue[1][1]),
                           parseReal(vec[5], defaultValue[1][2]),

                           parseReal(vec[6], defaultValue[2][0]),
                           parseReal(vec[7], defaultValue[2][1]),
                           parseReal(vec[8], defaultValue[2][2]));
        }
    }
    //-----------------------------------------------------------------------
    Matrix4 StringConverter::parseMatrix4(const String& val, const Matrix4& defaultValue)
    {
        // Split on space
        vector<String>::type vec = StringUtil::split(val);

        if (vec.size() != 16)
        {
            return defaultValue;
        }
        else
        {
            return Matrix4(parseReal(vec[0], defaultValue[0][0]),
                           parseReal(vec[1], defaultValue[0][1]),
                           parseReal(vec[2], defaultValue[0][2]),
                           parseReal(vec[3], defaultValue[0][3]),
                           
                           parseReal(vec[4], defaultValue[1][0]),
                           parseReal(vec[5], defaultValue[1][1]),
                           parseReal(vec[6], defaultValue[1][2]),
                           parseReal(vec[7], defaultValue[1][3]),
                           
                           parseReal(vec[8], defaultValue[2][0]),
                           parseReal(vec[9], defaultValue[2][1]),
                           parseReal(vec[10], defaultValue[2][2]),
                           parseReal(vec[11], defaultValue[2][3]),
                           
                           parseReal(vec[12], defaultValue[3][0]),
                           parseReal(vec[13], defaultValue[3][1]),
                           parseReal(vec[14], defaultValue[3][2]),
                           parseReal(vec[15], defaultValue[3][3]));
        }
    }
    //-----------------------------------------------------------------------
    Quaternion StringConverter::parseQuaternion(const String& val, const Quaternion& defaultValue)
    {
        // Split on space
        vector<String>::type vec = StringUtil::split(val);

        if (vec.size() != 4)
        {
            return defaultValue;
        }
        else
        {
            return Quaternion(parseReal(vec[0], defaultValue[0]),
                              parseReal(vec[1], defaultValue[1]),
                              parseReal(vec[2], defaultValue[2]),
                              parseReal(vec[3], defaultValue[3]));
        }
    }
    //-----------------------------------------------------------------------
    ColourValue StringConverter::parseColourValue(const String& val, const ColourValue& defaultValue)
    {
        // Split on space
        vector<String>::type vec = StringUtil::split(val);

        if (vec.size() == 4)
        {
            return ColourValue(parseReal(vec[0], defaultValue[0]),
                               parseReal(vec[1], defaultValue[1]),
                               parseReal(vec[2], defaultValue[2]),
                               parseReal(vec[3], defaultValue[3]));
        }
        else if (vec.size() == 3)
        {
            return ColourValue(parseReal(vec[0], defaultValue[0]),
                               parseReal(vec[1], defaultValue[1]),
                               parseReal(vec[2], defaultValue[2]),
                               1.0f);
        }
        else
        {
            return defaultValue;
        }
    }
    //-----------------------------------------------------------------------
    StringVector StringConverter::parseStringVector(const String& val)
    {
        return StringUtil::split(val);
    }
    //-----------------------------------------------------------------------
    bool StringConverter::isNumber(const String& val)
    {
        StringStream str(val);
        if (msUseLocale)
            str.imbue(msLocale);
        float tst;
        str >> tst;
        return !str.fail() && str.eof();
    }
}


