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
#include "OgreWireBoundingBox.h"

#include "OgreSimpleRenderable.h"
#include "OgreHardwareBufferManager.h"
#include "OgreCamera.h"

namespace Ogre {
    #define POSITION_BINDING 0

	WireBoundingBox::WireBoundingBox() 
    {
        mRenderOp.vertexData = OGRE_NEW VertexData();

        mRenderOp.indexData = 0;
		mRenderOp.vertexData->vertexCount = 24; 
		mRenderOp.vertexData->vertexStart = 0; 
		mRenderOp.operationType = RenderOperation::OT_LINE_LIST; 
		mRenderOp.useIndexes = false; 

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
        this->setMaterial("BaseWhiteNoLighting");


        
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

        float* pPos = static_cast<float*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));

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
        vbuf->unlock();
	}

    //-----------------------------------------------------------------------
	Real WireBoundingBox::getSquaredViewDepth(const Camera* cam) const
	{
		Vector3 min, max, mid, dist;
		min = mBox.getMinimum();
		max = mBox.getMaximum();
		mid = ((max - min) * 0.5) + min;
		dist = cam->getDerivedPosition() - mid;


		return dist.squaredLength();
	}



}

