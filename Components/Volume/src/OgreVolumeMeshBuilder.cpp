/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreVolumeMeshBuilder.h"

#include <limits.h>

#include "OgreHardwareBufferManager.h"
#include "OgreManualObject.h"
#include "OgreMeshManager.h"
#include "OgreSceneManager.h"
#include <sstream>

namespace Ogre {
namespace Volume {

    bool operator==(Vertex const& a, Vertex const& b)
    {
        return a.x == b.x &&
            a.y == b.y &&
            a.z == b.z &&
            a.nX == b.nX &&
            a.nY == b.nY &&
            a.nZ == b.nZ;
    }

    //-----------------------------------------------------------------------

    bool operator<(const Vertex& a, const Vertex& b)
    {
         return memcmp(&a, &b, sizeof(Vertex)) < 0;
    }

    //-----------------------------------------------------------------------

    const unsigned short MeshBuilder::MAIN_BINDING = 0;

    //-----------------------------------------------------------------------

    MeshBuilder::MeshBuilder(void) : mBoxInit(false)
    {
    }

    //-----------------------------------------------------------------------

    size_t MeshBuilder::generateBuffers(RenderOperation &operation)
    {
        // Early out if nothing to do.
        if (mIndices.empty())
        {
            return 0;
        }

        // Prepare vertex buffer
        operation.operationType = RenderOperation::OT_TRIANGLE_LIST;

        operation.vertexData = OGRE_NEW VertexData();
        operation.vertexData->vertexCount = mVertices.size();
        operation.vertexData->vertexStart = 0;

        VertexDeclaration *decl = operation.vertexData->vertexDeclaration;
        VertexBufferBinding *bind = operation.vertexData->vertexBufferBinding;

        size_t offset = 0;

        // Add vertex-positions to the buffer
        offset += decl->addElement(0, offset, VET_FLOAT3, VES_POSITION).getSize();

        // Add vertex-normals to the buffer
        decl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);

        HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            decl->getVertexSize(MAIN_BINDING),
            operation.vertexData->vertexCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY);

        bind->setBinding(0, vbuf);

        float* vertices = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
        for (const auto& v : mVertices)
        {
            *vertices++ = (float)v.x;
            *vertices++ = (float)v.y;
            *vertices++ = (float)v.z;
            *vertices++ = (float)v.nX;
            *vertices++ = (float)v.nY;
            *vertices++ = (float)v.nZ;
        }

        vbuf->unlock();

        // Get Indexarray
        operation.indexData = OGRE_NEW IndexData();
        operation.indexData->indexCount = mIndices.size();
        operation.indexData->indexStart = 0;

        if (operation.indexData->indexCount > USHRT_MAX)
        {
            operation.indexData->indexBuffer =
                HardwareBufferManager::getSingleton().createIndexBuffer(
                HardwareIndexBuffer::IT_32BIT,
                operation.indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

            unsigned int* indices = static_cast<unsigned int*>(
                operation.indexData->indexBuffer->lock(0,
                    operation.indexData->indexBuffer->getSizeInBytes(),
                    HardwareBuffer::HBL_DISCARD));

        for (const auto& i : mIndices)
            {
                *indices++ = static_cast<uint32>(i);
            }
        }
        else
        {
            operation.indexData->indexBuffer =
                HardwareBufferManager::getSingleton().createIndexBuffer(
                HardwareIndexBuffer::IT_16BIT,
                operation.indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

            unsigned short* indices = static_cast<unsigned short*>(
                operation.indexData->indexBuffer->lock(0,
                    operation.indexData->indexBuffer->getSizeInBytes(),
                    HardwareBuffer::HBL_DISCARD));

            for (const auto& i : mIndices)
            {
                *indices++ = (unsigned short)i;
            }
        }

        operation.indexData->indexBuffer->unlock();
        return mIndices.size() / 3;
    }

    //-----------------------------------------------------------------------

    AxisAlignedBox MeshBuilder::getBoundingBox(void)
    {
        return mBox;
    }

    //-----------------------------------------------------------------------

    Entity* MeshBuilder::generateWithManualObject(SceneManager *sceneManager, const String &name, const String &material)
    {
            ManualObject* manual = sceneManager->createManualObject();
            manual->begin(material, RenderOperation::OT_TRIANGLE_LIST);

            for (const auto& v : mVertices)
            {
                manual->position(Vector3(v.x, v.y, v.z));
                manual->normal(Vector3(v.nX, v.nY, v.nZ));
            }
            for (const auto& i : mIndices)
            {
                manual->index(static_cast<uint32>(i));
            }

            manual->end();
            StringStream meshName;
            meshName << name << "ManualObject";
            MeshManager::getSingleton().remove(meshName.str(), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            manual->convertToMesh(meshName.str());
            return sceneManager->createEntity(name, meshName.str());
    }

    //-----------------------------------------------------------------------

    void MeshBuilder::executeCallback(MeshBuilderCallback *callback, const SimpleRenderable *simpleRenderable, size_t level, int inProcess) const
    {
        callback->ready(simpleRenderable, mVertices, mIndices, level, inProcess);
    }

}
}
