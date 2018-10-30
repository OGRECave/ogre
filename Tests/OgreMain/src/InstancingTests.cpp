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

#include "Ogre.h"
#include "OgreInstancedEntity.h"
#include "OgreInstanceBatchShader.h"
#include "RootWithoutRenderSystemFixture.h"

using namespace Ogre;

typedef RootWithoutRenderSystemFixture Instancing;

TEST_F(Instancing, Bounds) {
    SceneManager* sceneMgr = mRoot->createSceneManager();
    Entity* entity = sceneMgr->createEntity("robot.mesh");

    MeshPtr mesh = entity->getMesh();
    InstanceBatchShader batch(NULL, mesh, entity->getSubEntity(0)->getMaterial(), 1, NULL, "");
    InstancedEntity instanced_entity(&batch, 0);

    SceneNode* node = sceneMgr->createSceneNode();
    node->attachObject(&instanced_entity);
    node->attachObject(entity);
    node->translate(Vector3::UNIT_X);
    node->setScale(Vector3(2, 2, 2));

    EXPECT_EQ(instanced_entity.getBoundingBox(), entity->getBoundingBox());
    EXPECT_EQ(instanced_entity.getBoundingRadius(), entity->getBoundingRadius());
}



