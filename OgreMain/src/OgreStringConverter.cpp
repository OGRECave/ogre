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

// A quick define to overcome different names for the same function
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#   define strtod_l _strtod_l
#   define strtoul_l _strtoul_l
#   define strtol_l _strtol_l
#   define strtoull_l _strtoull_l
#   define strtoll_l _strtoll_l
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN || \
	(OGRE_PLATFORM == OGRE_PLATFORM_LINUX && OGRE_NO_LOCALE_STRCONVERT == 1)
#   define strtod_l(ptr, end, l) strtod(ptr, end)
#   define strtoul_l(ptr, end, base, l) strtoul(ptr, end, base)
#   define strtol_l(ptr, end, base, l) strtol(ptr, end, base)
#   define strtoull_l(ptr, end, base, l) strtoull(ptr, end, base)
#   define strtoll_l(ptr, end, base, l) strtoll(ptr, end, base)
#endif

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT) && !defined(__MINGW32__)
#   define LC_NUMERIC_MASK LC_NUMERIC
#   define newlocale(cat, loc, base) _create_locale(cat, loc)
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN || defined(__MINGW32__)
#   define newlocale(cat, loc, base) 0
#endif

#ifdef __MINGW32__
#define _strtoull_l _strtoul_l
#define _strtoll_l _strtol_l
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
    String StringConverter::toString(int32 val,
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        return _toString(val, width, fill, flags);
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(uint32 val,
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        return _toString(val, width, fill, flags);
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(unsigned long val,
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        return _toString(val, width, fill, flags);
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(unsigned long long val,
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
    template <typename T> static bool assignValid(bool valid, const T& val, T& ret)
    {
        if (valid)
            ret = val;
        return valid;
    }

    bool StringConverter::parse(const String& val, float& ret)
    {
        char* end;
        auto tmp = (float)strtod_l(val.c_str(), &end, _numLocale);
        return assignValid(val.c_str() != end, tmp, ret);
    }
    bool StringConverter::parse(const String& val, double& ret)
    {
        char* end;
        auto tmp = strtod_l(val.c_str(), &end, _numLocale);
        return assignValid(val.c_str() != end, tmp, ret);
    }
    //-----------------------------------------------------------------------
    bool StringConverter::parse(const String& val, int32& ret)
    {
        char* end;
        auto tmp = (int32)strtol_l(val.c_str(), &end, 0, _numLocale);
        return assignValid(val.c_str() != end, tmp, ret);
    }
    //-----------------------------------------------------------------------
    bool StringConverter::parse(const String& val, int64& ret)
    {
        char* end;
        int64 tmp = strtoll_l(val.c_str(), &end, 0, _numLocale);
        return assignValid(val.c_str() != end, tmp, ret);
    }
    //-----------------------------------------------------------------------
    bool StringConverter::parse(const String& val, unsigned long& ret)
    {
        char* end;
        unsigned long tmp = strtoull_l(val.c_str(), &end, 0, _numLocale);
        return assignValid(val.c_str() != end, tmp, ret);
    }
    bool StringConverter::parse(const String& val, unsigned long long& ret)
    {
        char* end;
        unsigned long long tmp = strtoull_l(val.c_str(), &end, 0, _numLocale);
        return assignValid(val.c_str() != end, tmp, ret);
    }
    //-----------------------------------------------------------------------
    bool StringConverter::parse(const String& val, uint32& ret)
    {
        char* end;
        auto tmp = (uint32)strtoul_l(val.c_str(), &end, 0, _numLocale);
        return assignValid(val.c_str() != end, tmp, ret);
    }
    bool StringConverter::parse(const String& val, bool& ret)
    {
        //FIXME Returns both parsed value and error in same value - ambiguous.
        // Suggested alternatives: implement exception handling or make either
        // error or parsed value a parameter.
        if ((StringUtil::startsWith(val, "true") || StringUtil::startsWith(val, "yes")
             || StringUtil::startsWith(val, "1") ||  StringUtil::startsWith(val, "on")))
            ret = true;
        else if ((StringUtil::startsWith(val, "false") || StringUtil::startsWith(val, "no")
                  || StringUtil::startsWith(val, "0") ||  StringUtil::startsWith(val, "off")))
            ret = false;
        else
            return false;

        return true;
    }

    template<typename T>
    static bool parseReals(const String& val, T* dst, size_t n)
    {
        // Split on space
        std::vector<String> vec = StringUtil::split(val);
        if(vec.size() != n)
            return false;

        bool ret = true;
        for(size_t i = 0; i < n; i++)
            ret &= StringConverter::parse(vec[i], dst[i]);
        return ret;
    }

    //-----------------------------------------------------------------------
    bool StringConverter::parse(const String& val, Vector2& ret)
    {
        return parseReals(val, ret.ptr(), 2);
    }
    //-----------------------------------------------------------------------
    bool StringConverter::parse(const String& val, Vector3& ret)
    {
        return parseReals(val, ret.ptr(), 3);
    }
    //-----------------------------------------------------------------------
    bool StringConverter::parse(const String& val, Vector4& ret)
    {
        return parseReals(val, ret.ptr(), 4);
    }
    //-----------------------------------------------------------------------
    bool StringConverter::parse(const String& val, Matrix3& ret)
    {
        return parseReals(val, &ret[0][0], 9);
    }
    //-----------------------------------------------------------------------
    bool StringConverter::parse(const String& val, Matrix4& ret)
    {
        return parseReals(val, &ret[0][0], 16);
    }
    //-----------------------------------------------------------------------
    bool StringConverter::parse(const String& val, Quaternion& ret)
    {
        return parseReals(val, ret.ptr(), 4);
    }
    //-----------------------------------------------------------------------
    bool StringConverter::parse(const String& val, ColourValue& ret)
    {
        // Split on space
        std::vector<String> vec = StringUtil::split(val);

        if (vec.size() == 4)
        {
            return parse(vec[0], ret.r) && parse(vec[1], ret.g) && parse(vec[2], ret.b) &&
                   parse(vec[3], ret.a);
        }
        else if (vec.size() == 3)
        {
            ret.a = 1.0f;
            return parse(vec[0], ret.r) && parse(vec[1], ret.g) && parse(vec[2], ret.b);
        }
        else
        {
            return false;
        }
    }
    //-----------------------------------------------------------------------
    bool StringConverter::isNumber(const String& val)
    {
        char* end;
        strtod(val.c_str(), &end);
        return end == (val.c_str() + val.size());
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


