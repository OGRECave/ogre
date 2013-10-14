/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#include "OgreStableHeaders.h"
#include "OgrePrefabFactory.h"
#include "OgreHardwareBufferManager.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"

namespace Ogre {
	//---------------------------------------------------------------------
	bool PrefabFactory::createPrefab(Mesh* mesh)
	{
		const String& resourceName = mesh->getName();

		if(resourceName == "Prefab_Plane")
		{
			createPlane(mesh);
			return true;
		}
		else if(resourceName == "Prefab_Cube")
		{
			createCube(mesh);
			return true;
		}
		else if(resourceName == "Prefab_Sphere")
		{
			createSphere(mesh);
			return true;
		}
	
		return false;
	}
	//---------------------------------------------------------------------
	void PrefabFactory::createPlane(Mesh* mesh)
	{
		SubMesh* sub = mesh->createSubMesh();
		float vertices[32] = {
			-100, -100, 0,	// pos
			0,0,1,			// normal
			0,1,			// texcoord
			100, -100, 0,
			0,0,1,
			1,1,
			100,  100, 0,
			0,0,1,
			1,0,
			-100,  100, 0 ,
			0,0,1,
			0,0 
		};
		mesh->sharedVertexData = OGRE_NEW VertexData();
		mesh->sharedVertexData->vertexCount = 4;
		VertexDeclaration* decl = mesh->sharedVertexData->vertexDeclaration;
		VertexBufferBinding* bind = mesh->sharedVertexData->vertexBufferBinding;

		size_t offset = 0;
		decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		decl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
		offset += VertexElement::getTypeSize(VET_FLOAT2);

		HardwareVertexBufferSharedPtr vbuf = 
			HardwareBufferManager::getSingleton().createVertexBuffer(
			offset, 4, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		bind->setBinding(0, vbuf);

		vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);

		sub->useSharedVertices = true;
		HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().
			createIndexBuffer(
			HardwareIndexBuffer::IT_16BIT, 
			6, 
			HardwareBuffer::HBU_STATIC_WRITE_ONLY);

		unsigned short faces[6] = {0,1,2,
			0,2,3 };
		sub->indexData->indexBuffer = ibuf;
		sub->indexData->indexCount = 6;
		sub->indexData->indexStart =0;
		ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);

