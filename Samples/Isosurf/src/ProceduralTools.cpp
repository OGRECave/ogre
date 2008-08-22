#include "ProceduralTools.h"

#include <Ogre.h>

using namespace Ogre;

//Constants (copied as is from sample)

// Grid sizes (in vertices)
#define X_SIZE_LOG2		6
#define Y_SIZE_LOG2		6
#define Z_SIZE_LOG2		6
#define TOTAL_POINTS	(1<<(X_SIZE_LOG2 + Y_SIZE_LOG2 + Z_SIZE_LOG2))
#define CELLS_COUNT		(((1<<X_SIZE_LOG2) - 1) * ((1<<Y_SIZE_LOG2) - 1) * ((1<<Z_SIZE_LOG2) - 1))

#define SWIZZLE	1

#define MAKE_INDEX(x, y, z, sizeLog2)	(int)((x) | ((y) << sizeLog2[0]) | ((z) << (sizeLog2[0] + sizeLog2[1])))

//--------------------------------------------------------------------------------------
// Fills pPos with x, y, z de-swizzled from index with bitsizes in sizeLog2
//
//  Traversing the grid in a swizzled fashion improves locality of reference,
// and this is very beneficial when sampling a texture.
//--------------------------------------------------------------------------------------
void UnSwizzle(uint index, uint sizeLog2[3], uint * pPos)
{

    // force index traversal to occur in 2x2x2 blocks by giving each of x, y, and z one
    // of the bottom 3 bits
	pPos[0] = index & 1;
    index >>= 1;
    pPos[1] = index & 1;
    index >>= 1;
    pPos[2] = index & 1;
    index >>= 1;

    // Treat the rest of the index like a row, collumn, depth array
    // Each dimension needs to grab sizeLog2 - 1 bits
    // This will make the blocks traverse the grid in a raster style order
    index <<= 1;
    pPos[0] = pPos[0] | (index &  ( (1 << sizeLog2[0]) - 2));
    index >>=  sizeLog2[0] - 1;
    pPos[1] = pPos[1] | ( index &  ( (1 << sizeLog2[1]) - 2));
    index >>= sizeLog2[1] - 1;
    pPos[2] = pPos[2] | ( index &  ( (1 << sizeLog2[2]) - 2));
}


