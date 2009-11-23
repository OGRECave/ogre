/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreRectangle2D.h"

#include "OgreSimpleRenderable.h"
#include "OgreHardwareBufferManager.h"
#include "OgreCamera.h"

namespace Ogre {
#define POSITION_BINDING 0
#define NORMAL_BINDING 1
#define TEXCOORD_BINDING 2

    Rectangle2D::Rectangle2D(bool includeTextureCoords) 
    {
        // use identity projection and view matrices
        mUseIdentityProjection = true;
        mUseIdentityView = true;

        mRenderOp.vertexData = OGRE_NEW VertexData();

        mRenderOp.indexData = 0;
        mRenderOp.vertexData->vertexCount = 4; 
        mRenderOp.vertexData->vertexStart = 0; 
        mRenderOp.operationType = RenderOperation::OT_TRIANGLE_STRIP; 
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

		decl->addElement(NORMAL_BINDING, 0, VET_FLOAT3, VES_NORMAL);

		vbuf = 
			HardwareBufferManager::getSingleton().createVertexBuffer(
            decl->getVertexSize(NORMAL_BINDING),
            mRenderOp.vertexData->vertexCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY);

		bind->setBinding(NORMAL_BINDING, vbuf);

		float *pNorm = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
		*pNorm++ = 0.0f;
		*pNorm++ = 0.0f;
		*pNorm++ = 1.0f;

		*pNorm++ = 0.0f;
		*pNorm++ = 0.0f;
		*pNorm++ = 1.0f;

		*pNorm++ = 0.0f;
		*pNorm++ = 0.0f;
		*pNorm++ = 1.0f;

		*pNorm++ = 0.0f;
		*pNorm++ = 0.0f;
		*pNorm++ = 1.0f;

		vbuf->unlock();

        if (includeTextureCoords)
        {
            decl->addElement(TEXCOORD_BINDING, 0, VET_FLOAT2, VES_TEXTURE_COORDINATES);


            HardwareVertexBufferSharedPtr tvbuf = 
                HardwareBufferManager::getSingleton().createVertexBuffer(
                decl->getVertexSize(TEXCOORD_BINDING),
                mRenderOp.vertexData->vertexCount,
                HardwareBuffer::HBU_STATIC_WRITE_ONLY);

            // Bind buffer
            bind->setBinding(TEXCOORD_BINDING, tvbuf);

            // Set up basic tex coordinates
            float* pTex = static_cast<float*>(
                tvbuf->lock(HardwareBuffer::HBL_DISCARD));
            *pTex++ = 0.0f;
            *pTex++ = 0.0f;
            *pTex++ = 0.0f;
            *pTex++ = 1.0f;
            *pTex++ = 1.0f;
            *pTex++ = 0.0f;
            *pTex++ = 1.0f;
            *pTex++ = 1.0f;
            tvbuf->unlock();
        }

        // set basic white material
        this->setMaterial("BaseWhiteNoLighting");
    }

    Rectangle2D::~Rectangle2D() 
    {
        OGRE_DELETE mRenderOp.vertexData;
    }

    void Rectangle2D::setCorners(Real left, Real top, Real right, Real bottom, bool updateAABB) 
    {
        HardwareVertexBufferSharedPtr vbuf = 
            mRenderOp.vertexData->vertexBufferBinding->getBuffer(POSITION_BINDING);
        float* pFloat = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));

        *pFloat++ = left;
        *pFloat++ = top;
        *pFloat++ = -1;

        *pFloat++ = left;
        *pFloat++ = bottom;
        *pFloat++ = -1;

        *pFloat++ = right;
        *pFloat++ = top;
        *pFloat++ = -1;

        *pFloat++ = right;
        *pFloat++ = bottom;
        *pFloat++ = -1;

        vbuf->unlock();

		if(updateAABB)
		{
			mBox.setExtents(
				std::min(left, right), std::min(top, bottom), 0,
				std::max(left, right), std::max(top, bottom), 0);
		}
    }

	void Rectangle2D::setNormals(const Ogre::Vector3 &topLeft, const Ogre::Vector3 &bottomLeft, const Ogre::Vector3 &topRight, const Ogre::Vector3 &bottomRight)
	{
		HardwareVertexBufferSharedPtr vbuf = 
            mRenderOp.vertexData->vertexBufferBinding->getBuffer(NORMAL_BINDING);
        float* pFloat = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));

        *pFloat++ = topLeft.x;
        *pFloat++ = topLeft.y;
        *pFloat++ = topLeft.z;

        *pFloat++ = bottomLeft.x;
        *pFloat++ = bottomLeft.y;
        *pFloat++ = bottomLeft.z;

        *pFloat++ = topRight.x;
        *pFloat++ = topRight.y;
        *pFloat++ = topRight.z;

        *pFloat++ = bottomRight.x;
        *pFloat++ = bottomRight.y;
        *pFloat++ = bottomRight.z;

        vbuf->unlock();
	}

    // Override this method to prevent parent transforms (rotation,translation,scale)
    void Rectangle2D::getWorldTransforms( Matrix4* xform ) const
    {
        // return identity matrix to prevent parent transforms
        *xform = Matrix4::IDENTITY;
    }


}

