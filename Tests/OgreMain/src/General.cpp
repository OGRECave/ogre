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
#include <gtest/gtest.h>

#include "OgreRoot.h"
#include "OgreSceneNode.h"
#include "OgreEntity.h"
#include "OgreCamera.h"
#include "RootWithoutRenderSystemFixture.h"
#include "OgreStaticPluginLoader.h"

#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreMaterialManager.h"
#include "OgreConfigFile.h"
#include "OgreSTBICodec.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreMeshManager.h"
#include "OgreMeshSerializer.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreSkeletonManager.h"
#include "OgreCompositorManager.h"
#include "OgreTextureManager.h"
#include "OgreFileSystem.h"
#include "OgreArchiveManager.h"

#include "OgreHighLevelGpuProgram.h"

#include "OgreKeyFrame.h"
#include "OgreAnimable.h"

#include "OgreBillboardSet.h"
#include "OgreBillboard.h"
#include "OgreParticleEmitter.h"
#include "OgreParticleSystemManager.h"
#include "OgreParticleSystemRenderer.h"
#include "OgrePredefinedControllers.h"
#include "OgreAutoParamDataSource.h"

#include <random>
using std::minstd_rand;

using namespace Ogre;

typedef RootWithoutRenderSystemFixture CameraTests;
TEST_F(CameraTests,customProjectionMatrix)
{
    Camera cam("", NULL);
    std::vector<Vector3> corners(cam.getWorldSpaceCorners(), cam.getWorldSpaceCorners() + 8);
    RealRect extents = cam.getFrustumExtents();
    cam.setCustomProjectionMatrix(true, cam.getProjectionMatrix());
    for(int j = 0; j < 8; j++) {
        for(int k = 0; k < 3; k++) {
            if(OGRE_DOUBLE_PRECISION == 0)
                EXPECT_FLOAT_EQ(corners[j][k], cam.getWorldSpaceCorners()[j][k]);
            else
                EXPECT_DOUBLE_EQ(corners[j][k], cam.getWorldSpaceCorners()[j][k]);
        }
    }

    if(OGRE_DOUBLE_PRECISION == 0) {
        EXPECT_FLOAT_EQ(extents.bottom, cam.getFrustumExtents().bottom);
        EXPECT_FLOAT_EQ(extents.top, cam.getFrustumExtents().top);
        EXPECT_FLOAT_EQ(extents.left, cam.getFrustumExtents().left);
        EXPECT_FLOAT_EQ(extents.right, cam.getFrustumExtents().right);
    } else {
        EXPECT_DOUBLE_EQ(extents.bottom, cam.getFrustumExtents().bottom);
        EXPECT_DOUBLE_EQ(extents.top, cam.getFrustumExtents().top);
        EXPECT_DOUBLE_EQ(extents.left, cam.getFrustumExtents().left);
        EXPECT_DOUBLE_EQ(extents.right, cam.getFrustumExtents().right);
    }

}

TEST(Root, builtinFactoryTypes)
{
    Root root("");
    EXPECT_EQ(MOT_RECTANGLE2D, root.getMovableObjectFactory(MOT_RECTANGLE2D)->getType());
    EXPECT_EQ(MOT_STATIC_GEOMETRY, root.getMovableObjectFactory(MOT_STATIC_GEOMETRY)->getType());
}

TEST(Root,shutdown)
{
#ifdef OGRE_STATIC_LIB
    Root root("");
    OgreBites::StaticPluginLoader mStaticPluginLoader;
    mStaticPluginLoader.load();
#else
    Root root;
#endif
    root.shutdown();
}

TEST(SceneManager, removeAndDestroyAllChildren)
{
    Root root("");
    SceneManager* sm = root.createSceneManager();
    sm->getRootSceneNode()->createChildSceneNode();
    sm->getRootSceneNode()->createChildSceneNode();
    sm->getRootSceneNode()->removeAndDestroyAllChildren();
}

struct SceneNodeTest : public RootWithoutRenderSystemFixture {
    SceneManager* mSceneMgr;

    void SetUp() override {
        RootWithoutRenderSystemFixture::SetUp();
        mSceneMgr = mRoot->createSceneManager();
    }
};

TEST_F(SceneNodeTest, detachAllObjects){
	auto sinbad = mSceneMgr->createEntity("sinbad", "Sinbad.mesh");
	auto sinbad2 = mSceneMgr->createEntity("sinbad2", "Sinbad.mesh");
	SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode("parent");
    node->attachObject(sinbad);
    node->attachObject(sinbad2);

	auto sinbad3 = mSceneMgr->createEntity("sinbad3", "Sinbad.mesh");
	SceneNode* child = node->createChildSceneNode("child");
    child->attachObject(sinbad3);
    node->destroyAllObjects();
    EXPECT_FALSE(mSceneMgr->hasEntity("sinbad"));
    EXPECT_FALSE(mSceneMgr->hasEntity("sinbad2"));
    EXPECT_TRUE(mSceneMgr->hasEntity("sinbad3"));
    EXPECT_EQ(node->numAttachedObjects(), 0);
    EXPECT_EQ(child->numAttachedObjects(), 1);
}

TEST_F(SceneNodeTest, destroyAllChildrenAndObjects)
{
	auto sinbad = mSceneMgr->createEntity("sinbad", "Sinbad.mesh");
	SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode("parent");
    node->attachObject(sinbad);

	auto sinbad2 = mSceneMgr->createEntity("sinbad2", "Sinbad.mesh");
	SceneNode* child = node->createChildSceneNode("child");
    child->attachObject(sinbad2);

	auto sinbad3 = mSceneMgr->createEntity("sinbad3", "Sinbad.mesh");
	SceneNode* grandchild = node->createChildSceneNode("grandchild");
    grandchild->attachObject(sinbad3);

    node->destroyAllChildrenAndObjects();
    EXPECT_FALSE(mSceneMgr->hasSceneNode("grandchild"));
    EXPECT_FALSE(mSceneMgr->hasEntity("sinbad3"));
    EXPECT_FALSE(mSceneMgr->hasSceneNode("child"));
    EXPECT_FALSE(mSceneMgr->hasEntity("sinbad2"));
    EXPECT_FALSE(mSceneMgr->hasEntity("sinbad"));
    EXPECT_TRUE(mSceneMgr->hasSceneNode("parent"));
}

TEST_F(SceneNodeTest, destroyChildAndObjects)
{

	auto sinbad = mSceneMgr->createEntity("sinbad", "Sinbad.mesh");
	SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode("parent");
    node->attachObject(sinbad);

	auto sinbad2 = mSceneMgr->createEntity("sinbad2", "Sinbad.mesh");
	SceneNode* child = node->createChildSceneNode("child");
    child->attachObject(sinbad2);

	auto sinbad3 = mSceneMgr->createEntity("sinbad3", "Sinbad.mesh");
	SceneNode* grandchild = child->createChildSceneNode("grandchild");
    grandchild->attachObject(sinbad3);

    node->destroyChildAndObjects("child");

    EXPECT_FALSE(mSceneMgr->hasSceneNode("grandchild"));
    EXPECT_FALSE(mSceneMgr->hasSceneNode("child"));
    EXPECT_TRUE(mSceneMgr->hasSceneNode("parent"));
    EXPECT_FALSE(mSceneMgr->hasEntity("sinbad2"));
    EXPECT_FALSE(mSceneMgr->hasEntity("sinbad3"));
    EXPECT_TRUE(mSceneMgr->hasEntity("sinbad"));
}

