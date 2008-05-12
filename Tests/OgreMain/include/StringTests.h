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
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "OgreString.h"

class StringTests : public CppUnit::TestFixture
{
    // CppUnit macros for setting up the test suite
    CPPUNIT_TEST_SUITE( StringTests );
    CPPUNIT_TEST(testSplitFileNameNoPath);
    CPPUNIT_TEST(testSplitFileNameRelativePath);
    CPPUNIT_TEST(testSplitFileNameAbsolutePath);
    CPPUNIT_TEST(testMatchCaseSensitive);
    CPPUNIT_TEST(testMatchCaseInSensitive);
	CPPUNIT_TEST(testMatchGlobAll);
    CPPUNIT_TEST(testMatchGlobStart);
    CPPUNIT_TEST(testMatchGlobEnd);
    CPPUNIT_TEST(testMatchGlobStartAndEnd);
    CPPUNIT_TEST(testMatchGlobMiddle);
    CPPUNIT_TEST(testMatchSuperGlobtastic);
	CPPUNIT_TEST(testParseReal);
	CPPUNIT_TEST(testParseInt);
	CPPUNIT_TEST(testParseLong);
	CPPUNIT_TEST(testParseUnsignedLong);
	CPPUNIT_TEST(testParseVector3);
	CPPUNIT_TEST(testParseMatrix4);
	CPPUNIT_TEST(testParseQuaternion);
	CPPUNIT_TEST(testParseBool);
	CPPUNIT_TEST(testParseColourValue);

    CPPUNIT_TEST_SUITE_END();
protected:
	Ogre::String testFileNoPath;
	Ogre::String testFileRelativePathWindows;
	Ogre::String testFileRelativePathUnix;
	Ogre::String testFileAbsolutePathWindows;
	Ogre::String testFileAbsolutePathUnix;
public:
    void setUp();
    void tearDown();
	// StringUtil::splitFileName tests
    void testSplitFileNameNoPath();
    void testSplitFileNameRelativePath();
    void testSplitFileNameAbsolutePath();
	// StringUtil::match tests
    void testMatchCaseSensitive();
    void testMatchCaseInSensitive();
	void testMatchGlobAll();
	void testMatchGlobStart();
	void testMatchGlobEnd();
	void testMatchGlobStartAndEnd();
	void testMatchGlobMiddle();
	void testMatchSuperGlobtastic();
	// StringConverter tests
	void testParseReal();
	void testParseInt();
	void testParseLong();
	void testParseUnsignedLong();
	void testParseVector3();
	void testParseMatrix4();
	void testParseQuaternion();
	void testParseBool();
	void testParseColourValue();


};
