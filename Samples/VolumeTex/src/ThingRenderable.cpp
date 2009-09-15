/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/
#include "ThingRenderable.h"
#include <OgreHardwareVertexBuffer.h>
#include <OgreHardwareIndexBuffer.h>
#include <OgreHardwareBufferManager.h>
#include <OgreCamera.h>
using namespace Ogre;

ThingRenderable::ThingRenderable(float radius, size_t count, float qsize):
	mRadius(radius),
	mCount(count),
	mQSize(qsize)
{
	mBox = Ogre::AxisAlignedBox(-radius, -radius, -radius, radius, radius, radius);
	initialise();
	fillBuffer();
}
ThingRenderable::~ThingRenderable()
{
    // need to release IndexData and vertexData created for renderable
    delete mRenderOp.indexData;
    delete mRenderOp.vertexData;
}

void ThingRenderable::addTime(float t)
{
	for(size_t x=0; x<mCount; x++)
	{
		Quaternion dest = things[x] * orbits[x];
		things[x] = things[x] + t * (dest - things[x]);
		things[x].normalise();
	}
	fillBuffer();
}
// Generate float between -1 and 1
float randFloat()
{
	return ((float)rand()/RAND_MAX)*2.0f-1.0f;
}
void ThingRenderable::initialise()
{
	// Fill array with randomly oriented quads
	Vector3 ax, ay, az;
	size_t x;
	Quaternion q;
	things.clear(); orbits.clear();
	for(x=0; x<mCount; x++)
	{
		ax = Vector3(randFloat(), randFloat(), randFloat());
		ay = Vector3(randFloat(), randFloat(), randFloat());
		az = ax.crossProduct(ay);
		ay = az.crossProduct(ax);
		ax.normalise(); ay.normalise(); az.normalise();
		q.FromAxes(ax, ay, az);
		//std::cerr << ax.dotProduct(ay) << " " << ay.dotProduct(az) << " " << az.dotProduct(ax) << std::endl;
		things.push_back(q);
		
		ax = Vector3(randFloat(), randFloat(), randFloat());
		ay = Vector3(randFloat(), randFloat(), randFloat());
		az = ax.crossProduct(ay);
		ay = az.crossProduct(ax);
		ax.normalise(); ay.normalise(); az.normalise();
		q.FromAxes(ax, ay, az);
		orbits.push_back(q);
	}
	
	// Create buffers
	size_t nvertices = mCount*4; // n+1 planes
	//size_t elemsize = 2*3; // position, normal
	//size_t dsize = elemsize*nvertices;
	
	Ogre::IndexData *idata = new Ogre::IndexData();
	Ogre::VertexData *vdata = new Ogre::VertexData();

	// Quads
	unsigned short *faces = new unsigned short[mCount*6];
	for(x=0; x<mCount; x++) 
	{
		faces[x*6+0] = x*4+0;
		faces[x*6+1] = x*4+1;
		faces[x*6+2] = x*4+2;
		faces[x*6+3] = x*4+0;
		faces[x*6+4] = x*4+2;
		faces[x*6+5] = x*4+3;
	}
	// Setup buffers
	vdata->vertexStart = 0;
	vdata->vertexCount = nvertices;
	
	VertexDeclaration* decl = vdata->vertexDeclaration;
	VertexBufferBinding* bind = vdata->vertexBufferBinding;

	size_t offset = 0;
	decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
	offset += VertexElement::getTypeSize(VET_FLOAT3);

	vbuf = 
	HardwareBufferManager::getSingleton().createVertexBuffer(
		offset, nvertices, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

	bind->setBinding(0, vbuf);

	//vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);
	
	HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().
		createIndexBuffer(
			HardwareIndexBuffer::IT_16BIT, 
			mCount*6, 
			HardwareBuffer::HBU_STATIC_WRITE_ONLY);

	idata->indexBuffer = ibuf;
	idata->indexCount = mCount*6;
	idata->indexStart = 0;
	ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);

	// Delete temporary buffers
	delete [] faces;
	
	// Now make the render operation
	mRenderOp.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
	mRenderOp.indexData = idata;
	mRenderOp.vertexData = vdata;
	mRenderOp.useIndexes = true;
}

void ThingRenderable::fillBuffer()
{
	// Transfer vertices and normals
	float *vIdx = static_cast<float*>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
	size_t elemsize = 1*3; // position only
	size_t planesize = 4*elemsize; // four vertices per plane
	for(size_t x=0; x<mCount; x++) 
	{
		Vector3 ax, ay, az;
		things[x].ToAxes(ax, ay, az);
		Vector3 pos = az * mRadius; // scale to radius
		ax *= mQSize;
		ay *= mQSize;
		Vector3 pos1 = pos - ax - ay;
		Vector3 pos2 = pos + ax - ay;
		Vector3 pos3 = pos + ax + ay;
		Vector3 pos4 = pos - ax + ay;
		vIdx[x*planesize + 0*elemsize + 0] = pos1.x;
		vIdx[x*planesize + 0*elemsize + 1] = pos1.y;
		vIdx[x*planesize + 0*elemsize + 2] = pos1.z;
		vIdx[x*planesize + 1*elemsize + 0] = pos2.x;
		vIdx[x*planesize + 1*elemsize + 1] = pos2.y;
		vIdx[x*planesize + 1*elemsize + 2] = pos2.z;
		vIdx[x*planesize + 2*elemsize + 0] = pos3.x;
		vIdx[x*planesize + 2*elemsize + 1] = pos3.y;
		vIdx[x*planesize + 2*elemsize + 2] = pos3.z;
		vIdx[x*planesize + 3*elemsize + 0] = pos4.x;
		vIdx[x*planesize + 3*elemsize + 1] = pos4.y;
		vIdx[x*planesize + 3*elemsize + 2] = pos4.z;
	}
	vbuf->unlock();
}
Ogre::Real ThingRenderable::getBoundingRadius() const
{
	return mRadius;
}
Ogre::Real ThingRenderable::getSquaredViewDepth(const Ogre::Camera* cam) const
{
	Ogre::Vector3 min, max, mid, dist;

	min = mBox.getMinimum();
	max = mBox.getMaximum();
	mid = ((min - max) * 0.5) + min;
	dist = cam->getDerivedPosition() - mid;
                                                                        
	return dist.squaredLength();
}

