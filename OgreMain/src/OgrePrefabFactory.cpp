/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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

namespace Ogre {
    //-----------------------------------------------------------------------
    void PrefabFactory::loadResource(Resource* res)
    {
        Mesh* msh = static_cast<Mesh*>(res);

        // attempt to create a prefab mesh
        const String& resourceName = msh->getName();
        if(resourceName == "Prefab_Plane")
        {
            createPlane(msh);
            return;
        }
        else if(resourceName == "Prefab_Cube")
        {
            createCube(msh);
            return;
        }
        else if(resourceName == "Prefab_Sphere")
        {
            createSphere(msh);
            return;
        }

        // the mesh was not a prefab.. grab build parameters
        auto any = msh->getUserObjectBindings().getUserAny("_MeshBuildParams");

        if (!any.has_value())
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "No prefab parameters in " + res->getName());

        auto params = any_cast<MeshBuildParams>(any);

        switch(params.type)
        {
        case MBT_PLANE:
            loadManualPlane(msh, params);
            return;
        case MBT_CURVED_ILLUSION_PLANE:
            loadManualCurvedIllusionPlane(msh, params);
            return;
        case MBT_CURVED_PLANE:
            loadManualCurvedPlane(msh, params);
            return;
        }

        OgreAssertDbg(false, "unknown prefab type");
    }
    //---------------------------------------------------------------------
    void PrefabFactory::createPlane(Mesh* mesh)
    {
        //! [manual_plane_geometry]
        float vertices[32] = {
            -100, -100, 0,  // pos
            0,0,1,          // normal
            0,1,            // texcoord
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

        uint16 faces[6] = {0,1,2,
                           0,2,3 };
        //! [manual_plane_geometry]

        //! [vertex_data]
        mesh->sharedVertexData = new VertexData();
        mesh->sharedVertexData->vertexCount = 4;
        VertexDeclaration* decl = mesh->sharedVertexData->vertexDeclaration;
        VertexBufferBinding* bind = mesh->sharedVertexData->vertexBufferBinding;
        //! [vertex_data]

        //! [vertex_decl]
        size_t offset = 0;
        offset += decl->addElement(0, offset, VET_FLOAT3, VES_POSITION).getSize();
        offset += decl->addElement(0, offset, VET_FLOAT3, VES_NORMAL).getSize();
        offset += decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0).getSize();
        //! [vertex_decl]

        //! [vertex_buffer]
        HardwareVertexBufferPtr vbuf =
            HardwareBufferManager::getSingleton().createVertexBuffer(offset, 4, HBU_GPU_ONLY);
        vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);
        bind->setBinding(0, vbuf);

        HardwareIndexBufferPtr ibuf =
            HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, 6, HBU_GPU_ONLY);
        ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);
        //! [vertex_buffer]

        //! [sub_mesh]
        SubMesh* sub = mesh->createSubMesh();
        sub->useSharedVertices = true;
        sub->indexData->indexBuffer = ibuf;
        sub->indexData->indexCount = 6;
        sub->indexData->indexStart = 0;
        //! [sub_mesh]

        mesh->_setBounds(AxisAlignedBox(-100,-100,0,100,100,0), true);
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
            -CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE,   // pos
            0,0,1,  // normal
            0,1,    // texcoord
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
        offset += decl->addElement(0, offset, VET_FLOAT3, VES_POSITION).getSize();
        offset += decl->addElement(0, offset, VET_FLOAT3, VES_NORMAL).getSize();
        offset += decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0).getSize();

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
        currOffset += vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_POSITION).getSize();
        currOffset += vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_NORMAL).getSize();
        currOffset += vertexDecl->addElement(0, currOffset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0).getSize();

        // allocate the vertex buffer
        vertexData->vertexCount = (NUM_RINGS + 1) * (NUM_SEGMENTS+1);
        HardwareVertexBufferSharedPtr vBuf = HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(0), vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
        VertexBufferBinding* binding = vertexData->vertexBufferBinding;
        binding->setBinding(0, vBuf);
        HardwareBufferLockGuard vBufLock(vBuf, HardwareBuffer::HBL_DISCARD);
        float* pVertex = static_cast<float*>(vBufLock.pData);

        // allocate index buffer
        pSphereVertex->indexData->indexCount = 6 * NUM_RINGS * (NUM_SEGMENTS + 1);
        pSphereVertex->indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, pSphereVertex->indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
        HardwareIndexBufferSharedPtr iBuf = pSphereVertex->indexData->indexBuffer;
        HardwareBufferLockGuard iBufLock(iBuf, HardwareBuffer::HBL_DISCARD);
        unsigned short* pIndices = static_cast<unsigned short*>(iBufLock.pData);

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

        // Generate face list
        pSphereVertex->useSharedVertices = true;

        // the original code was missing this line:
        mesh->_setBounds( AxisAlignedBox( Vector3(-SPHERE_RADIUS, -SPHERE_RADIUS, -SPHERE_RADIUS), 
            Vector3(SPHERE_RADIUS, SPHERE_RADIUS, SPHERE_RADIUS) ), false );

        mesh->_setBoundingSphereRadius(SPHERE_RADIUS);
    }
    //-----------------------------------------------------------------------
    static void tesselate2DMesh(SubMesh* sm, unsigned short meshWidth, unsigned short meshHeight, bool doubleSided,
                                HardwareBuffer::Usage indexBufferUsage, bool indexShadowBuffer)
    {
        // The mesh is built, just make a list of indexes to spit out the triangles
        unsigned short vInc, v, iterations;

        if (doubleSided)
        {
            iterations = 2;
            vInc = 1;
            v = 0; // Start with front
        }
        else
        {
            iterations = 1;
            vInc = 1;
            v = 0;
        }

        // Allocate memory for faces
        // Num faces, width*height*2 (2 tris per square), index count is * 3 on top
        sm->indexData->indexCount = (meshWidth-1) * (meshHeight-1) * 2 * iterations * 3;
        sm->indexData->indexBuffer = HardwareBufferManager::getSingleton().
            createIndexBuffer(HardwareIndexBuffer::IT_16BIT,
            sm->indexData->indexCount, indexBufferUsage, indexShadowBuffer);

        unsigned short v1, v2, v3;
        //bool firstTri = true;
        HardwareIndexBufferSharedPtr ibuf = sm->indexData->indexBuffer;
        // Lock the whole buffer
        HardwareBufferLockGuard ibufLock(ibuf, HardwareBuffer::HBL_DISCARD);
        unsigned short* pIndexes = static_cast<unsigned short*>(ibufLock.pData);

        while (iterations--)
        {
            // Make tris in a zigzag pattern (compatible with strips)
            unsigned short u = 0;
            unsigned short uInc = 1; // Start with moving +u
            unsigned short vCount = meshHeight - 1;

            while (vCount--)
            {
                unsigned short uCount = meshWidth - 1;
                while (uCount--)
                {
                    // First Tri in cell
                    // -----------------
                    v1 = ((v + vInc) * meshWidth) + u;
                    v2 = (v * meshWidth) + u;
                    v3 = ((v + vInc) * meshWidth) + (u + uInc);
                    // Output indexes
                    *pIndexes++ = v1;
                    *pIndexes++ = v2;
                    *pIndexes++ = v3;
                    // Second Tri in cell
                    // ------------------
                    v1 = ((v + vInc) * meshWidth) + (u + uInc);
                    v2 = (v * meshWidth) + u;
                    v3 = (v * meshWidth) + (u + uInc);
                    // Output indexes
                    *pIndexes++ = v1;
                    *pIndexes++ = v2;
                    *pIndexes++ = v3;

                    // Next column
                    u += uInc;
                }
                // Next row
                v += vInc;
                u = 0;


            }

            // Reverse vInc for double sided
            v = meshHeight - 1;
            vInc = -vInc;

        }
    }
    //-----------------------------------------------------------------------
    void PrefabFactory::loadManualPlane(Mesh* pMesh, MeshBuildParams& params)
    {
        if ((params.xsegments + 1) * (params.ysegments + 1) > 65536)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Plane tessellation is too high, must generate max 65536 vertices");
        SubMesh *pSub = pMesh->createSubMesh();

        // Set up vertex data
        // Use a single shared buffer
        pMesh->sharedVertexData = OGRE_NEW VertexData();
        VertexData* vertexData = pMesh->sharedVertexData;
        // Set up Vertex Declaration
        VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
        size_t currOffset = 0;
        // We always need positions
        currOffset += vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_POSITION).getSize();
        // Optional normals
        if(params.normals)
        {
            currOffset += vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_NORMAL).getSize();
        }

        for (unsigned short i = 0; i < params.numTexCoordSets; ++i)
        {
            // Assumes 2D texture coords
            currOffset += vertexDecl->addElement(0, currOffset, VET_FLOAT2, VES_TEXTURE_COORDINATES, i).getSize();
        }

        vertexData->vertexCount = (params.xsegments + 1) * (params.ysegments + 1);

        // Allocate vertex buffer
        HardwareVertexBufferSharedPtr vbuf =
            pMesh->getHardwareBufferManager()->createVertexBuffer(
                vertexDecl->getVertexSize(0), vertexData->vertexCount,
                params.vertexBufferUsage, params.vertexShadowBuffer);

        // Set up the binding (one source only)
        VertexBufferBinding* binding = vertexData->vertexBufferBinding;
        binding->setBinding(0, vbuf);

        // Work out the transform required
        // Default orientation of plane is normal along +z, distance 0
        Affine3 xlate, xform, rot;
        Matrix3 rot3;
        xlate = rot = Affine3::IDENTITY;
        // Determine axes
        Vector3 zAxis, yAxis, xAxis;
        zAxis = params.plane.normal;
        zAxis.normalise();
        yAxis = params.upVector;
        yAxis.normalise();
        xAxis = yAxis.crossProduct(zAxis);
        if (xAxis.squaredLength() == 0)
        {
            //upVector must be wrong
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "The upVector you supplied is parallel to the plane normal, so is not valid.");
        }
        xAxis.normalise();

        rot3.FromAxes(xAxis, yAxis, zAxis);
        rot = rot3;

        // Set up standard transform from origin
        xlate.setTrans(params.plane.normal * -params.plane.d);

        // concatenate
        xform = xlate * rot;

        // Generate vertex data
        // Lock the whole buffer
        HardwareBufferLockGuard vbufLock(vbuf, HardwareBuffer::HBL_DISCARD);
        float* pReal = static_cast<float*>(vbufLock.pData);
        Real xSpace = params.width / params.xsegments;
        Real ySpace = params.height / params.ysegments;
        Real halfWidth = params.width / 2;
        Real halfHeight = params.height / 2;
        Real xTex = (1.0f * params.xTile) / params.xsegments;
        Real yTex = (1.0f * params.yTile) / params.ysegments;
        Vector3 vec;
        AxisAlignedBox aabb;

        for (int y = 0; y < params.ysegments + 1; ++y)
        {
            for (int x = 0; x < params.xsegments + 1; ++x)
            {
                // Work out centered on origin
                vec.x = (x * xSpace) - halfWidth;
                vec.y = (y * ySpace) - halfHeight;
                vec.z = 0.0f;
                // Transform by orientation and distance
                vec = xform * vec;
                // Assign to geometry
                *pReal++ = vec.x;
                *pReal++ = vec.y;
                *pReal++ = vec.z;

                // Build bounds as we go
                aabb.merge(vec);

                if (params.normals)
                {
                    // Default normal is along unit Z
                    vec = Vector3::UNIT_Z;
                    // Rotate
                    vec = rot * vec;

                    *pReal++ = vec.x;
                    *pReal++ = vec.y;
                    *pReal++ = vec.z;
                }

                for (unsigned short i = 0; i < params.numTexCoordSets; ++i)
                {
                    *pReal++ = x * xTex;
                    *pReal++ = 1 - (y * yTex);
                }


            } // x
        } // y

        // Unlock
        vbufLock.unlock();
        // Generate face list
        pSub->useSharedVertices = true;
        tesselate2DMesh(pSub, params.xsegments + 1, params.ysegments + 1, false,
            params.indexBufferUsage, params.indexShadowBuffer);

        pMesh->_setBounds(aabb, true);
    }
    //-----------------------------------------------------------------------
    void PrefabFactory::loadManualCurvedPlane(Mesh* pMesh, MeshBuildParams& params)
    {
        if ((params.xsegments + 1) * (params.ysegments + 1) > 65536)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Plane tessellation is too high, must generate max 65536 vertices");
        SubMesh *pSub = pMesh->createSubMesh();

        // Set options
        pMesh->sharedVertexData = OGRE_NEW VertexData();
        pMesh->sharedVertexData->vertexStart = 0;
        VertexBufferBinding* bind = pMesh->sharedVertexData->vertexBufferBinding;
        VertexDeclaration* decl = pMesh->sharedVertexData->vertexDeclaration;

        pMesh->sharedVertexData->vertexCount = (params.xsegments + 1) * (params.ysegments + 1);

        size_t offset = 0;
        offset += decl->addElement(0, offset, VET_FLOAT3, VES_POSITION).getSize();
        if (params.normals)
        {
            offset += decl->addElement(0, 0, VET_FLOAT3, VES_NORMAL).getSize();
        }

        for (unsigned short i = 0; i < params.numTexCoordSets; ++i)
        {
            offset += decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, i).getSize();
        }


        // Allocate memory
        HardwareVertexBufferSharedPtr vbuf =
            pMesh->getHardwareBufferManager()->createVertexBuffer(
                offset, pMesh->sharedVertexData->vertexCount,
                params.vertexBufferUsage, params.vertexShadowBuffer);
        bind->setBinding(0, vbuf);

        // Work out the transform required
        // Default orientation of plane is normal along +z, distance 0
        Affine3 xlate, xform, rot;
        Matrix3 rot3;
        xlate = rot = Affine3::IDENTITY;
        // Determine axes
        Vector3 zAxis, yAxis, xAxis;
        zAxis = params.plane.normal;
        zAxis.normalise();
        yAxis = params.upVector;
        yAxis.normalise();
        xAxis = yAxis.crossProduct(zAxis);
        if (xAxis.squaredLength() == 0)
        {
            //upVector must be wrong
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "The upVector you supplied is parallel to the plane normal, so is not valid.");
        }
        xAxis.normalise();

        rot3.FromAxes(xAxis, yAxis, zAxis);
        rot = rot3;

        // Set up standard transform from origin
        xlate.setTrans(params.plane.normal * -params.plane.d);

        // concatenate
        xform = xlate * rot;

        // Generate vertex data
        HardwareBufferLockGuard vbufLock(vbuf, HardwareBuffer::HBL_DISCARD);
        float* pFloat = static_cast<float*>(vbufLock.pData);
        Real xSpace = params.width / params.xsegments;
        Real ySpace = params.height / params.ysegments;
        Real halfWidth = params.width / 2;
        Real halfHeight = params.height / 2;
        Real xTex = (1.0f * params.xTile) / params.xsegments;
        Real yTex = (1.0f * params.yTile) / params.ysegments;
        Vector3 vec;

        AxisAlignedBox aabb;

        Real diff_x, diff_y, dist;

        for (int y = 0; y < params.ysegments + 1; ++y)
        {
            for (int x = 0; x < params.xsegments + 1; ++x)
            {
                // Work out centered on origin
                vec.x = (x * xSpace) - halfWidth;
                vec.y = (y * ySpace) - halfHeight;

                // Here's where curved plane is different from standard plane.  Amazing, I know.
                diff_x = (x - ((params.xsegments) / 2)) / static_cast<Real>((params.xsegments));
                diff_y = (y - ((params.ysegments) / 2)) / static_cast<Real>((params.ysegments));
                dist = std::sqrt(diff_x*diff_x + diff_y * diff_y );
                vec.z = (-std::sin((1-dist) * (Math::PI/2)) * params.curvature) + params.curvature;

                // Transform by orientation and distance
                Vector3 pos = xform * vec;
                // Assign to geometry
                *pFloat++ = pos.x;
                *pFloat++ = pos.y;
                *pFloat++ = pos.z;

                // Record bounds
                aabb.merge(vec);

                if (params.normals)
                {
                    // This part is kinda 'wrong' for curved planes... but curved planes are
                    //   very valuable outside sky planes, which don't typically need normals
                    //   so I'm not going to mess with it for now.

                    // Default normal is along unit Z
                    //vec = Vector3::UNIT_Z;
                    // Rotate
                    vec = rot * vec;
                    vec.normalise();

                    *pFloat++ = vec.x;
                    *pFloat++ = vec.y;
                    *pFloat++ = vec.z;
                }

                for (unsigned short i = 0; i < params.numTexCoordSets; ++i)
                {
                    *pFloat++ = x * xTex;
                    *pFloat++ = 1 - (y * yTex);
                }

            } // x
        } // y
        vbufLock.unlock();

        // Generate face list
        tesselate2DMesh(pSub, params.xsegments + 1, params.ysegments + 1,
            false, params.indexBufferUsage, params.indexShadowBuffer);

        pMesh->_setBounds(aabb, true);
    }
    //-----------------------------------------------------------------------
    void PrefabFactory::loadManualCurvedIllusionPlane(Mesh* pMesh, MeshBuildParams& params)
    {
        if (params.ySegmentsToKeep == -1) params.ySegmentsToKeep = params.ysegments;

        if ((params.xsegments + 1) * (params.ySegmentsToKeep + 1) > 65536)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Plane tessellation is too high, must generate max 65536 vertices");
        SubMesh *pSub = pMesh->createSubMesh();


        // Set up vertex data
        // Use a single shared buffer
        pMesh->sharedVertexData = OGRE_NEW VertexData();
        VertexData* vertexData = pMesh->sharedVertexData;
        // Set up Vertex Declaration
        VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
        size_t currOffset = 0;
        // We always need positions
        currOffset += vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_POSITION).getSize();
        // Optional normals
        if(params.normals)
        {
            currOffset += vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_NORMAL).getSize();
        }

        for (unsigned short i = 0; i < params.numTexCoordSets; ++i)
        {
            // Assumes 2D texture coords
            currOffset += vertexDecl->addElement(0, currOffset, VET_FLOAT2, VES_TEXTURE_COORDINATES, i).getSize();
        }

        vertexData->vertexCount = (params.xsegments + 1) * (params.ySegmentsToKeep + 1);

        // Allocate vertex buffer
        HardwareVertexBufferSharedPtr vbuf =
            pMesh->getHardwareBufferManager()->createVertexBuffer(
                vertexDecl->getVertexSize(0), vertexData->vertexCount,
                params.vertexBufferUsage, params.vertexShadowBuffer);

        // Set up the binding (one source only)
        VertexBufferBinding* binding = vertexData->vertexBufferBinding;
        binding->setBinding(0, vbuf);

        // Work out the transform required
        // Default orientation of plane is normal along +z, distance 0
        Affine3 xlate, xform, rot;
        Matrix3 rot3;
        xlate = rot = Affine3::IDENTITY;
        // Determine axes
        Vector3 zAxis, yAxis, xAxis;
        zAxis = params.plane.normal;
        zAxis.normalise();
        yAxis = params.upVector;
        yAxis.normalise();
        xAxis = yAxis.crossProduct(zAxis);
        if (xAxis.squaredLength() == 0)
        {
            //upVector must be wrong
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "The upVector you supplied is parallel to the plane normal, so is not valid.");
        }
        xAxis.normalise();

        rot3.FromAxes(xAxis, yAxis, zAxis);
        rot = rot3;

        // Set up standard transform from origin
        xlate.setTrans(params.plane.normal * -params.plane.d);

        // concatenate
        xform = xlate * rot;

        // Generate vertex data
        // Imagine a large sphere with the camera located near the top
        // The lower the curvature, the larger the sphere
        // Use the angle from viewer to the points on the plane
        // Credit to Aftershock for the general approach
        Real camPos;      // Camera position relative to sphere center

        // Derive sphere radius
        Vector3 vertPos;  // position relative to camera
        Real sphDist;      // Distance from camera to sphere along box vertex vector
        // Vector3 camToSph; // camera position to sphere
        Real sphereRadius;// Sphere radius
        // Actual values irrelevant, it's the relation between sphere radius and camera position that's important
        const Real SPHERE_RAD = 100.0;
        const Real CAM_DIST = 5.0;

        sphereRadius = SPHERE_RAD - params.curvature;
        camPos = sphereRadius - CAM_DIST;

        // Lock the whole buffer
        HardwareBufferLockGuard vbufLock(vbuf, HardwareBuffer::HBL_DISCARD);
        float* pFloat = static_cast<float*>(vbufLock.pData);
        Real xSpace = params.width / params.xsegments;
        Real ySpace = params.height / params.ysegments;
        Real halfWidth = params.width / 2;
        Real halfHeight = params.height / 2;
        Vector3 vec, norm;
        AxisAlignedBox aabb;

        for (int y = params.ysegments - params.ySegmentsToKeep; y < params.ysegments + 1; ++y)
        {
            for (int x = 0; x < params.xsegments + 1; ++x)
            {
                // Work out centered on origin
                vec.x = (x * xSpace) - halfWidth;
                vec.y = (y * ySpace) - halfHeight;
                vec.z = 0.0f;
                // Transform by orientation and distance
                vec = xform * vec;
                // Assign to geometry
                *pFloat++ = vec.x;
                *pFloat++ = vec.y;
                *pFloat++ = vec.z;

                // Build bounds as we go
                aabb.merge(vec);

                if (params.normals)
                {
                    // Default normal is along unit Z
                    norm = Vector3::UNIT_Z;
                    // Rotate
                    norm = params.orientation * norm;

                    *pFloat++ = norm.x;
                    *pFloat++ = norm.y;
                    *pFloat++ = norm.z;
                }

                // Generate texture coords
                // Normalise position
                // modify by orientation to return +y up
                vec = params.orientation.Inverse() * vec;
                vec.normalise();
                // Find distance to sphere
                sphDist = Math::Sqrt(camPos*camPos * (vec.y*vec.y-1.0f) + sphereRadius*sphereRadius) - camPos*vec.y;

                vec.x *= sphDist;
                vec.z *= sphDist;

                // Use x and y on sphere as texture coordinates, tiled
                Real s = vec.x * (0.01f * params.xTile);
                Real t = 1.0f - (vec.z * (0.01f * params.yTile));
                for (unsigned short i = 0; i < params.numTexCoordSets; ++i)
                {
                    *pFloat++ = s;
                    *pFloat++ = t;
                }


            } // x
        } // y

        // Unlock
        vbufLock.unlock();
        // Generate face list
        pSub->useSharedVertices = true;
        tesselate2DMesh(pSub, params.xsegments + 1, params.ySegmentsToKeep + 1, false,
            params.indexBufferUsage, params.indexShadowBuffer);

        pMesh->_setBounds(aabb, true);
    }
}
