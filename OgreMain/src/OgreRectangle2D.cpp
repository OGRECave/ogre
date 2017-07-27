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
#include "OgreRectangle2D.h"

#include "OgreVertexIndexData.h"

#include "OgreHardwareBufferManager.h"
#include "OgreMaterialManager.h"
#include "OgreStringConverter.h"

namespace Ogre
{
namespace v1
{
    Rectangle2D::Rectangle2D( bool bQuad, IdType id, ObjectMemoryManager *objectMemoryManager,
                              SceneManager *manager ) :
            MovableObject( id, objectMemoryManager, manager, 1 ),
            mPosition( Vector3::ZERO ),
            mOrientation( Quaternion::IDENTITY ),
            mScale( Vector3::UNIT_SCALE ),
            mQuad( bQuad )
    {
        initRectangle2D();

        //By default we want Rectangle2Ds to still work in wireframe mode
        setPolygonModeOverrideable( false );
    }
    //-----------------------------------------------------------------------------------
    void Rectangle2D::initRectangle2D(void)
    {
        // use identity projection and view matrices
        mUseIdentityProjection  = true;
        mUseIdentityView        = true;

        mRenderOp.vertexData = OGRE_NEW VertexData();

        mRenderOp.indexData                 = 0;
        mRenderOp.vertexData->vertexCount   = mQuad ? 4 : 3;
        mRenderOp.vertexData->vertexStart   = 0;
        //Strip or list is fine for triangle, but using lists avoids tiny unnecessary state
        //switches (assuming most of normal render is indexed list).
        mRenderOp.operationType             = mQuad ? OT_TRIANGLE_STRIP : OT_TRIANGLE_LIST;
        mRenderOp.useIndexes                                    = false; 
        mRenderOp.useGlobalInstancingVertexBufferIsAvailable    = false;

        VertexDeclaration* decl     = mRenderOp.vertexData->vertexDeclaration;
        VertexBufferBinding* bind   = mRenderOp.vertexData->vertexBufferBinding;

        size_t offset = 0;
        decl->addElement( 0, 0, VET_FLOAT3, VES_POSITION );
        offset += VertexElement::getTypeSize( VET_FLOAT3 );
        decl->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES );

        HardwareVertexBufferSharedPtr vbuf = 
            HardwareBufferManager::getSingleton().createVertexBuffer(
            decl->getVertexSize( 0 ), mRenderOp.vertexData->vertexCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY );

        // Bind buffer
        bind->setBinding( 0, vbuf );

        float *pVerts = static_cast<float*>( vbuf->lock(HardwareBuffer::HBL_DISCARD) );
        if( mQuad )
        {
            //1st Top-left
            *pVerts++ = -1.0f;
            *pVerts++ =  1.0f;
            *pVerts++ = -1.0f;

            *pVerts++ =  0.0f;
            *pVerts++ =  0.0f;

            //2nd Bottom-left
            *pVerts++ = -1.0f;
            *pVerts++ = -1.0f;
            *pVerts++ = -1.0f;

            *pVerts++ =  0.0f;
            *pVerts++ =  1.0f;

            //3rd Top-right
            *pVerts++ =  1.0f;
            *pVerts++ =  1.0f;
            *pVerts++ = -1.0f;

            *pVerts++ =  1.0f;
            *pVerts++ =  0.0f;

            //4th Bottom-right
            *pVerts++ =  1.0f;
            *pVerts++ = -1.0f;
            *pVerts++ = -1.0f;

            *pVerts++ =  1.0f;
            *pVerts++ =  1.0f;
        }
        else
        {
            //1st Top-left
            *pVerts++ = -1.0f;
            *pVerts++ =  1.0f;
            *pVerts++ = -1.0f;

            *pVerts++ =  0.0f;
            *pVerts++ =  0.0f;

            //2nd Bottom-left
            *pVerts++ = -1.0f;
            *pVerts++ = -3.0f; //3 = lerp( -1, 1, 2 );
            *pVerts++ = -1.0f;

            *pVerts++ =  0.0f;
            *pVerts++ =  2.0f;

            //3rd Top-right
            *pVerts++ =  3.0f;
            *pVerts++ =  1.0f;
            *pVerts++ = -1.0f;

            *pVerts++ =  2.0f;
            *pVerts++ =  0.0f;
        }

