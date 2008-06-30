/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgrePatchMesh.h"
#include "OgreSubMesh.h"
#include "OgreHardwareBufferManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    PatchMesh::PatchMesh(ResourceManager* creator, const String& name, ResourceHandle handle,
        const String& group)
        : Mesh(creator, name, handle, group, false, 0)
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
    void PatchMesh::setSubdivision(Real factor)
    {
        mSurface.setSubdivisionFactor(factor);
        SubMesh* sm = this->getSubMesh(0);
        sm->indexData->indexCount = mSurface.getCurrentIndexCount();
        
    }
    //-----------------------------------------------------------------------
    void PatchMesh::loadImpl(void)
    {
        SubMesh* sm = this->createSubMesh();
        sm->vertexData = OGRE_NEW VertexData();
        sm->useSharedVertices = false;

        // Set up vertex buffer
        sm->vertexData->vertexStart = 0;
        sm->vertexData->vertexCount = mSurface.getRequiredVertexCount();
        sm->vertexData->vertexDeclaration = mDeclaration;
        HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().
            createVertexBuffer(
                mDeclaration->getVertexSize(0), 
                sm->vertexData->vertexCount, 
                mVertexBufferUsage, 
                mVertexBufferShadowBuffer);
        sm->vertexData->vertexBufferBinding->setBinding(0, vbuf);

        // Set up index buffer
        sm->indexData->indexStart = 0;
        sm->indexData->indexCount = mSurface.getRequiredIndexCount();
        sm->indexData->indexBuffer = HardwareBufferManager::getSingleton().
            createIndexBuffer(
                HardwareIndexBuffer::IT_16BIT, // only 16-bit indexes supported, patches shouldn't be bigger than that
                sm->indexData->indexCount,
                mIndexBufferUsage, 
                mIndexBufferShadowBuffer);
        
        // Build patch
        mSurface.build(vbuf, 0, sm->indexData->indexBuffer, 0);

        // Set bounds
        this->_setBounds(mSurface.getBounds(), true);
        this->_setBoundingSphereRadius(mSurface.getBoundingSphereRadius());

    }

}

