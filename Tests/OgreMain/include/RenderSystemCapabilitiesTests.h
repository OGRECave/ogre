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
