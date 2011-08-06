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

PlayPen_PointSprites::PlayPen_PointSprites()
{
	mInfo["Title"] = "PlayPen_PointSprites";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(10);
}
//----------------------------------------------------------------------------

void PlayPen_PointSprites::setupContent()
{
	MaterialPtr mat = MaterialManager::getSingleton().create("spriteTest1", 
	TRANSIENT_RESOURCE_GROUP);
	Pass* p = mat->getTechnique(0)->getPass(0);
	p->setPointSpritesEnabled(true);
	p->createTextureUnitState("flare.png");
	p->setLightingEnabled(false);
	p->setDepthWriteEnabled(false);
	p->setSceneBlending(SBT_ADD);
	p->setPointAttenuation(true);
	p->setPointSize(1);
	
	ManualObject* man = mSceneMgr->createManualObject("man");
	man->begin("spriteTest1", RenderOperation::OT_POINT_LIST);
	
	for (size_t i = 0; i < 1000; ++i)
	{
		man->position(Math::SymmetricRandom() * 500, 
		Math::SymmetricRandom() * 500, 
		Math::SymmetricRandom() * 500);
		man->colour(Math::RangeRandom(0.5f, 1.0f), 
		Math::RangeRandom(0.5f, 1.0f), Math::RangeRandom(0.5f, 1.0f));
	}
	
	/*for (size_t i = 0; i < 20; ++i)
	{
		for (size_t j = 0; j < 20; ++j)
		{
			for (size_t k = 0; k < 20; ++k)
			{
				if(rand()%10 == 0)
					man->position(i * 30, j * 30, k * 30);
			}
		}
	}*/
	
	man->end();
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(man);

	mCamera->setPosition(0,0,1000);
	
}
