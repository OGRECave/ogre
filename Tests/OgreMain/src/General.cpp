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

#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1600)
#include <random>
using std::minstd_rand;
#else
#include <tr1/random>
using std::tr1::minstd_rand;
#endif

using namespace Ogre;

TEST(Root,shutdown)
{
    Root root;
    root.shutdown();
}

TEST(SceneManager,removeAndDestroyAllChildren)
{
    Root root;
    SceneManager* sm = root.createSceneManager();
    sm->getRootSceneNode()->createChildSceneNode();
    sm->getRootSceneNode()->createChildSceneNode();
    sm->getRootSceneNode()->removeAndDestroyAllChildren();
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
        nodePos.x *= float(rng())/rng.max();
        nodePos.y *= float(rng())/rng.max();
        nodePos.z *= float(rng())/rng.max();
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

    void SetUp() {
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
    for (SceneQueryMovableIntersectionList::iterator mov = results.movables2movables.begin();
         mov != results.movables2movables.end(); ++mov)
    {
        SceneQueryMovableObjectPair& thepair = *mov;
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
