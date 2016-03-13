
#include "OgreStableHeaders.h"

#include "OgreWireAabb.h"

#include "Vao/OgreVaoManager.h"
#include "Vao/OgreVertexArrayObject.h"

#include "OgreSceneManager.h"

#include "OgreRoot.h"
#include "OgreHlms.h"
#include "OgreHlmsManager.h"

namespace Ogre
{
    WireAabb::WireAabb( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager *manager) :
        MovableObject( id, objectMemoryManager, manager, 0 ),
        Renderable(),
        mTrackedObject( 0 )
    {
        Aabb aabb( Vector3::ZERO, Vector3::UNIT_SCALE );
        mObjectData.mLocalAabb->setFromAabb( aabb, mObjectData.mIndex );
        mObjectData.mWorldAabb->setFromAabb( aabb, mObjectData.mIndex );
        mObjectData.mLocalRadius[mObjectData.mIndex] = aabb.getRadius();
        mObjectData.mWorldRadius[mObjectData.mIndex] = aabb.getRadius();

        createBuffers();

        setCastShadows( false );
        mRenderables.push_back( this );

        this->setDatablock( Root::getSingleton().getHlmsManager()->
                            getHlms( HLMS_UNLIT )->getDefaultDatablock() );
    }
    //-----------------------------------------------------------------------------------
    WireAabb::~WireAabb()
    {
        track( (MovableObject*)0 );

        VaoManager *vaoManager = mManager->getDestinationRenderSystem()->getVaoManager();

        VertexArrayObjectArray::const_iterator itor = mVaoPerLod[0].begin();
        VertexArrayObjectArray::const_iterator end  = mVaoPerLod[0].end();
        while( itor != end )
        {
            VertexArrayObject *vao = *itor;

            const VertexBufferPackedVec &vertexBuffers = vao->getVertexBuffers();
            VertexBufferPackedVec::const_iterator itBuffers = vertexBuffers.begin();
            VertexBufferPackedVec::const_iterator enBuffers = vertexBuffers.end();

            while( itBuffers != enBuffers )
            {
                vaoManager->destroyVertexBuffer( *itBuffers );
                ++itBuffers;
            }

            if( vao->getIndexBuffer() )
                vaoManager->destroyIndexBuffer( vao->getIndexBuffer() );
            vaoManager->destroyVertexArrayObject( vao );

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void WireAabb::track( const MovableObject *movableObject )
    {
        assert( this != movableObject );

        if( mTrackedObject )
            mManager->_removeWireAabb( this );

        mTrackedObject = movableObject;

        if( mTrackedObject )
            mManager->_addWireAabb( this );

        if( !mTrackedObject && mParentNode )
        {
            //Not tracking anymore, need to get rid of our SceneNode
            SceneNode *sceneNode = getParentSceneNode();
            sceneNode->getParentSceneNode()->removeAndDestroyChild( sceneNode );
            mParentNode = 0;
        }
        else if( mTrackedObject && !mParentNode )
        {
            //Started tracking, we need a node of our own.
            SceneNode *newNode = mManager->getRootSceneNode()->createChildSceneNode();
            newNode->attachObject( this );
        }
    }
    //-----------------------------------------------------------------------------------
    void WireAabb::setToAabb( const Aabb &aabb )
    {
        if( mTrackedObject )
            track( (MovableObject*)0 );

        if( !mParentNode )
        {
            //We need a node of our own.
            SceneNode *newNode = mManager->getRootSceneNode()->createChildSceneNode();
            newNode->attachObject( this );
        }

        setVisible( true );
        mParentNode->setPosition( aabb.mCenter );
        mParentNode->setScale( aabb.mHalfSize );
    }
    //-----------------------------------------------------------------------------------
    void WireAabb::_updateTracking(void)
    {
        if( !mTrackedObject->isAttached() )
        {
            this->setVisible( false );
        }
        else
        {
            this->setVisible( mTrackedObject->isVisible() );
            Aabb trackedAabb = mTrackedObject->getWorldAabb();
            mParentNode->setPosition( trackedAabb.mCenter );
            mParentNode->setScale( trackedAabb.mHalfSize );
        }
    }
    //-----------------------------------------------------------------------------------
    void WireAabb::createBuffers(void)
    {
        const float c_vertexData[8*3] =
        {
            -1, -1,  1,
             1, -1,  1,
             1,  1,  1,
            -1,  1,  1,
            -1, -1, -1,
             1, -1, -1,
             1,  1, -1,
            -1,  1, -1
        };

        //Create the indices.
        const Ogre::uint16 c_indexData[2 * 4 * 3] =
        {
            0, 1,   1, 2,   2, 3,   3, 0,	//Front
            4, 5,   5, 6,   6, 7,   7, 4,	//Back
            0, 4,   1, 5,   2, 6,   3, 7
        };

        Ogre::uint16 *cubeIndices = reinterpret_cast<Ogre::uint16*>( OGRE_MALLOC_SIMD(
                                                                         sizeof(Ogre::uint16) * 2 * 4 * 3,
                                                                         Ogre::MEMCATEGORY_GEOMETRY ) );
        memcpy( cubeIndices, c_indexData, sizeof( c_indexData ) );

        VaoManager *vaoManager = mManager->getDestinationRenderSystem()->getVaoManager();
        Ogre::IndexBufferPacked *indexBuffer = 0;

        try
        {
            indexBuffer = vaoManager->createIndexBuffer( Ogre::IndexBufferPacked::IT_16BIT,
                                                         2 * 4 * 3,
                                                         Ogre::BT_IMMUTABLE,
                                                         cubeIndices, true );
        }
        catch( Ogre::Exception &e )
        {
            // When keepAsShadow = true, the memory will be freed when the index buffer is destroyed.
            // However if for some weird reason there is an exception raised, the memory will
            // not be freed, so it is up to us to do so.
            // The reasons for exceptions are very rare. But we're doing this for correctness.
            OGRE_FREE_SIMD( indexBuffer, Ogre::MEMCATEGORY_GEOMETRY );
            indexBuffer = 0;
            throw e;
        }

        //Create the vertex buffer

        //Vertex declaration
        VertexElement2Vec vertexElements;
        vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_POSITION ) );

        //For immutable buffers, it is mandatory that cubeVertices is not a null pointer.
        float *cubeVertices = reinterpret_cast<float*>( OGRE_MALLOC_SIMD( sizeof(float) * 8 * 3,
                                                                          Ogre::MEMCATEGORY_GEOMETRY ) );
        //Fill the data.
        memcpy( cubeVertices, c_vertexData, sizeof(float) * 8 * 3 );

        Ogre::VertexBufferPacked *vertexBuffer = 0;
        try
        {
            //Create the actual vertex buffer.
            vertexBuffer = vaoManager->createVertexBuffer( vertexElements, 8,
                                                           BT_IMMUTABLE,
                                                           cubeVertices, true );
        }
        catch( Ogre::Exception &e )
        {
            OGRE_FREE_SIMD( vertexBuffer, Ogre::MEMCATEGORY_GEOMETRY );
            vertexBuffer = 0;
            throw e;
        }

        //Now the Vao. We'll just use one vertex buffer source
        VertexBufferPackedVec vertexBuffers;
        vertexBuffers.push_back( vertexBuffer );
        Ogre::VertexArrayObject *vao = vaoManager->createVertexArrayObject(
                    vertexBuffers, indexBuffer, OT_LINE_LIST );

        mVaoPerLod[0].push_back( vao );
        mVaoPerLod[1].push_back( vao );
    }
    //-----------------------------------------------------------------------------------
    const String& WireAabb::getMovableType(void) const
    {
        return WireAabbFactory::FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------------------
    const LightList& WireAabb::getLights(void) const
    {
        return this->queryLights(); //Return the data from our MovableObject base class.
    }
    //-----------------------------------------------------------------------------------
    void WireAabb::getRenderOperation( v1::RenderOperation& op , bool casterPass )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                        "WireAabb do not implement getRenderOperation."
                        " You've put a v2 object in "
                        "the wrong RenderQueue ID (which is set to be compatible with "
                        "v1::Entity). Do not mix v2 and v1 objects",
                        "WireAabb::getRenderOperation" );
    }
    //-----------------------------------------------------------------------------------
    void WireAabb::getWorldTransforms( Matrix4* xform ) const
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                        "WireAabb do not implement getWorldTransforms."
                        " You've put a v2 object in "
                        "the wrong RenderQueue ID (which is set to be compatible with "
                        "v1::Entity). Do not mix v2 and v1 objects",
                        "WireAabb::getRenderOperation" );
    }
    //-----------------------------------------------------------------------------------
    bool WireAabb::getCastsShadows(void) const
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                        "WireAabb do not implement getCastsShadows."
                        " You've put a v2 object in "
                        "the wrong RenderQueue ID (which is set to be compatible with "
                        "v1::Entity). Do not mix v2 and v1 objects",
                        "WireAabb::getRenderOperation" );
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String WireAabbFactory::FACTORY_TYPE_NAME = "WireAabb";
    //-----------------------------------------------------------------------
    const String& WireAabbFactory::getType(void) const
    {
        return FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    MovableObject* WireAabbFactory::createInstanceImpl( IdType id,
                                                              ObjectMemoryManager *objectMemoryManager,
                                                              SceneManager *manager,
                                                              const NameValuePairList* params )
    {
        return OGRE_NEW WireAabb( id, objectMemoryManager, manager );
    }
    //-----------------------------------------------------------------------
    void WireAabbFactory::destroyInstance( MovableObject* obj )
    {
        OGRE_DELETE obj;
    }
}
