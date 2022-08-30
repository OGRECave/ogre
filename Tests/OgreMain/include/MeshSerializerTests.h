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

#include <gtest/gtest.h>
#include "OgreMesh.h"
#include "OgreMeshSerializer.h"
#include "OgreFileSystemLayer.h"

using namespace Ogre;

class MeshSerializerTests : public ::testing::Test
{

protected:
    MeshPtr mMesh;
    MeshPtr mOrigMesh;
    String mMeshFullPath;
    String mSkeletonFullPath;
    SkeletonPtr mSkeleton;
    Real mErrorFactor;
    FileSystemLayer* mFSLayer;

public:
    void SetUp() override;
    void TearDown() override;
    void testMesh(MeshVersion version);
    void assertMeshClone(Mesh* a, Mesh* b, MeshVersion version = MESH_VERSION_LATEST);
    void assertVertexDataClone(VertexData* a, VertexData* b, MeshVersion version = MESH_VERSION_LATEST);
    void assertIndexDataClone(IndexData* a, IndexData* b, MeshVersion version = MESH_VERSION_LATEST);
    void assertEdgeDataClone(EdgeData* a, EdgeData* b, MeshVersion version = MESH_VERSION_LATEST);
    void assertLodUsageClone(const MeshLodUsage& a, const MeshLodUsage& b, MeshVersion version = MESH_VERSION_LATEST);

    template<typename T>
    bool isContainerClone(T& a, T& b);
    template<typename K, typename V>
    bool isHashMapClone(const std::unordered_map<K, V>& a, const std::unordered_map<K, V>& b);

    void getResourceFullPath(const ResourcePtr& resource, String& outPath);
    bool copyFile(const String& srcPath, const String& dstPath);
    bool isLodMixed(const Mesh* pMesh);
    bool isEqual(Real a, Real b);
    bool isEqual(const Vector3& a, const Vector3& b);
};

#endif