TEST_F(SceneNodeTest, removeFromParent) {
    SceneNode* parent = mSceneMgr->getRootSceneNode()->createChildSceneNode("parent");
    SceneNode* child = parent->createChildSceneNode("child");
    EXPECT_EQ(parent->getChild("child"), child);
    child->removeFromParent(); //detach when we have a parent.
    EXPECT_EQ(child->getParent(), nullptr);
    child->removeFromParent(); //detach when we don't a parent.
    EXPECT_EQ(child->getParent(), nullptr); //it is a no-op
}

static testing::AssertionResult AssertDirectionEquals(const char* m_expr,
                                               const char* n_expr,
                                               const Vector3& m,
                                               const Vector3& n) {
    if (m.directionEquals(n, Degree(0.01f)))
        return testing::AssertionSuccess();

    return testing::AssertionFailure()
        << m_expr << " and " << n_expr << " do not have the same direction\n"
        << "  Actual: " << m << "\n"
        << "Expected: " << n;
}
#define EXPECT_DIR_EQ(val1, val2) EXPECT_PRED_FORMAT2(AssertDirectionEquals, val1, val2)

TEST_F(SceneNodeTest, setDirection) {
    SceneNode* parent = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    parent->setOrientation(Quaternion(Degree(90), Vector3::UNIT_Y));

    SceneNode* child = parent->createChildSceneNode();

    child->setDirection(Vector3::UNIT_X, Node::TS_PARENT);
    EXPECT_DIR_EQ(child->getOrientation() * Vector3::NEGATIVE_UNIT_Z, Vector3::UNIT_X);

    child->setDirection(Vector3::UNIT_Z, Node::TS_WORLD);
    EXPECT_DIR_EQ(child->_getDerivedOrientation() * Vector3::NEGATIVE_UNIT_Z, Vector3::UNIT_Z);

    child->setOrientation(Quaternion::IDENTITY);
    child->setDirection(Vector3::UNIT_X, Node::TS_LOCAL);
    EXPECT_DIR_EQ(child->getOrientation() * Vector3::NEGATIVE_UNIT_Z, Vector3::UNIT_X);

    child->setFixedYawAxis(true, Vector3::UNIT_Z);
    child->setDirection(Vector3::UNIT_X, Node::TS_PARENT);
    EXPECT_DIR_EQ(child->getOrientation() * Vector3::NEGATIVE_UNIT_Z, Vector3::UNIT_X);
}

static void createRandomEntityClones(Entity* ent, size_t cloneCount, const Vector3& min,
                                     const Vector3& max, SceneManager* mgr)
{
    // we want cross platform consistent sequence
    minstd_rand rng;

    for (size_t n = 0; n < cloneCount; ++n)
    {
        // Create a new node under the root.
        SceneNode* node = mgr->createSceneNode();
        // Random translate.
        Vector3 nodePos = max - min;
        nodePos.x *= double(rng())/rng.max();
        nodePos.y *= double(rng())/rng.max();
        nodePos.z *= double(rng())/rng.max();
        nodePos += min;
        node->setPosition(nodePos);
        mgr->getRootSceneNode()->addChild(node);
        Entity* cloneEnt = ent->clone(StringConverter::toString(n));
        // Attach to new node.
        node->attachObject(cloneEnt);
    }
}

struct SceneQueryTest : public RootWithoutRenderSystemFixture {
    SceneManager* mSceneMgr;
    Camera* mCamera;
    SceneNode* mCameraNode;

    void SetUp() override {
        RootWithoutRenderSystemFixture::SetUp();

        mSceneMgr = mRoot->createSceneManager();
        mCamera = mSceneMgr->createCamera("Camera");
        mCameraNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mCameraNode->attachObject(mCamera);
        mCameraNode->setPosition(0,0,500);
        mCameraNode->lookAt(Vector3(0, 0, 0), Node::TS_PARENT);

        // Create a set of random balls
        Entity* ent = mSceneMgr->createEntity("501", "sphere.mesh", "General");

        // stick one at the origin so one will always be hit by ray
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
        createRandomEntityClones(ent, 500, Vector3(-2500,-2500,-2500), Vector3(2500,2500,2500), mSceneMgr);

        mSceneMgr->_updateSceneGraph(mCamera);
    }
};

TEST_F(SceneQueryTest,Intersection)
{
    IntersectionSceneQuery* intersectionQuery = mSceneMgr->createIntersectionQuery();

    int expected[][2] = {
        {0, 391},   {1, 8},     {117, 128}, {118, 171}, {118, 24},  {121, 72},  {121, 95},
        {132, 344}, {14, 227},  {14, 49},   {144, 379}, {151, 271}, {153, 28},  {164, 222},
        {169, 212}, {176, 20},  {179, 271}, {185, 238}, {190, 47},  {193, 481}, {201, 210},
        {205, 404}, {235, 366}, {239, 3},   {250, 492}, {256, 67},  {26, 333},  {260, 487},
        {263, 272}, {265, 319}, {265, 472}, {270, 45},  {284, 329}, {289, 405}, {316, 80},
        {324, 388}, {334, 337}, {336, 436}, {34, 57},   {340, 440}, {342, 41},  {348, 82},
        {35, 478},  {372, 412}, {380, 460}, {398, 92},  {417, 454}, {432, 99},  {448, 79},
        {498, 82},  {72, 77}
    };

    IntersectionSceneQueryResult& results = intersectionQuery->execute();
    EXPECT_EQ(results.movables2movables.size(), sizeof(expected)/sizeof(expected[0]));

    int i = 0;
    for (auto & thepair : results.movables2movables)
    {
        // printf("{%d, %d},", StringConverter::parseInt(thepair.first->getName()), StringConverter::parseInt(thepair.second->getName()));
        ASSERT_EQ(expected[i][0], StringConverter::parseInt(thepair.first->getName()));
        ASSERT_EQ(expected[i][1], StringConverter::parseInt(thepair.second->getName()));
        i++;
    }
    // printf("\n");
}

TEST_F(SceneQueryTest, Ray) {
    RaySceneQuery* rayQuery = mSceneMgr->createRayQuery(mCamera->getCameraToViewportRay(0.5, 0.5));
    rayQuery->setSortByDistance(true, 2);

    RaySceneQueryResult& results = rayQuery->execute();

    ASSERT_EQ("501", results[0].movable->getName());
    ASSERT_EQ("397", results[1].movable->getName());
}

struct MeshSerializerBufferRange : public RootWithoutRenderSystemFixture
{
};

typedef RootWithoutRenderSystemFixture MeshTests;
TEST_F(MeshTests, BoundsFromVertexStart)
{
    MeshPtr mesh = MeshManager::getSingleton().createManual("BoundsVertexStart", RGN_DEFAULT);
    mesh->sharedVertexData = new VertexData();
    mesh->sharedVertexData->vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
    auto vertexBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(float) * 3, 2, HBU_CPU_TO_GPU);
    float positions[2][3] = {{9, 9, 9}, {1, 1, 0}};
    vertexBuffer->writeData(0, sizeof(positions), positions, true);
    mesh->sharedVertexData->vertexBufferBinding->setBinding(0, vertexBuffer);
    mesh->sharedVertexData->vertexStart = 1;
    mesh->sharedVertexData->vertexCount = 1;

    mesh->_updateBoundsFromVertexBuffers();
    EXPECT_EQ(mesh->getBounds(), AxisAlignedBox(Vector3(1, 1, 0), Vector3(1, 1, 0)));
}

