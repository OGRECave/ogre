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
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgrePlatform.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#   define LC_NUMERIC_MASK LC_NUMERIC
#   define newlocale(cat, loc, base) _create_locale(cat, loc)
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
#   define newlocale(cat, loc, base) 0
#endif

namespace Ogre {
    locale_t StringConverter::_numLocale = newlocale(LC_NUMERIC_MASK, OGRE_DEFAULT_LOCALE, NULL);

    template<typename T>
    String StringConverter::_toString(T val, uint16 width, char fill, std::ios::fmtflags flags)
    {
        StringStream stream;
        stream.width(width);
        stream.fill(fill);
        if (flags & std::ios::basefield) {
            stream.setf(flags, std::ios::basefield);
            stream.setf((flags & ~std::ios::basefield) | std::ios::showbase);
        }
        else if (flags)
            stream.setf(flags);

        stream << val;

        return stream.str();
    }

    //-----------------------------------------------------------------------
    String StringConverter::toString(float val, unsigned short precision,
                                     unsigned short width, char fill, std::ios::fmtflags flags)
    {
        StringStream stream;
        stream.precision(precision);
        stream.width(width);
        stream.fill(fill);
        if (flags)
            stream.setf(flags);
        stream << val;
        return stream.str();
    }

    //-----------------------------------------------------------------------
    String StringConverter::toString(double val, unsigned short precision,
                                     unsigned short width, char fill, std::ios::fmtflags flags)
    {
        StringStream stream;
        stream.precision(precision);
        stream.width(width);
        stream.fill(fill);
        if (flags)
            stream.setf(flags);
        stream << val;
        return stream.str();
    }

    //-----------------------------------------------------------------------
    String StringConverter::toString(int val,
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        return _toString(val, width, fill, flags);
    }
#if OGRE_PLATFORM != OGRE_PLATFORM_NACL &&  ( OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_64 || OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS )
    //-----------------------------------------------------------------------
    String StringConverter::toString(unsigned int val,
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        return _toString(val, width, fill, flags);
    }
#if OGRE_COMPILER == OGRE_COMPILER_MSVC || defined(__MINGW32__)
    //-----------------------------------------------------------------------
    String StringConverter::toString(unsigned long val,
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        return _toString(val, width, fill, flags);
    }
#endif
#else
    //-----------------------------------------------------------------------
    String StringConverter::toString(unsigned long val,
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        return _toString(val, width, fill, flags);
    }
#endif
    //-----------------------------------------------------------------------
    String StringConverter::toString(size_t val,
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        return _toString(val, width, fill, flags);
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(long val,
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        return _toString(val, width, fill, flags);
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const Vector2& val)
    {
        StringStream stream;
        stream << val.x << " " << val.y;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const Vector3& val)
    {
        StringStream stream;
        stream << val.x << " " << val.y << " " << val.z;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const Vector4& val)
    {
        StringStream stream;
        stream << val.x << " " << val.y << " " << val.z << " " << val.w;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const Matrix3& val)
    {
        StringStream stream;
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
        stream  << val.w << " " << val.x << " " << val.y << " " << val.z;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const ColourValue& val)
    {
        StringStream stream;
        stream << val.r << " " << val.g << " " << val.b << " " << val.a;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const StringVector& val)
    {
        StringStream stream;
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
        char* end;
        Real ret = (Real)strtod_l(val.c_str(), &end, _numLocale);
        return val.c_str() == end ? defaultValue : ret;
    }
    //-----------------------------------------------------------------------
    int StringConverter::parseInt(const String& val, int defaultValue)
    {
        char* end;
        int ret = (int)strtoul_l(val.c_str(), &end, 0, _numLocale);
        return val.c_str() == end ? defaultValue : ret;
    }
    //-----------------------------------------------------------------------
    unsigned int StringConverter::parseUnsignedInt(const String& val, unsigned int defaultValue)
    {
        char* end;
        unsigned int ret = (unsigned int)strtoul_l(val.c_str(), &end, 0, _numLocale);
        return val.c_str() == end ? defaultValue : ret;
    }
    //-----------------------------------------------------------------------
    long StringConverter::parseLong(const String& val, long defaultValue)
    {
        char* end;
        long ret = strtol_l(val.c_str(), &end, 0, _numLocale);
        return val.c_str() == end ? defaultValue : ret;
    }
    //-----------------------------------------------------------------------
    unsigned long StringConverter::parseUnsignedLong(const String& val, unsigned long defaultValue)
    {
        char* end;
        unsigned long ret = strtoul_l(val.c_str(), &end, 0, _numLocale);
        return val.c_str() == end ? defaultValue : ret;
    }
    //-----------------------------------------------------------------------
    size_t StringConverter::parseSizeT(const String& val, size_t defaultValue)
    {
        size_t ret;
        return sscanf(val.c_str(),
#if OGRE_COMPILER == OGRE_COMPILER_MSVC && OGRE_COMP_VER < 1900
                "%Iu"
#else
                "%zu"
#endif
                , &ret) == 1 ? ret : defaultValue;
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
        char* end;
        strtod(val.c_str(), &end);
        return end == (val.c_str() + val.size());
    }
	//-----------------------------------------------------------------------
    String StringConverter::toString(ColourBufferType val)
    {
		StringStream stream;
		switch (val)
		{
		case CBT_BACK:
		  stream << "Back";
		  break;
		case CBT_BACK_LEFT:
		  stream << "Back Left";
		  break;
		case CBT_BACK_RIGHT:
		  stream << "Back Right";
		  break;
		default:
		  OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Unsupported colour buffer value", "StringConverter::toString(const ColourBufferType& val)");
		}

		return stream.str();
    }
    //-----------------------------------------------------------------------
    ColourBufferType StringConverter::parseColourBuffer(const String& val, ColourBufferType defaultValue)
    {
		ColourBufferType result = defaultValue;
		if (val.compare("Back") == 0)
		{
			result = CBT_BACK;
		}
		else if (val.compare("Back Left") == 0)
		{
			result = CBT_BACK_LEFT;
		}
		else if (val.compare("Back Right") == 0)
		{
			result = CBT_BACK_RIGHT;
		}		
		
		return result;
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(StereoModeType val)
    {
		StringStream stream;
		switch (val)
		{
		case SMT_NONE:
		  stream << "None";
		  break;
		case SMT_FRAME_SEQUENTIAL:
		  stream << "Frame Sequential";
		  break;
		default:
		  OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Unsupported stereo mode value", "StringConverter::toString(const StereoModeType& val)");
		}

		return stream.str();
    }
    //-----------------------------------------------------------------------
    StereoModeType StringConverter::parseStereoMode(const String& val, StereoModeType defaultValue)
    {
		StereoModeType result = defaultValue;
		if (val.compare("None") == 0)
		{
			result = SMT_NONE;
		}
		else if (val.compare("Frame Sequential") == 0)
		{
			result = SMT_FRAME_SEQUENTIAL;
		}
		
		return result;
    }
	//-----------------------------------------------------------------------
}


