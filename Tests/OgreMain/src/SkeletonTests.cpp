// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "RootWithoutRenderSystemFixture.h"

#include "OgreEntity.h"
#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreSceneManager.h"
#include "OgreSkeleton.h"
#include "OgreSkeletonInstance.h"
#include "OgreSkeletonManager.h"

using namespace Ogre;

class SkeletonTests : public RootWithoutRenderSystemFixture
{
protected:
    MeshPtr mMesh;
};

TEST_F(SkeletonTests, linkedSkeletonAnimationSource)
{
    auto sceneMgr = mRoot->createSceneManager();
    auto entity = sceneMgr->createEntity("jaiqua.mesh");
    entity->getSkeleton()->addLinkedSkeletonAnimationSource("ninja.skeleton");
    entity->refreshAvailableAnimationState();
    EXPECT_TRUE(entity->getAnimationState("Stealth")); // animation from ninja.sekeleton
}

TEST_F(SkeletonTests, MeshWithSkeletonDestroyedAfterRoot)
{
    mMesh = MeshManager::getSingleton().createManual("Mesh", RGN_DEFAULT);
    auto skeleton = SkeletonManager::getSingleton().create("Skeleton", RGN_DEFAULT, true);
    skeleton->load();
    mMesh->_notifySkeleton(skeleton);

    // drop Root while the fixture, and with it the Mesh and its Skeleton, live on
    mRoot.reset();
}