TEST_F(MeshTests, PoseBlendFromVertexStart)
{
    VertexData vertexData;
    vertexData.vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
    auto vertexBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(float) * 3, 2, HBU_CPU_TO_GPU);
    float positions[2][3] = {};
    vertexBuffer->writeData(0, sizeof(positions), positions, true);
    vertexData.vertexBufferBinding->setBinding(0, vertexBuffer);
    vertexData.vertexStart = 1;
    vertexData.vertexCount = 1;

    std::map<uint32, Vector3f> offsets = {{0, Vector3f(1, 1, 1)}}, normals;
    Mesh::softwareVertexPoseBlend(1, offsets, normals, &vertexData);

    vertexBuffer->readData(0, sizeof(positions), positions);
    EXPECT_EQ(positions[1][0], 1);
}

TEST_F(MeshTests, MorphFromVertexStart)
{
    VertexData vertexData;
    vertexData.vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
    auto vertexBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(float) * 3, 2, HBU_CPU_TO_GPU);
    float positions[2][3] = {};
    vertexBuffer->writeData(0, sizeof(positions), positions, true);
    vertexData.vertexBufferBinding->setBinding(0, vertexBuffer);
    vertexData.vertexStart = 1;
    vertexData.vertexCount = 1;

    auto keyFrame = HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(float) * 3, 1, HBU_CPU_TO_GPU);
    float key[3] = {1, 1, 1};
    keyFrame->writeData(0, sizeof(key), key, true);
    Mesh::softwareVertexMorph(0, keyFrame, keyFrame, &vertexData);

    vertexBuffer->readData(0, sizeof(positions), positions);
    EXPECT_EQ(positions[1][0], 1);
}

TEST_F(MeshTests, SkinningFromVertexStart)
{
    VertexData src;
    src.vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
    src.vertexDeclaration->addElement(1, 0, VET_UBYTE4, VES_BLEND_INDICES);
    src.vertexDeclaration->addElement(1, 4, VET_FLOAT1, VES_BLEND_WEIGHTS);
    auto srcPositions = HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(float) * 3, 2, HBU_CPU_TO_GPU);
    float positions[2][3] = {{0, 0, 0}, {1, 1, 1}};
    srcPositions->writeData(0, sizeof(positions), positions, true);
    auto blendBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(8, 2, HBU_CPU_TO_GPU);
    struct { uint8 indices[4]; float weight; } blend[2] = {{{0, 0, 0, 0}, 1}, {{0, 0, 0, 0}, 1}};
    blendBuffer->writeData(0, sizeof(blend), blend, true);
    src.vertexBufferBinding->setBinding(0, srcPositions);
    src.vertexBufferBinding->setBinding(1, blendBuffer);
    src.vertexStart = 1;
    src.vertexCount = 1;

    VertexData dst;
    dst.vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
    auto dstPositions = HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(float) * 3, 2, HBU_CPU_TO_GPU);
    float zeros[2][3] = {};
    dstPositions->writeData(0, sizeof(zeros), zeros, true);
    dst.vertexBufferBinding->setBinding(0, dstPositions);
    dst.vertexStart = 1;
    dst.vertexCount = 1;

    const Affine3* matrices[1] = {&Affine3::IDENTITY};
    Mesh::softwareVertexBlend(&src, &dst, matrices, 1, false);

    dstPositions->readData(0, sizeof(positions), positions);
    EXPECT_EQ(positions[1][0], 1);
}

TEST_F(MeshTests, PoseNormalsFromVertexStart)
{
    VertexData vertexData;
    vertexData.vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_NORMAL);
    auto vertexBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(float) * 3, 3, HBU_CPU_TO_GPU);
    float normals[3][3] = {{0, 0, 0}, {0, 0, 0}, {1, 1, 1}};
    vertexBuffer->writeData(0, sizeof(normals), normals, true);
    vertexData.vertexBufferBinding->setBinding(0, vertexBuffer);
    vertexData.vertexStart = 1;
    vertexData.vertexCount = 2;

    Pose pose(0);
    pose.addVertex(0, Vector3f(0, 0, 0), Vector3f(0, 0, 1));

    float poseData[2][6];
    pose._getHardwareVertexBuffer(&vertexData)->readData(0, sizeof(poseData), poseData);
    EXPECT_EQ(poseData[1][3], 1);
}

TEST_F(MeshTests, EntityAfterMeshUnload)
{
    auto sceneMgr = mRoot->createSceneManager();
    auto entity = sceneMgr->createEntity("ogrehead.mesh");

    entity->getMesh()->unload();
    EXPECT_EQ(entity->getNumSubEntities(), 0u);

    entity->getMesh()->load();
    EXPECT_GT(entity->getNumSubEntities(), 0u);
}

TEST_F(MeshTests, CurvedPlaneOddSegmentsCentred)
{
    // single segment: the 4 corners are equidistant from the centre, so equally bent
    MeshPtr mesh = MeshManager::getSingleton().createCurvedPlane(
        "CurvedPlane", RGN_DEFAULT, Plane(Vector3::UNIT_Z, 0), 2, 2, 1, 1, 1, false, 0);

    float pos[4][3];
    mesh->sharedVertexData->vertexBufferBinding->getBuffer(0)->readData(0, sizeof(pos), pos);
    for (auto& p : pos)
        EXPECT_FLOAT_EQ(p[2], pos[0][2]);
}

typedef RootWithoutRenderSystemFixture SubMeshTests;
TEST_F(SubMeshTests, ExtremesFromSharedVertices)
{
    MeshPtr mesh = MeshManager::getSingleton().createManual("ExtremesSharedVertices", RGN_DEFAULT);
    mesh->sharedVertexData = new VertexData();
    mesh->sharedVertexData->vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
    auto vertexBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(float) * 3, 3, HBU_CPU_TO_GPU);
    float positions[3][3] = {{9, 9, 9}, {0, 0, 0}, {1, 1, 0}};
    vertexBuffer->writeData(0, sizeof(positions), positions, true);
    mesh->sharedVertexData->vertexBufferBinding->setBinding(0, vertexBuffer);
    mesh->sharedVertexData->vertexStart = 1;
    mesh->sharedVertexData->vertexCount = 2;

    SubMesh* subMesh = mesh->createSubMesh(); // shared vertices, no indices
    subMesh->generateExtremes(2);
    EXPECT_EQ(subMesh->extremityPoints, std::vector<Vector3>({Vector3(0, 0, 0), Vector3(1, 1, 0)}));
}

