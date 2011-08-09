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

PlayPen_ManualObjectIndexedUpdateLarger::PlayPen_ManualObjectIndexedUpdateLarger()
{
	mInfo["Title"] = "PlayPen_ManualObjectIndexedUpdateLarger";
	mInfo["Description"] = "Tests.";
}
//----------------------------------------------------------------------------

void PlayPen_ManualObjectIndexedUpdateLarger::setupContent()
{
	PlayPen_ManualObjectIndexed::setupContent();
	ManualObject* man = static_cast<ManualObject*>(
	mSceneMgr->getMovableObject("test", ManualObjectFactory::FACTORY_TYPE_NAME));
	
	
	man->beginUpdate(0);
	// 1 quad larger
	man->position(-20, 20, 20);
	man->normal(0, 0, 1);
	man->textureCoord(0, 0);
	
	man->position(-20, -20, 20);
	man->normal(0, 0, 1);
	man->textureCoord(0, 1);
	
	man->position(20, -20, 20);
	man->normal(0, 0, 1);
	man->textureCoord(1, 1);
	
	man->position(20, 20, 20);
	man->normal(0, 0, 1);
	man->textureCoord(1, 0);
	
	
	man->position(-20, 40, 20);
	man->normal(0, 0, 1);
	man->textureCoord(0, 0);
	
	man->position(-20, 20, 20);
	man->normal(0, 0, 1);
	man->textureCoord(0, 1);
	
	man->position(20, 20, 20);
	man->normal(0, 0, 1);
	man->textureCoord(1, 1);
	
	man->position(20, 40, 20);
	man->normal(0, 0, 1);
	man->textureCoord(1, 0);
	
	man->quad(0, 1, 2, 3);
	man->quad(4, 5, 6, 7);
	
	man->end();
	
}