        vbuf->unlock();

        //Add the normals.
        decl->addElement( 1, 0, VET_FLOAT3, VES_NORMAL );

        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
                decl->getVertexSize( 1 ), mRenderOp.vertexData->vertexCount,
                HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE );

        bind->setBinding( 1, vbuf );

        float *pNorm = static_cast<float*>( vbuf->lock(HardwareBuffer::HBL_DISCARD) );
        *pNorm++ = 0.0f;
        *pNorm++ = 0.0f;
        *pNorm++ = 1.0f;

        *pNorm++ = 0.0f;
        *pNorm++ = 0.0f;
        *pNorm++ = 1.0f;

        *pNorm++ = 0.0f;
        *pNorm++ = 0.0f;
        *pNorm++ = 1.0f;

        if( mQuad )
        {
            *pNorm++ = 0.0f;
            *pNorm++ = 0.0f;
            *pNorm++ = 1.0f;
        }

        vbuf->unlock();
    }
    //-----------------------------------------------------------------------------------
    Rectangle2D::~Rectangle2D()
    {
        OGRE_DELETE mRenderOp.vertexData;
    }
    //-----------------------------------------------------------------------------------
    void Rectangle2D::setCorners( Real left, Real top, Real width, Real height )
    {
        mPosition   = Vector3( left, top, 0.0f );
        mScale      = Vector3( width, height, 0.0f );
    }
    //-----------------------------------------------------------------------------------
    void Rectangle2D::setNormals( const Ogre::Vector3 &topLeft, const Ogre::Vector3 &bottomLeft,
                                    const Ogre::Vector3 &topRight, const Ogre::Vector3 &bottomRight)
    {
        HardwareVertexBufferSharedPtr vbuf = mRenderOp.vertexData->vertexBufferBinding->getBuffer( 1 );
        float* pFloat = static_cast<float*>( vbuf->lock(HardwareBuffer::HBL_DISCARD) );

        *pFloat++ = topLeft.x;
        *pFloat++ = topLeft.y;
        *pFloat++ = topLeft.z;

        if( mQuad )
        {
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
        else
        {
            *pFloat++ = bottomLeft.x;
            *pFloat++ = Math::lerp( topLeft.y, bottomLeft.y, 2.0f );
            *pFloat++ = bottomLeft.z;

            *pFloat++ = Math::lerp( topLeft.x, topRight.x, 2.0f );
            *pFloat++ = topRight.y;
            *pFloat++ = topRight.z;
        }

        vbuf->unlock();
    }
    //-----------------------------------------------------------------------------------
    void Rectangle2D::getWorldTransforms( Matrix4* xform ) const
    {
        xform->makeTransform( mPosition, mScale, mOrientation );
    }
    //-----------------------------------------------------------------------------------
    void Rectangle2D::getRenderOperation( RenderOperation& op, bool casterPass )
    {
        op = mRenderOp;
    }
    //-----------------------------------------------------------------------------------
    const LightList& Rectangle2D::getLights(void) const
    {
        static const LightList l;
        return l;
    }
    //-----------------------------------------------------------------------
    const String& Rectangle2D::getMovableType(void) const
    {
        return Rectangle2DFactory::FACTORY_TYPE_NAME;
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String Rectangle2DFactory::FACTORY_TYPE_NAME = "Rectangle2D";
    //-----------------------------------------------------------------------
    const String& Rectangle2DFactory::getType(void) const
    {
        return FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    MovableObject* Rectangle2DFactory::createInstanceImpl( IdType id,
                                                           ObjectMemoryManager *objectMemoryManager,
                                                           SceneManager *manager,
                                                           const NameValuePairList* params )
    {
        bool bQuad = true;
        if (params != 0)
        {
            NameValuePairList::const_iterator ni;

            ni = params->find("quad");
            if (ni != params->end())
            {
                bQuad = StringConverter::parseBool( ni->second, true );
            }

        }

        return OGRE_NEW Rectangle2D( bQuad, id, objectMemoryManager, manager );
    }
    //-----------------------------------------------------------------------
    void Rectangle2DFactory::destroyInstance( MovableObject* obj)
    {
        OGRE_DELETE obj;
    }
}
}
