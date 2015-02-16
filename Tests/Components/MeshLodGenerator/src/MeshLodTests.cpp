#include "MeshLodTests.h"
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

#include "UnitTestSuite.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include "macUtils.h"
#endif

// Register the test suite
CPPUNIT_TEST_SUITE_REGISTRATION(MeshLodTests);

//--------------------------------------------------------------------------
void MeshLodTests::setUp()
{
    mFSLayer = OGRE_NEW_T(Ogre::FileSystemLayer, Ogre::MEMCATEGORY_GENERAL)(OGRE_VERSION_NAME);

#ifdef OGRE_STATIC_LIB
    mStaticPluginLoader = OGRE_NEW StaticPluginLoader();
    Root* root = OGRE_NEW Root(BLANKSTRING);        
    mStaticPluginLoader.load();
#else
    String pluginsPath = mFSLayer->getConfigFilePath("plugins.cfg");
    Root* root = OGRE_NEW Root(pluginsPath);
#endif

    CPPUNIT_ASSERT(!root->getAvailableRenderers().empty());
    root->setRenderSystem(root->getAvailableRenderers().back());
    root->initialise(false); // Needed for setting up HardwareBufferManager

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
    Ogre::NameValuePairList misc;
    // Tell OGRE that we're using cocoa, so it doesn't need to make a window for us
    misc["macAPI"] = "cocoa";
    root->createRenderWindow("", 320, 240, false, &misc)->setHidden(true);
#else
    root->createRenderWindow("", 320, 240, false, NULL)->setHidden(true);
#endif

    new MeshLodGenerator;

    // Load resource paths from config file
    ConfigFile cf;
    String resourcesPath;

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    resourcesPath = mFSLayer->getConfigFilePath("resources.cfg");
#else
    resourcesPath = mFSLayer->getConfigFilePath("bin/resources.cfg");
#endif

    cf.load(resourcesPath);
    // Go through all sections & settings in the file
    ConfigFile::SectionIterator seci = cf.getSectionIterator();

    String secName, typeName, archName;
    while (seci.hasMoreElements()) {
        secName = seci.peekNextKey();
        ConfigFile::SettingsMultiMap* settings = seci.getNext();
        ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i) {
            typeName = i->first;
            archName = i->second;
            ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }

    // Create the mesh for testing
    mMesh = MeshManager::getSingleton().load("Sinbad.mesh", ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
}
//--------------------------------------------------------------------------
void MeshLodTests::tearDown()
{
    if (!mMesh.isNull()) {
        mMesh->unload();
        mMesh.setNull();
    }
    OGRE_DELETE MeshLodGenerator::getSingletonPtr();
    OGRE_DELETE Root::getSingletonPtr();
    OGRE_DELETE_T(mFSLayer, FileSystemLayer, Ogre::MEMCATEGORY_GENERAL);
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
void MeshLodTests::testMeshLodGenerator()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

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
    CPPUNIT_ASSERT(config.levels.size() == config2.levels.size());
    for (size_t i = 0; i < config.levels.size(); i++) 
    {
        CPPUNIT_ASSERT(config.levels[i].outSkipped == config2.levels[i].outSkipped);
        CPPUNIT_ASSERT(config.levels[i].outUniqueVertexCount == config2.levels[i].outUniqueVertexCount);
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
        wq->processResponses(); // Injects the Lod if ready
        if (mesh->getNumLodLevels() != 1) {
            success = true;
            break;
        }
    }
    // timeout
    CPPUNIT_ASSERT(success);
}
//--------------------------------------------------------------------------
void MeshLodTests::testLodConfigSerializer()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    LodConfig config, config2;
    setTestLodConfig(config);
    addProfile(config);
    LodConfigSerializer serializer;
    serializer.exportLodConfig(config, "testLodConfigSerializer.lodconfig");
    serializer.importLodConfig(&config2, "testLodConfigSerializer.lodconfig");
    CPPUNIT_ASSERT(config.mesh->getHandle() == config2.mesh->getHandle());
    CPPUNIT_ASSERT(config.strategy == config2.strategy);
    CPPUNIT_ASSERT(config.advanced.outsideWalkAngle == config.advanced.outsideWalkAngle);
    CPPUNIT_ASSERT(config.advanced.outsideWeight == config.advanced.outsideWeight);
    CPPUNIT_ASSERT(config.advanced.useBackgroundQueue == config.advanced.useBackgroundQueue);
    CPPUNIT_ASSERT(config.advanced.useCompression == config.advanced.useCompression);
    CPPUNIT_ASSERT(config.advanced.useVertexNormals == config.advanced.useVertexNormals);

    {
        // Compare profiles
        LodProfile& p1 = config.advanced.profile;
        LodProfile& p2 = config2.advanced.profile;
        bool isProfileSameSize = (p1.size() == p2.size());
        CPPUNIT_ASSERT(isProfileSameSize);
        if (isProfileSameSize) 
        {
            for (size_t i = 0; i < p1.size(); i++) 
            {
                CPPUNIT_ASSERT(p1[i].src == p2[i].src);
                CPPUNIT_ASSERT(p1[i].dst == p2[i].dst);
                CPPUNIT_ASSERT(isEqual(p1[i].cost, p2[i].cost));
            }
        }
    }

    {
        // Compare Lod Levels
        LodConfig::LodLevelList& l1 = config.levels;
        LodConfig::LodLevelList& l2 = config2.levels;
        bool isLevelsSameSize = (l1.size() == l2.size());
        CPPUNIT_ASSERT(isLevelsSameSize);
        if (isLevelsSameSize) 
        {
            for (size_t i = 0; i < l1.size(); i++) 
            {
                CPPUNIT_ASSERT(l1[i].distance == l2[i].distance);
                CPPUNIT_ASSERT(l1[i].manualMeshName == l2[i].manualMeshName);
                CPPUNIT_ASSERT(l1[i].reductionMethod == l2[i].reductionMethod);
                CPPUNIT_ASSERT(isEqual(l1[i].reductionValue, l2[i].reductionValue));
            }
        }
    }
}
//--------------------------------------------------------------------------
void MeshLodTests::testManualLodLevels()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    MeshLodGenerator& gen = MeshLodGenerator::getSingleton();
    LodConfig config;
    setTestLodConfig(config);
    gen.generateLodLevels(config, LodCollapseCostPtr(new LodCollapseCostQuadric()));
}
//--------------------------------------------------------------------------
void MeshLodTests::testQuadricError()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

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
bool MeshLodTests::isEqual(Real a, Real b)
{
    Real absoluteError = static_cast<Real>(std::abs(a * 0.05));
    return ((a - absoluteError) <= b) && ((a + absoluteError) >= b);
}
//--------------------------------------------------------------------------