TEST_F(MeshSerializerBufferRange, ExportsUsedRange)
{
    MeshPtr mesh = MeshManager::getSingleton().createManual("BufferRangeSrc", RGN_DEFAULT);
    SubMesh* subMesh = mesh->createSubMesh();
    subMesh->useSharedVertices = false;

    auto vertexData = new VertexData();
    vertexData->vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);

    auto vertexBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
        vertexData->vertexDeclaration->getVertexSize(0), 7, HBU_CPU_TO_GPU);
    float positions[7][3] = {
        {9, 9, 9}, {9, 9, 9}, {9, 9, 9}, {9, 9, 9},
        {0, 0, 0}, {1, 0, 0}, {0, 1, 0},
    };
    vertexBuffer->writeData(0, sizeof(positions), positions, true);
    vertexData->vertexBufferBinding->setBinding(0, vertexBuffer);
    vertexData->vertexStart = 4;
    vertexData->vertexCount = 3;
    subMesh->resetVertexData(vertexData);

    auto indexData = new IndexData();
    indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
        HardwareIndexBuffer::IT_16BIT, 5, HBU_CPU_TO_GPU);
    uint16 indices[] = {7, 7, 0, 1, 2};
    indexData->indexBuffer->writeData(0, sizeof(indices), indices, true);
    indexData->indexStart = 2;
    indexData->indexCount = 3;
    subMesh->indexData = indexData;

    mesh->_setBounds(AxisAlignedBox(Vector3(0, 0, 0), Vector3(1, 1, 0)));
    mesh->load();

    MeshSerializer serializer;
    std::vector<char> buffer(4096);
    auto stream = std::make_shared<MemoryDataStream>("BufferRange.mesh", buffer.data(), buffer.size(),
                                                     false, false);
    serializer.exportMesh(mesh.get(), stream);

    auto readStream = std::make_shared<MemoryDataStream>("BufferRange.mesh", buffer.data(), stream->tell(),
                                                         false, true);
    MeshPtr imported = MeshManager::getSingleton().createManual("BufferRangeDst", RGN_DEFAULT);
    serializer.importMesh(readStream, imported.get());

    SubMesh* importedSubMesh = imported->getSubMesh(0);
    ASSERT_TRUE(importedSubMesh->vertexData);
    EXPECT_EQ(importedSubMesh->vertexData->vertexStart, 0u);
    EXPECT_EQ(importedSubMesh->vertexData->vertexCount, 3u);
    ASSERT_TRUE(importedSubMesh->indexData);
    EXPECT_EQ(importedSubMesh->indexData->indexStart, 0u);
    EXPECT_EQ(importedSubMesh->indexData->indexCount, 3u);

    auto importedVertexBuffer = importedSubMesh->vertexData->vertexBufferBinding->getBuffer(0);
    HardwareBufferLockGuard vertexLock(importedVertexBuffer, HardwareBuffer::HBL_READ_ONLY);
    const float* importedPositions = static_cast<const float*>(vertexLock.pData);
    for (size_t i = 0; i < 3; ++i)
        EXPECT_EQ(Vector3(importedPositions + i * 3), Vector3(positions[4 + i]));

    auto importedIndexBuffer = importedSubMesh->indexData->indexBuffer;
    HardwareBufferLockGuard indexLock(importedIndexBuffer, HardwareBuffer::HBL_READ_ONLY);
    const uint16* importedIndices = static_cast<const uint16*>(indexLock.pData);
    for (size_t i = 0; i < 3; ++i)
        EXPECT_EQ(importedIndices[i], uint16(i));

    MeshManager::getSingleton().remove(imported);
    MeshManager::getSingleton().remove(mesh);
}

namespace
{
/// a triangle whose index data and generated LODs all live in one shared index buffer
MeshPtr createSharedLodMesh(const String& name, const std::vector<std::pair<size_t, size_t>>& lodRanges)
{
    MeshPtr mesh = MeshManager::getSingleton().createManual(name, RGN_DEFAULT);
    SubMesh* subMesh = mesh->createSubMesh();
    subMesh->useSharedVertices = false;

    auto vertexData = new VertexData();
    vertexData->vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
    auto vertexBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
        vertexData->vertexDeclaration->getVertexSize(0), 3, HBU_CPU_TO_GPU);
    float positions[3][3] = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
    vertexBuffer->writeData(0, sizeof(positions), positions, true);
    vertexData->vertexBufferBinding->setBinding(0, vertexBuffer);
    vertexData->vertexCount = 3;
    subMesh->resetVertexData(vertexData);

    // 99 marks indices that belong to someone else and must not be exported
    std::vector<uint16> indices(32, 99);
    indices[0] = 0, indices[1] = 1, indices[2] = 2;
    for (const auto& range : lodRanges)
    {
        for (size_t i = 0; i < range.second; ++i)
            indices[range.first + i] = uint16((range.first + i) % 3);
    }

    auto indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
        HardwareIndexBuffer::IT_16BIT, indices.size(), HBU_CPU_TO_GPU);
    indexBuffer->writeData(0, indexBuffer->getSizeInBytes(), indices.data(), true);

    subMesh->indexData->indexBuffer = indexBuffer;
    subMesh->indexData->indexStart = 0;
    subMesh->indexData->indexCount = 3;

    mesh->_setLodInfo(ushort(lodRanges.size() + 1));
    for (size_t lod = 0; lod < lodRanges.size(); ++lod)
    {
        MeshLodUsage usage;
        usage.userValue = Real(100 * (lod + 1));
        mesh->_setLodUsage(ushort(lod + 1), usage);

        auto lodData = new IndexData();
        lodData->indexBuffer = indexBuffer;
        lodData->indexStart = lodRanges[lod].first;
        lodData->indexCount = lodRanges[lod].second;
        subMesh->mLodFaceList[lod] = lodData;
    }

    mesh->_setBounds(AxisAlignedBox(Vector3(0, 0, 0), Vector3(1, 1, 0)));
    mesh->load();
    return mesh;
}

MeshPtr exportAndImport(const MeshPtr& mesh, const String& name, size_t* exportedSize = nullptr)
{
    MeshSerializer serializer;
    std::vector<char> buffer(4096);
    auto stream = std::make_shared<MemoryDataStream>(name, buffer.data(), buffer.size(), false, false);
    serializer.exportMesh(mesh.get(), stream);
    if (exportedSize)
        *exportedSize = stream->tell();

    auto readStream = std::make_shared<MemoryDataStream>(name, buffer.data(), stream->tell(), false, true);
    MeshPtr imported = MeshManager::getSingleton().createManual(name, RGN_DEFAULT);
    serializer.importMesh(readStream, imported.get());
    return imported;
}

std::vector<uint16> readLodIndices(const IndexData* indexData)
{
    HardwareBufferLockGuard lock(indexData->indexBuffer, indexData->indexStart * sizeof(uint16),
                                 indexData->indexCount * sizeof(uint16), HardwareBuffer::HBL_READ_ONLY);
    const uint16* data = static_cast<const uint16*>(lock.pData);
    return std::vector<uint16>(data, data + indexData->indexCount);
}
} // namespace

TEST_F(MeshSerializerBufferRange, ExportsUsedLodRanges)
{
    // disjoint LOD ranges in a buffer shared with other geometry, as in a GeometryPool page
    MeshPtr mesh = createSharedLodMesh("LodRangeSrc", {{10, 3}, {20, 3}});
    MeshPtr imported = exportAndImport(mesh, "LodRangeDst");

    SubMesh* subMesh = imported->getSubMesh(0);
    ASSERT_EQ(imported->getNumLodLevels(), 3);
    ASSERT_EQ(subMesh->mLodFaceList.size(), 2u);

    for (size_t lod = 0; lod < 2; ++lod)
    {
        const IndexData* lodData = subMesh->mLodFaceList[lod];
        EXPECT_EQ(lodData->indexBuffer->getNumIndexes(), 3u) << "LOD " << lod + 1;
        EXPECT_EQ(lodData->indexStart, 0u);
        EXPECT_EQ(readLodIndices(lodData), readLodIndices(mesh->getSubMesh(0)->mLodFaceList[lod]));
    }
    EXPECT_NE(subMesh->mLodFaceList[0]->indexBuffer, subMesh->mLodFaceList[1]->indexBuffer);

    MeshManager::getSingleton().remove(imported);
    MeshManager::getSingleton().remove(mesh);
}

