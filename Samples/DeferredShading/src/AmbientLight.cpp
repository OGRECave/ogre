/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/


#include "AmbientLight.h"
#include "GeomUtils.h"
#include "OgreMaterialManager.h"

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
