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
#include "OgreLogManager.h"

using namespace Ogre;
using Ogre::ushort;

class MeshWithoutIndexDataTests : public CppUnit::TestFixture
{
    // CppUnit macros for setting up the test suite
    CPPUNIT_TEST_SUITE( MeshWithoutIndexDataTests );
    CPPUNIT_TEST(testCreateSimpleLine);
    CPPUNIT_TEST(testCreateLineList);
    CPPUNIT_TEST(testCreateLineStrip);
    CPPUNIT_TEST(testCreatePointList);
    CPPUNIT_TEST(testCreateLineWithMaterial);
    CPPUNIT_TEST(testCreateMesh);
    CPPUNIT_TEST(testCloneMesh);
    CPPUNIT_TEST(testEdgeList);
    CPPUNIT_TEST(testGenerateExtremes);
    CPPUNIT_TEST(testBuildTangentVectors);
    CPPUNIT_TEST(testGenerateLodLevels);
    CPPUNIT_TEST_SUITE_END();

protected:
    HardwareBufferManager* mBufMgr;
    MeshManager* mMeshMgr;
    ArchiveManager* archiveMgr;

public:
    void setUp();
    void tearDown();
    void testCreateSimpleLine();
    void testCreateLineList();
    void testCreateLineStrip();
    void testCreatePointList();
    void testCreateLineWithMaterial();
    void testCreateMesh();
    void testCloneMesh();
    void testEdgeList();
    void testGenerateExtremes();
    void testBuildTangentVectors();
    void testGenerateLodLevels();

};
