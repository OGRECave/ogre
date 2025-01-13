// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "RootWithoutRenderSystemFixture.h"

#include "Ogre.h"
#include "OgreDefaultHardwareBufferManager.h"

using namespace Ogre;

void RootWithoutRenderSystemFixture::SetUp()
{
    mRoot.reset(new Root(""));
    mFSLayer.reset(new FileSystemLayer(OGRE_VERSION_NAME));
    mHBM.reset(new DefaultHardwareBufferManager);

    MaterialManager::getSingleton().initialise();

    // Load resource paths from config file
    ConfigFile cf;
    String resourcesPath = mFSLayer->getConfigFilePath("resources.cfg");

    cf.load(resourcesPath);
    // Go through all sections & settings in the file
    String secName, typeName, archName;
    for(const auto& e : cf.getSettingsBySection()) {
        secName = e.first;
        const ConfigFile::SettingsMultiMap& settings = e.second;
        for (const auto& s : settings)
        {
            typeName = s.first;
            archName = s.second;
            ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
        }
    }
}

void RootWithoutRenderSystemFixture::TearDown()
{

}
