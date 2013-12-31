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
#include "VolumeRenderable.h"
#include <OgreCamera.h>
#include <OgreSceneNode.h>
#include <OgreHardwareVertexBuffer.h>
#include <OgreHardwareIndexBuffer.h>
#include <OgreHardwareBufferManager.h>
#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreTextureUnitState.h>
#include <OgreTextureManager.h>
#include <OgreMaterialManager.h>
using namespace Ogre;

VolumeRenderable::VolumeRenderable(size_t nSlices, float size, const String &texture):
	mSlices(nSlices),
	mSize(size),
	mTexture(texture)
{
	mRadius = sqrtf(size*size+size*size+size*size)/2.0f;
	mBox = Ogre::AxisAlignedBox(-size, -size, -size, size, size, size);
	
	// No shadows
	setCastShadows(false);
	
	initialise();
}
VolumeRenderable::~VolumeRenderable()
{
	// Remove private material
	MaterialManager::getSingleton().remove(mTexture);
    // need to release IndexData and vertexData created for renderable
    delete mRenderOp.indexData;
    delete mRenderOp.vertexData;

}

void VolumeRenderable::_notifyCurrentCamera( Camera* cam )
{
	MovableObject::_notifyCurrentCamera(cam);

	// Fake orientation toward camera
	Vector3 zVec = getParentNode()->_getDerivedPosition() - cam->getDerivedPosition();
	zVec.normalise();
	Vector3 fixedAxis = cam->getDerivedOrientation() * Vector3::UNIT_Y ;
	
	Vector3 xVec = fixedAxis.crossProduct( zVec );
	xVec.normalise();

	Vector3 yVec = zVec.crossProduct( xVec );
	yVec.normalise();
	
	Quaternion oriQuat;
	oriQuat.FromAxes( xVec, yVec, zVec );
	
	oriQuat.ToRotationMatrix(mFakeOrientation);
	
	Matrix3 tempMat;
	Quaternion q = getParentNode()->_getDerivedOrientation().UnitInverse() * oriQuat ;
	q.ToRotationMatrix(tempMat);
	
	Matrix4 rotMat = Matrix4::IDENTITY;
	rotMat = tempMat;
	rotMat.setTrans(Vector3(0.5f, 0.5f, 0.5f));
	mUnit->setTextureTransform(rotMat);
}



void VolumeRenderable::getWorldTransforms( Matrix4* xform ) const
{
	Matrix4 destMatrix(Matrix4::IDENTITY); // this initialisation is needed
	
	const Vector3 &position = getParentNode()->_getDerivedPosition();
	const Vector3 &scale = getParentNode()->_getDerivedScale();
	Matrix3 scale3x3(Matrix3::ZERO);
	scale3x3[0][0] = scale.x;
	scale3x3[1][1] = scale.y;
	scale3x3[2][2] = scale.z;

	destMatrix = mFakeOrientation * scale3x3;
	destMatrix.setTrans(position);
		
	*xform = destMatrix;
}

