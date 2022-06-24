#include "RootWithoutRenderSystemFixture.h"

#include "OgreLodConfig.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreVertexIndexData.h"
#include "OgreEdgeListBuilder.h"
#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreSubMesh.h"
#include "OgreMeshSerializer.h"
#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreArchive.h"
#include "OgreArchiveManager.h"
#include "OgreFileSystem.h"
#include "OgreConfigFile.h"
#include "OgreMeshLodGenerator.h"
#include "OgrePixelCountLodStrategy.h"
#include "OgreLodCollapseCostQuadric.h"
#include "OgreRenderWindow.h"
#include "OgreLodConfigSerializer.h"
#include "OgreWorkQueue.h"

using namespace Ogre;

class MeshLodTests : public RootWithoutRenderSystemFixture
{
public:
    MeshPtr mMesh;

    void SetUp() override;
    void TearDown() override;
    void runMeshLodConfigTests(LodConfig::Advanced& advanced);
    void blockedWaitForLodGeneration(const MeshPtr& mesh);
    void addProfile(LodConfig& config);
    void setTestLodConfig(LodConfig& config);
};

//--------------------------------------------------------------------------
void MeshLodTests::SetUp()
{
    RootWithoutRenderSystemFixture::SetUp();

    new MeshLodGenerator;
    // Create the mesh for testing
    mMesh = MeshManager::getSingleton().load("Sinbad.mesh", ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
}
//--------------------------------------------------------------------------
void MeshLodTests::TearDown()
{
    if (mMesh) {
        mMesh->unload();
        mMesh.reset();
    }
    OGRE_DELETE MeshLodGenerator::getSingletonPtr();
    RootWithoutRenderSystemFixture::TearDown();
}
//--------------------------------------------------------------------------
void MeshLodTests::addProfile(LodConfig& config)
{
    // Get the first two vertices and put the edge into the profile
    // It doesn't matter if there is no such edge, because edges are removed and created dynamically.
    // The vertex positions should exist or you get an assert.
    VertexData* vertexData = config.mesh->getSubMesh(0)->vertexData;
    const VertexElement* elemPos = vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
    HardwareVertexBufferSharedPtr vbuf = vertexData->vertexBufferBinding->getBuffer(elemPos->getSource());
    assert(vbuf->getNumVertices() > 2);
    unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(HardwareBuffer::HBL_READ_ONLY));
    float* pFloat;
    elemPos->baseVertexPointerToElement(vertex, &pFloat);
    ProfiledEdge edge;
    edge.src.x = *pFloat++;
    edge.src.y = *pFloat++;
    edge.src.z = *pFloat;
    vertex += vbuf->getVertexSize();
    elemPos->baseVertexPointerToElement(vertex, &pFloat);
    edge.dst.x = *pFloat++;
    edge.dst.y = *pFloat++;
    edge.dst.z = *pFloat;
    edge.cost = LodData::NEVER_COLLAPSE_COST;
    config.advanced.profile.push_back(edge);
    vbuf->unlock();
}
//--------------------------------------------------------------------------
TEST_F(MeshLodTests,MeshLodGenerator)
{
    LodConfig config;
    setTestLodConfig(config);

    MeshLodGenerator& gen = MeshLodGenerator::getSingleton();
    gen.generateLodLevels(config);
    addProfile(config);
    config.advanced.useBackgroundQueue = false;
    config.advanced.useCompression = false;
    config.advanced.useVertexNormals = false;
    gen.generateLodLevels(config);

    LodConfig config2(config);
    config2.advanced.useBackgroundQueue = true;
    config2.mesh->removeLodLevels();
    gen.generateLodLevels(config);
    blockedWaitForLodGeneration(config.mesh);
    EXPECT_TRUE(config.levels.size() == config2.levels.size());
    for (size_t i = 0; i < config.levels.size(); i++) 
    {
        EXPECT_TRUE(config.levels[i].outSkipped == config2.levels[i].outSkipped);
        EXPECT_TRUE(config.levels[i].outUniqueVertexCount == config2.levels[i].outUniqueVertexCount);
    }
}
//--------------------------------------------------------------------------
void MeshLodTests::blockedWaitForLodGeneration(const MeshPtr& mesh)
{
    bool success = false;
    const int timeout = 5000;
    WorkQueue* wq = Root::getSingleton().getWorkQueue();
    for (int i = 0; i < timeout; i++) 
    {
        OGRE_THREAD_SLEEP(1);
        wq->processMainThreadTasks(); // Injects the Lod if ready
        if (mesh->getNumLodLevels() != 1) {
            success = true;
            break;
        }
    }
    // timeout
    EXPECT_TRUE(success);
}
//--------------------------------------------------------------------------
TEST_F(MeshLodTests,LodConfigSerializer)
{
    LodConfig config, config2;
    setTestLodConfig(config);
    addProfile(config);
    LodConfigSerializer serializer;
    serializer.exportLodConfig(config, "testLodConfigSerializer.lodconfig");
    serializer.importLodConfig(&config2, "testLodConfigSerializer.lodconfig");
    EXPECT_EQ(config.mesh->getHandle(), config2.mesh->getHandle());
    EXPECT_EQ(config.strategy, config2.strategy);
    EXPECT_EQ(config.advanced.outsideWalkAngle, config.advanced.outsideWalkAngle);
    EXPECT_EQ(config.advanced.outsideWeight, config.advanced.outsideWeight);
    EXPECT_EQ(config.advanced.useBackgroundQueue, config.advanced.useBackgroundQueue);
    EXPECT_EQ(config.advanced.useCompression, config.advanced.useCompression);
    EXPECT_EQ(config.advanced.useVertexNormals, config.advanced.useVertexNormals);

    {
        // Compare profiles
        LodProfile& p1 = config.advanced.profile;
        LodProfile& p2 = config2.advanced.profile;

        ASSERT_EQ(p1.size(), p2.size());
        for (size_t i = 0; i < p1.size(); i++)
        {
            EXPECT_EQ(p1[i].src, p2[i].src);
            EXPECT_EQ(p1[i].dst, p2[i].dst);
            EXPECT_FLOAT_EQ(p1[i].cost, p2[i].cost);
        }
    }

    {
        // Compare Lod Levels
        LodConfig::LodLevelList& l1 = config.levels;
        LodConfig::LodLevelList& l2 = config2.levels;

        ASSERT_EQ(l1.size(), l2.size());
        for (size_t i = 0; i < l1.size(); i++)
        {
            EXPECT_EQ(l1[i].distance , l2[i].distance);
            EXPECT_EQ(l1[i].manualMeshName, l2[i].manualMeshName);
            EXPECT_EQ(l1[i].reductionMethod, l2[i].reductionMethod);
            EXPECT_FLOAT_EQ(l1[i].reductionValue, l2[i].reductionValue);
        }
    }
}
//--------------------------------------------------------------------------
TEST_F(MeshLodTests,ManualLodLevels)
{
    MeshLodGenerator& gen = MeshLodGenerator::getSingleton();
    LodConfig config;
    setTestLodConfig(config);
    gen.generateLodLevels(config, LodCollapseCostPtr(new LodCollapseCostQuadric()));
}
//--------------------------------------------------------------------------
TEST_F(MeshLodTests,QuadricError)
{
    LodConfig config;
    setTestLodConfig(config);
    MeshLodGenerator& gen = MeshLodGenerator::getSingleton();
    gen.generateLodLevels(config, LodCollapseCostPtr(new LodCollapseCostQuadric()));
}
//--------------------------------------------------------------------------
void MeshLodTests::setTestLodConfig(LodConfig& config)
{
    config.mesh = mMesh;
    config.strategy = PixelCountLodStrategy::getSingletonPtr();
    config.levels.clear();
    config.createGeneratedLodLevel(10, 0.1);
    config.createGeneratedLodLevel(9, 0.2);
    config.createGeneratedLodLevel(8, 0.3);
    config.advanced.outsideWeight = 1.0;
    config.advanced.useCompression = true;
    config.advanced.useVertexNormals = true;
    config.advanced.useBackgroundQueue = false;
}
//--------------------------------------------------------------------------
