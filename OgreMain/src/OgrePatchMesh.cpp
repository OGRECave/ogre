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
#include "OgrePatchMesh.h"
#include "OgreSubMesh.h"
#include "OgreHardwareBufferManager.h"

namespace Ogre {
namespace v1 {

    //-----------------------------------------------------------------------
    PatchMesh::PatchMesh(ResourceManager* creator, const String& name, ResourceHandle handle,
        const String& group)
        : Mesh(creator, name, handle, group, false, 0), mDeclaration(0)
    {
    }
    //-----------------------------------------------------------------------
    void PatchMesh::define(void* controlPointBuffer, 
            VertexDeclaration *declaration, size_t width, size_t height,
            size_t uMaxSubdivisionLevel, size_t vMaxSubdivisionLevel,
            PatchSurface::VisibleSide visibleSide, HardwareBuffer::Usage vbUsage, 
            HardwareBuffer::Usage ibUsage,
            bool vbUseShadow, bool ibUseShadow) 
    {
        mVertexBufferUsage = vbUsage;
        mVertexBufferShadowBuffer = vbUseShadow;
        mIndexBufferUsage = ibUsage;
        mIndexBufferShadowBuffer = ibUseShadow;

        // Init patch builder
        // define the surface
        // NB clone the declaration to make it independent
        mDeclaration = declaration->clone();
        mSurface.defineSurface(controlPointBuffer, mDeclaration, width, height, 
            PatchSurface::PST_BEZIER, uMaxSubdivisionLevel, vMaxSubdivisionLevel, 
            visibleSide);

    }
    //-----------------------------------------------------------------------
    void PatchMesh::update(void* controlPointBuffer, size_t width, size_t height,
                           size_t uMaxSubdivisionLevel, size_t vMaxSubdivisionLevel,
                           PatchSurface::VisibleSide visibleSide)
    {
        mSurface.defineSurface(controlPointBuffer, mDeclaration, width, height, PatchSurface::PST_BEZIER, uMaxSubdivisionLevel, vMaxSubdivisionLevel, visibleSide);
        SubMesh* sm = this->getSubMesh(0);
        VertexData* vertex_data = sm->useSharedVertices ? this->sharedVertexData[0] : sm->vertexData[0];
        const VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
        HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

        // Build patch with new control points
        mSurface.build(vbuf, 0, sm->indexData[1]->indexBuffer, 0);
    }
    //-----------------------------------------------------------------------
    void PatchMesh::setSubdivision(Real factor)
    {
        mSurface.setSubdivisionFactor(factor);
        SubMesh* sm = this->getSubMesh(0);
        sm->indexData[0]->indexCount = mSurface.getCurrentIndexCount();
    }
    //-----------------------------------------------------------------------
    void PatchMesh::loadImpl(void)
    {
        SubMesh* sm = this->createSubMesh();
        sm->vertexData[0] = OGRE_NEW VertexData();
        sm->useSharedVertices = false;

        // Set up vertex buffer
        sm->vertexData[0]->vertexStart = 0;
        sm->vertexData[0]->vertexCount = mSurface.getRequiredVertexCount();
        sm->vertexData[0]->vertexDeclaration = mDeclaration;
        HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().
            createVertexBuffer(
                mDeclaration->getVertexSize(0), 
                sm->vertexData[0]->vertexCount,
                mVertexBufferUsage, 
                mVertexBufferShadowBuffer);
        sm->vertexData[0]->vertexBufferBinding->setBinding(0, vbuf);

        // Set up index buffer
        sm->indexData[0]->indexStart = 0;
        sm->indexData[0]->indexCount = mSurface.getRequiredIndexCount();
        sm->indexData[0]->indexBuffer = HardwareBufferManager::getSingleton().
            createIndexBuffer(
                HardwareIndexBuffer::IT_16BIT, // only 16-bit indexes supported, patches shouldn't be bigger than that
                sm->indexData[0]->indexCount,
                mIndexBufferUsage, 
                mIndexBufferShadowBuffer);
        
        // Build patch
        mSurface.build(vbuf, 0, sm->indexData[0]->indexBuffer, 0);

        this->prepareForShadowMapping( true );

        // Set bounds
        this->_setBounds(mSurface.getBounds(), true);
        this->_setBoundingSphereRadius(mSurface.getBoundingSphereRadius());

    }
}
}

