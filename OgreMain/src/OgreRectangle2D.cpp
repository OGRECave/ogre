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
#define NORMAL_BINDING 1
#define TEXCOORD_BINDING 2

    Rectangle2D::Rectangle2D(bool includeTextureCoords, Ogre::HardwareBuffer::Usage vBufUsage)
    : SimpleRenderable()
    {
        _initRectangle2D(includeTextureCoords, vBufUsage);
    }

    Rectangle2D::Rectangle2D(const String& name, bool includeTextureCoords, Ogre::HardwareBuffer::Usage vBufUsage)
    : SimpleRenderable(name)
    {
        _initRectangle2D(includeTextureCoords, vBufUsage);
    }

    void Rectangle2D::_initRectangle2D(bool includeTextureCoords, Ogre::HardwareBuffer::Usage vBufUsage) 
    {
        // use identity projection and view matrices
        mUseIdentityProjection = true;
        mUseIdentityView = true;

        mBox.setInfinite(); // screenspace -> never culled

        mRenderOp.vertexData = OGRE_NEW VertexData();

        mRenderOp.indexData = 0;
        mRenderOp.vertexData->vertexCount = 4; 
        mRenderOp.vertexData->vertexStart = 0; 
        mRenderOp.operationType = RenderOperation::OT_TRIANGLE_STRIP; 
        mRenderOp.useIndexes = false; 
        mRenderOp.useGlobalInstancingVertexBufferIsAvailable = false;

        VertexDeclaration* decl = mRenderOp.vertexData->vertexDeclaration;
        VertexBufferBinding* bind = mRenderOp.vertexData->vertexBufferBinding;

        decl->addElement(POSITION_BINDING, 0, VET_FLOAT3, VES_POSITION);

        auto& hbm = HardwareBufferManager::getSingleton();
        auto vbuf =
            hbm.createVertexBuffer(decl->getVertexSize(POSITION_BINDING), mRenderOp.vertexData->vertexCount, vBufUsage);

        // Bind buffer
        bind->setBinding(POSITION_BINDING, vbuf);

        setCorners(-1, 1, 1, -1, false);

        decl->addElement(NORMAL_BINDING, 0, VET_FLOAT3, VES_NORMAL);

        vbuf =
            hbm.createVertexBuffer(decl->getVertexSize(NORMAL_BINDING), mRenderOp.vertexData->vertexCount, vBufUsage);

        bind->setBinding(NORMAL_BINDING, vbuf);

        setNormals({0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1});

        if (includeTextureCoords)
        {
            decl->addElement(TEXCOORD_BINDING, 0, VET_FLOAT2, VES_TEXTURE_COORDINATES);

            auto tvbuf = hbm.createVertexBuffer(decl->getVertexSize(TEXCOORD_BINDING),
                                                mRenderOp.vertexData->vertexCount, vBufUsage);

            // Bind buffer
            bind->setBinding(TEXCOORD_BINDING, tvbuf);

            // Set up basic tex coordinates
            setDefaultUVs();
        }

        // set basic white material
        mMaterial = MaterialManager::getSingleton().getDefaultMaterial(false);
        mMaterial->load();
    }

    Rectangle2D::~Rectangle2D() 
    {
        OGRE_DELETE mRenderOp.vertexData;
    }

    void Rectangle2D::setCorners(float left, float top, float right, float bottom, bool updateAABB)
    {
        HardwareVertexBufferSharedPtr vbuf = 
            mRenderOp.vertexData->vertexBufferBinding->getBuffer(POSITION_BINDING);
        HardwareBufferLockGuard vbufLock(vbuf, HardwareBuffer::HBL_DISCARD);
        float* pFloat = static_cast<float*>(vbufLock.pData);

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

        if(updateAABB)
        {
            mBox.setExtents(
                std::min(left, right), std::min(top, bottom), 0,
                std::max(left, right), std::max(top, bottom), 0);
        }
    }

    void Rectangle2D::setNormals(const Vector3& topLeft, const Vector3& bottomLeft, const Vector3& topRight,
                                 const Vector3& bottomRight)
    {
        HardwareVertexBufferSharedPtr vbuf = 
            mRenderOp.vertexData->vertexBufferBinding->getBuffer(NORMAL_BINDING);
        HardwareBufferLockGuard vbufLock(vbuf, HardwareBuffer::HBL_DISCARD);
        float* pFloat = static_cast<float*>(vbufLock.pData);

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
    }

    void Rectangle2D::setUVs(const Vector2& topLeft, const Vector2& bottomLeft, const Vector2& topRight,
                             const Vector2& bottomRight)
    {
        OgreAssert(mRenderOp.vertexData->vertexDeclaration->getElementCount() > TEXCOORD_BINDING,
                   "Vertex data wasn't built with UV buffer");

        HardwareVertexBufferSharedPtr vbuf = 
            mRenderOp.vertexData->vertexBufferBinding->getBuffer(TEXCOORD_BINDING);
        HardwareBufferLockGuard vbufLock(vbuf, HardwareBuffer::HBL_DISCARD);
        float* pFloat = static_cast<float*>(vbufLock.pData);

        *pFloat++ = topLeft.x;
        *pFloat++ = topLeft.y;

        *pFloat++ = bottomLeft.x;
        *pFloat++ = bottomLeft.y;

        *pFloat++ = topRight.x;
        *pFloat++ = topRight.y;

        *pFloat++ = bottomRight.x;
        *pFloat++ = bottomRight.y;
    }

    void Rectangle2D::setDefaultUVs()
    {
        setUVs( Vector2::ZERO, Vector2::UNIT_Y, Vector2::UNIT_X, Vector2::UNIT_SCALE );
    }

    // Override this method to prevent parent transforms (rotation,translation,scale)
    void Rectangle2D::getWorldTransforms( Matrix4* xform ) const
    {
        // return identity matrix to prevent parent transforms
        *xform = Matrix4::IDENTITY;
    }

    const String& Rectangle2D::getMovableType() const
    {
        return Rectangle2DFactory::FACTORY_TYPE_NAME;
    }

    const String Rectangle2DFactory::FACTORY_TYPE_NAME = "Rectangle2D";

    MovableObject* Rectangle2DFactory::createInstanceImpl(const String& name, const NameValuePairList* params)
    {
        bool includeTextureCoords = false;
        if (params)
        {
            auto ni = params->find("includeTextureCoords");
            if (ni != params->end())
                StringConverter::parse(ni->second, includeTextureCoords);
        }
        return new Rectangle2D(includeTextureCoords);
    }
}

