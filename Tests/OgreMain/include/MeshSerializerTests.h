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

#ifndef __MeshSerializerTests_H__
#define __MeshSerializerTests_H__

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "OgreMesh.h"
#include "OgreMeshSerializer.h"
#include "OgreFileSystemLayer.h"

using namespace Ogre;

class MeshSerializerTests : public CppUnit::TestFixture
{
    // CppUnit macros for setting up the test suite
    CPPUNIT_TEST_SUITE(MeshSerializerTests);
    CPPUNIT_TEST(testMesh_XML);
    CPPUNIT_TEST(testSkeleton_Version_1_8);
    CPPUNIT_TEST(testSkeleton_Version_1_0);
    CPPUNIT_TEST(testMesh_clone);
    CPPUNIT_TEST(testMesh_Version_1_10);
    CPPUNIT_TEST(testMesh_Version_1_8);
    CPPUNIT_TEST(testMesh_Version_1_41);
    CPPUNIT_TEST(testMesh_Version_1_4);
    CPPUNIT_TEST(testMesh_Version_1_3);
    CPPUNIT_TEST(testMesh_Version_1_2);
    CPPUNIT_TEST_SUITE_END();

protected:
    MeshPtr mMesh;
    MeshPtr mOrigMesh;
    String mMeshFullPath;
    String mSkeletonFullPath;
    SkeletonPtr mSkeleton;
    Real mErrorFactor;
    FileSystemLayer* mFSLayer;

public:
    void setUp();
    void tearDown();
    void testSkeleton_Version_1_8();
    void testSkeleton_Version_1_0();
    void testMesh_clone();
    void testMesh_Version_1_10();
    void testMesh_Version_1_8();
    void testMesh_Version_1_41();
    void testMesh_Version_1_4();
    void testMesh_Version_1_3();
    void testMesh_Version_1_2();
    void testMesh_XML();
    void testMesh(MeshVersion version);
    void assertMeshClone(Mesh* a, Mesh* b, MeshVersion version = MESH_VERSION_LATEST);
    void assertVertexDataClone(VertexData* a, VertexData* b, MeshVersion version = MESH_VERSION_LATEST);
    void assertIndexDataClone(IndexData* a, IndexData* b, MeshVersion version = MESH_VERSION_LATEST);
    void assertEdgeDataClone(EdgeData* a, EdgeData* b, MeshVersion version = MESH_VERSION_LATEST);
    void assertLodUsageClone(const MeshLodUsage& a, const MeshLodUsage& b, MeshVersion version = MESH_VERSION_LATEST);

    template<typename T>
    bool isContainerClone(T& a, T& b);
    template<typename K, typename V>
    bool isHashMapClone(const OGRE_HashMap<K, V>& a, const OGRE_HashMap<K, V>& b);

    void getResourceFullPath(const ResourcePtr& resource, String& outPath);
    bool copyFile(const String& srcPath, const String& dstPath);
    bool isLodMixed(const Mesh* pMesh);
    bool isEqual(Real a, Real b);
    bool isEqual(const Vector3& a, const Vector3& b);
};

#endif