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

#include "OgreMaterialSerializer.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreMaterialManager.h"
#include "OgreConfigFile.h"
#include "OgreSTBICodec.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreMeshManager.h"
#include "OgreMesh.h"
#include "OgreSkeletonManager.h"
#include "OgreSkeletonInstance.h"
#include "OgreCompositorManager.h"
#include "OgreTextureManager.h"
#include "OgreFileSystem.h"
#include "OgreArchiveManager.h"

#include "OgreHighLevelGpuProgram.h"

#include "OgreKeyFrame.h"

#include "OgreBillboardSet.h"
#include "OgreBillboard.h"

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

TEST(MaterialSerializer, Basic)
{
    Root root;
    DefaultTextureManager texMgr;

    String group = "General";

    auto mat = std::make_shared<Material>(nullptr, "Material Name", 0, group);
    auto pass = mat->createTechnique()->createPass();
    auto tus = pass->createTextureUnitState();
    tus->setContentType(TextureUnitState::CONTENT_SHADOW);
    tus->setName("Test TUS");
    pass->setAmbient(ColourValue::Green);

    pass->createTextureUnitState("TextureName");

    // export to string
    MaterialSerializer ser;
    ser.queueForExport(mat);
    auto str = ser.getQueuedAsString();

    // printf("%s\n", str.c_str());

    // load again
    DataStreamPtr stream = std::make_shared<MemoryDataStream>("memory.material", &str[0], str.size());
    MaterialManager::getSingleton().parseScript(stream, group);

    auto mat2 = MaterialManager::getSingleton().getByName("Material Name", group);
    ASSERT_TRUE(mat2);
    EXPECT_EQ(mat2->getTechniques().size(), mat->getTechniques().size());
    EXPECT_EQ(mat2->getTechniques()[0]->getPasses()[0]->getAmbient(), ColourValue::Green);
    EXPECT_EQ(mat2->getTechniques()[0]->getPasses()[0]->getTextureUnitState(0)->getName(),
              "Test TUS");
    EXPECT_EQ(mat2->getTechniques()[0]->getPasses()[0]->getTextureUnitState("Test TUS")->getContentType(),
              TextureUnitState::CONTENT_SHADOW);
    EXPECT_EQ(mat2->getTechniques()[0]->getPasses()[0]->getTextureUnitState(1)->getTextureName(),
              "TextureName");
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

typedef RootWithoutRenderSystemFixture SkeletonTests;
TEST_F(SkeletonTests, linkedSkeletonAnimationSource)
{
    auto sceneMgr = mRoot->createSceneManager();
    auto entity = sceneMgr->createEntity("jaiqua.mesh");
    entity->getSkeleton()->addLinkedSkeletonAnimationSource("ninja.skeleton");
    entity->refreshAvailableAnimationState();
    EXPECT_TRUE(entity->getAnimationState("Stealth")); // animation from ninja.sekeleton
}

TEST(MaterialLoading, LateShadowCaster)
{
    Root root("");
    auto tech = MaterialManager::getSingleton().create("Material", RGN_DEFAULT)->createTechnique();
    tech->setShadowCasterMaterial("Caster");
    EXPECT_FALSE(tech->getShadowCasterMaterial());

    MaterialManager::getSingleton().create("Caster", RGN_DEFAULT);

    // force call _load() due to missing rendersystem
    tech->_load();

    EXPECT_TRUE(tech->getShadowCasterMaterial());
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