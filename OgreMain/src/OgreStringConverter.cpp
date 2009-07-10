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
#include "OgreStableHeaders.h"
#include "OgreStringConverter.h"
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreVector4.h"
#include "OgreMatrix3.h"
#include "OgreMatrix4.h"
#include "OgreQuaternion.h"
#include "OgreColourValue.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    String StringConverter::toString(Real val, unsigned short precision, 
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        stringstream stream;
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
        stringstream stream;
		stream.width(width);
        stream.fill(fill);
        if (flags)
            stream.setf(flags);
        stream << val;
        return stream.str();
    }
    //-----------------------------------------------------------------------
#if OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_64 || OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
    String StringConverter::toString(unsigned int val, 
        unsigned short width, char fill, std::ios::fmtflags flags)
    {
        stringstream stream;
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
        stringstream stream;
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
        stringstream stream;
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
        stringstream stream;
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
        stringstream stream;
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
        stringstream stream;
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
        stringstream stream;
		stream << val.x << " " << val.y;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const Vector3& val)
    {
        stringstream stream;
		stream << val.x << " " << val.y << " " << val.z;
        return stream.str();
    }
	//-----------------------------------------------------------------------
    String StringConverter::toString(const Vector4& val)
    {
        stringstream stream;
		stream << val.x << " " << val.y << " " << val.z << " " << val.w;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const Matrix3& val)
    {
		stringstream stream;
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
		stringstream stream;
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
		stringstream stream;
        stream  << val.w << " " << val.x << " " << val.y << " " << val.z;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const ColourValue& val)
    {
		stringstream stream;
        stream << val.r << " " << val.g << " " << val.b << " " << val.a;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const StringVector& val)
    {
		stringstream stream;
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
    Real StringConverter::parseReal(const String& val)
    {
		// Use istringstream for direct correspondence with toString
		StringStream str(val);
		Real ret = 0;
		str >> ret;

        return ret;
    }
    //-----------------------------------------------------------------------
    int StringConverter::parseInt(const String& val)
    {
		// Use istringstream for direct correspondence with toString
		StringStream str(val);
		int ret = 0;
		str >> ret;

        return ret;
    }
    //-----------------------------------------------------------------------
    unsigned int StringConverter::parseUnsignedInt(const String& val)
    {
		// Use istringstream for direct correspondence with toString
		StringStream str(val);
		unsigned int ret = 0;
		str >> ret;

		return ret;
    }
    //-----------------------------------------------------------------------
    long StringConverter::parseLong(const String& val)
    {
		// Use istringstream for direct correspondence with toString
		StringStream str(val);
		long ret = 0;
		str >> ret;

		return ret;
    }
    //-----------------------------------------------------------------------
    unsigned long StringConverter::parseUnsignedLong(const String& val)
    {
		// Use istringstream for direct correspondence with toString
		StringStream str(val);
		unsigned long ret = 0;
		str >> ret;

		return ret;
    }
    //-----------------------------------------------------------------------
    bool StringConverter::parseBool(const String& val)
    {
		return (StringUtil::startsWith(val, "true") || StringUtil::startsWith(val, "yes")
			|| StringUtil::startsWith(val, "1"));
    }
    //-----------------------------------------------------------------------
    Vector2 StringConverter::parseVector2(const String& val)
    {
        // Split on space
        vector<String>::type vec = StringUtil::split(val);

        if (vec.size() != 2)
        {
            return Vector2::ZERO;
        }
        else
        {
            return Vector2(parseReal(vec[0]),parseReal(vec[1]));
        }
    }
	//-----------------------------------------------------------------------
    Vector3 StringConverter::parseVector3(const String& val)
    {
        // Split on space
        vector<String>::type vec = StringUtil::split(val);

        if (vec.size() != 3)
        {
            return Vector3::ZERO;
        }
        else
        {
            return Vector3(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]));
        }
    }
	//-----------------------------------------------------------------------
    Vector4 StringConverter::parseVector4(const String& val)
    {
        // Split on space
        vector<String>::type vec = StringUtil::split(val);

        if (vec.size() != 4)
        {
            return Vector4::ZERO;
        }
        else
        {
            return Vector4(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]),parseReal(vec[3]));
        }
    }
    //-----------------------------------------------------------------------
    Matrix3 StringConverter::parseMatrix3(const String& val)
    {
        // Split on space
        vector<String>::type vec = StringUtil::split(val);

        if (vec.size() != 9)
        {
            return Matrix3::IDENTITY;
        }
        else
        {
            return Matrix3(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]),
                parseReal(vec[3]),parseReal(vec[4]),parseReal(vec[5]),
                parseReal(vec[6]),parseReal(vec[7]),parseReal(vec[8]));
        }
    }
    //-----------------------------------------------------------------------
    Matrix4 StringConverter::parseMatrix4(const String& val)
    {
        // Split on space
        vector<String>::type vec = StringUtil::split(val);

        if (vec.size() != 16)
        {
            return Matrix4::IDENTITY;
        }
        else
        {
            return Matrix4(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]), parseReal(vec[3]),
                parseReal(vec[4]),parseReal(vec[5]), parseReal(vec[6]), parseReal(vec[7]),
                parseReal(vec[8]),parseReal(vec[9]), parseReal(vec[10]), parseReal(vec[11]),
                parseReal(vec[12]),parseReal(vec[13]), parseReal(vec[14]), parseReal(vec[15]));
        }
    }
    //-----------------------------------------------------------------------
    Quaternion StringConverter::parseQuaternion(const String& val)
    {
        // Split on space
        vector<String>::type vec = StringUtil::split(val);

        if (vec.size() != 4)
        {
            return Quaternion::IDENTITY;
        }
        else
        {
            return Quaternion(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]), parseReal(vec[3]));
        }
    }
    //-----------------------------------------------------------------------
    ColourValue StringConverter::parseColourValue(const String& val)
    {
        // Split on space
        vector<String>::type vec = StringUtil::split(val);

        if (vec.size() == 4)
        {
            return ColourValue(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]), parseReal(vec[3]));
        }
        else if (vec.size() == 3)
        {
            return ColourValue(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]), 1.0f);
        }
        else
        {
            return ColourValue::Black;
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
		float tst;
		str >> tst;
		return !str.fail() && str.eof();
	}
}


