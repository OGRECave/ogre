/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/
Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "PlayPenTests.h"

PlayPen_ReinitialiseEntityAlteredMesh::PlayPen_ReinitialiseEntityAlteredMesh()
{
	mInfo["Title"] = "PlayPen_ReinitialiseEntityAlteredMesh";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(100);
	mTimer = 0.5f;
}
//----------------------------------------------------------------------------

bool PlayPen_ReinitialiseEntityAlteredMesh::frameStarted(const FrameEvent& evt)
{
	if(mTimer > 0.f)
	{
		mTimer -= evt.timeSinceLastFrame;

		if(mTimer <= 0.f)
		{
			// change the mesh, add a new submesh

			// Load another mesh
			MeshPtr msh = MeshManager::getSingleton().load("ogrehead.mesh", 
				ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

			for (unsigned short e = 0; e < msh->getNumSubMeshes(); ++e)
			{
				SubMesh* sm = msh->getSubMesh(e);

				SubMesh* newsm = mUpdate->createSubMesh();
				newsm->useSharedVertices = false;
				newsm->operationType = sm->operationType;
				newsm->vertexData = sm->vertexData->clone();
				newsm->indexData = sm->indexData->clone();
			}

			mTimer = -5.f;
		}
	}

	return true;
}
//----------------------------------------------------------------------------

void PlayPen_ReinitialiseEntityAlteredMesh::setupContent()
{
	// test whether an Entity picks up that Mesh has changed
	// and therefore rebuild SubEntities
	
	mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
	
	Light* l = mSceneMgr->createLight("l1");
	l->setPosition(200, 300, 0);

	Ogre::MeshManager::getSingleton().load("knot.mesh", 
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME)->clone("knot_REINIT.mesh");
	
	Entity* pEnt = mSceneMgr->createEntity("testEnt", "knot_REINIT.mesh");
	mUpdate = pEnt->getMesh().get();
	
	mSceneMgr->getRootSceneNode()->attachObject(pEnt);
	
	mCamera->setPosition(0,0,200);
}
