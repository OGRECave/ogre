/*
 * -----------------------------------------------------------------------------
 * This source file is part of OGRE
 * (Object-oriented Graphics Rendering Engine)
 * For the latest info, see http://www.ogre3d.org/
 *
 * Copyright (c) 2000-2014 Torus Knot Software Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * -----------------------------------------------------------------------------
 */

#include "OgreMeshLodPrecompiledHeaders.h"

namespace Ogre
{
    void LodIndexBuffer::fillBuffer( Ogre::IndexData* data )
    {
        indexCount = data->indexCount;
        if (indexCount > 0) {
            const HardwareIndexBufferSharedPtr& hwIndexBuffer = data->indexBuffer;
            indexSize = hwIndexBuffer->getIndexSize();
            unsigned char* pBuffer = (unsigned char*) hwIndexBuffer->lock(HardwareBuffer::HBL_READ_ONLY);
            size_t offset = data->indexStart * indexSize;
            indexBuffer = Ogre::SharedPtr<unsigned char>(new unsigned char[indexCount * indexSize]);
            indexStart = 0;
            indexBufferSize = 0;
            memcpy(indexBuffer.get(), pBuffer + offset, indexCount * indexSize);
            hwIndexBuffer->unlock();
        }
    }

    void LodVertexBuffer::fillBuffer( Ogre::VertexData* data )
    {
        vertexCount = data->vertexCount;
        if (vertexCount > 0) {
            // Locate position element and the buffer to go with it.
            const VertexElement* elemPos = data->vertexDeclaration->findElementBySemantic(VES_POSITION);

            // Only float supported.
            OgreAssert(elemPos->getSize() == 12, "");

            HardwareVertexBufferSharedPtr vbuf = data->vertexBufferBinding->getBuffer(elemPos->getSource());
            vertexBuffer = Ogre::SharedPtr<Vector3>(new Vector3[vertexCount]);

            // Lock the buffer for reading.
            unsigned char* vStart = static_cast<unsigned char*>(vbuf->lock(HardwareBuffer::HBL_READ_ONLY));
            unsigned char* vertex = vStart;
            size_t vSize = vbuf->getVertexSize();

            const VertexElement* elemNormal = 0;
            HardwareVertexBufferSharedPtr vNormalBuf;
            unsigned char* vNormal = NULL;
            Vector3* pNormalOut = NULL;
            size_t vNormalSize = 0;
            bool useVertexNormals = true;
            elemNormal = data->vertexDeclaration->findElementBySemantic(VES_NORMAL);
            useVertexNormals = useVertexNormals && (elemNormal != 0);
            if(useVertexNormals){
                vertexNormalBuffer = Ogre::SharedPtr<Vector3>(new Vector3[vertexCount]);
                pNormalOut = vertexNormalBuffer.get();
                if(elemNormal->getSource() == elemPos->getSource()){
                    vNormalBuf = vbuf;
                    vNormal = vStart;
                }  else {
                    vNormalBuf = data->vertexBufferBinding->getBuffer(elemNormal->getSource());
                    vNormal = static_cast<unsigned char*>(vNormalBuf->lock(HardwareBuffer::HBL_READ_ONLY));
                }
                vNormalSize = vNormalBuf->getVertexSize();
            }

            // Loop through all vertices and insert them to the Unordered Map.
            Vector3* pOut = vertexBuffer.get();
            Vector3* pEnd = pOut + vertexCount;
            for (; pOut < pEnd; pOut++) {
                float* pFloat;
                elemPos->baseVertexPointerToElement(vertex, &pFloat);
                pOut->x = *pFloat;
                pOut->y = *(++pFloat);
                pOut->z = *(++pFloat);
                vertex += vSize;
                if(useVertexNormals){
                    elemNormal->baseVertexPointerToElement(vNormal, &pFloat);
                    pNormalOut->x = *pFloat;
                    pNormalOut->y = *(++pFloat);
                    pNormalOut->z = *(++pFloat);
                    pNormalOut++;
                    vNormal += vNormalSize;
                }
            }
            vbuf->unlock();
            if(elemNormal && elemNormal->getSource() != elemPos->getSource()){
                vNormalBuf->unlock();
            }
        }
    }

    void LodInputBuffer::fillBuffer( Ogre::MeshPtr mesh )
    {
        meshName = mesh->getName();
        boundingSphereRadius = mesh->getBoundingSphereRadius();
        bool sharedVerticesAdded = false;
        size_t submeshCount = mesh->getNumSubMeshes();
        submesh.resize(submeshCount);
        for (size_t i = 0; i < submeshCount; i++) {
            const SubMesh* ogresubmesh = mesh->getSubMesh(i);
            LodInputBuffer::Submesh& outsubmesh = submesh[i];
            outsubmesh.indexBuffer.fillBuffer(ogresubmesh->indexData);
            outsubmesh.useSharedVertexBuffer = ogresubmesh->useSharedVertices;
            if (!outsubmesh.useSharedVertexBuffer) {
                outsubmesh.vertexBuffer.fillBuffer(ogresubmesh->vertexData);
            } else if (!sharedVerticesAdded) {
                sharedVerticesAdded = true;
                sharedVertexBuffer.fillBuffer(mesh->sharedVertexData);
            }
        }
    }

 }
