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
#include "OgreVector2.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */
    /** Class for converting the core Ogre data types to/from Strings.
    @remarks
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
    @author
        Steve Streeting
    */
    class _OgreExport StringConverter
    {
    public:

        /** Converts a Real to a String. */
        static String toString(Real val, unsigned short precision = 6, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0));

// This counter-intuitive guard is correct. In case of enabled double precision
// the toString() version above using Ogre::Real already provides a double precision
// version and hence we need to explicitly declare a float version as well.
#if OGRE_DOUBLE_PRECISION == 1
        /** Converts a float to a String. */
        static String toString(float val, unsigned short precision = 6,
                               unsigned short width = 0, char fill = ' ',
                               std::ios::fmtflags flags = std::ios::fmtflags(0));
#else
        /** Converts a double to a String. */
        static String toString(double val, unsigned short precision = 6,
                               unsigned short width = 0, char fill = ' ',
                               std::ios::fmtflags flags = std::ios::fmtflags(0));
#endif
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
        /** Converts an int to a String. */
        static String toString(int val, unsigned short width = 0, 
            char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0));
#if OGRE_PLATFORM != OGRE_PLATFORM_NACL &&  ( OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_64 || OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS )
        /** Converts an unsigned int to a String. */
        static String toString(unsigned int val, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0));
        /** Converts a size_t to a String. */
        static String toString(size_t val, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0));
        #if OGRE_COMPILER == OGRE_COMPILER_MSVC
        /** Converts an unsigned long to a String. */
        static String toString(unsigned long val, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0));
        #endif
#else
        /** Converts a size_t to a String. */
        static String toString(size_t val, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0));
        /** Converts an unsigned long to a String. */
        static String toString(unsigned long val, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0));
