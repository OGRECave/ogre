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

namespace Ogre {
    #define POSITION_BINDING 0

    WireBoundingBox::WireBoundingBox()
    : SimpleRenderable()
    {
        _initWireBoundingBox();
    }

    WireBoundingBox::WireBoundingBox(const String& name) 
    : SimpleRenderable(name)
    {
        _initWireBoundingBox();
    }

    void WireBoundingBox::_initWireBoundingBox()
    {
        mRenderOp.vertexData = OGRE_NEW VertexData();

        mRenderOp.indexData = 0;
        mRenderOp.vertexData->vertexCount = 24; 
        mRenderOp.vertexData->vertexStart = 0; 
        mRenderOp.operationType = RenderOperation::OT_LINE_LIST; 
        mRenderOp.useIndexes = false; 
        mRenderOp.useGlobalInstancingVertexBufferIsAvailable = false;

        VertexDeclaration* decl = mRenderOp.vertexData->vertexDeclaration;
        VertexBufferBinding* bind = mRenderOp.vertexData->vertexBufferBinding;

        decl->addElement(POSITION_BINDING, 0, VET_FLOAT3, VES_POSITION);


        HardwareVertexBufferSharedPtr vbuf = 
            HardwareBufferManager::getSingleton().createVertexBuffer(
                decl->getVertexSize(POSITION_BINDING),
                mRenderOp.vertexData->vertexCount,
                HardwareBuffer::HBU_STATIC_WRITE_ONLY);

        // Bind buffer
        bind->setBinding(POSITION_BINDING, vbuf);

        // set basic white material
        mMaterial = MaterialManager::getSingleton().getDefaultMaterial(false);
        mMaterial->load();
    }
    
    WireBoundingBox::~WireBoundingBox() 
    {
        OGRE_DELETE mRenderOp.vertexData;
    }

    void WireBoundingBox::setupBoundingBox(const AxisAlignedBox& aabb) 
    {
        // init the vertices to the aabb
        setupBoundingBoxVertices(aabb);

        // setup the bounding box of this SimpleRenderable
        setBoundingBox(aabb);

    }

    // Override this method to prevent parent transforms (rotation,translation,scale)
    void WireBoundingBox::getWorldTransforms( Matrix4* xform ) const
    {
        // return identity matrix to prevent parent transforms
        *xform = Matrix4::IDENTITY;
    }
    //-----------------------------------------------------------------------
    void WireBoundingBox::setupBoundingBoxVertices(const AxisAlignedBox& aab) {

        Vector3 vmax = aab.getMaximum();
        Vector3 vmin = aab.getMinimum();

        Real sqLen = std::max(vmax.squaredLength(), vmin.squaredLength());
        mRadius = Math::Sqrt(sqLen);
        

        
        
        Real maxx = vmax.x;
        Real maxy = vmax.y;
        Real maxz = vmax.z;
        
        Real minx = vmin.x;
        Real miny = vmin.y;
        Real minz = vmin.z;
        
        // fill in the Vertex buffer: 12 lines with 2 endpoints each make up a box
        HardwareVertexBufferSharedPtr vbuf =
            mRenderOp.vertexData->vertexBufferBinding->getBuffer(POSITION_BINDING);     

        HardwareBufferLockGuard vbufLock(vbuf, HardwareBuffer::HBL_DISCARD);
        float* pPos = static_cast<float*>(vbufLock.pData);

        // line 0
        *pPos++ = minx;
        *pPos++ = miny;
        *pPos++ = minz;
        *pPos++ = maxx;
        *pPos++ = miny;
        *pPos++ = minz;
        // line 1
        *pPos++ = minx;
        *pPos++ = miny;
        *pPos++ = minz;
        *pPos++ = minx;
        *pPos++ = miny;
        *pPos++ = maxz;
        // line 2
        *pPos++ = minx;
        *pPos++ = miny;
        *pPos++ = minz;
        *pPos++ = minx;
        *pPos++ = maxy;
        *pPos++ = minz;
        // line 3
        *pPos++ = minx;
        *pPos++ = maxy;
        *pPos++ = minz;
        *pPos++ = minx;
        *pPos++ = maxy;
        *pPos++ = maxz;
        // line 4
        *pPos++ = minx;
        *pPos++ = maxy;
        *pPos++ = minz;
        *pPos++ = maxx;
        *pPos++ = maxy;
        *pPos++ = minz;
        // line 5
        *pPos++ = maxx;
        *pPos++ = miny;
        *pPos++ = minz;
        *pPos++ = maxx;
        *pPos++ = miny;
        *pPos++ = maxz;
        // line 6
        *pPos++ = maxx;
        *pPos++ = miny;
        *pPos++ = minz;
        *pPos++ = maxx;
        *pPos++ = maxy;
        *pPos++ = minz;
        // line 7
        *pPos++ = minx;
        *pPos++ = maxy;
        *pPos++ = maxz;
        *pPos++ = maxx;
        *pPos++ = maxy;
        *pPos++ = maxz;
        // line 8
        *pPos++ = minx;
        *pPos++ = maxy;
        *pPos++ = maxz;
        *pPos++ = minx;
        *pPos++ = miny;
        *pPos++ = maxz;
        // line 9
        *pPos++ = maxx;
        *pPos++ = maxy;
        *pPos++ = minz;
        *pPos++ = maxx;
        *pPos++ = maxy;
        *pPos++ = maxz;
        // line 10
        *pPos++ = maxx;
        *pPos++ = miny;
        *pPos++ = maxz;
        *pPos++ = maxx;
        *pPos++ = maxy;
        *pPos++ = maxz;
        // line 11
        *pPos++ = minx;
        *pPos++ = miny;
        *pPos++ = maxz;
        *pPos++ = maxx;
        *pPos++ = miny;
        *pPos++ = maxz;
    }

    //-----------------------------------------------------------------------
    Real WireBoundingBox::getSquaredViewDepth(const Camera* cam) const
    {
        return (cam->getDerivedPosition() - mBox.getCenter()).squaredLength();
    }



}

