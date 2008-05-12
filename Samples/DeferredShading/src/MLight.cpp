/******************************************************************************
Copyright (c) W.J. van der Laan

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software  and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject 
to the following conditions:

The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/
#include "MLight.h"

#include "OgreStringConverter.h"
#include "OgreException.h"

#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreResourceGroupManager.h"
#include "OgreMeshManager.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreHardwareBufferManager.h"

#include "OgreRoot.h"

#include "OgreCamera.h"

#include "MaterialGenerator.h"

using namespace Ogre;
//-----------------------------------------------------------------------
MLight::MLight(MaterialGenerator *sys):
	bIgnoreWorld(false), mGenerator(sys),mPermutation(0)
{
	// Set up geometry
	//setMaterial("DeferredShading/Post/LightMaterial");
	// Set render priority to high (just after normal postprocess)
	setRenderQueueGroup(RENDER_QUEUE_2);
	// Allocate render operation
	mRenderOp.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
	mRenderOp.indexData = 0;
	mRenderOp.vertexData = 0;
	mRenderOp.useIndexes = true;

	// Diffuse and specular colour
	setDiffuseColour(Ogre::ColourValue(1,1,1));
	setSpecularColour(Ogre::ColourValue(0,0,0));
	// Set Attenuation
	setAttenuation(1.0f,0.0f,0.0f);
}
//-----------------------------------------------------------------------
MLight::~MLight()
{
	// need to release IndexData and vertexData created for renderable
    delete mRenderOp.indexData;
    delete mRenderOp.vertexData;
}
//-----------------------------------------------------------------------
void MLight::setAttenuation(float c, float b, float a)
{
	// Set Attenuation parameter to shader
	setCustomParameter(3, Vector4(c, b, a, 0));

	/// There is attenuation? Set material accordingly
	if(c != 1.0f || b != 0.0f || a != 0.0f)
		mPermutation |= MI_ATTENUATED;
	else
		mPermutation &= ~MI_ATTENUATED;

	// Calculate radius from Attenuation
	int threshold_level = 15;// differece of 10-15 levels deemed unnoticable
	float threshold = 1.0f/((float)threshold_level/256.0f); 

	// Use quadratic formula to determine outer radius
	c = c-threshold;
	float d=sqrt(b*b-4*a*c);
	float x=(-2*c)/(b+d);

	rebuildGeometry(x);
}
//-----------------------------------------------------------------------
void MLight::setDiffuseColour(const Ogre::ColourValue &col)
{
	setCustomParameter(1, Vector4(col.r, col.g, col.b, col.a));
}
//-----------------------------------------------------------------------
void MLight::setSpecularColour(const Ogre::ColourValue &col)
{
	setCustomParameter(2, Vector4(col.r, col.g, col.b, col.a));
	/// There is a specular component? Set material accordingly
	
	if(col.r != 0.0f || col.g != 0.0f || col.b != 0.0f)
		mPermutation |= MI_SPECULAR;
	else
		mPermutation &= ~MI_SPECULAR;
		
}
//-----------------------------------------------------------------------
Ogre::ColourValue MLight::getDiffuseColour()
{
	Ogre::Vector4 val = getCustomParameter(1);
	return Ogre::ColourValue(val[0], val[1], val[2], val[3]);
}
//-----------------------------------------------------------------------
Ogre::ColourValue MLight::getSpecularColour()
{
	Ogre::Vector4 val = getCustomParameter(2);
	return Ogre::ColourValue(val[0], val[1], val[2], val[3]);
}
//-----------------------------------------------------------------------
void MLight::rebuildGeometry(float radius)
{
	// Scale node to radius
	
	if(radius > 10000.0f)
	{
		createRectangle2D();
		mPermutation |= MI_QUAD;
	}
	else
	{
		/// XXX some more intelligent expression for rings and segments
		createSphere(radius, 5, 5);
		mPermutation &= ~MI_QUAD;
	}	
}
//-----------------------------------------------------------------------
void MLight::createRectangle2D()
{
	/// XXX this RenderOp should really be re-used between MLight objects,
	/// not generated every time
	delete mRenderOp.vertexData; 
	delete mRenderOp.indexData; 
	mRenderOp.vertexData = new VertexData();
    mRenderOp.indexData = 0;

    mRenderOp.vertexData->vertexCount = 4; 
    mRenderOp.vertexData->vertexStart = 0; 
    mRenderOp.operationType = RenderOperation::OT_TRIANGLE_STRIP; 
    mRenderOp.useIndexes = false; 

    VertexDeclaration* decl = mRenderOp.vertexData->vertexDeclaration;
    VertexBufferBinding* bind = mRenderOp.vertexData->vertexBufferBinding;

    decl->addElement(0, 0, VET_FLOAT3, VES_POSITION);

    HardwareVertexBufferSharedPtr vbuf = 
        HardwareBufferManager::getSingleton().createVertexBuffer(
        decl->getVertexSize(0),
        mRenderOp.vertexData->vertexCount,
        HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    // Bind buffer
    bind->setBinding(0, vbuf);
	// Upload data
	float data[]={
		-1,1,-1,  // corner 1
		-1,-1,-1, // corner 2
		1,1,-1,   // corner 3
		1,-1,-1}; // corner 4
	vbuf->writeData(0, sizeof(data), data, true);

	// Set bounding
    setBoundingBox(AxisAlignedBox(-10000,-10000,-10000,10000,10000,10000));
	mRadius = 15000;
	bIgnoreWorld = true;
}
//-----------------------------------------------------------------------
void MLight::createSphere(const float r, const int nRings, const int nSegments)
{
	delete mRenderOp.vertexData; 
	delete mRenderOp.indexData;
	mRenderOp.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
	mRenderOp.indexData = new IndexData();
	mRenderOp.vertexData = new VertexData();
	mRenderOp.useIndexes = true;

	VertexData* vertexData = mRenderOp.vertexData;
	IndexData* indexData = mRenderOp.indexData;

	// define the vertex format
	VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
	size_t currOffset = 0;
	// only generate positions
	vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_POSITION);
	currOffset += VertexElement::getTypeSize(VET_FLOAT3);
	// allocate the vertex buffer
	vertexData->vertexCount = (nRings + 1) * (nSegments+1);
	HardwareVertexBufferSharedPtr vBuf = HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(0), vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
	VertexBufferBinding* binding = vertexData->vertexBufferBinding;
	binding->setBinding(0, vBuf);
	float* pVertex = static_cast<float*>(vBuf->lock(HardwareBuffer::HBL_DISCARD));

	// allocate index buffer
	indexData->indexCount = 6 * nRings * (nSegments + 1);
	indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
	HardwareIndexBufferSharedPtr iBuf = indexData->indexBuffer;
	unsigned short* pIndices = static_cast<unsigned short*>(iBuf->lock(HardwareBuffer::HBL_DISCARD));

	float fDeltaRingAngle = (Math::PI / nRings);
	float fDeltaSegAngle = (2 * Math::PI / nSegments);
	unsigned short wVerticeIndex = 0 ;

	// Generate the group of rings for the sphere
	for( int ring = 0; ring <= nRings; ring++ ) {
		float r0 = r * sinf (ring * fDeltaRingAngle);
		float y0 = r * cosf (ring * fDeltaRingAngle);

		// Generate the group of segments for the current ring
		for(int seg = 0; seg <= nSegments; seg++) {
			float x0 = r0 * sinf(seg * fDeltaSegAngle);
			float z0 = r0 * cosf(seg * fDeltaSegAngle);

			// Add one vertex to the strip which makes up the sphere
			*pVertex++ = x0;
			*pVertex++ = y0;
			*pVertex++ = z0;

			if (ring != nRings) 
			{
                // each vertex (except the last) has six indicies pointing to it
				*pIndices++ = wVerticeIndex + nSegments + 1;
				*pIndices++ = wVerticeIndex;               
				*pIndices++ = wVerticeIndex + nSegments;
				*pIndices++ = wVerticeIndex + nSegments + 1;
				*pIndices++ = wVerticeIndex + 1;
				*pIndices++ = wVerticeIndex;
				wVerticeIndex ++;
			}
		}; // end for seg
	} // end for ring

	// Unlock
	vBuf->unlock();
	iBuf->unlock();

	// Set bounding box and sphere
	setBoundingBox( AxisAlignedBox( Vector3(-r, -r, -r), Vector3(r, r, r) ) );
	mRadius = r;
	bIgnoreWorld = false;
}								 
//-----------------------------------------------------------------------
Real MLight::getBoundingRadius(void) const
{
	return mRadius;
}
//-----------------------------------------------------------------------
Ogre::Real MLight::getSquaredViewDepth(const Ogre::Camera* cam) const
{
	if(bIgnoreWorld)
	{
		return 0.0f;
	}
	else
	{
		Vector3 dist = cam->getDerivedPosition() - getParentSceneNode()->_getDerivedPosition();
		return dist.squaredLength();
	}
}
//-----------------------------------------------------------------------
const MaterialPtr& MLight::getMaterial(void) const
{
	return mGenerator->getMaterial(mPermutation);
}