TEST_F(MeshSerializerBufferRange, SharesOverlappingLodRanges)
{
    // overlapping ranges, as written by the compressed LOD generator, keep sharing one buffer
    MeshPtr mesh = createSharedLodMesh("LodOverlapSrc", {{4, 6}, {7, 6}});
    MeshPtr imported = exportAndImport(mesh, "LodOverlapDst");

    SubMesh* subMesh = imported->getSubMesh(0);
    ASSERT_EQ(subMesh->mLodFaceList.size(), 2u);

    const IndexData* lod1 = subMesh->mLodFaceList[0];
    const IndexData* lod2 = subMesh->mLodFaceList[1];
    EXPECT_EQ(lod1->indexBuffer, lod2->indexBuffer);
    EXPECT_EQ(lod1->indexBuffer->getNumIndexes(), 9u);
    EXPECT_EQ(lod1->indexStart, 0u);
    EXPECT_EQ(lod2->indexStart, 3u);
    EXPECT_EQ(readLodIndices(lod1), readLodIndices(mesh->getSubMesh(0)->mLodFaceList[0]));
    EXPECT_EQ(readLodIndices(lod2), readLodIndices(mesh->getSubMesh(0)->mLodFaceList[1]));

    MeshManager::getSingleton().remove(imported);
    MeshManager::getSingleton().remove(mesh);
}

TEST(Image, FlipV)
{
    ResourceGroupManager mgr;
    STBIImageCodec::startup();
    ConfigFile cf;
    cf.load(FileSystemLayer(OGRE_VERSION_NAME).getConfigFilePath("resources.cfg"));
    auto testPath = cf.getSettings("Tests").begin()->second;

    Image ref;
    ref.load(Root::openFileStream(testPath+"/decal1vflip.png"), "png");

    Image img;
    img.load(Root::openFileStream(testPath+"/decal1.png"), "png");
    img.flipAroundX();

    // img.save(testPath+"/decal1vflip.png");

    STBIImageCodec::shutdown();
    ASSERT_TRUE(!memcmp(img.getData(), ref.getData(), ref.getSize()));
}

TEST(Image, FlipAllFacesSlicesAndMipmaps)
{
    // 8 bytes per pixel: a cubemap with mipmaps and a volume
    Image cube, volume;
    cube.create(PF_FLOAT16_RGBA, 2, 2, 1, 6, 1);
    volume.create(PF_FLOAT16_RGBA, 2, 2, 2);

    for (Image* src : {&cube, &volume})
    {
        for (uint32 face = 0; face < src->getNumFaces(); face++)
            for (uint32 mip = 0; mip <= src->getNumMipmaps(); mip++)
            {
                PixelBox box = src->getPixelBox(face, mip);
                for (uint32 z = 0; z < box.getDepth(); z++)
                    for (uint32 y = 0; y < box.getHeight(); y++)
                        for (uint32 x = 0; x < box.getWidth(); x++)
                            box.setColourAt(ColourValue(x, y, z, face * 2 + mip), x, y, z);
            }

        for (bool vertical : {false, true})
        {
            Image img = *src;
            vertical ? img.flipAroundX() : img.flipAroundY();
            ASSERT_EQ(img.getNumMipmaps(), src->getNumMipmaps());

            for (uint32 face = 0; face < img.getNumFaces(); face++)
                for (uint32 mip = 0; mip <= img.getNumMipmaps(); mip++)
                {
                    PixelBox box = img.getPixelBox(face, mip);
                    uint32 w = box.getWidth(), h = box.getHeight();
                    for (uint32 z = 0; z < box.getDepth(); z++)
                        for (uint32 y = 0; y < h; y++)
                            for (uint32 x = 0; x < w; x++)
                                EXPECT_EQ(box.getColourAt(x, y, z),
                                          ColourValue(vertical ? x : w - 1 - x, vertical ? h - 1 - y : y, z,
                                                      face * 2 + mip));
                }
        }
    }
}

TEST(Image, Resize)
{
    ResourceGroupManager mgr;
    STBIImageCodec::startup();
    ConfigFile cf;
    cf.load(FileSystemLayer(OGRE_VERSION_NAME).getConfigFilePath("resources.cfg"));
    auto testPath = cf.getSettings("Tests").begin()->second;

    Image ref;
    ref.load(Root::openFileStream(testPath+"/decal1small.png"), "png");

    Image img;
    img.load(Root::openFileStream(testPath+"/decal1.png"), "png");
    img.resize(128, 128);

    //img.save(testPath+"/decal1small.png");

    STBIImageCodec::shutdown();
    ASSERT_TRUE(!memcmp(img.getData(), ref.getData(), ref.getSize()));
}


TEST(Image, Combine)
{
    ResourceGroupManager mgr;
    FileSystemArchiveFactory fs;
    ArchiveManager amgr;
    amgr.addArchiveFactory(&fs);
    STBIImageCodec::startup();
    ConfigFile cf;
    cf.load(FileSystemLayer(OGRE_VERSION_NAME).getConfigFilePath("resources.cfg"));
    mgr.addResourceLocation(cf.getSettings("General").begin()->second+"/../materials/textures", fs.getType());
    mgr.initialiseAllResourceGroups();

    auto testPath = cf.getSettings("Tests").begin()->second;
    Image ref;
    ref.load(Root::openFileStream(testPath+"/rockwall_flare.png"), "png");

    Image combined;
    // pick 2 files that are the same size, alpha texture will be made greyscale
    combined.loadTwoImagesAsRGBA("rockwall.tga", "flare.png", RGN_DEFAULT, PF_BYTE_RGBA);

    // combined.save(testPath+"/rockwall_flare.png");
    STBIImageCodec::shutdown();
    ASSERT_TRUE(!memcmp(combined.getData(), ref.getData(), ref.getSize()));
}

