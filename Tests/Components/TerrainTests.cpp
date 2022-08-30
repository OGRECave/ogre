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
#include "OgreTerrain.h"
#include "OgreFileSystemLayer.h"

#include "OgreBuildSettings.h"
#include "OgreStaticPluginLoader.h"
#include "OgreConfigFile.h"
#include "OgreResourceGroupManager.h"
#include "OgreLogManager.h"
#include "OgreSTBICodec.h"
#include "OgreStreamSerialiser.h"
#include "OgreDefaultHardwareBufferManager.h"

using namespace Ogre;

class TerrainTests : public ::testing::Test
{
public:
    Root* mRoot;
    SceneManager* mSceneMgr;
    TerrainGlobalOptions* mTerrainOpts;

    void SetUp() override;
    void TearDown() override;
};

//--------------------------------------------------------------------------
void TerrainTests::SetUp()
{
    mRoot = OGRE_NEW Root("");


    STBIImageCodec::startup();

    mTerrainOpts = OGRE_NEW TerrainGlobalOptions();

    // Load resource paths from config file
    ConfigFile cf;
    cf.load(FileSystemLayer(OGRE_VERSION_NAME).getConfigFilePath("resources.cfg"));

    // Go through all sections & settings in the file
    String secName, typeName, archName;
    ConfigFile::SettingsBySection_::const_iterator seci;
    for(seci = cf.getSettingsBySection().begin(); seci != cf.getSettingsBySection().end(); ++seci) {
        secName = seci->first;
        const ConfigFile::SettingsMultiMap& settings = seci->second;
        ConfigFile::SettingsMultiMap::const_iterator i;
        for (i = settings.begin(); i != settings.end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }

    mSceneMgr = mRoot->createSceneManager();
}
//--------------------------------------------------------------------------
void TerrainTests::TearDown()
{
    STBIImageCodec::shutdown();
    OGRE_DELETE mTerrainOpts;
    OGRE_DELETE mRoot;
}
//--------------------------------------------------------------------------
TEST_F(TerrainTests, create)
{
    Terrain* t = OGRE_NEW Terrain(mSceneMgr);
    Image img;
    img.load("terrain.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    Terrain::ImportData imp;
    imp.inputImage = &img;
    imp.terrainSize = 513;
    imp.worldSize = 1000;
    imp.minBatchSize = 33;
    imp.maxBatchSize = 65;
    ASSERT_TRUE(t->prepare(imp));

    {
        DefaultHardwareBufferManager hbm;
        StreamSerialiser ser(Root::createFileStream("TerrainTest.dat"));
        t->save(ser);
        OGRE_DELETE t;
    }

    // read-back from file
    t = OGRE_NEW Terrain(mSceneMgr);
    StreamSerialiser ser(Root::openFileStream("TerrainTest.dat"));
    ASSERT_TRUE(t->prepare(ser));
    ASSERT_EQ(t->getSize(), imp.terrainSize);

    OGRE_DELETE t;

    FileSystemLayer::removeFile("TerrainTest.dat");
}
//--------------------------------------------------------------------------
