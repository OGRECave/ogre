
// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <OgrePlugin.h>
#include <OgreConfigFile.h>
#include <OgreEntity.h>
#include <OgreSubEntity.h>

#include "RootWithoutRenderSystemFixture.h"

using namespace Ogre;

typedef RootWithoutRenderSystemFixture DotSceneTests;

TEST_F(DotSceneTests, exportImport)
{
    String pluginsCfg = mFSLayer->getConfigFilePath("plugins.cfg");
    ConfigFile cf;
    cf.load(pluginsCfg);

    auto pluginDir = cf.getSetting("PluginFolder");

    try
    {
        mRoot->loadPlugin(pluginDir+"/Plugin_DotScene");
    }
    catch (const std::exception& e)
    {
        return;
    }

    mRoot->getInstalledPlugins().front()->initialise();

    auto sceneMgr = mRoot->createSceneManager();
    auto entity = sceneMgr->createEntity("Entity", "jaiqua.mesh");
    auto entityUnlit = sceneMgr->createEntity("EntityUnlit", "jaiqua.mesh");
    entityUnlit->setMaterialName("BaseWhiteNoLighting");
    auto camera = sceneMgr->createCamera("MainCamera");
    auto light = sceneMgr->createLight();
    sceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(1, 0, 0))->attachObject(entity);
    sceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 1, 0))->attachObject(camera);
    sceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 0, 1))->attachObject(light);
    sceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(1, 1, 1))->attachObject(entityUnlit);

    sceneMgr->getRootSceneNode()->saveChildren("DotSceneTest.scene");

    sceneMgr->clearScene();
    sceneMgr->destroyAllCameras();

    EXPECT_TRUE(sceneMgr->getRootSceneNode()->getChildren().empty());

    sceneMgr->getRootSceneNode()->loadChildren("DotSceneTest.scene");

    EXPECT_EQ(sceneMgr->getRootSceneNode()->getChildren().size(), 4);

    for (auto c : sceneMgr->getRootSceneNode()->getChildren())
        EXPECT_EQ(dynamic_cast<SceneNode*>(c)->getAttachedObjects().size(), 1);

    EXPECT_TRUE(sceneMgr->getCamera("MainCamera")->getParentSceneNode()->getPosition() == Vector3(0, 1, 0));
    EXPECT_TRUE(sceneMgr->getEntity("Entity")->getParentSceneNode()->getPosition() == Vector3(1, 0, 0));
    EXPECT_EQ(sceneMgr->getEntity("EntityUnlit")->getSubEntity(0)->getMaterialName(), "BaseWhiteNoLighting");

    FileSystemLayer::removeFile("DotSceneTest.scene");
}