#endif
        /** Converts a long to a String. */
        static String toString(long val, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0));
        /** Converts a boolean to a String. 
        @param yesNo If set to true, result is 'yes' or 'no' instead of 'true' or 'false'
        */
        static String toString(bool val, bool yesNo = false);
        /** Converts a Vector2 to a String. 
        @remarks
            Format is "x y" (i.e. 2x Real values, space delimited)
        */
        static String toString(const Vector2& val);
        /** Converts a Vector3 to a String. 
        @remarks
            Format is "x y z" (i.e. 3x Real values, space delimited)
        */
        static String toString(const Vector3& val);
        /** Converts a Vector4 to a String. 
        @remarks
            Format is "x y z w" (i.e. 4x Real values, space delimited)
        */
        static String toString(const Vector4& val);
        /** Converts a Matrix3 to a String. 
        @remarks
            Format is "00 01 02 10 11 12 20 21 22" where '01' means row 0 column 1 etc.
        */
        static String toString(const Matrix3& val);
        /** Converts a Matrix4 to a String. 
        @remarks
            Format is "00 01 02 03 10 11 12 13 20 21 22 23 30 31 32 33" where 
            '01' means row 0 column 1 etc.
        */
        static String toString(const Matrix4& val);
        /** Converts a Quaternion to a String. 
        @remarks
            Format is "w x y z" (i.e. 4x Real values, space delimited)
        */
        static String toString(const Quaternion& val);
        /** Converts a ColourValue to a String. 
        @remarks
            Format is "r g b a" (i.e. 4x Real values, space delimited). 
        */
        static String toString(const ColourValue& val);
        /** Converts a StringVector to a string.
        @remarks
            Strings must not contain spaces since space is used as a delimiter in
            the output.
        */
        static String toString(const StringVector& val);

        /** Converts a String to a Real. 
        @return
            0.0 if the value could not be parsed, otherwise the Real version of the String.
        */
        static Real parseReal(const String& val, Real defaultValue = 0);
        /** Converts a String to a Angle. 
        @return
            0.0 if the value could not be parsed, otherwise the Angle version of the String.
        */
        static inline Radian parseAngle(const String& val, Radian defaultValue = Radian(0)) {
            return Angle(parseReal(val, defaultValue.valueRadians()));
        }
        /** Converts a String to a whole number. 
        @return
            0.0 if the value could not be parsed, otherwise the numeric version of the String.
        */
        static int parseInt(const String& val, int defaultValue = 0);
        /** Converts a String to a whole number. 
        @return
            0.0 if the value could not be parsed, otherwise the numeric version of the String.
        */
        static unsigned int parseUnsignedInt(const String& val, unsigned int defaultValue = 0);
        /** Converts a String to a whole number. 
        @return
            0.0 if the value could not be parsed, otherwise the numeric version of the String.
        */
        static long parseLong(const String& val, long defaultValue = 0);
        /** Converts a String to a whole number. 
        @return
            0.0 if the value could not be parsed, otherwise the numeric version of the String.
        */
        static unsigned long parseUnsignedLong(const String& val, unsigned long defaultValue = 0);
        /** Converts a String to size_t. 
        @return
            defaultValue if the value could not be parsed, otherwise the numeric version of the String.
        */
        static size_t parseSizeT(const String& val, size_t defaultValue = 0);
        /** Converts a String to a boolean. 
        @remarks
            Returns true if case-insensitive match of the start of the string
            matches "true", "yes", "1", or "on", false if "false", "no", "0" 
            or "off".
        */
        static bool parseBool(const String& val, bool defaultValue = 0, bool* error = NULL);
        /** Parses a Vector2 out of a String.
        @remarks
            Format is "x y" ie. 2 Real components, space delimited. Failure to parse returns
            Vector2::ZERO.
        */
        static Vector2 parseVector2(const String& val, const Vector2& defaultValue = Vector2::ZERO);
        /** Parses a Vector3 out of a String.
        @remarks
            Format is "x y z" ie. 3 Real components, space delimited. Failure to parse returns
            Vector3::ZERO.
        */
        static Vector3 parseVector3(const String& val, const Vector3& defaultValue = Vector3::ZERO);
        /** Parses a Vector4 out of a String.
        @remarks
            Format is "x y z w" ie. 4 Real components, space delimited. Failure to parse returns
            Vector4::ZERO.
        */
        static Vector4 parseVector4(const String& val, const Vector4& defaultValue = Vector4::ZERO);
        /** Parses a Matrix3 out of a String.
        @remarks
            Format is "00 01 02 10 11 12 20 21 22" where '01' means row 0 column 1 etc.
            Failure to parse returns Matrix3::IDENTITY.
        */
        static Matrix3 parseMatrix3(const String& val, const Matrix3& defaultValue = Matrix3::IDENTITY);
        /** Parses a Matrix4 out of a String.
        @remarks
            Format is "00 01 02 03 10 11 12 13 20 21 22 23 30 31 32 33" where 
            '01' means row 0 column 1 etc. Failure to parse returns Matrix4::IDENTITY.
        */
        static Matrix4 parseMatrix4(const String& val, const Matrix4& defaultValue = Matrix4::IDENTITY);
        /** Parses a Quaternion out of a String. 
        @remarks
            Format is "w x y z" (i.e. 4x Real values, space delimited). 
            Failure to parse returns Quaternion::IDENTITY.
        */
        static Quaternion parseQuaternion(const String& val, const Quaternion& defaultValue = Quaternion::IDENTITY);
        /** Parses a ColourValue out of a String. 
        @remarks
            Format is "r g b a" (i.e. 4x Real values, space delimited), or "r g b" which implies
            an alpha value of 1.0 (opaque). Failure to parse returns ColourValue::Black.
        */
        static ColourValue parseColourValue(const String& val, const ColourValue& defaultValue = ColourValue::Black);

        /** Parses a StringVector from a string.
        @remarks
            Strings must not contain spaces since space is used as a delimiter in
            the output.
        */
        static StringVector parseStringVector(const String& val);
        /** Checks the String is a valid number value. */
        static bool isNumber(const String& val);

		/** Converts a ColourBufferType to a String.
		@remarks
			String output format is "Back", "Back Left", "Back Right", etc.
		*/
		static String toString(ColourBufferType val);

		/** Converts a String to a ColourBufferType.
		@remarks
			String input format should be "Back", "Back Left", "Back Right", etc.
		*/
		static ColourBufferType parseColourBuffer(const String& val, ColourBufferType defaultValue = CBT_BACK);

		/** Converts a StereoModeType to a String
		@remarks
			String output format is "None", "Frame Sequential", etc.
		*/
		static String toString(StereoModeType val);

		/** Converts a String to a StereoModeType
		@remarks
			String input format should be "None", "Frame Sequential", etc.
		*/
		static StereoModeType parseStereoMode(const String& val, StereoModeType defaultValue = SMT_NONE);
		
        //-----------------------------------------------------------------------
        static void setDefaultStringLocale(const String &loc)
        {
            msDefaultStringLocale = loc;
            msLocale = std::locale(msDefaultStringLocale.c_str());
        }
        //-----------------------------------------------------------------------
        static String getDefaultStringLocale(void) { return msDefaultStringLocale; }
        //-----------------------------------------------------------------------
        static void setUseLocale(bool useLocale) { msUseLocale = useLocale; }
        //-----------------------------------------------------------------------
        static bool isUseLocale() { return msUseLocale; }
        //-----------------------------------------------------------------------

    protected:
        static String msDefaultStringLocale;
        static std::locale msLocale;
        static bool msUseLocale;		
    };

    /** @} */
    /** @} */

}



#endif

