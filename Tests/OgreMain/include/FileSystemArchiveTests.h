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

using namespace Ogre;

class FileSystemArchiveTests : public CppUnit::TestFixture
{
    // CppUnit macros for setting up the test suite
    CPPUNIT_TEST_SUITE( FileSystemArchiveTests );
    CPPUNIT_TEST(testListNonRecursive);
    CPPUNIT_TEST(testListRecursive);
    CPPUNIT_TEST(testListFileInfoNonRecursive);
    CPPUNIT_TEST(testListFileInfoRecursive);
    CPPUNIT_TEST(testFindNonRecursive);
    CPPUNIT_TEST(testFindRecursive);
    CPPUNIT_TEST(testFindFileInfoNonRecursive);
    CPPUNIT_TEST(testFindFileInfoRecursive);
    CPPUNIT_TEST(testFileRead);
    CPPUNIT_TEST(testReadInterleave);
	CPPUNIT_TEST(testCreateAndRemoveFile);
    CPPUNIT_TEST_SUITE_END();
protected:
    String testPath;
public:
    void setUp();
    void tearDown();

    void testListNonRecursive();
    void testListRecursive();
    void testListFileInfoNonRecursive();
    void testListFileInfoRecursive();
    void testFindNonRecursive();
    void testFindRecursive();
    void testFindFileInfoNonRecursive();
    void testFindFileInfoRecursive();
    void testFileRead();
    void testReadInterleave();
	void testCreateAndRemoveFile();

};
