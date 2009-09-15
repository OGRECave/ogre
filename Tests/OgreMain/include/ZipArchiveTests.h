/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "OgreString.h"

class ZipArchiveTests : public CppUnit::TestFixture
{
    // CppUnit macros for setting up the test suite
    CPPUNIT_TEST_SUITE( ZipArchiveTests );
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
    CPPUNIT_TEST_SUITE_END();
protected:
    Ogre::String testPath;
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

};
