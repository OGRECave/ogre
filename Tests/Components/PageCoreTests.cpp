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
#include "OgrePageManager.h"
#include "OgreGrid2DPageStrategy.h"
#include "OgreBuildSettings.h"


#include "OgreStaticPluginLoader.h"
#include "OgrePaging.h"
#include "OgreLogManager.h"

using namespace Ogre;

class PageCoreTests : public ::testing::Test
{
public:
    Root* mRoot;
    PageManager* mPageManager;
    SceneManager* mSceneMgr;

    void SetUp();
    void TearDown();
};
// Register the test suite

//--------------------------------------------------------------------------
void PageCoreTests::SetUp()
{    
    mRoot = OGRE_NEW Root("");

    mPageManager = OGRE_NEW PageManager();

    // make certain the resource location is NOT read-only
    ResourceGroupManager::getSingleton().addResourceLocation("./", "FileSystem",
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, false, false);

    mSceneMgr = mRoot->createSceneManager();
}
//--------------------------------------------------------------------------
void PageCoreTests::TearDown()
{
    OGRE_DELETE mPageManager;
    OGRE_DELETE mRoot;
}
//--------------------------------------------------------------------------
TEST_F(PageCoreTests,SimpleCreateSaveLoadWorld)
{
    String worldName = "MyWorld";
    String filename = "myworld.world";
    String sectionName1 = "Section1";
    String sectionName2 = "Section2";
    PagedWorld* world = mPageManager->createWorld(worldName);
    PagedWorldSection* section = world->createSection("Grid2D", mSceneMgr, sectionName1);
    section = world->createSection("Grid2D", mSceneMgr, sectionName2);

    // Create a page
    Page* p = section->loadOrCreatePage(Vector3::ZERO);

    p->createContentCollection("Simple");

    world->save(filename);

    mPageManager->destroyWorld(world);
    world = 0;
    world = mPageManager->loadWorld(filename);

    EXPECT_EQ(worldName, world->getName());
    EXPECT_EQ((size_t)2, world->getSectionCount());

    section = world->getSection(sectionName1);
    EXPECT_TRUE(section != 0);
    section = world->getSection(sectionName2);
    EXPECT_TRUE(section != 0);
}
//--------------------------------------------------------------------------

