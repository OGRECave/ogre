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

#ifndef __StringConverter_H__
#define __StringConverter_H__

#include "OgrePrerequisites.h"
#include "OgreMath.h"
#include "OgreString.h"
#include "OgreStringVector.h"

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
            std::ios::fmtflags flags = std::ios::fmtflags(0) );
        /** Converts a Radian to a String. */
        static String toString(Radian val, unsigned short precision = 6, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0) )
        {
            return toString(val.valueAngleUnits(), precision, width, fill, flags);
        }
        /** Converts a Degree to a String. */
        static String toString(Degree val, unsigned short precision = 6, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0) )
        {
            return toString(val.valueAngleUnits(), precision, width, fill, flags);
        }
        /** Converts an int to a String. */
        static String toString(int val, unsigned short width = 0, 
            char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0) );
#if OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_64 || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        /** Converts an unsigned int to a String. */
        static String toString(unsigned int val, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0) );
        /** Converts a size_t to a String. */
        static String toString(size_t val, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0) );
        #if OGRE_COMPILER == OGRE_COMPILER_MSVC
                /** Converts an unsigned long to a String. */
                static String toString(unsigned long val, 
                    unsigned short width = 0, char fill = ' ', 
                    std::ios::fmtflags flags = std::ios::fmtflags(0) );

        #endif
#else
        /** Converts a size_t to a String. */
        static String toString(size_t val, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0) );
        /** Converts an unsigned long to a String. */
        static String toString(unsigned long val, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0) );
#endif
        /** Converts a long to a String. */
        static String toString(long val, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0) );
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
        @returns
            0.0 if the value could not be parsed, otherwise the Real version of the String.
        */
        static Real parseReal(const String& val);
        /** Converts a String to a Angle. 
        @returns
            0.0 if the value could not be parsed, otherwise the Angle version of the String.
        */
        static inline Radian parseAngle(const String& val) {
			return Angle(parseReal(val));
		}
        /** Converts a String to a whole number. 
        @returns
            0.0 if the value could not be parsed, otherwise the numeric version of the String.
        */
        static int parseInt(const String& val);
        /** Converts a String to a whole number. 
        @returns
            0.0 if the value could not be parsed, otherwise the numeric version of the String.
        */
        static unsigned int parseUnsignedInt(const String& val);
        /** Converts a String to a whole number. 
        @returns
            0.0 if the value could not be parsed, otherwise the numeric version of the String.
        */
        static long parseLong(const String& val);
        /** Converts a String to a whole number. 
        @returns
            0.0 if the value could not be parsed, otherwise the numeric version of the String.
        */
        static unsigned long parseUnsignedLong(const String& val);
        /** Converts a String to a boolean. 
        @remarks
            Returns true if case-insensitive match of the start of the string
			matches "true", "yes" or "1", false otherwise.
        */
        static bool parseBool(const String& val);
		/** Parses a Vector2 out of a String.
        @remarks
            Format is "x y" ie. 2 Real components, space delimited. Failure to parse returns
            Vector2::ZERO.
        */
        static Vector2 parseVector2(const String& val);
		/** Parses a Vector3 out of a String.
        @remarks
            Format is "x y z" ie. 3 Real components, space delimited. Failure to parse returns
            Vector3::ZERO.
        */
        static Vector3 parseVector3(const String& val);
        /** Parses a Vector4 out of a String.
        @remarks
            Format is "x y z w" ie. 4 Real components, space delimited. Failure to parse returns
            Vector4::ZERO.
        */
        static Vector4 parseVector4(const String& val);
        /** Parses a Matrix3 out of a String.
        @remarks
            Format is "00 01 02 10 11 12 20 21 22" where '01' means row 0 column 1 etc.
            Failure to parse returns Matrix3::IDENTITY.
        */
        static Matrix3 parseMatrix3(const String& val);
        /** Parses a Matrix4 out of a String.
        @remarks
            Format is "00 01 02 03 10 11 12 13 20 21 22 23 30 31 32 33" where 
            '01' means row 0 column 1 etc. Failure to parse returns Matrix4::IDENTITY.
        */
        static Matrix4 parseMatrix4(const String& val);
        /** Parses a Quaternion out of a String. 
        @remarks
            Format is "w x y z" (i.e. 4x Real values, space delimited). 
            Failure to parse returns Quaternion::IDENTITY.

        */
        static Quaternion parseQuaternion(const String& val);
        /** Parses a ColourValue out of a String. 
        @remarks
            Format is "r g b a" (i.e. 4x Real values, space delimited), or "r g b" which implies
            an alpha value of 1.0 (opaque). Failure to parse returns ColourValue::Black.
        */
        static ColourValue parseColourValue(const String& val);

        /** Pareses a StringVector from a string.
        @remarks
            Strings must not contain spaces since space is used as a delimiter in
            the output.
        */
        static StringVector parseStringVector(const String& val);
        /** Checks the String is a valid number value. */
        static bool isNumber(const String& val);
    };

	/** @} */
	/** @} */

}



#endif

