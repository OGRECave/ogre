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

PlayPen_GeometryShaders::PlayPen_GeometryShaders()
{
	mInfo["Title"] = "PlayPen_GeometryShaders";
	mInfo["Description"] = "Tests.";
	addScreenshotFrame(250);
}
//----------------------------------------------------------------------------

void PlayPen_GeometryShaders::testCapabilities(const Ogre::RenderSystemCapabilities* caps)
{
	if(!caps->hasCapability(RSC_GEOMETRY_PROGRAM))
		throw Ogre::Exception(999, "Video card doesn't support geometry shaders.", "testCapabilities");
}
//----------------------------------------------------------------------------

void PlayPen_GeometryShaders::setupContent()
{
	const String GLSL_MATERIAL_NAME = "Ogre/GPTest/SwizzleGLSL";
	const String ASM_MATERIAL_NAME = "Ogre/GPTest/SwizzleASM";
	const String CG_MATERIAL_NAME = "Ogre/GPTest/SwizzleCG";
	
	// Check capabilities
	const RenderSystemCapabilities* caps = Root::getSingleton().getRenderSystem()->getCapabilities();
	if (!caps->hasCapability(RSC_GEOMETRY_PROGRAM))
	{
		OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED, "Your card does not support geometry programs, so cannot "
		"run this demo. Sorry!", 
		"GeometryShading::createScene");
	}
	
	int maxOutputVertices = caps->getGeometryProgramNumOutputVertices();
	Ogre::LogManager::getSingleton().getDefaultLog()->stream() << 
	"Num output vertices per geometry shader run : " << maxOutputVertices;
	
	Entity *ent = mSceneMgr->createEntity("head", "ogrehead.mesh");
	mCamera->setPosition(20, 0, 100);
	mCamera->lookAt(0,0,0);
	
	//String materialName = GLSL_MATERIAL_NAME;
	String materialName = ASM_MATERIAL_NAME;
	//String materialName = CG_MATERIAL_NAME;
	
	// Set all of the material's sub entities to use the new material
	for (unsigned int i=0; i<ent->getNumSubEntities(); i++)
	{
		ent->getSubEntity(i)->setMaterialName(materialName);
	}
	
	// Add entity to the root scene node
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
	
	mWindow->getViewport(0)->setBackgroundColour(ColourValue::Green);
}
