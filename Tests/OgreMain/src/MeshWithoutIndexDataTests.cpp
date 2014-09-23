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
#include <stdio.h>
#include "Ogre.h"
#include "OgreDistanceLodStrategy.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreFileSystem.h"
#include "OgreArchiveManager.h"
#include "MeshWithoutIndexDataTests.h"
#include "OgreLodStrategyManager.h"
#include "OgreLodConfig.h"

#include "UnitTestSuite.h"

#ifdef OGRE_BUILD_COMPONENT_MESHLODGENERATOR
#include "OgreMeshLodGenerator.h"
#endif

// Register the test suite
CPPUNIT_TEST_SUITE_REGISTRATION(MeshWithoutIndexDataTests);

//--------------------------------------------------------------------------
void MeshWithoutIndexDataTests::setUp()
{
    UnitTestSuite::getSingletonPtr()->startTestSetup(__FUNCTION__);
    
    OGRE_NEW ResourceGroupManager();
    OGRE_NEW LodStrategyManager();
    mBufMgr = OGRE_NEW DefaultHardwareBufferManager();
    mMeshMgr = OGRE_NEW MeshManager();
    mArchiveMgr = OGRE_NEW ArchiveManager();
    mArchiveMgr->addArchiveFactory(OGRE_NEW FileSystemArchiveFactory());

    MaterialManager* matMgr = OGRE_NEW MaterialManager();
    matMgr->initialise();
}
//--------------------------------------------------------------------------
void MeshWithoutIndexDataTests::tearDown()
{
    OGRE_DELETE MaterialManager::getSingletonPtr();
    OGRE_DELETE mArchiveMgr;
    OGRE_DELETE mMeshMgr;
    OGRE_DELETE mBufMgr;
    OGRE_DELETE LodStrategyManager::getSingletonPtr();
    OGRE_DELETE ResourceGroupManager::getSingletonPtr();
}
//--------------------------------------------------------------------------
void MeshWithoutIndexDataTests::testCreateSimpleLine()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);
//--------------------------------------------------------------------------
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    ManualObject* line = OGRE_NEW ManualObject("line");
    line->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_LIST);
    line->position(0, 50, 0);
    line->position(50, 100, 0);
    line->end();
    String fileName = "line.mesh";
    MeshPtr lineMesh = line->convertToMesh(fileName);
    OGRE_DELETE line;

    CPPUNIT_ASSERT(lineMesh->getNumSubMeshes() == 1);
    CPPUNIT_ASSERT(lineMesh->getSubMesh(0)->indexData->indexCount == 0);
    RenderOperation rop;
    lineMesh->getSubMesh(0)->_getRenderOperation(rop);
    CPPUNIT_ASSERT(rop.useIndexes == false);
    CPPUNIT_ASSERT(lineMesh->getSubMesh(0)->vertexData->vertexCount == 2);

    MeshSerializer meshWriter;
    meshWriter.exportMesh(lineMesh.get(), fileName);

    mMeshMgr->remove(fileName);

    ResourceGroupManager::getSingleton().addResourceLocation(".", "FileSystem");
    MeshPtr loadedLine = mMeshMgr->load(fileName, "General");

    remove(fileName.c_str());

    CPPUNIT_ASSERT(loadedLine->getNumSubMeshes() == 1);
    CPPUNIT_ASSERT(loadedLine->getSubMesh(0)->indexData->indexCount == 0);
    loadedLine->getSubMesh(0)->_getRenderOperation(rop);
    CPPUNIT_ASSERT(rop.useIndexes == false);
    CPPUNIT_ASSERT(lineMesh->getSubMesh(0)->vertexData->vertexCount == 2);

    mMeshMgr->remove(fileName);
}
//--------------------------------------------------------------------------
void MeshWithoutIndexDataTests::testCreateLineList()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);
//--------------------------------------------------------------------------
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    ManualObject* lineList = OGRE_NEW ManualObject("line");
    lineList->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_LIST);
    lineList->position(0, 50, 0);
    lineList->position(50, 100, 0);
    lineList->position(50, 50, 0);
    lineList->position(100, 100, 0);
    lineList->position(0, 50, 0);
    lineList->position(50, 50, 0);
    lineList->end();
    String fileName = "lineList.mesh";
    MeshPtr lineListMesh = lineList->convertToMesh(fileName);
    OGRE_DELETE lineList;

    CPPUNIT_ASSERT(lineListMesh->getNumSubMeshes() == 1);
    CPPUNIT_ASSERT(lineListMesh->getSubMesh(0)->indexData->indexCount == 0);
    RenderOperation rop;
    lineListMesh->getSubMesh(0)->_getRenderOperation(rop);
    CPPUNIT_ASSERT(rop.useIndexes == false);
    CPPUNIT_ASSERT(lineListMesh->getSubMesh(0)->vertexData->vertexCount == 6);

    MeshSerializer meshWriter;
    meshWriter.exportMesh(lineListMesh.get(), fileName);

    mMeshMgr->remove(fileName);

    ResourceGroupManager::getSingleton().addResourceLocation(".", "FileSystem");
    MeshPtr loadedLineList = mMeshMgr->load(fileName, "General");

    remove(fileName.c_str());

    CPPUNIT_ASSERT(loadedLineList->getNumSubMeshes() == 1);
    CPPUNIT_ASSERT(loadedLineList->getSubMesh(0)->indexData->indexCount == 0);
    loadedLineList->getSubMesh(0)->_getRenderOperation(rop);
    CPPUNIT_ASSERT(rop.useIndexes == false);
    CPPUNIT_ASSERT(loadedLineList->getSubMesh(0)->vertexData->vertexCount == 6);

    mMeshMgr->remove(fileName);
}
//--------------------------------------------------------------------------
void MeshWithoutIndexDataTests::testCreateLineStrip()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);
//--------------------------------------------------------------------------
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    ManualObject* lineStrip = OGRE_NEW ManualObject("line");
    lineStrip->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_STRIP);
    lineStrip->position(50, 100, 0);
    lineStrip->position(0, 50, 0);
    lineStrip->position(50, 50, 0);
    lineStrip->position(100, 100, 0);
    lineStrip->end();
    String fileName = "lineStrip.mesh";
    MeshPtr lineStripMesh = lineStrip->convertToMesh(fileName);
    OGRE_DELETE lineStrip;

    CPPUNIT_ASSERT(lineStripMesh->getNumSubMeshes() == 1);
    CPPUNIT_ASSERT(lineStripMesh->getSubMesh(0)->indexData->indexCount == 0);
    RenderOperation rop;
    lineStripMesh->getSubMesh(0)->_getRenderOperation(rop);
    CPPUNIT_ASSERT(rop.useIndexes == false);
    CPPUNIT_ASSERT(lineStripMesh->getSubMesh(0)->vertexData->vertexCount == 4);

    MeshSerializer meshWriter;
    meshWriter.exportMesh(lineStripMesh.get(), fileName);

    mMeshMgr->remove(fileName);

    ResourceGroupManager::getSingleton().addResourceLocation(".", "FileSystem");
    MeshPtr loadedLineStrip = mMeshMgr->load(fileName, "General");

    remove(fileName.c_str());

    CPPUNIT_ASSERT(loadedLineStrip->getNumSubMeshes() == 1);
    CPPUNIT_ASSERT(loadedLineStrip->getSubMesh(0)->indexData->indexCount == 0);
    loadedLineStrip->getSubMesh(0)->_getRenderOperation(rop);
    CPPUNIT_ASSERT(rop.useIndexes == false);
    CPPUNIT_ASSERT(loadedLineStrip->getSubMesh(0)->vertexData->vertexCount == 4);

    mMeshMgr->remove(fileName);
}
//--------------------------------------------------------------------------
void MeshWithoutIndexDataTests::testCreatePointList()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);
//--------------------------------------------------------------------------
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    ManualObject* pointList = OGRE_NEW ManualObject("line");
    pointList->begin("BaseWhiteNoLighting", RenderOperation::OT_POINT_LIST);
    pointList->position(50, 100, 0);
    pointList->position(0, 50, 0);
    pointList->position(50, 50, 0);
    pointList->position(100, 100, 0);
    pointList->end();
    String fileName = "pointList.mesh";
    MeshPtr pointListMesh = pointList->convertToMesh(fileName);
    OGRE_DELETE pointList;

    CPPUNIT_ASSERT(pointListMesh->getNumSubMeshes() == 1);
    CPPUNIT_ASSERT(pointListMesh->getSubMesh(0)->indexData->indexCount == 0);
    RenderOperation rop;
    pointListMesh->getSubMesh(0)->_getRenderOperation(rop);
    CPPUNIT_ASSERT(rop.useIndexes == false);
    CPPUNIT_ASSERT(pointListMesh->getSubMesh(0)->vertexData->vertexCount == 4);

    MeshSerializer meshWriter;
    meshWriter.exportMesh(pointListMesh.get(), fileName);

    mMeshMgr->remove(fileName);

    ResourceGroupManager::getSingleton().addResourceLocation(".", "FileSystem");
    MeshPtr loadedPointList = mMeshMgr->load(fileName, "General");

    remove(fileName.c_str());

    CPPUNIT_ASSERT(loadedPointList->getNumSubMeshes() == 1);
    CPPUNIT_ASSERT(loadedPointList->getSubMesh(0)->indexData->indexCount == 0);
    loadedPointList->getSubMesh(0)->_getRenderOperation(rop);
    CPPUNIT_ASSERT(rop.useIndexes == false);
    CPPUNIT_ASSERT(loadedPointList->getSubMesh(0)->vertexData->vertexCount == 4);

    mMeshMgr->remove(fileName);
}
//--------------------------------------------------------------------------
void MeshWithoutIndexDataTests::testCreateLineWithMaterial()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);
//--------------------------------------------------------------------------
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    String matName = "lineMat";
    MaterialPtr matPtr = MaterialManager::getSingleton().create(matName, "General").staticCast<Material>();
    Pass* pass = matPtr->getTechnique(0)->getPass(0);
    pass->setDiffuse(1.0, 0.1, 0.1, 0);

    ManualObject* line = OGRE_NEW ManualObject("line");
    line->begin(matName, RenderOperation::OT_LINE_LIST);
    line->position(0, 50, 0);
    line->position(50, 100, 0);
    line->end();
    String fileName = "lineWithMat.mesh";
    MeshPtr lineMesh = line->convertToMesh(fileName);
    OGRE_DELETE line;

    CPPUNIT_ASSERT(lineMesh->getNumSubMeshes() == 1);
    CPPUNIT_ASSERT(lineMesh->getSubMesh(0)->indexData->indexCount == 0);
    RenderOperation rop;
    lineMesh->getSubMesh(0)->_getRenderOperation(rop);
    CPPUNIT_ASSERT(rop.useIndexes == false);
    CPPUNIT_ASSERT(lineMesh->getSubMesh(0)->vertexData->vertexCount == 2);

    MeshSerializer meshWriter;
    meshWriter.exportMesh(lineMesh.get(), fileName);
    MaterialSerializer matWriter;
    matWriter.exportMaterial(
        MaterialManager::getSingleton().getByName(matName).staticCast<Material>(),
        matName + ".material");

    mMeshMgr->remove(fileName);

    ResourceGroupManager::getSingleton().addResourceLocation(".", "FileSystem");
    MeshPtr loadedLine = mMeshMgr->load(fileName, "General");

    remove(fileName.c_str());
    remove((matName + ".material").c_str());

    CPPUNIT_ASSERT(loadedLine->getNumSubMeshes() == 1);
    CPPUNIT_ASSERT(loadedLine->getSubMesh(0)->indexData->indexCount == 0);
    loadedLine->getSubMesh(0)->_getRenderOperation(rop);
    CPPUNIT_ASSERT(rop.useIndexes == false);
    CPPUNIT_ASSERT(lineMesh->getSubMesh(0)->vertexData->vertexCount == 2);

    mMeshMgr->remove(fileName);
}
//--------------------------------------------------------------------------
void createMeshWithMaterial(String fileName)
{
    String matFileNameSuffix = ".material";
    String matName1 = "red";
    String matFileName1 = matName1 + matFileNameSuffix;
    MaterialPtr matPtr = MaterialManager::getSingleton().create(matName1, "General").staticCast<Material>();
    Pass* pass = matPtr->getTechnique(0)->getPass(0);
    pass->setDiffuse(1.0, 0.1, 0.1, 0);

    String matName2 = "green";
    String matFileName2 = matName2 + matFileNameSuffix;
    matPtr = MaterialManager::getSingleton().create(matName2, "General").staticCast<Material>();
    pass = matPtr->getTechnique(0)->getPass(0);
    pass->setDiffuse(0.1, 1.0, 0.1, 0);

    String matName3 = "blue";
    String matFileName3 = matName3 + matFileNameSuffix;
    matPtr = MaterialManager::getSingleton().create(matName3, "General").staticCast<Material>();
    pass = matPtr->getTechnique(0)->getPass(0);
    pass->setDiffuse(0.1, 0.1, 1.0, 0);

    String matName4 = "yellow";
    String matFileName4 = matName4 + matFileNameSuffix;
    matPtr = MaterialManager::getSingleton().create(matName4, "General").staticCast<Material>();
    pass = matPtr->getTechnique(0)->getPass(0);
    pass->setDiffuse(1.0, 1.0, 0.1, 0);

    ManualObject* manObj = OGRE_NEW ManualObject("mesh");
    manObj->begin(matName1, RenderOperation::OT_TRIANGLE_LIST);
    manObj->position(0, 50, 0);
    manObj->position(50, 50, 0);
    manObj->position(0, 100, 0);
    manObj->triangle(0, 1, 2);
    manObj->position(50, 100, 0);
    manObj->position(0, 100, 0);
    manObj->position(50, 50, 0);
    manObj->triangle(3, 4, 5);
    manObj->end();
    manObj->begin(matName2, RenderOperation::OT_LINE_LIST);
    manObj->position(0, 100, 0);
    manObj->position(-50, 50, 0);
    manObj->position(-50, 0, 0);
    manObj->position(-50, 50, 0);
    manObj->position(-100, 0, 0);
    manObj->position(-50, 0, 0);
    manObj->end();
    manObj->begin(matName3, RenderOperation::OT_LINE_STRIP);
    manObj->position(50, 100, 0);
    manObj->position(100, 50, 0);
    manObj->position(100, 0, 0);
    manObj->position(150, 0, 0);
    manObj->end();
    manObj->begin(matName4, RenderOperation::OT_POINT_LIST);
    manObj->position(50, 0, 0);
    manObj->position(0, 0, 0);
    manObj->end();
    manObj->convertToMesh(fileName);
    OGRE_DELETE manObj;
}
//--------------------------------------------------------------------------
void MeshWithoutIndexDataTests::testCreateMesh()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);
//--------------------------------------------------------------------------
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    String fileName = "indexMix.mesh";
    createMeshWithMaterial(fileName);
    MeshPtr mesh = mMeshMgr->getByName(fileName).staticCast<Mesh>();

    CPPUNIT_ASSERT(mesh->getNumSubMeshes() == 4);
    RenderOperation rop;
    for (int i=0; i<4; ++i)
    {
        mesh->getSubMesh(i)->_getRenderOperation(rop);
        // First submesh has indexes, the others does not.
        CPPUNIT_ASSERT( rop.useIndexes == (i == 0) );
    }

    MeshSerializer meshWriter;
    meshWriter.exportMesh(mesh.get(), fileName);

    mMeshMgr->remove(fileName);

    ResourceGroupManager::getSingleton().addResourceLocation(".", "FileSystem");
    MeshPtr loadedMesh = mMeshMgr->load(fileName, "General");

    remove(fileName.c_str());

    CPPUNIT_ASSERT(loadedMesh->getNumSubMeshes() == 4);

    mMeshMgr->remove(fileName);
}
//--------------------------------------------------------------------------
void MeshWithoutIndexDataTests::testCloneMesh()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);
//--------------------------------------------------------------------------
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    String originalName = "toClone.mesh";
    createMeshWithMaterial(originalName);
    MeshPtr mesh = mMeshMgr->getByName(originalName).staticCast<Mesh>();

    String fileName = "clone.mesh";
    MeshPtr clone = mesh->clone(fileName);
    CPPUNIT_ASSERT(mesh->getNumSubMeshes() == 4);

    MeshSerializer meshWriter;
    meshWriter.exportMesh(mesh.get(), fileName);

    mMeshMgr->remove(fileName);

    ResourceGroupManager::getSingleton().addResourceLocation(".", "FileSystem");
    MeshPtr loadedMesh = mMeshMgr->load(fileName, "General");

    remove(fileName.c_str());

    CPPUNIT_ASSERT(loadedMesh->getNumSubMeshes() == 4);

    mMeshMgr->remove(fileName);
}
//--------------------------------------------------------------------------
void MeshWithoutIndexDataTests::testEdgeList()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);
//--------------------------------------------------------------------------
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    String fileName = "testEdgeList.mesh";
    ManualObject* line = OGRE_NEW ManualObject("line");
    line->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_LIST);
    line->position(0, 50, 0);
    line->position(50, 100, 0);
    line->end();
    MeshPtr mesh = line->convertToMesh(fileName);
    OGRE_DELETE line;

    // whole mesh must not contain index data, for this test
    CPPUNIT_ASSERT(mesh->getNumSubMeshes() == 1);
    CPPUNIT_ASSERT(mesh->getSubMesh(0)->indexData->indexCount == 0);

    mesh->buildEdgeList();
    MeshSerializer meshWriter;
    // if it does not crash here, test is passed
    meshWriter.exportMesh(mesh.get(), fileName);

    remove(fileName.c_str());

    mMeshMgr->remove(fileName);
}
//--------------------------------------------------------------------------
void MeshWithoutIndexDataTests::testGenerateExtremes()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);
//--------------------------------------------------------------------------
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    String fileName = "testGenerateExtremes.mesh";
    createMeshWithMaterial(fileName);
    MeshPtr mesh = mMeshMgr->getByName(fileName).staticCast<Mesh>();

    const size_t NUM_EXTREMES = 4;
    for (ushort i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        mesh->getSubMesh(i)->generateExtremes(NUM_EXTREMES);
    }
    for (ushort i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        SubMesh* subMesh = mesh->getSubMesh(i);
        // According to generateExtremes, extremes are built based upon the bounding box indices.
        // But it also creates indices for all bounding boxes even if the mesh does not have any.
        // So...there should always be some extremity points. The number of which may vary.
        if (subMesh->indexData->indexCount > 0)
        {
            CPPUNIT_ASSERT(subMesh->extremityPoints.size() == NUM_EXTREMES);
        }
    }

    mMeshMgr->remove(fileName);
}
//--------------------------------------------------------------------------
void MeshWithoutIndexDataTests::testBuildTangentVectors()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);
//--------------------------------------------------------------------------
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    String fileName = "testBuildTangentVectors.mesh";
    createMeshWithMaterial(fileName);
    MeshPtr mesh = mMeshMgr->getByName(fileName).staticCast<Mesh>();

    try
    {
        // Make sure correct exception is thrown
        mesh->buildTangentVectors();
        CPPUNIT_FAIL("Expected InvalidParametersException!");
    }
    catch (const InvalidParametersException&)
    {
        // Ok
    }
    
    mMeshMgr->remove(fileName);
}
//--------------------------------------------------------------------------
void MeshWithoutIndexDataTests::testGenerateLodLevels()
{
#ifdef OGRE_BUILD_COMPONENT_MESHLODGENERATOR
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    String fileName = "testGenerateLodLevels.mesh";
    createMeshWithMaterial(fileName);
    MeshPtr mesh = mMeshMgr->getByName(fileName).staticCast<Mesh>();

    LodConfig lodConfig(mesh);
    lodConfig.createGeneratedLodLevel(600, 2, LodLevel::VRM_CONSTANT);
    MeshLodGenerator().generateLodLevels(lodConfig);
    // It may be less then 2, when two levels have the same vertex count it will be optimized out and lodLevel.outSkipped=true
    CPPUNIT_ASSERT(mesh->getNumLodLevels() == 2);
    for (ushort i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        SubMesh* subMesh = mesh->getSubMesh(i);
        for (ushort j = 0; j < mesh->getNumLodLevels() - 1; ++j)
        {
            if (subMesh->indexData->indexCount > 0)
            {
                // This may not be true for all meshes, but in this test we don't have reduced to 0.
                CPPUNIT_ASSERT(subMesh->mLodFaceList[j]->indexCount > 0);
            }
            else
            {
                // Should be 3 because of the dummy triangle being generated
                CPPUNIT_ASSERT(subMesh->mLodFaceList[j]->indexCount == 3);
            }
        }
    }

    MeshSerializer meshWriter;
    meshWriter.exportMesh(mesh.get(), fileName);

    remove(fileName.c_str());

    mMeshMgr->remove(fileName);
#endif
}
//--------------------------------------------------------------------------//--------------------------------------------------------------------------