void VolumeRenderable::initialise()
{
	// Create geometry
	size_t nvertices = mSlices*4; // n+1 planes
	size_t elemsize = 3*3;
	size_t dsize = elemsize*nvertices;
	size_t x;
	
	Ogre::IndexData *idata = new Ogre::IndexData();
	Ogre::VertexData *vdata = new Ogre::VertexData();
	
	// Create  structures
	float *vertices = new float[dsize];
	
	float coords[4][2] = {
		{0.0f, 0.0f},
		{0.0f, 1.0f},
		{1.0f, 0.0f},
		{1.0f, 1.0f}
	};
	for(x=0; x<mSlices; x++) 
	{
		for(size_t y=0; y<4; y++)
		{
			float xcoord = coords[y][0]-0.5f;
			float ycoord = coords[y][1]-0.5f;
			float zcoord = -((float)x/(float)(mSlices-1)  - 0.5f);
			// 1.0f .. a/(a+1)
			// coordinate
			vertices[x*4*elemsize+y*elemsize+0] = xcoord*(mSize/2.0f);
			vertices[x*4*elemsize+y*elemsize+1] = ycoord*(mSize/2.0f);
			vertices[x*4*elemsize+y*elemsize+2] = zcoord*(mSize/2.0f);
			// normal
			vertices[x*4*elemsize+y*elemsize+3] = 0.0f;
			vertices[x*4*elemsize+y*elemsize+4] = 0.0f;
			vertices[x*4*elemsize+y*elemsize+5] = 1.0f;
			// tex
			vertices[x*4*elemsize+y*elemsize+6] = xcoord*sqrtf(3.0f);
			vertices[x*4*elemsize+y*elemsize+7] = ycoord*sqrtf(3.0f);
			vertices[x*4*elemsize+y*elemsize+8] = zcoord*sqrtf(3.0f);
		} 
	}
	unsigned short *faces = new unsigned short[mSlices*6];
	for(x=0; x<mSlices; x++) 
	{
		faces[x*6+0] = x*4+0;
		faces[x*6+1] = x*4+1;
		faces[x*6+2] = x*4+2;
		faces[x*6+3] = x*4+1;
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
	decl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
	offset += VertexElement::getTypeSize(VET_FLOAT3);
	decl->addElement(0, offset, VET_FLOAT3, VES_TEXTURE_COORDINATES);
	offset += VertexElement::getTypeSize(VET_FLOAT3);

	HardwareVertexBufferSharedPtr vbuf = 
	HardwareBufferManager::getSingleton().createVertexBuffer(
		offset, nvertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

	bind->setBinding(0, vbuf);

	vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);
	
	HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().
		createIndexBuffer(
			HardwareIndexBuffer::IT_16BIT, 
			mSlices*6, 
			HardwareBuffer::HBU_STATIC_WRITE_ONLY);

	idata->indexBuffer = ibuf;
	idata->indexCount = mSlices*6;
	idata->indexStart = 0;
	ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);

	// Delete temporary buffers
	delete [] vertices;
	delete [] faces;
	
	// Now make the render operation
	mRenderOp.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
	mRenderOp.indexData = idata;
	mRenderOp.vertexData = vdata;
	mRenderOp.useIndexes = true;
	
	 // Create a brand new private material
	if (!ResourceGroupManager::getSingleton().resourceGroupExists("VolumeRenderable"))
	{
		ResourceGroupManager::getSingleton().createResourceGroup("VolumeRenderable");
	}
	MaterialPtr material = 
		MaterialManager::getSingleton().create(mTexture, "VolumeRenderable",
			false, 0); // Manual, loader

	// Remove pre-created technique from defaults
	material->removeAllTechniques();
	
	// Create a techinique and a pass and a texture unit
 	Technique * technique = material->createTechnique();
	Pass * pass = technique->createPass();
	TextureUnitState * textureUnit = pass->createTextureUnitState();
	
	// Set pass parameters
	pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
	pass->setDepthWriteEnabled(false);
	pass->setCullingMode(CULL_NONE);
	pass->setLightingEnabled(false);
	
	// Set texture unit parameters
	textureUnit->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
	textureUnit->setTextureName(mTexture, TEX_TYPE_3D);
	textureUnit->setTextureFiltering(TFO_TRILINEAR);
	
	mUnit = textureUnit;
	mMaterial = material;
}

Ogre::Real VolumeRenderable::getBoundingRadius() const
{
	return mRadius;
}
Ogre::Real VolumeRenderable::getSquaredViewDepth(const Ogre::Camera* cam) const
{
	Ogre::Vector3 min, max, mid, dist;

	min = mBox.getMinimum();
	max = mBox.getMaximum();
	mid = ((min - max) * 0.5) + min;
	dist = cam->getDerivedPosition() - mid;
                                                                        
	return dist.squaredLength();
}