TEST(Image, Compressed)
{
    Root root;
    ConfigFile cf;
    cf.load(FileSystemLayer(OGRE_VERSION_NAME).getConfigFilePath("resources.cfg"));
    auto testPath = cf.getSettings("Tests").begin()->second;

    Image img;
#if OGRE_NO_PVRTC_CODEC == 0
    // 2bpp
    img.load(Root::openFileStream(testPath+"/ogreborderUp_pvr2.pvr"), "pvr");
    EXPECT_EQ(img.getFormat(), PF_PVRTC_RGB2);
    // 2bpp alpha
    img.load(Root::openFileStream(testPath+"/ogreborderUp_pvr2a.pvr"), "pvr");
    EXPECT_EQ(img.getFormat(), PF_PVRTC_RGBA2);
    // 4bpp
    img.load(Root::openFileStream(testPath+"/ogreborderUp_pvr4.pvr"), "pvr");
    EXPECT_EQ(img.getFormat(), PF_PVRTC_RGB4);
    // 4 bpp alpha
    img.load(Root::openFileStream(testPath+"/ogreborderUp_pvr4a.pvr"), "pvr");
    EXPECT_EQ(img.getFormat(), PF_PVRTC_RGBA4);
#endif

#if OGRE_NO_ETC_CODEC == 0
    img.load(Root::openFileStream(testPath+"/Texture.pkm"), "pkm");
    EXPECT_EQ(img.getFormat(), PF_ETC2_RGB8);
    img.load(Root::openFileStream(testPath+"/etc2-rgba8.ktx"), "ktx");
    EXPECT_EQ(img.getFormat(), PF_ETC2_RGBA8);
#endif

#if OGRE_NO_ASTC_CODEC == 0
    img.load(Root::openFileStream(testPath+"/Earth-Color10x6.astc"), "astc");
    EXPECT_EQ(img.getFormat(), PF_ASTC_RGBA_10X6_LDR);
#endif

#if OGRE_NO_DDS_CODEC == 0
    img.load(Root::openFileStream(testPath+"/ogreborderUp_dxt3.dds"), "dds");
    EXPECT_EQ(img.getFormat(), PF_BYTE_RGBA); // no RenderSystem available, will decompress
#endif
}

#if OGRE_NO_PVRTC_CODEC == 0
TEST(Image, PVRTextureArray)
{
    Root root;

    // PVR3 header for a single 8x8 PVRTC 4bpp surface, but claiming two array members.
    // The zeroed tail is 32 bytes of pixel data for each of them.
    uint32 pvr[13 + 2 * 8] = {0x03525650, 0, 2, 0, 0, 0, 8, 8, 1, 2, 1, 0, 0};

    Image img;
    EXPECT_THROW(img.load(DataStreamPtr(new MemoryDataStream(pvr, sizeof(pvr))), "pvr"), InvalidParametersException);
}
#endif

#if OGRE_NO_DDS_CODEC == 0
TEST(Image, DDSTruncated)
{
    Root root;

    // "DDS " + header for a 2x2 RGBA8 image, followed by its 16 bytes of pixel data
    uint32 dds[32 + 4] = {0x20534444, 124, 0x1007, 2, 2};
    uint32 rgba8[] = {32, 0x41, 0, 32, 0xff, 0xff00, 0xff0000, 0xff000000, 0x1000};
    std::copy(rgba8, rgba8 + 9, dds + 19);

    auto load = [&](size_t size) {
        Image img;
        img.load(DataStreamPtr(new MemoryDataStream(dds, size)), "dds");
    };

    EXPECT_NO_THROW(load(sizeof(dds)));
    EXPECT_THROW(load(sizeof(dds) - 1), InvalidParametersException); // pixel data cut short
    EXPECT_THROW(load(127), InvalidParametersException);             // header cut short

    // 8x8 DXT1 needs 4 blocks, only provide one. No RenderSystem, so it gets decompressed
    dds[3] = dds[4] = 8;
    dds[20] = 0x4;        // DDPF_FOURCC
    dds[21] = 0x31545844; // DXT1
    EXPECT_THROW(load(sizeof(dds) - 8), InvalidParametersException);
}
#endif

struct UsePreviousResourceLoadingListener : public ResourceLoadingListener
{
    bool resourceCollision(Resource *resource, ResourceManager *resourceManager) override { return false; }
};

typedef RootWithoutRenderSystemFixture ResourceLoading;
TEST_F(ResourceLoading, CollsionUseExisting)
{
    UsePreviousResourceLoadingListener listener;
    ResourceGroupManager::getSingleton().setLoadingListener(&listener);

    MaterialPtr mat = MaterialManager::getSingleton().create("Collision", "Tests");
    EXPECT_TRUE(mat);
    EXPECT_FALSE(MaterialManager::getSingleton().create("Collision", "Tests"));
    EXPECT_FALSE(mat->clone("Collision"));

    MeshPtr mesh = MeshManager::getSingleton().create("Collision", "Tests");
    EXPECT_TRUE(mesh);
    EXPECT_FALSE(MeshManager::getSingleton().create("Collision", "Tests"));
    EXPECT_FALSE(mesh->clone("Collision"));

    EXPECT_TRUE(SkeletonManager::getSingleton().create("Collision", "Tests"));
    EXPECT_FALSE(SkeletonManager::getSingleton().create("Collision", "Tests"));

    EXPECT_TRUE(CompositorManager::getSingleton().create("Collision", "Tests"));
    EXPECT_FALSE(CompositorManager::getSingleton().create("Collision", "Tests"));

    EXPECT_TRUE(HighLevelGpuProgramManager::getSingleton().createProgram(
        "Collision", "Tests", "null", GPT_VERTEX_PROGRAM));
    EXPECT_FALSE(HighLevelGpuProgramManager::getSingleton().createProgram(
        "Collision", "Tests", "null", GPT_VERTEX_PROGRAM));
}

struct DeletePreviousResourceLoadingListener : public ResourceLoadingListener
{
    bool resourceCollision(Resource* resource, ResourceManager* resourceManager) override
    {
        resourceManager->remove(resource->getName(), resource->getGroup());
        return true;
    }
};

TEST_F(ResourceLoading, CollsionDeleteExisting)
{
    DeletePreviousResourceLoadingListener listener;
    ResourceGroupManager::getSingleton().setLoadingListener(&listener);
    ResourceGroupManager::getSingleton().createResourceGroup("EmptyGroup", false);

    MaterialPtr mat = MaterialManager::getSingleton().create("Collision", "EmptyGroup");
    EXPECT_TRUE(mat);
    EXPECT_TRUE(MaterialManager::getSingleton().create("Collision", "EmptyGroup"));
    EXPECT_TRUE(mat->clone("Collision"));
}

typedef RootWithoutRenderSystemFixture TextureTests;
TEST_F(TextureTests, Blank)
{
    auto mat = std::make_shared<Material>(nullptr, "Material Name", 0, "Group");
    auto tus = mat->createTechnique()->createPass()->createTextureUnitState();

    EXPECT_EQ(tus->isBlank(), true);
    EXPECT_EQ(tus->getTextureName(), "");
    EXPECT_EQ(tus->getTextureType(), TEX_TYPE_2D);
    EXPECT_EQ(tus->getNumMipmaps(), MIP_DEFAULT);
    EXPECT_EQ(tus->getDesiredFormat(), PF_UNKNOWN);
    EXPECT_EQ(tus->getFrameTextureName(0), "");
    EXPECT_EQ(tus->getGamma(), 1.0f);
    EXPECT_EQ(tus->isHardwareGammaEnabled(), false);
}

TEST(GpuSharedParameters, align)
{
    Root root("");
    GpuSharedParameters params("dummy");

    // trivial case
    params.addConstantDefinition("a", GCT_FLOAT1);
    EXPECT_EQ(params.getConstantDefinition("a").logicalIndex, 0);

    // 16 byte alignment
    params.addConstantDefinition("b", GCT_FLOAT4);
    EXPECT_EQ(params.getConstantDefinition("b").logicalIndex, 16);

    // break alignment again
    params.addConstantDefinition("c", GCT_FLOAT1);
    EXPECT_EQ(params.getConstantDefinition("c").logicalIndex, 32);

    // 16 byte alignment
    params.addConstantDefinition("d", GCT_MATRIX_4X4);
    EXPECT_EQ(params.getConstantDefinition("d").logicalIndex, 48);
}

