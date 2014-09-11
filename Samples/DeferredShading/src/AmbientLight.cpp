/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/


#include "AmbientLight.h"
#include "GeomUtils.h"
#include "OgreMaterialManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

using namespace Ogre;

AmbientLight::AmbientLight()
{
	setRenderQueueGroup(RENDER_QUEUE_2);

	mRenderOp.vertexData = new VertexData();
	mRenderOp.indexData = 0;

	GeomUtils::createQuad(mRenderOp.vertexData);

	mRenderOp.operationType = RenderOperation::OT_TRIANGLE_STRIP; 
	mRenderOp.useIndexes = false; 

	// Set bounding
	setBoundingBox(AxisAlignedBox(-10000,-10000,-10000,10000,10000,10000));
	mRadius = 15000;

	mMatPtr = MaterialManager::getSingleton().getByName("DeferredShading/AmbientLight");
	assert(mMatPtr.isNull()==false);
	mMatPtr->load();

    // Explicitly bind samplers for OpenGL
    if(Root::getSingleton().getRenderSystem()->getName().find("OpenGL 3+") != String::npos)
    {
        mMatPtr->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->setNamedConstant("Tex0", 0);
        mMatPtr->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->setNamedConstant("Tex1", 1);
    }

    //This shader needs to be aware if its running under OpenGL or DirectX.
    //Real depthFactor = (Root::getSingleton().getRenderSystem()->getName() ==
    //    "OpenGL Rendering Subsystem") ? 2.0 : 1.0;
    //mMatPtr->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->setNamedConstant(
    //        "depthFactor", depthFactor);
}

AmbientLight::~AmbientLight()
{
	// need to release IndexData and vertexData created for renderable
	delete mRenderOp.indexData;
	delete mRenderOp.vertexData;
}

/** @copydoc MovableObject::getBoundingRadius */
Real AmbientLight::getBoundingRadius(void) const
{
	return mRadius;

}
/** @copydoc Renderable::getSquaredViewDepth */
Real AmbientLight::getSquaredViewDepth(const Camera*) const
{
	return 0.0;
}

const MaterialPtr& AmbientLight::getMaterial(void) const
{
	return mMatPtr;
}

void AmbientLight::getWorldTransforms(Ogre::Matrix4* xform) const
{
	*xform = Matrix4::IDENTITY;
}
void AmbientLight::updateFromCamera(Ogre::Camera* camera)
{
	Ogre::Technique* tech = getMaterial()->getBestTechnique();
	Ogre::Vector3 farCorner = camera->getViewMatrix(true) * camera->getWorldSpaceCorners()[4];

	for (unsigned short i=0; i<tech->getNumPasses(); i++) 
	{
		Ogre::Pass* pass = tech->getPass(i);
		// get the vertex shader parameters
		Ogre::GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();
		// set the camera's far-top-right corner
		if (params->_findNamedConstantDefinition("farCorner"))
			params->setNamedConstant("farCorner", farCorner);
	    
		params = pass->getFragmentProgramParameters();
		if (params->_findNamedConstantDefinition("farCorner"))
			params->setNamedConstant("farCorner", farCorner);
	}
}
