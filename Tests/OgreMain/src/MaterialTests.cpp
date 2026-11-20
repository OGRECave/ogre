// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "OgreDataStream.h"
#include "OgreGpuProgramManager.h"
#include "OgreLight.h"
#include "OgreMaterial.h"
#include "OgreMaterialManager.h"
#include "OgreMaterialSerializer.h"
#include "OgrePass.h"
#include "OgreRoot.h"
#include "OgreTechnique.h"
#include "OgreTextureManager.h"
#include "OgreTextureUnitState.h"

using namespace Ogre;

TEST(MaterialTests, Serializer)
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

TEST(MaterialTests, LateShadowCaster)
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

TEST(MaterialTests, TessellationProgramRefs)
{
    Root root;

    String group = "General";
    auto& gpm = GpuProgramManager::getSingleton();
    gpm.createProgram("hs", group, "null", GPT_HULL_PROGRAM);
    gpm.createProgram("ds", group, "null", GPT_DOMAIN_PROGRAM);

    auto mat = std::make_shared<Material>(nullptr, "Material Name", 0, group);
    auto pass = mat->createTechnique()->createPass();
    pass->setGpuProgram(GPT_HULL_PROGRAM, gpm.getByName("hs", group));
    pass->setGpuProgram(GPT_DOMAIN_PROGRAM, gpm.getByName("ds", group));

    MaterialSerializer ser;
    ser.queueForExport(mat);
    auto str = ser.getQueuedAsString();

    DataStreamPtr stream = std::make_shared<MemoryDataStream>("memory.material", &str[0], str.size());
    MaterialManager::getSingleton().parseScript(stream, group);

    auto mat2 = MaterialManager::getSingleton().getByName("Material Name", group);
    ASSERT_TRUE(mat2);
    EXPECT_TRUE(mat2->getTechnique(0)->getPass(0)->hasTessellationHullProgram());
    EXPECT_TRUE(mat2->getTechnique(0)->getPass(0)->hasTessellationDomainProgram());
}

TEST(MaterialTests, IterateRectLights)
{
    Root root("");
    String src = "material Mat { technique { pass { iteration once_per_light rect } } }";
    DataStreamPtr stream = std::make_shared<MemoryDataStream>("memory.material", &src[0], src.size());
    MaterialManager::getSingleton().parseScript(stream, RGN_DEFAULT);

    auto pass = MaterialManager::getSingleton().getByName("Mat")->getTechnique(0)->getPass(0);
    EXPECT_EQ(pass->getOnlyLightType(), Light::LT_RECTLIGHT);
}