typedef RootWithoutRenderSystemFixture HighLevelGpuProgramTest;
TEST_F(HighLevelGpuProgramTest, resolveIncludes)
{
    auto mat = MaterialManager::getSingleton().create("Dummy", RGN_DEFAULT);

    auto& rgm = ResourceGroupManager::getSingleton();
    rgm.addResourceLocation(".", "FileSystem", RGN_DEFAULT, false, false);

    // recursive inclusion
    String bar = "World";
    rgm.createResource("bar.cg", RGN_DEFAULT)->write(bar.c_str(), bar.size());
    String foo = "Hello\n#include <bar.cg>\n";
    rgm.createResource("foo.cg", RGN_DEFAULT)->write(foo.c_str(), foo.size());
    const char* src = "#include <foo.cg>";

    String res = HighLevelGpuProgram::_resolveIncludes(src, mat.get(), "main.cg", true);
    rgm.deleteResource("foo.cg", RGN_DEFAULT);
    rgm.deleteResource("bar.cg", RGN_DEFAULT);

    String ref = "#line 1  \"foo.cg\"\n"
                 "Hello\n"
                 "#line 1  \"bar.cg\"\n"
                 "World\n"
                 "#line 3 \"foo.cg\"";

    ASSERT_EQ(res.substr(0, ref.size()), ref);
}

TEST(Math, TriangleRayIntersection)
{
    Vector3 tri[3] = {{-1, 0, 0}, {1, 0, 0}, {0, 1, 0}};
    auto ray = Ray({0, 0.5, 1}, {0, 0, -1});

    EXPECT_TRUE(Math::intersects(ray, tri[0], tri[1], tri[2], true, true).first);
    EXPECT_TRUE(Math::intersects(ray, tri[0], tri[1], tri[2], true, false).first);
    EXPECT_FALSE(Math::intersects(ray, tri[0], tri[1], tri[2], false, true).first);
    EXPECT_FALSE(Math::intersects(ray, tri[0], tri[1], tri[2], false, false).first);

    ray = Ray({0, 0.5, -1}, {0, 0, 1});

    EXPECT_TRUE(Math::intersects(ray, tri[0], tri[1], tri[2], true, true).first);
    EXPECT_FALSE(Math::intersects(ray, tri[0], tri[1], tri[2], true, false).first);
    EXPECT_TRUE(Math::intersects(ray, tri[0], tri[1], tri[2], false, true).first);
    EXPECT_FALSE(Math::intersects(ray, tri[0], tri[1], tri[2], false, false).first);
}

TEST(Light, AnimableValue)
{
    Light l;

    l.setDiffuseColour(0, 0, 0);
    auto diffuseColour = l.createAnimableValue("diffuseColour");
    diffuseColour->applyDeltaValue(ColourValue(1, 2, 3, 0));
    EXPECT_EQ(l.getDiffuseColour(), ColourValue(1, 2, 3));

    l.setSpecularColour(0, 0, 0);
    auto specularColour = l.createAnimableValue("specularColour");
    specularColour->applyDeltaValue(ColourValue(1, 2, 3, 0));
    EXPECT_EQ(l.getSpecularColour(), ColourValue(1, 2, 3));

    l.setAttenuation(0, 0, 0, 0);
    auto attenuation = l.createAnimableValue("attenuation");
    attenuation->applyDeltaValue(Vector4(1, 2, 3, 4));
    EXPECT_EQ(l.getAttenuation(), Vector4f(1, 2, 3, 4));

    l.setSpotlightInnerAngle(Radian(0));
    auto spotlightInner = l.createAnimableValue("spotlightInner");
    spotlightInner->applyDeltaValue(Radian(1));
    EXPECT_EQ(l.getSpotlightInnerAngle(), Radian(1));

    l.setSpotlightOuterAngle(Radian(0));
    auto spotlightOuter = l.createAnimableValue("spotlightOuter");
    spotlightOuter->applyDeltaValue(Radian(1));
    EXPECT_EQ(l.getSpotlightOuterAngle(), Radian(1));

    l.setSpotlightFalloff(0);
    auto spotlightFalloff = l.createAnimableValue("spotlightFalloff");
    spotlightFalloff->applyDeltaValue(Real(1));
    EXPECT_EQ(l.getSpotlightFalloff(), 1);
}

TEST(Light, AnimationTrack)
{
    Light l;
    l.setDiffuseColour(0, 0, 0);
    l.setAttenuation(0, 0, 0, 0);

    Animation anim("test", 1.0);
    auto diffuse = anim.createNumericTrack(0, l.createAnimableValue("diffuseColour"));
    diffuse->createNumericKeyFrame(0)->setValue(ColourValue(1, 2, 3, 0));
    diffuse->createNumericKeyFrame(1)->setValue(ColourValue(2, 4, 6, 0));

    diffuse->apply(0.5);

    EXPECT_EQ(l.getDiffuseColour(), ColourValue(1.5, 3, 4.5));

    auto attenuation = anim.createNumericTrack(1, l.createAnimableValue("attenuation"));
    attenuation->createNumericKeyFrame(0)->setValue(Vector4(1, 2, 3, 4));
    attenuation->createNumericKeyFrame(1)->setValue(Vector4(2, 4, 6, 8));

    attenuation->apply(0.5);
    EXPECT_EQ(l.getAttenuation(), Vector4f(1.5, 3, 4.5, 6));
}

namespace
{
/// Minimal AnimableValue implementation for testing the numeric track helpers
class TestIntAnimableValue : public AnimableValue
{
public:
    TestIntAnimableValue(int& value) : AnimableValue(INT), mValue(value) {}
    void setCurrentStateAsBaseValue() override { setAsBaseValue(mValue); }
    void setValue(int val) override { mValue = val; }
    void applyDeltaValue(int val) override { mValue += val; }

private:
    int& mValue;
};
} // namespace

TEST(AnimableValue, IntAnimationTrack)
{
    int value = 0;
    auto animable = std::make_shared<TestIntAnimableValue>(value);

    Animation anim("test", 1.0);
    auto track = anim.createNumericTrack(0, animable);
    track->createNumericKeyFrame(0)->setValue(10);
    track->createNumericKeyFrame(1)->setValue(20);

    // scaled value must stay an int, otherwise applyDeltaValue() cannot cast it
    track->apply(0.5);
    EXPECT_EQ(value, 15);
}

TEST(GpuProgramParams, Variability)
{
    auto constants = std::make_shared<GpuNamedConstants>();
    constants->map["parameter"] = GpuConstantDefinition();
    constants->map["parameter"].constType = GCT_MATRIX_4X4;

    GpuProgramParameters params;
    params._setNamedConstants(constants);
    params.setNamedAutoConstant("parameter", GpuProgramParameters::ACT_WORLD_MATRIX);

    GpuProgramParameters params2;
    params2._setNamedConstants(constants);
    params2.clearNamedAutoConstant("parameter");

    EXPECT_EQ(params.getConstantDefinition("parameter").variability, GPV_PER_OBJECT);
}

