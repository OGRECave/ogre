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

#ifndef __StringConverter_H__
#define __StringConverter_H__

#include "OgreCommon.h"
#include "OgrePrerequisites.h"
#include "OgreStringVector.h"
#include "OgreColourValue.h"
#include "OgreMatrix4.h"
#include "OgreVector.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#   define locale_t _locale_t
#elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
#   define locale_t int
#endif

// If compiling with make on macOS, these headers need to be included to get
// definitions of locale_t, strtod_l, etc...
// See: http://www.unix.com/man-page/osx/3/strtod_l/
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include <stdlib.h>
#   include <xlocale.h>
#endif

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */
    /** Class for converting the core Ogre data types to/from Strings.

        The code for converting values to and from strings is here as a separate
        class to avoid coupling String to other datatypes (and vice-versa) which reduces
        compilation dependency: important given how often the core types are used.
    @par
        This class is mainly used for parsing settings in text files. External applications
        can also use it to interface with classes which use the StringInterface template
        class.
    @par
        The String formats of each of the major types is listed with the methods. The basic types
        like int and Real just use the underlying C runtime library atof and atoi family methods,
        however custom types like Vector3, ColourValue and Matrix4 are also supported by this class
        using custom formats.
    */
    class _OgreExport StringConverter
    {
    public:
        static String toString(int32 val) { return std::to_string(val); };
        static String toString(uint32 val) { return std::to_string(val); };
        static String toString(unsigned long val) { return std::to_string(val); };
        static String toString(unsigned long long val) { return std::to_string(val); };
        static String toString(long val) { return std::to_string(val); };

        /** Converts a float to a String. */
        static String toString(float val, unsigned short precision = 6,
                               unsigned short width = 0, char fill = ' ',
                               std::ios::fmtflags flags = std::ios::fmtflags(0));

        /** Converts a double to a String. */
        static String toString(double val, unsigned short precision = 6,
                               unsigned short width = 0, char fill = ' ',
                               std::ios::fmtflags flags = std::ios::fmtflags(0));

        /** Converts a Radian to a String. */
        static String toString(Radian val, unsigned short precision = 6, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0))
        {
            return toString(val.valueAngleUnits(), precision, width, fill, flags);
        }
        /** Converts a Degree to a String. */
        static String toString(Degree val, unsigned short precision = 6, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0))
        {
            return toString(val.valueAngleUnits(), precision, width, fill, flags);
        }
        /// @deprecated use StringUtil::format
        OGRE_DEPRECATED static String toString(int32 val, unsigned short width,
            char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0));
        /// @deprecated use StringUtil::format
        OGRE_DEPRECATED static String toString(uint32 val, unsigned short width,
            char fill = ' ',
            std::ios::fmtflags flags = std::ios::fmtflags(0));
        // provide both long long and long to catch size_t on all platforms
        /// @deprecated use StringUtil::format
        OGRE_DEPRECATED static String toString(unsigned long val,
            unsigned short width, char fill = ' ',
            std::ios::fmtflags flags = std::ios::fmtflags(0));
        /// @deprecated use StringUtil::format
        OGRE_DEPRECATED static String toString(unsigned long long val,
            unsigned short width, char fill = ' ',
            std::ios::fmtflags flags = std::ios::fmtflags(0));

        /// @deprecated use StringUtil::format
        OGRE_DEPRECATED static String toString(long val,
            unsigned short width, char fill = ' ',
            std::ios::fmtflags flags = std::ios::fmtflags(0));

        /** Converts a boolean to a String.
        @param val
        @param yesNo If set to true, result is 'yes' or 'no' instead of 'true' or 'false'
        */
        static String toString(bool val, bool yesNo = false);
        /** Converts a Vector2 to a String. 

            Format is "x y" (i.e. 2x Real values, space delimited)
        */
        static String toString(const Vector2& val);
        /** Converts a Vector3 to a String. 

            Format is "x y z" (i.e. 3x Real values, space delimited)
        */
        static String toString(const Vector3& val);
        /** Converts a Vector4 to a String. 

            Format is "x y z w" (i.e. 4x Real values, space delimited)
        */
        static String toString(const Vector4& val);
        /** Converts a Matrix3 to a String. 

            Format is "00 01 02 10 11 12 20 21 22" where '01' means row 0 column 1 etc.
        */
        static String toString(const Matrix3& val);
        /** Converts a Matrix4 to a String. 

            Format is "00 01 02 03 10 11 12 13 20 21 22 23 30 31 32 33" where 
            '01' means row 0 column 1 etc.
        */
        static String toString(const Matrix4& val);
        /** Converts a Quaternion to a String. 

            Format is "w x y z" (i.e. 4x Real values, space delimited)
        */
        static String toString(const Quaternion& val);
        /** Converts a ColourValue to a String. 

            Format is "r g b a" (i.e. 4x Real values, space delimited). 
        */
        static String toString(const ColourValue& val);
        /** Converts a StringVector to a string.

            Strings must not contain spaces since space is used as a delimiter in
            the output.
        */
        static String toString(const StringVector& val);

        /** Converts a String to a basic value type
            @return whether the conversion was successful
        */
        static bool parse(const String& str, ColourValue& v);
        static bool parse(const String& str, Quaternion& v);
        static bool parse(const String& str, Matrix4& v);
        static bool parse(const String& str, Matrix3& v);
        static bool parse(const String& str, Vector4& v);
        static bool parse(const String& str, Vector3& v);
        static bool parse(const String& str, Vector2& v);
        static bool parse(const String& str, int32& v);
        static bool parse(const String& str, uint32& v);
        static bool parse(const String& str, int64& v);
        // provide both long long and long to catch size_t on all platforms
        static bool parse(const String& str, unsigned long& v);
        static bool parse(const String& str, unsigned long long& v);
        static bool parse(const String& str, bool& v);
        static bool parse(const String& str, double& v);
        static bool parse(const String& str, float& v);

        /** Converts a String to a Real. 
        @return
            0.0 if the value could not be parsed, otherwise the Real version of the String.
        */
        static Real parseReal(const String& val, Real defaultValue = 0)
        {
            Real ret;
            return parse(val, ret) ? ret : defaultValue;
        }
        /** Converts a String to a Angle. 
        @return
            0.0 if the value could not be parsed, otherwise the Angle version of the String.
        */
        static Radian parseAngle(const String& val, Radian defaultValue = Radian(0)) {
            return Angle(parseReal(val, defaultValue.valueRadians()));
        }
        /** Converts a String to a whole number. 
        @return
            0.0 if the value could not be parsed, otherwise the numeric version of the String.
        */
        static int32 parseInt(const String& val, int32 defaultValue = 0)
        {
            int32 ret;
            return parse(val, ret) ? ret : defaultValue;
        }
        /** Converts a String to a whole number. 
        @return
            0.0 if the value could not be parsed, otherwise the numeric version of the String.
        */
        static uint32 parseUnsignedInt(const String& val, uint32 defaultValue = 0)
        {
            uint32 ret;
            return parse(val, ret) ? ret : defaultValue;
        }
        /// @deprecated
        OGRE_DEPRECATED static int64 parseLong(const String& val, int64 defaultValue = 0)
        {
            int64 ret;
            return parse(val, ret) ? ret : defaultValue;
        }
        /// @deprecated
        OGRE_DEPRECATED static uint64 parseUnsignedLong(const String& val, uint64 defaultValue = 0)
        {
            uint64 ret;
            return parse(val, ret) ? ret : defaultValue;
        }
        /** Converts a String to size_t. 
        @return
            defaultValue if the value could not be parsed, otherwise the numeric version of the String.
        */
        static size_t parseSizeT(const String& val, size_t defaultValue = 0)
        {
            size_t ret;
            return parse(val, ret) ? ret : defaultValue;
        }
        /** Converts a String to a boolean. 

            Returns true if case-insensitive match of the start of the string
            matches "true", "yes", "1", or "on", false if "false", "no", "0" 
            or "off".
        */
        static bool parseBool(const String& val, bool defaultValue = 0)
        {
            bool ret;
            return parse(val, ret) ? ret : defaultValue;
        }
        /** Parses a Vector2 out of a String.

            Format is "x y" ie. 2 Real components, space delimited. Failure to parse returns
            Vector2::ZERO.
        */
        static Vector2 parseVector2(const String& val, const Vector2& defaultValue = Vector2::ZERO)
        {
            Vector2 ret;
            return parse(val, ret) ? ret : defaultValue;
        }
        /** Parses a Vector3 out of a String.

            Format is "x y z" ie. 3 Real components, space delimited. Failure to parse returns
            Vector3::ZERO.
        */
        static Vector3 parseVector3(const String& val, const Vector3& defaultValue = Vector3::ZERO)
        {
            Vector3 ret;
            return parse(val, ret) ? ret : defaultValue;
        }
        /** Parses a Vector4 out of a String.

            Format is "x y z w" ie. 4 Real components, space delimited. Failure to parse returns
            Vector4::ZERO.
        */
        static Vector4 parseVector4(const String& val, const Vector4& defaultValue = Vector4::ZERO)
        {
            Vector4 ret;
            return parse(val, ret) ? ret : defaultValue;
        }
        /** Parses a Matrix3 out of a String.

            Format is "00 01 02 10 11 12 20 21 22" where '01' means row 0 column 1 etc.
            Failure to parse returns Matrix3::IDENTITY.
        */
        static Matrix3 parseMatrix3(const String& val, const Matrix3& defaultValue = Matrix3::IDENTITY)
        {
            Matrix3 ret;
            return parse(val, ret) ? ret : defaultValue;
        }
        /** Parses a Matrix4 out of a String.

            Format is "00 01 02 03 10 11 12 13 20 21 22 23 30 31 32 33" where 
            '01' means row 0 column 1 etc. Failure to parse returns Matrix4::IDENTITY.
        */
        static Matrix4 parseMatrix4(const String& val, const Matrix4& defaultValue = Matrix4::IDENTITY)
        {
            Matrix4 ret;
            return parse(val, ret) ? ret : defaultValue;
        }
        /** Parses a Quaternion out of a String. 

            Format is "w x y z" (i.e. 4x Real values, space delimited). 
            Failure to parse returns Quaternion::IDENTITY.
        */
        static Quaternion parseQuaternion(const String& val, const Quaternion& defaultValue = Quaternion::IDENTITY)
        {
            Quaternion ret;
            return parse(val, ret) ? ret : defaultValue;
        }
        /** Parses a ColourValue out of a String. 

            Format is "r g b a" (i.e. 4x Real values, space delimited), or "r g b" which implies
            an alpha value of 1.0 (opaque). Failure to parse returns ColourValue::Black.
        */
        static ColourValue parseColourValue(const String& val, const ColourValue& defaultValue = ColourValue::Black)
        {
            ColourValue ret;
            return parse(val, ret) ? ret : defaultValue;
        }

        /// @deprecated use StringUtil::split
        OGRE_DEPRECATED static StringVector parseStringVector(const String& val) { return StringUtil::split(val); }
        /// @deprecated use @ref parse()
        OGRE_DEPRECATED static bool isNumber(const String& val);

		static locale_t _numLocale;
    private:
        template<typename T>
        static String _toString(T val, uint16 width, char fill, std::ios::fmtflags flags);
    };

    inline String to_string(const Quaternion& v) { return StringConverter::toString(v); }
    inline String to_string(const ColourValue& v) { return StringConverter::toString(v); }
    inline String to_string(const Vector2& v) { return StringConverter::toString(v); }
    inline String to_string(const Vector3& v) { return StringConverter::toString(v); }
    inline String to_string(const Vector4& v) { return StringConverter::toString(v); }
    inline String to_string(const Matrix3& v) { return StringConverter::toString(v); }
    inline String to_string(const Matrix4& v) { return StringConverter::toString(v); }
    /** @} */
    /** @} */
}



#endif

