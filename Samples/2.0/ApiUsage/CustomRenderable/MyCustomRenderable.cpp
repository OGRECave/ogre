
#include "MyCustomRenderable.h"

#include "Vao/OgreVaoManager.h"
#include "Vao/OgreVertexArrayObject.h"

#include "OgreSceneManager.h"

#include "OgreRoot.h"
#include "OgreHlms.h"
#include "OgreHlmsManager.h"

namespace Ogre
{
    struct CubeVertices
    {
        float px, py, pz;   //Position
        float nx, ny, nz;   //Normals

        CubeVertices() {}
        CubeVertices( float _px, float _py, float _pz,
                      float _nx, float _ny, float _nz ) :
            px( _px ), py( _py ), pz( _pz ),
            nx( _nx ), ny( _ny ), nz( _nz )
        {
        }
    };

    const CubeVertices c_originalVertices[8] =
    {
        CubeVertices( -1, -1,  1, -0.57737, -0.57737,  0.57737 ),
        CubeVertices(  1, -1,  1,  0.57737, -0.57737,  0.57737 ),
        CubeVertices(  1,  1,  1,  0.57737,  0.57737,  0.57737 ),
        CubeVertices( -1,  1,  1, -0.57737,  0.57737,  0.57737 ),
        CubeVertices( -1, -1, -1, -0.57737, -0.57737, -0.57737 ),
        CubeVertices(  1, -1, -1,  0.57737, -0.57737, -0.57737 ),
        CubeVertices(  1,  1, -1,  0.57737,  0.57737, -0.57737 ),
        CubeVertices( -1,  1, -1, -0.57737,  0.57737, -0.57737 )
    };

    MyCustomRenderable::MyCustomRenderable( IdType id, ObjectMemoryManager *objectMemoryManager,
                                            SceneManager *manager, uint8 renderQueueId ) :
        MovableObject( id, objectMemoryManager, manager, renderQueueId ),
        Renderable()
    {
        //Set the bounds!!! Very important! If you don't set it, the object will not
        //appear on screen as it will always fail the frustum culling.
        //This example uses an infinite aabb; but you really want to use an Aabb as tight
        //as possible for maximum efficiency (so Ogre avoids rendering an object that
        //is off-screen)
        //Note the WorldAabb and the WorldRadius will be automatically updated by Ogre
        //every frame as rendering begins (it's calculated based on the local version
        //combined with the scene node's transform).
        Aabb aabb( Aabb::BOX_INFINITE );
        mObjectData.mLocalAabb->setFromAabb( aabb, mObjectData.mIndex );
        mObjectData.mWorldAabb->setFromAabb( aabb, mObjectData.mIndex );
        mObjectData.mLocalRadius[mObjectData.mIndex] = std::numeric_limits<Real>::max();
        mObjectData.mWorldRadius[mObjectData.mIndex] = std::numeric_limits<Real>::max();

        createBuffers();

        //This is very important!!! A MovableObject must tell what Renderables to render
        //through this array. Since we derive from both MovableObject & Renderable, add
        //ourselves to the array. Otherwise, nothing will be rendered.
        //Tip: You can use this array as a rough way to show or hide Renderables
        //that belong to this MovableObject.
        mRenderables.push_back( this );

        //If we don't set a datablock, we'll crash Ogre.
        this->setDatablock( Root::getSingleton().getHlmsManager()->
                            getHlms( HLMS_PBS )->getDefaultDatablock() );
    }
    //-----------------------------------------------------------------------------------
    MyCustomRenderable::~MyCustomRenderable()
    {
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
    void MyCustomRenderable::createBuffers(void)
    {
        //Create the indices.
        const Ogre::uint16 c_indexData[3 * 2 * 6] =
        {
            0, 1, 2, 2, 3, 0, //Front face
            6, 5, 4, 4, 7, 6, //Back face

            3, 2, 6, 6, 7, 3, //Top face
            5, 1, 0, 0, 4, 5, //Bottom face

            4, 0, 3, 3, 7, 4, //Left face
            6, 2, 1, 1, 5, 6, //Right face
        };

        Ogre::uint16 *cubeIndices = reinterpret_cast<Ogre::uint16*>( OGRE_MALLOC_SIMD(
                                                                         sizeof(Ogre::uint16) * 3 * 2 * 6,
                                                                         Ogre::MEMCATEGORY_GEOMETRY ) );
        memcpy( cubeIndices, c_indexData, sizeof( c_indexData ) );

        VaoManager *vaoManager = mManager->getDestinationRenderSystem()->getVaoManager();
        Ogre::IndexBufferPacked *indexBuffer = 0;

        try
        {
            indexBuffer = vaoManager->createIndexBuffer( Ogre::IndexBufferPacked::IT_16BIT,
                                                         3 * 2 * 6,
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
        vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_NORMAL ) );

        //For immutable buffers, it is mandatory that cubeVertices is not a null pointer.
        CubeVertices *cubeVertices = reinterpret_cast<CubeVertices*>( OGRE_MALLOC_SIMD(
                                                                          sizeof(CubeVertices) * 8,
                                                                          Ogre::MEMCATEGORY_GEOMETRY ) );
        //Fill the data.
        memcpy( cubeVertices, c_originalVertices, sizeof(CubeVertices) * 8 );

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

        //Now the Vao. We'll just use one vertex buffer source (multi-source not working yet)
        VertexBufferPackedVec vertexBuffers;
        vertexBuffers.push_back( vertexBuffer );
        Ogre::VertexArrayObject *vao = vaoManager->createVertexArrayObject(
                    vertexBuffers, indexBuffer, OT_TRIANGLE_LIST );

        mVaoPerLod[0].push_back( vao );
        //Use the same geometry for shadow casting. You can optimize performance by creating
        //a different Vao that only uses VES_POSITION, VES_BLEND_INDICES & VES_BLEND_WEIGHTS
        //(packed together to fit the caches) and avoids duplicated vertices (usually
        //needed by normals, UVs, etc)
        mVaoPerLod[1].push_back( vao );
    }
    //-----------------------------------------------------------------------------------
    const String& MyCustomRenderable::getMovableType(void) const
    {
        return BLANKSTRING;
    }
    //-----------------------------------------------------------------------------------
    const LightList& MyCustomRenderable::getLights(void) const
    {
        return this->queryLights(); //Return the data from our MovableObject base class.
    }
    //-----------------------------------------------------------------------------------
    void MyCustomRenderable::getRenderOperation( v1::RenderOperation& op , bool casterPass )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                        "MyCustomRenderable do not implement getRenderOperation."
                        " You've put a v2 object in "
                        "the wrong RenderQueue ID (which is set to be compatible with "
                        "v1::Entity). Do not mix v2 and v1 objects",
                        "MyCustomRenderable::getRenderOperation" );
    }
    //-----------------------------------------------------------------------------------
    void MyCustomRenderable::getWorldTransforms( Matrix4* xform ) const
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                        "MyCustomRenderable do not implement getWorldTransforms."
                        " You've put a v2 object in "
                        "the wrong RenderQueue ID (which is set to be compatible with "
                        "v1::Entity). Do not mix v2 and v1 objects",
                        "MyCustomRenderable::getRenderOperation" );
    }
    //-----------------------------------------------------------------------------------
    bool MyCustomRenderable::getCastsShadows(void) const
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                        "MyCustomRenderable do not implement getCastsShadows."
                        " You've put a v2 object in "
                        "the wrong RenderQueue ID (which is set to be compatible with "
                        "v1::Entity). Do not mix v2 and v1 objects",
                        "MyCustomRenderable::getRenderOperation" );
    }
}
