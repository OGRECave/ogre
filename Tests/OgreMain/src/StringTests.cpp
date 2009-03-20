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
#include "StringTests.h"
#include "OgreStringConverter.h"
#include "OgreVector3.h"
#include "OgreQuaternion.h"
#include "OgreMatrix4.h"
#include "OgreColourValue.h"

using namespace Ogre;

// Regsiter the suite
CPPUNIT_TEST_SUITE_REGISTRATION( StringTests );

void StringTests::setUp()
{
	testFileNoPath = "testfile.txt";
	testFileRelativePathUnix = "this/is/relative/testfile.txt";
	testFileRelativePathWindows = "this\\is\\relative\\testfile.txt";
	testFileAbsolutePathUnix = "/this/is/absolute/testfile.txt";
	testFileAbsolutePathWindows = "c:\\this\\is\\absolute\\testfile.txt";
}
void StringTests::tearDown()
{
}

void StringTests::testSplitFileNameNoPath()
{
	String basename, path;
	StringUtil::splitFilename(testFileNoPath, basename, path);

    CPPUNIT_ASSERT_EQUAL(testFileNoPath, basename);
    CPPUNIT_ASSERT(path.empty());
}
void StringTests::testSplitFileNameRelativePath()
{
	String basename, path;
	// Unix
	StringUtil::splitFilename(testFileRelativePathUnix, basename, path);
    CPPUNIT_ASSERT_EQUAL(String("testfile.txt"), basename);
    CPPUNIT_ASSERT_EQUAL(String("this/is/relative/"), path);
	// Windows
	StringUtil::splitFilename(testFileRelativePathWindows, basename, path);
    CPPUNIT_ASSERT_EQUAL(String("testfile.txt"), basename);
    CPPUNIT_ASSERT_EQUAL(String("this/is/relative/"), path);

}
void StringTests::testSplitFileNameAbsolutePath()
{
	String basename, path;
	// Unix
	StringUtil::splitFilename(testFileAbsolutePathUnix, basename, path);
    CPPUNIT_ASSERT_EQUAL(String("testfile.txt"), basename);
    CPPUNIT_ASSERT_EQUAL(String("/this/is/absolute/"), path);
	// Windows
	StringUtil::splitFilename(testFileAbsolutePathWindows, basename, path);
    CPPUNIT_ASSERT_EQUAL(String("testfile.txt"), basename);
	CPPUNIT_ASSERT_EQUAL(String("c:/this/is/absolute/"), path);
}

void StringTests::testMatchCaseSensitive()
{
	// Test positive
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, testFileNoPath, true));
	// Test negative
	String upperCase = testFileNoPath;
    StringUtil::toUpperCase(upperCase);
	CPPUNIT_ASSERT(!StringUtil::match(testFileNoPath, upperCase, true));
}
void StringTests::testMatchCaseInSensitive()
{
	// Test positive
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, testFileNoPath, false));
	// Test positive
	String upperCase = testFileNoPath;
	StringUtil::toUpperCase(upperCase);
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, upperCase, false));
}
void StringTests::testMatchGlobAll()
{
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "*", true));
}
void StringTests::testMatchGlobStart()
{
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "*stfile.txt", true));
	CPPUNIT_ASSERT(!StringUtil::match(testFileNoPath, "*astfile.txt", true));
}
void StringTests::testMatchGlobEnd()
{
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "testfile.*", true));
	CPPUNIT_ASSERT(!StringUtil::match(testFileNoPath, "testfile.d*", true));
}
void StringTests::testMatchGlobStartAndEnd()
{
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "*stfile.*", true));
	CPPUNIT_ASSERT(!StringUtil::match(testFileNoPath, "*astfile.d*", true));
}
void StringTests::testMatchGlobMiddle()
{
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "test*.txt", true));
	CPPUNIT_ASSERT(!StringUtil::match(testFileNoPath, "last*.txt*", true));
}
void StringTests::testMatchSuperGlobtastic()
{
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "*e*tf*e.t*t", true));
}
void StringTests::testParseReal()
{
	Real r = 23.454;

	String s = StringConverter::toString(r);
	Real t = StringConverter::parseReal(s);

	CPPUNIT_ASSERT_EQUAL(r, t);


}
void StringTests::testParseInt()
{

	int r = 223546;

	String s = StringConverter::toString(r);
	int t = StringConverter::parseInt(s);

	CPPUNIT_ASSERT_EQUAL(r, t);
}
void StringTests::testParseLong()
{
	long r = -2147483647;

	String s = StringConverter::toString(r);
	long t = StringConverter::parseLong(s);

	CPPUNIT_ASSERT_EQUAL(r, t);

}
void StringTests::testParseUnsignedLong()
{
	unsigned long r = 4294967295UL;

	String s = StringConverter::toString(r);
	unsigned long t = StringConverter::parseUnsignedLong(s);

	CPPUNIT_ASSERT_EQUAL(r, t);

}
void StringTests::testParseVector3()
{
	Vector3 r(0.12, 3.22, -4.04);

	String s = StringConverter::toString(r);
	Vector3 t = StringConverter::parseVector3(s);

	CPPUNIT_ASSERT_EQUAL(r, t);

}
void StringTests::testParseMatrix4()
{
	Matrix4 r(1.12, 0, 0, 34, 0, 0.87, 0, 20, 0, 0, 0.56, 10, 0, 0, 0, 1);

	String s = StringConverter::toString(r);
	Matrix4 t = StringConverter::parseMatrix4(s);

	CPPUNIT_ASSERT_EQUAL(r, t);

}
void StringTests::testParseQuaternion()
{
	Quaternion r(1.12, 0.87, 0.67, 1);

	String s = StringConverter::toString(r);
	Quaternion t = StringConverter::parseQuaternion(s);

	CPPUNIT_ASSERT_EQUAL(r, t);

}
void StringTests::testParseBool()
{
	bool r = true;
	String s = StringConverter::toString(r);
	bool t = StringConverter::parseBool(s);
	CPPUNIT_ASSERT_EQUAL(r, t);

	r = false;
	s = StringConverter::toString(r);
	t = StringConverter::parseBool(s);
	CPPUNIT_ASSERT_EQUAL(r, t);

}
void StringTests::testParseColourValue()
{
	ColourValue r(0.34, 0.44, 0.77, 1.0);

	String s = StringConverter::toString(r);
	ColourValue t = StringConverter::parseColourValue(s);
	CPPUNIT_ASSERT_EQUAL(r, t);

}
