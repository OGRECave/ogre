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

#include "OgrePrerequisites.h"
#include "OgreArchiveManager.h"
#include "OgreFileSystem.h"
#include "OgreRenderSystemCapabilitiesManager.h"

using namespace Ogre;


class RenderSystemCapabilitiesTests : public CppUnit::TestFixture
{
    // CppUnit macros for setting up the test suite
    CPPUNIT_TEST_SUITE( RenderSystemCapabilitiesTests );

    CPPUNIT_TEST(testIsShaderProfileSupported);
    CPPUNIT_TEST(testHasCapability);

    CPPUNIT_TEST(testSerializeBlank);
    CPPUNIT_TEST(testSerializeEnumCapability);
    CPPUNIT_TEST(testSerializeStringCapability);
    CPPUNIT_TEST(testSerializeBoolCapability);
    CPPUNIT_TEST(testSerializeIntCapability);
    CPPUNIT_TEST(testSerializeRealCapability);
    CPPUNIT_TEST(testSerializeShaderCapability);

    CPPUNIT_TEST(testWriteSimpleCapabilities);
    CPPUNIT_TEST(testWriteAllFalseCapabilities);
    CPPUNIT_TEST(testWriteAllTrueCapabilities);

    CPPUNIT_TEST(testWriteAndReadComplexCapabilities);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testIsShaderProfileSupported();
    void testHasCapability();

    // Tests for basic functionality in RenderSystemCapabilitiesSerializer
    void testSerializeBlank();
    void testSerializeEnumCapability();
    void testSerializeStringCapability();
    void testSerializeBoolCapability();
    void testSerializeIntCapability();
    void testSerializeRealCapability();
    void testSerializeShaderCapability();

    // Test for serializing capabilities to file
    void testWriteSimpleCapabilities();
    void testWriteAllFalseCapabilities();
    void testWriteAllTrueCapabilities();

    // Test serializing to and from the file
    void testWriteAndReadComplexCapabilities();

    // For serializing .rendercaps we need RSCManager
    RenderSystemCapabilitiesManager* mRenderSystemCapabilitiesManager;
    // Need these for loading .rendercaps from the file system
    ArchiveManager* mArchiveManager;
    FileSystemArchiveFactory* mFileSystemArchiveFactory;

};
