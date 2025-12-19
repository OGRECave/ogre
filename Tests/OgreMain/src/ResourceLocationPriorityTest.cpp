/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2019 Torus Knot Software Ltd

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

#include "OgreConfigFile.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreMaterialManager.h"
#include "OgreMeshManager.h"
#include "OgreTextureManager.h"
#include "ResourceLocationPriorityTest.h"
#include "RootWithoutRenderSystemFixture.h"
#include "OgreMesh.h"
#include "OgreSkeleton.h"
#include "OgreSubMesh.h"
#include "OgreTechnique.h"

#include "OgreArchiveManager.h"

TEST(ResourceGroupLocationTest, ResourceLocationPriority)
{
    std::unique_ptr<DummyArchiveFactory> fact = std::unique_ptr<DummyArchiveFactory>(new DummyArchiveFactory);
    Ogre::Root root("");
    Ogre::ArchiveManager& archiveMgr = Ogre::ArchiveManager::getSingleton();
    archiveMgr.addArchiveFactory(fact.get());

    Ogre::ResourceGroupManager& resGrpMgr = Ogre::ResourceGroupManager::getSingleton();
    resGrpMgr.addResourceLocation("ResourceLocationPriority0", "DummyArchive");
    resGrpMgr.addResourceLocation("ResourceLocationPriority1", "DummyArchive");

    Ogre::DataStreamPtr filePtr = resGrpMgr.openResource("dummyArchiveTest");
    EXPECT_TRUE(filePtr);

    unsigned char contents;
    filePtr->read(&contents, 1);
    // If the archive added first is returned then the file will contain 0x1,
    // otherwise it will contain 0x2.
    // Expect 0x1, as locations added first are preferred over locations added later.
    EXPECT_EQ(contents, 1);

    resGrpMgr.removeResourceLocation("ResourceLocationPriority0");
    resGrpMgr.removeResourceLocation("ResourceLocationPriority1");
}

TEST(ResourceGroupLocationTest, NonUniqueResourceNames)
{
    using namespace Ogre;
    DefaultHardwareBufferManager defaultHwBufMgr;
    Root root("");
    DefaultTextureManager defaultTexMgr;

    ConfigFile cf;
    cf.load(FileSystemLayer(OGRE_VERSION_NAME).getConfigFilePath("resources.cfg"));
    auto testPath = cf.getSettings("Tests").begin()->second;

    auto& rgm = ResourceGroupManager::getSingleton();
    auto group = "Model1";
    rgm.createResourceGroup(group, false);
    rgm.addResourceLocation(testPath + "/" + group, "FileSystem", group);

    group = "Model2";
    rgm.createResourceGroup(group, false);
    rgm.addResourceLocation(testPath + "/" + group, "FileSystem", group);
    rgm.initialiseAllResourceGroups();

    auto model1 = MeshManager::getSingleton().load("UniqueModel.MESH", "Model1");
    auto model2 = MeshManager::getSingleton().load("UniqueModel.MESH", "Model2");

    EXPECT_NEAR(model1->getBoundingSphereRadius(), 10, 0.1f);
    EXPECT_EQ(model1->getSkeleton()->getNumBones(), 4);
    EXPECT_FLOAT_EQ(model1->getSubMesh(0)->getMaterial()->getTechnique(0)->getPass(0)->getShininess(), 10);

    EXPECT_NEAR(model2->getBoundingSphereRadius(), 40.1, 0.1f);
    EXPECT_EQ(model2->getSkeleton()->getNumBones(), 7);
    EXPECT_FLOAT_EQ(model2->getSubMesh(0)->getMaterial()->getTechnique(0)->getPass(0)->getShininess(), 20);
}