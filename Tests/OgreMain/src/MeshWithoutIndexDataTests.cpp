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


#ifdef OGRE_BUILD_COMPONENT_MESHLODGENERATOR
#include "OgreMeshLodGenerator.h"
#endif

// Register the test suite

//--------------------------------------------------------------------------
void MeshWithoutIndexDataTests::SetUp()
{    
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
void MeshWithoutIndexDataTests::TearDown()
{
    OGRE_DELETE MaterialManager::getSingletonPtr();
    OGRE_DELETE mArchiveMgr;
    OGRE_DELETE mMeshMgr;
    OGRE_DELETE mBufMgr;
    OGRE_DELETE LodStrategyManager::getSingletonPtr();
    OGRE_DELETE ResourceGroupManager::getSingletonPtr();
}
//--------------------------------------------------------------------------
TEST_F(MeshWithoutIndexDataTests,CreateSimpleLine)
{
    ManualObject* line = OGRE_NEW ManualObject("line");
    line->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_LIST);
    line->position(0, 50, 0);
    line->position(50, 100, 0);
    line->end();
    String fileName = "line.mesh";
    MeshPtr lineMesh = line->convertToMesh(fileName);
    OGRE_DELETE line;

    EXPECT_TRUE(lineMesh->getNumSubMeshes() == 1);
    EXPECT_TRUE(lineMesh->getSubMesh(0)->indexData->indexCount == 0);
    RenderOperation rop;
    lineMesh->getSubMesh(0)->_getRenderOperation(rop);
    EXPECT_TRUE(rop.useIndexes == false);
    EXPECT_TRUE(lineMesh->getSubMesh(0)->vertexData->vertexCount == 2);

    MeshSerializer meshWriter;
    meshWriter.exportMesh(lineMesh.get(), fileName);

    mMeshMgr->remove(fileName, "General");

    ResourceGroupManager::getSingleton().addResourceLocation(".", "FileSystem");
    MeshPtr loadedLine = mMeshMgr->load(fileName, "General");

    remove(fileName.c_str());

    EXPECT_TRUE(loadedLine->getNumSubMeshes() == 1);
    EXPECT_TRUE(loadedLine->getSubMesh(0)->indexData->indexCount == 0);
    loadedLine->getSubMesh(0)->_getRenderOperation(rop);
    EXPECT_TRUE(rop.useIndexes == false);
    EXPECT_TRUE(lineMesh->getSubMesh(0)->vertexData->vertexCount == 2);

    mMeshMgr->remove(fileName, "General");
}
//--------------------------------------------------------------------------
TEST_F(MeshWithoutIndexDataTests,CreateLineList)
{
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

    EXPECT_TRUE(lineListMesh->getNumSubMeshes() == 1);
    EXPECT_TRUE(lineListMesh->getSubMesh(0)->indexData->indexCount == 0);
    RenderOperation rop;
    lineListMesh->getSubMesh(0)->_getRenderOperation(rop);
    EXPECT_TRUE(rop.useIndexes == false);
    EXPECT_TRUE(lineListMesh->getSubMesh(0)->vertexData->vertexCount == 6);

    MeshSerializer meshWriter;
    meshWriter.exportMesh(lineListMesh.get(), fileName);

    mMeshMgr->remove(fileName, "General");

    ResourceGroupManager::getSingleton().addResourceLocation(".", "FileSystem");
    MeshPtr loadedLineList = mMeshMgr->load(fileName, "General");

    remove(fileName.c_str());

    EXPECT_TRUE(loadedLineList->getNumSubMeshes() == 1);
    EXPECT_TRUE(loadedLineList->getSubMesh(0)->indexData->indexCount == 0);
    loadedLineList->getSubMesh(0)->_getRenderOperation(rop);
    EXPECT_TRUE(rop.useIndexes == false);
    EXPECT_TRUE(loadedLineList->getSubMesh(0)->vertexData->vertexCount == 6);

    mMeshMgr->remove(fileName, "General");
}
//--------------------------------------------------------------------------
TEST_F(MeshWithoutIndexDataTests,CreateLineStrip)
{
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

    EXPECT_TRUE(lineStripMesh->getNumSubMeshes() == 1);
    EXPECT_TRUE(lineStripMesh->getSubMesh(0)->indexData->indexCount == 0);
    RenderOperation rop;
    lineStripMesh->getSubMesh(0)->_getRenderOperation(rop);
    EXPECT_TRUE(rop.useIndexes == false);
    EXPECT_TRUE(lineStripMesh->getSubMesh(0)->vertexData->vertexCount == 4);

    MeshSerializer meshWriter;
    meshWriter.exportMesh(lineStripMesh.get(), fileName);

    mMeshMgr->remove(fileName, "General");

    ResourceGroupManager::getSingleton().addResourceLocation(".", "FileSystem");
    MeshPtr loadedLineStrip = mMeshMgr->load(fileName, "General");

    remove(fileName.c_str());

    EXPECT_TRUE(loadedLineStrip->getNumSubMeshes() == 1);
    EXPECT_TRUE(loadedLineStrip->getSubMesh(0)->indexData->indexCount == 0);
    loadedLineStrip->getSubMesh(0)->_getRenderOperation(rop);
    EXPECT_TRUE(rop.useIndexes == false);
    EXPECT_TRUE(loadedLineStrip->getSubMesh(0)->vertexData->vertexCount == 4);

    mMeshMgr->remove(fileName, "General");
}
//--------------------------------------------------------------------------
TEST_F(MeshWithoutIndexDataTests,CreatePointList)
{
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

    EXPECT_TRUE(pointListMesh->getNumSubMeshes() == 1);
    EXPECT_TRUE(pointListMesh->getSubMesh(0)->indexData->indexCount == 0);
    RenderOperation rop;
    pointListMesh->getSubMesh(0)->_getRenderOperation(rop);
    EXPECT_TRUE(rop.useIndexes == false);
    EXPECT_TRUE(pointListMesh->getSubMesh(0)->vertexData->vertexCount == 4);

    MeshSerializer meshWriter;
    meshWriter.exportMesh(pointListMesh.get(), fileName);

    mMeshMgr->remove(fileName, "General");

    ResourceGroupManager::getSingleton().addResourceLocation(".", "FileSystem");
    MeshPtr loadedPointList = mMeshMgr->load(fileName, "General");

    remove(fileName.c_str());

    EXPECT_TRUE(loadedPointList->getNumSubMeshes() == 1);
    EXPECT_TRUE(loadedPointList->getSubMesh(0)->indexData->indexCount == 0);
    loadedPointList->getSubMesh(0)->_getRenderOperation(rop);
    EXPECT_TRUE(rop.useIndexes == false);
    EXPECT_TRUE(loadedPointList->getSubMesh(0)->vertexData->vertexCount == 4);

    mMeshMgr->remove(fileName, "General");
}
//--------------------------------------------------------------------------
TEST_F(MeshWithoutIndexDataTests,CreateLineWithMaterial)
{
    String matName = "lineMat";
    MaterialPtr matPtr = MaterialManager::getSingleton().create(matName, "General");
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

    EXPECT_TRUE(lineMesh->getNumSubMeshes() == 1);
    EXPECT_TRUE(lineMesh->getSubMesh(0)->indexData->indexCount == 0);
    RenderOperation rop;
    lineMesh->getSubMesh(0)->_getRenderOperation(rop);
    EXPECT_TRUE(rop.useIndexes == false);
    EXPECT_TRUE(lineMesh->getSubMesh(0)->vertexData->vertexCount == 2);

    MeshSerializer meshWriter;
    meshWriter.exportMesh(lineMesh.get(), fileName);
    MaterialSerializer matWriter;
    matWriter.exportMaterial(
        MaterialManager::getSingleton().getByName(matName),
        matName + ".material");

    mMeshMgr->remove(fileName, "General");

    ResourceGroupManager::getSingleton().addResourceLocation(".", "FileSystem");
    MeshPtr loadedLine = mMeshMgr->load(fileName, "General");

    remove(fileName.c_str());
    remove((matName + ".material").c_str());

    EXPECT_TRUE(loadedLine->getNumSubMeshes() == 1);
    EXPECT_TRUE(loadedLine->getSubMesh(0)->indexData->indexCount == 0);
    loadedLine->getSubMesh(0)->_getRenderOperation(rop);
    EXPECT_TRUE(rop.useIndexes == false);
    EXPECT_TRUE(lineMesh->getSubMesh(0)->vertexData->vertexCount == 2);

    mMeshMgr->remove(fileName, "General");
}
//--------------------------------------------------------------------------
static void createMeshWithMaterial(String fileName)
{
    String matFileNameSuffix = ".material";
    String matName1 = "red";
    String matFileName1 = matName1 + matFileNameSuffix;
    MaterialPtr matPtr = MaterialManager::getSingleton().create(matName1, "General");
    Pass* pass = matPtr->getTechnique(0)->getPass(0);
    pass->setDiffuse(1.0, 0.1, 0.1, 0);

    String matName2 = "green";
    String matFileName2 = matName2 + matFileNameSuffix;
    matPtr = MaterialManager::getSingleton().create(matName2, "General");
    pass = matPtr->getTechnique(0)->getPass(0);
    pass->setDiffuse(0.1, 1.0, 0.1, 0);

    String matName3 = "blue";
    String matFileName3 = matName3 + matFileNameSuffix;
    matPtr = MaterialManager::getSingleton().create(matName3, "General");
    pass = matPtr->getTechnique(0)->getPass(0);
    pass->setDiffuse(0.1, 0.1, 1.0, 0);

    String matName4 = "yellow";
    String matFileName4 = matName4 + matFileNameSuffix;
    matPtr = MaterialManager::getSingleton().create(matName4, "General");
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
TEST_F(MeshWithoutIndexDataTests,CreateMesh)
{
    String fileName = "indexMix.mesh";
    createMeshWithMaterial(fileName);
    MeshPtr mesh = mMeshMgr->getByName(fileName, "General");

    EXPECT_TRUE(mesh->getNumSubMeshes() == 4);
    RenderOperation rop;
    for (int i=0; i<4; ++i)
    {
        mesh->getSubMesh(i)->_getRenderOperation(rop);
        // First submesh has indexes, the others does not.
        EXPECT_TRUE( rop.useIndexes == (i == 0) );
    }

    MeshSerializer meshWriter;
    meshWriter.exportMesh(mesh.get(), fileName);

    mMeshMgr->remove(fileName, "General");

    ResourceGroupManager::getSingleton().addResourceLocation(".", "FileSystem");
    MeshPtr loadedMesh = mMeshMgr->load(fileName, "General");

    remove(fileName.c_str());

    EXPECT_TRUE(loadedMesh->getNumSubMeshes() == 4);

    mMeshMgr->remove(fileName, "General");
}
//--------------------------------------------------------------------------
TEST_F(MeshWithoutIndexDataTests,CloneMesh)
{
    String originalName = "toClone.mesh";
    createMeshWithMaterial(originalName);
    MeshPtr mesh = mMeshMgr->getByName(originalName, "General");

    String fileName = "clone.mesh";
    MeshPtr clone = mesh->clone(fileName);
    EXPECT_TRUE(mesh->getNumSubMeshes() == 4);

    MeshSerializer meshWriter;
    meshWriter.exportMesh(mesh.get(), fileName);

    mMeshMgr->remove(fileName, "General");

    ResourceGroupManager::getSingleton().addResourceLocation(".", "FileSystem");
    MeshPtr loadedMesh = mMeshMgr->load(fileName, "General");

    remove(fileName.c_str());

    EXPECT_TRUE(loadedMesh->getNumSubMeshes() == 4);

    mMeshMgr->remove(fileName, "General");
}
//--------------------------------------------------------------------------
TEST_F(MeshWithoutIndexDataTests,EdgeList)
{
    String fileName = "testEdgeList.mesh";
    ManualObject* line = OGRE_NEW ManualObject("line");
    line->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_LIST);
    line->position(0, 50, 0);
    line->position(50, 100, 0);
    line->end();
    MeshPtr mesh = line->convertToMesh(fileName);
    OGRE_DELETE line;

    // whole mesh must not contain index data, for this test
    EXPECT_TRUE(mesh->getNumSubMeshes() == 1);
    EXPECT_TRUE(mesh->getSubMesh(0)->indexData->indexCount == 0);

    mesh->buildEdgeList();
    MeshSerializer meshWriter;
    // if it does not crash here, test is passed
    meshWriter.exportMesh(mesh.get(), fileName);

    remove(fileName.c_str());

    mMeshMgr->remove(fileName, "General");
}
//--------------------------------------------------------------------------
TEST_F(MeshWithoutIndexDataTests,GenerateExtremes)
{
    String fileName = "testGenerateExtremes.mesh";
    createMeshWithMaterial(fileName);
    MeshPtr mesh = mMeshMgr->getByName(fileName, "General");

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
            EXPECT_TRUE(subMesh->extremityPoints.size() == NUM_EXTREMES);
        }
    }

    mMeshMgr->remove(fileName, "General");
}
//--------------------------------------------------------------------------
TEST_F(MeshWithoutIndexDataTests,BuildTangentVectors)
{
    String fileName = "testBuildTangentVectors.mesh";
    createMeshWithMaterial(fileName);
    MeshPtr mesh = mMeshMgr->getByName(fileName, "General");

    EXPECT_THROW(mesh->buildTangentVectors(), InvalidParametersException);
    
    mMeshMgr->remove(fileName, "General");
}
//--------------------------------------------------------------------------
TEST_F(MeshWithoutIndexDataTests,GenerateLodLevels)
{
#ifdef OGRE_BUILD_COMPONENT_MESHLODGENERATOR
    String fileName = "testGenerateLodLevels.mesh";
    createMeshWithMaterial(fileName);
    MeshPtr mesh = mMeshMgr->getByName(fileName, "General");

    LodConfig lodConfig(mesh);
    lodConfig.createGeneratedLodLevel(600, 2, LodLevel::VRM_CONSTANT);
    MeshLodGenerator().generateLodLevels(lodConfig);
    // It may be less then 2, when two levels have the same vertex count it will be optimized out and lodLevel.outSkipped=true
    EXPECT_TRUE(mesh->getNumLodLevels() == 2);
    for (ushort i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        SubMesh* subMesh = mesh->getSubMesh(i);
        for (ushort j = 0; j < mesh->getNumLodLevels() - 1; ++j)
        {
            if (subMesh->indexData->indexCount > 0)
            {
                // This may not be true for all meshes, but in this test we don't have reduced to 0.
                EXPECT_TRUE(subMesh->mLodFaceList[j]->indexCount > 0);
            }
            else
            {
                // Should be 3 because of the dummy triangle being generated
                EXPECT_TRUE(subMesh->mLodFaceList[j]->indexCount == 3);
            }
        }
    }

    MeshSerializer meshWriter;
    meshWriter.exportMesh(mesh.get(), fileName);

    remove(fileName.c_str());

    mMeshMgr->remove(fileName, "General");
#endif
}
//--------------------------------------------------------------------------//--------------------------------------------------------------------------