MeshPtr ProceduralTools::generateTetrahedra()
{
	MeshPtr tetrahedraMesh = Ogre::MeshManager::getSingleton().createManual
		("TetrahedraMesh", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	SubMesh* tetrahedraSubMesh = tetrahedraMesh->createSubMesh();
	tetrahedraSubMesh->operationType = RenderOperation::OT_LINE_LIST;
	tetrahedraSubMesh->setMaterialName("Ogre/IsoSurf/TessellateTetrahedra");
	
	uint sizeLog2[3] = { X_SIZE_LOG2, Y_SIZE_LOG2, Z_SIZE_LOG2 };
	uint nTotalBits = sizeLog2[0] + sizeLog2[1] + sizeLog2[2];
	uint nPointsTotal = 1 << nTotalBits;

	tetrahedraSubMesh->useSharedVertices = false;
	tetrahedraSubMesh->vertexData = new VertexData;
	tetrahedraSubMesh->indexData = new IndexData;

	tetrahedraSubMesh->vertexData->vertexDeclaration->addElement(0, 0, 
		VET_FLOAT4, VES_POSITION);

	HardwareVertexBufferSharedPtr vertexBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
		tetrahedraSubMesh->vertexData->vertexDeclaration->getVertexSize(0), 
		nPointsTotal, 
		HardwareBuffer::HBU_STATIC_WRITE_ONLY);

	HardwareIndexBufferSharedPtr indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
		HardwareIndexBuffer::IT_32BIT, 
		CELLS_COUNT * sizeof(uint) * 24, 
		HardwareBuffer::HBU_STATIC_WRITE_ONLY);
	
	tetrahedraSubMesh->vertexData->vertexBufferBinding->setBinding(0, vertexBuffer);
	tetrahedraSubMesh->vertexData->vertexCount = nPointsTotal;
	tetrahedraSubMesh->vertexData->vertexStart = 0;
	
	tetrahedraSubMesh->indexData->indexBuffer = indexBuffer;
	
	float* positions = static_cast<float*>(vertexBuffer->lock(HardwareBuffer::HBL_DISCARD));
	
	//Generate positions
	for(uint i=0; i<nPointsTotal; i++) {
		uint pos[3];
		pos[0] = i & ((1<<X_SIZE_LOG2)-1);
		pos[1] = (i >> X_SIZE_LOG2) & ((1<<Y_SIZE_LOG2)-1);
		pos[2] = (i >> (X_SIZE_LOG2+Y_SIZE_LOG2)) & ((1<<Z_SIZE_LOG2)-1);

		*positions++ = (float(pos[0]) / float(1<<X_SIZE_LOG2))*2.0-1.0;
		*positions++ = (float(pos[1]) / float(1<<Y_SIZE_LOG2))*2.0-1.0;
		*positions++ = (float(pos[2]) / float(1<<Z_SIZE_LOG2))*2.0-1.0;
		*positions++ = 1.0f;

		float* logPositions = positions - 4;
	}
	vertexBuffer->unlock();
	
	uint numIndices = 0;

	//Generate indices
	uint32* indices = static_cast<uint32*>(indexBuffer->lock(HardwareBuffer::HBL_DISCARD));

	for (uint i = 0; i<nPointsTotal; i++) {

		uint pos[3];
#if SWIZZLE
		UnSwizzle(i, sizeLog2, pos);	// un-swizzle current index to get x, y, z for the current sampling point
#else
		pos[0] = i & ((1<<X_SIZE_LOG2)-1);
		pos[1] = (i >> X_SIZE_LOG2) & ((1<<Y_SIZE_LOG2)-1);
		pos[2] = (i >> (X_SIZE_LOG2+Y_SIZE_LOG2)) & ((1<<Z_SIZE_LOG2)-1);
#endif
		if (pos[0] == (1 << sizeLog2[0]) - 1 || pos[1] == (1 << sizeLog2[1]) - 1 || pos[2] == (1 << sizeLog2[2]) - 1)
			continue;	// skip extra cells

		numIndices += 24; //Got to this point, adding 24 indices

		// NOTE: order of vertices matters! important for backface culling

		// T0
		*indices++ = MAKE_INDEX(pos[0] + 1, pos[1], pos[2], sizeLog2);
		*indices++ = MAKE_INDEX(pos[0], pos[1], pos[2], sizeLog2);
		*indices++ = MAKE_INDEX(pos[0] + 1, pos[1] + 1, pos[2], sizeLog2);
		*indices++ = MAKE_INDEX(pos[0] + 1, pos[1] + 1, pos[2] + 1, sizeLog2);

		// T1
		*indices++ = MAKE_INDEX(pos[0] + 1, pos[1] + 1, pos[2] + 1, sizeLog2);
		*indices++ = MAKE_INDEX(pos[0], pos[1], pos[2], sizeLog2);
		*indices++ = MAKE_INDEX(pos[0] + 1, pos[1] + 1, pos[2], sizeLog2);
		*indices++ = MAKE_INDEX(pos[0], pos[1] + 1, pos[2], sizeLog2);

		// T2
		*indices++ = MAKE_INDEX(pos[0], pos[1] + 1, pos[2], sizeLog2);
		*indices++ = MAKE_INDEX(pos[0], pos[1], pos[2], sizeLog2);
		*indices++ = MAKE_INDEX(pos[0], pos[1] + 1, pos[2] + 1, sizeLog2);
		*indices++ = MAKE_INDEX(pos[0] + 1, pos[1] + 1, pos[2] + 1, sizeLog2);

		// T3
		*indices++ = MAKE_INDEX(pos[0], pos[1], pos[2], sizeLog2);
		*indices++ = MAKE_INDEX(pos[0], pos[1], pos[2] + 1, sizeLog2);
		*indices++ = MAKE_INDEX(pos[0], pos[1] + 1, pos[2] + 1, sizeLog2);
		*indices++ = MAKE_INDEX(pos[0] + 1, pos[1] + 1, pos[2] + 1, sizeLog2);

		// T4
		*indices++ = MAKE_INDEX(pos[0], pos[1], pos[2] + 1, sizeLog2);
		*indices++ = MAKE_INDEX(pos[0], pos[1], pos[2], sizeLog2);
		*indices++ = MAKE_INDEX(pos[0] + 1, pos[1], pos[2] + 1, sizeLog2);
		*indices++ = MAKE_INDEX(pos[0] + 1, pos[1] + 1, pos[2] + 1, sizeLog2);

		// T5
		*indices++ = MAKE_INDEX(pos[0], pos[1], pos[2], sizeLog2);
		*indices++ = MAKE_INDEX(pos[0] + 1, pos[1], pos[2], sizeLog2);
		*indices++ = MAKE_INDEX(pos[0] + 1, pos[1], pos[2] + 1, sizeLog2);
		*indices++ = MAKE_INDEX(pos[0] + 1, pos[1] + 1, pos[2] + 1, sizeLog2);
	}
	
	indexBuffer->unlock();

	tetrahedraSubMesh->indexData->indexCount = numIndices;
	tetrahedraSubMesh->indexData->indexStart = 0;

	AxisAlignedBox meshBounds;
	meshBounds.setMinimum(-1,-1,-1);
	meshBounds.setMaximum(1,1,1);
	tetrahedraMesh->_setBounds(meshBounds);
	tetrahedraMesh->_setBoundingSphereRadius(2);

	return tetrahedraMesh;
}