TEST(Billboard, TextureCoords)
{
    Root root("");
    MaterialManager::getSingleton().initialise();

    float xsegs = 3;
    float ysegs = 3;
    BillboardSet bbs("name");
    bbs.setTextureStacksAndSlices(ysegs, xsegs);

    auto& texcoords = bbs.getTextureCoords();


    float width = 300;
    float height = 300;
    float gap = 20;

    for (int y = 0; y < ysegs; ++y)
    {
        for (int x = 0; x < xsegs; ++x)
        {
            FloatRect ref((x + 0) / xsegs, (ysegs - y - 1) / ysegs, // uv
                          (x + 1) / xsegs, (ysegs - y - 0) / ysegs);
            auto& genRect = texcoords[(ysegs - y - 1)*xsegs + x];
            EXPECT_EQ(genRect, ref);

            // only for visualisation
            Billboard* bb = bbs.createBillboard({(x * width / xsegs) + ((x - 1) * gap), // position
                                                 (y * height / ysegs) + ((y - 1) * gap), 0});
            bb->setTexcoordIndex((ysegs - y - 1)*xsegs + x);
        }
    }
}

struct ConstantRandom : public Math::RandomValueProvider
{
    Real value;
    explicit ConstantRandom(Real v) : value(v) {}
    Real getRandomUnit() override { return value; }
};

struct TestEmitter : public ParticleEmitter
{
    TestEmitter() : ParticleEmitter(NULL) {}
    using ParticleEmitter::genEmissionDirection;
};

TEST(ParticleEmitter, AngleAbove180EmitsInAllDirections)
{
    // the largest random value picks the largest deviation from the base direction
    ConstantRandom rand(1);
    Math::SetRandomValueProvider(&rand);

    TestEmitter emitter;
    emitter.setDirection(Vector3::UNIT_X);

    for (Real degrees : {180, 270, 360})
    {
        emitter.setAngle(Degree(degrees));

        Vector3 dir;
        emitter.genEmissionDirection(Vector3::ZERO, dir);
        EXPECT_TRUE(dir.positionEquals(Vector3::NEGATIVE_UNIT_X, 1e-4)) << degrees << " degrees: " << dir;
    }

    Math::SetRandomValueProvider(NULL);
}

typedef RootWithoutRenderSystemFixture ParticleSystemTests;
TEST_F(ParticleSystemTests, SelfAssignmentKeepsSettings)
{
    ParticleSystemManager::getSingleton()._initialise();
    ParticleSystem ps("ps", RGN_DEFAULT);
    ps.getRenderer()->setParameter("billboard_type", "oriented_self");

    ParticleSystem& alias = ps;
    ps = alias;

    EXPECT_EQ(ps.getRenderer()->getParameter("billboard_type"), "oriented_self");
}

TEST(ConfigFile, MissingKeyReturnsDefault)
{
    static char text[] = "b = 2\n";
    ConfigFile cf;
    cf.load(std::make_shared<MemoryDataStream>(text, sizeof(text) - 1));

    EXPECT_EQ(cf.getSetting("b"), "2");
    EXPECT_EQ(cf.getSetting("a", "", "default"), "default");
}

TEST(LinearControllerFunction, InputOnOrOutsideKeys)
{
    LinearControllerFunction f({0, 0.5, 1}, {10, 20, 40}, 1, false);

    EXPECT_FLOAT_EQ(f.calculate(0), 10);
    EXPECT_FLOAT_EQ(f.calculate(0.25), 15);
    EXPECT_FLOAT_EQ(f.calculate(0.5), 20);
    EXPECT_FLOAT_EQ(f.calculate(0.75), 30);
    EXPECT_FLOAT_EQ(f.calculate(1), 40);
}

TEST(GpuSharedParameters, LightArrayAutoConstants)
{
    Light red, green;
    red.setDiffuseColour(ColourValue::Red);
    green.setDiffuseColour(ColourValue::Green);
    LightList lights{&red, &green};
    AutoParamDataSource source;
    source.setCurrentLightList(&lights);

    GpuSharedParameters params("lights");
    params.setUseLinearColours(true);
    params.addConstantDefinition("padding", GCT_FLOAT4);
    params.addConstantDefinition("colours", GCT_FLOAT4, 2);
    params.setAutoConstant("colours", GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED_ARRAY);
    params._updateAutoParams(&source);

    const float* colours = params.getFloatPointer(params.getConstantDefinition("colours").physicalIndex);
    EXPECT_EQ(Vector4f(colours), Vector4f(1, 0, 0, 1));
    EXPECT_EQ(Vector4f(colours + 4), Vector4f(0, 1, 0, 1));
}

TEST(AutoParamDataSource, NeutralFogWithoutFog)
{
    // shaders that apply fog whatever the mode must leave the colour as is under FOG_NONE
    AutoParamDataSource source;
    source.setFog(FOG_NONE, ColourValue::Black, 0.5, 0, 1);
    const Vector4f& p = source.getFogParams();
    for (float d : {0.0f, 0.001f, 1.0f, 100.0f, 1e6f})
    {
        EXPECT_GE((p[2] - d) * p[3], 1.0f); // linear
        EXPECT_EQ(std::exp(-d * p[0]), 1.0f); // exp
    }

    source.setFog(FOG_LINEAR, ColourValue::Black, 0.5, 10, 20);
    EXPECT_EQ(source.getFogParams(), Vector4f(0.5, 10, 20, 0.1));
}

TEST_F(SceneNodeTest, detachObjectFromBone)
{
    auto ent = mSceneMgr->createEntity("sinbad", "Sinbad.mesh");
    auto sword = mSceneMgr->createLight("sword");
    auto shield = mSceneMgr->createLight("shield");
    ent->attachObjectToBone("Handle.L", sword);
    ent->attachObjectToBone("Handle.R", shield);

    EXPECT_EQ(ent->detachObjectFromBone("sword"), sword);
    EXPECT_FALSE(sword->isAttached());
    EXPECT_TRUE(shield->isAttached());
}

TEST_F(SceneNodeTest, attachDuplicateName)
{
    auto light = mSceneMgr->createLight("obj");
    auto camera = mSceneMgr->createCamera("obj");
    SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    node->attachObject(light);
    EXPECT_THROW(node->attachObject(camera), Exception);
    EXPECT_FALSE(camera->isAttached());
    mSceneMgr->destroyCamera(camera);
}

TEST(GpuProgramParams, ShadowSceneDepthRangeArray)
{
    Root root("");
    AutoParamDataSource source;
    source.setCurrentSceneManager(root.createSceneManager());

    auto constants = std::make_shared<GpuNamedConstants>();
    auto& def = constants->map["ranges"];
    def.constType = GCT_FLOAT4;
    def.physicalIndex = 0;
    def.elementSize = 4;
    def.arraySize = 2;
    constants->bufferSize = 8;

    GpuProgramParameters params;
    params._setNamedConstants(constants);
    params.setNamedAutoConstant("ranges", GpuProgramParameters::ACT_SHADOW_SCENE_DEPTH_RANGE_ARRAY, 2);
    params._updateAutoParams(&source, GPV_ALL);

    const float* ranges = params.getFloatPointer(0);
    EXPECT_EQ(Vector4f(ranges), Vector4f(source.getShadowSceneDepthRange(0).ptr()));
    EXPECT_EQ(Vector4f(ranges + 4), Vector4f(source.getShadowSceneDepthRange(1).ptr()));
}