		mesh->_setBounds(AxisAlignedBox(-100,-100,0,100,100,0), true);
		mesh->_setBoundingSphereRadius(Math::Sqrt(100*100+100*100));
	}
	//---------------------------------------------------------------------
	void PrefabFactory::createCube(Mesh* mesh)
	{
		SubMesh* sub = mesh->createSubMesh();

		const int NUM_VERTICES = 4 * 6; // 4 vertices per side * 6 sides
		const int NUM_ENTRIES_PER_VERTEX = 8;
		const int NUM_VERTEX_ENTRIES = NUM_VERTICES * NUM_ENTRIES_PER_VERTEX;
		const int NUM_INDICES = 3 * 2 * 6; // 3 indices per face * 2 faces per side * 6 sides

		const float CUBE_SIZE = 100.0f;
		const float CUBE_HALF_SIZE = CUBE_SIZE / 2.0f;

		// Create 4 vertices per side instead of 6 that are shared for the whole cube.
		// The reason for this is with only 6 vertices the normals will look bad
		// since each vertex can "point" in a different direction depending on the face it is included in.
		float vertices[NUM_VERTEX_ENTRIES] = {
			// front side
			-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE,	// pos
			0,0,1,	// normal
			0,1,	// texcoord
			CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE,
			0,0,1,
			1,1,
			CUBE_HALF_SIZE,  CUBE_HALF_SIZE, CUBE_HALF_SIZE,
			0,0,1,
			1,0,
			-CUBE_HALF_SIZE,  CUBE_HALF_SIZE, CUBE_HALF_SIZE ,
			0,0,1,
			0,0,

			// back side
			CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
			0,0,-1,
			0,1,
			-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
			0,0,-1,
			1,1,
			-CUBE_HALF_SIZE, CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
			0,0,-1,
			1,0,
			CUBE_HALF_SIZE, CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
			0,0,-1,
			0,0,

			// left side
			-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
			-1,0,0,
			0,1,
			-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE,
			-1,0,0,
			1,1,
			-CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE,
			-1,0,0,
			1,0,
			-CUBE_HALF_SIZE, CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
			-1,0,0,
			0,0, 

			// right side
			CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE,
			1,0,0,
			0,1,
			CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
			1,0,0,
			1,1,
			CUBE_HALF_SIZE, CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
			1,0,0,
			1,0,
			CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE,
			1,0,0,
			0,0,

			// up side
			-CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE,
			0,1,0,
			0,1,
			CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE,
			0,1,0,
			1,1,
			CUBE_HALF_SIZE, CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
			0,1,0,
			1,0,
			-CUBE_HALF_SIZE, CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
			0,1,0,
			0,0,

			// down side
			-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
			0,-1,0,
			0,1,
			CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
			0,-1,0,
			1,1,
			CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE,
			0,-1,0,
			1,0,
			-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE,
			0,-1,0,
			0,0 
		};

		mesh->sharedVertexData = OGRE_NEW VertexData();
		mesh->sharedVertexData->vertexCount = NUM_VERTICES;
		VertexDeclaration* decl = mesh->sharedVertexData->vertexDeclaration;
		VertexBufferBinding* bind = mesh->sharedVertexData->vertexBufferBinding;

		size_t offset = 0;
		decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		decl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
		offset += VertexElement::getTypeSize(VET_FLOAT2);

		HardwareVertexBufferSharedPtr vbuf = 
			HardwareBufferManager::getSingleton().createVertexBuffer(
			offset, NUM_VERTICES, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		bind->setBinding(0, vbuf);

		vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);

		sub->useSharedVertices = true;
		HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().
			createIndexBuffer(
			HardwareIndexBuffer::IT_16BIT, 
			NUM_INDICES,
			HardwareBuffer::HBU_STATIC_WRITE_ONLY);

		unsigned short faces[NUM_INDICES] = {
			// front
			0,1,2,
			0,2,3,

			// back
			4,5,6,
			4,6,7,

			// left
			8,9,10,
			8,10,11,

			// right
			12,13,14,
			12,14,15,

			// up
			16,17,18,
			16,18,19,

			// down
			20,21,22,
			20,22,23
		};

		sub->indexData->indexBuffer = ibuf;
		sub->indexData->indexCount = NUM_INDICES;
		sub->indexData->indexStart = 0;
		ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);

		mesh->_setBounds(AxisAlignedBox(-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
			CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE), true);

		mesh->_setBoundingSphereRadius(CUBE_HALF_SIZE);
	}
	//---------------------------------------------------------------------
	void PrefabFactory::createSphere(Mesh* mesh)
	{
		// sphere creation code taken from the DeferredShading sample, originally from the wiki
		SubMesh *pSphereVertex = mesh->createSubMesh();

		const int NUM_SEGMENTS = 16;
		const int NUM_RINGS = 16;
		const Real SPHERE_RADIUS = 50.0;

		mesh->sharedVertexData = OGRE_NEW VertexData();
		VertexData* vertexData = mesh->sharedVertexData;

		// define the vertex format
		VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
		size_t currOffset = 0;
		// positions
		vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_POSITION);
		currOffset += VertexElement::getTypeSize(VET_FLOAT3);
		// normals
		vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_NORMAL);
		currOffset += VertexElement::getTypeSize(VET_FLOAT3);
		// two dimensional texture coordinates
		vertexDecl->addElement(0, currOffset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);

		// allocate the vertex buffer
		vertexData->vertexCount = (NUM_RINGS + 1) * (NUM_SEGMENTS+1);
		HardwareVertexBufferSharedPtr vBuf = HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(0), vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		VertexBufferBinding* binding = vertexData->vertexBufferBinding;
		binding->setBinding(0, vBuf);
		float* pVertex = static_cast<float*>(vBuf->lock(HardwareBuffer::HBL_DISCARD));

		// allocate index buffer
		pSphereVertex->indexData->indexCount = 6 * NUM_RINGS * (NUM_SEGMENTS + 1);
		pSphereVertex->indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, pSphereVertex->indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		HardwareIndexBufferSharedPtr iBuf = pSphereVertex->indexData->indexBuffer;
		unsigned short* pIndices = static_cast<unsigned short*>(iBuf->lock(HardwareBuffer::HBL_DISCARD));

		float fDeltaRingAngle = (Math::PI / NUM_RINGS);
		float fDeltaSegAngle = (2 * Math::PI / NUM_SEGMENTS);
		unsigned short wVerticeIndex = 0 ;

		// Generate the group of rings for the sphere
		for( int ring = 0; ring <= NUM_RINGS; ring++ ) {
			float r0 = SPHERE_RADIUS * sinf (ring * fDeltaRingAngle);
			float y0 = SPHERE_RADIUS * cosf (ring * fDeltaRingAngle);

			// Generate the group of segments for the current ring
			for(int seg = 0; seg <= NUM_SEGMENTS; seg++) {
				float x0 = r0 * sinf(seg * fDeltaSegAngle);
				float z0 = r0 * cosf(seg * fDeltaSegAngle);

				// Add one vertex to the strip which makes up the sphere
				*pVertex++ = x0;
				*pVertex++ = y0;
				*pVertex++ = z0;

				Vector3 vNormal = Vector3(x0, y0, z0).normalisedCopy();
				*pVertex++ = vNormal.x;
				*pVertex++ = vNormal.y;
				*pVertex++ = vNormal.z;

				*pVertex++ = (float) seg / (float) NUM_SEGMENTS;
				*pVertex++ = (float) ring / (float) NUM_RINGS;

				if (ring != NUM_RINGS) {
					// each vertex (except the last) has six indicies pointing to it
					*pIndices++ = wVerticeIndex + NUM_SEGMENTS + 1;
					*pIndices++ = wVerticeIndex;               
					*pIndices++ = wVerticeIndex + NUM_SEGMENTS;
					*pIndices++ = wVerticeIndex + NUM_SEGMENTS + 1;
					*pIndices++ = wVerticeIndex + 1;
					*pIndices++ = wVerticeIndex;
					wVerticeIndex ++;
				}
			}; // end for seg
		} // end for ring

		// Unlock
		vBuf->unlock();
		iBuf->unlock();
		// Generate face list
		pSphereVertex->useSharedVertices = true;

		// the original code was missing this line:
		mesh->_setBounds( AxisAlignedBox( Vector3(-SPHERE_RADIUS, -SPHERE_RADIUS, -SPHERE_RADIUS), 
			Vector3(SPHERE_RADIUS, SPHERE_RADIUS, SPHERE_RADIUS) ), false );

		mesh->_setBoundingSphereRadius(SPHERE_RADIUS);
	}
	//---------------------------------------------------------------------
}
