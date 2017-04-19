
#include "DynamicGeometryGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"
#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreMesh2.h"
#include "OgreMeshManager2.h"
#include "OgreSubMesh2.h"
#include "OgreMesh2Serializer.h"

#include "OgreRoot.h"
#include "Vao/OgreVaoManager.h"
#include "Vao/OgreVertexArrayObject.h"

#include "OgreCamera.h"
#include "OgreRenderWindow.h"

using namespace Demo;

namespace Demo
{
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

    DynamicGeometryGameState::DynamicGeometryGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mRotationTime( 0.0f )
    {
        memset( mDynamicVertexBuffer, 0, sizeof( mDynamicVertexBuffer ) );
    }
    //-----------------------------------------------------------------------------------
    Ogre::IndexBufferPacked* DynamicGeometryGameState::createIndexBuffer(void)
    {
        Ogre::IndexBufferPacked *indexBuffer = 0;

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

        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::RenderSystem *renderSystem = root->getRenderSystem();
        Ogre::VaoManager *vaoManager = renderSystem->getVaoManager();

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

        return indexBuffer;
    }
    //-----------------------------------------------------------------------------------
    Ogre::MeshPtr DynamicGeometryGameState::createStaticMesh( bool partialMesh )
    {
        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::RenderSystem *renderSystem = root->getRenderSystem();
        Ogre::VaoManager *vaoManager = renderSystem->getVaoManager();

        //Create the mesh
        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual(
                    partialMesh ? "My PartialMesh" : "My StaticMesh",
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

        //Create one submesh
        Ogre::SubMesh *subMesh = mesh->createSubMesh();

        //Vertex declaration
        Ogre::VertexElement2Vec vertexElements;
        vertexElements.push_back( Ogre::VertexElement2( Ogre::VET_FLOAT3, Ogre::VES_POSITION ) );
        vertexElements.push_back( Ogre::VertexElement2( Ogre::VET_FLOAT3, Ogre::VES_NORMAL ) );

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
                                                           partialMesh ? Ogre::BT_DEFAULT :
                                                                         Ogre::BT_IMMUTABLE,
                                                           cubeVertices, true );
        }
        catch( Ogre::Exception &e )
        {
            OGRE_FREE_SIMD( vertexBuffer, Ogre::MEMCATEGORY_GEOMETRY );
            vertexBuffer = 0;
            throw e;
        }

        //Now the Vao. We'll just use one vertex buffer source (multi-source not working yet)
        Ogre::VertexBufferPackedVec vertexBuffers;
        vertexBuffers.push_back( vertexBuffer );
        Ogre::IndexBufferPacked *indexBuffer = createIndexBuffer(); //Create the actual index buffer
        Ogre::VertexArrayObject *vao = vaoManager->createVertexArrayObject(
                    vertexBuffers, indexBuffer, Ogre::OT_TRIANGLE_LIST );

        //Each Vao pushed to the vector refers to an LOD level.
        //Must be in sync with mesh->mLodValues & mesh->mNumLods if you use more than one level
        subMesh->mVao[Ogre::VpNormal].push_back( vao );
        //Use the same geometry for shadow casting.
        subMesh->mVao[Ogre::VpShadow].push_back( vao );

        //Set the bounds to get frustum culling and LOD to work correctly.
        mesh->_setBounds( Ogre::Aabb( Ogre::Vector3::ZERO, Ogre::Vector3::UNIT_SCALE ), false );
        mesh->_setBoundingSphereRadius( 1.732f );

        return mesh;
    }
    //-----------------------------------------------------------------------------------
    std::pair<Ogre::MeshPtr, Ogre::VertexBufferPacked*> DynamicGeometryGameState::createDynamicMesh(
                                                                                        size_t idx )
    {
        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::RenderSystem *renderSystem = root->getRenderSystem();
        Ogre::VaoManager *vaoManager = renderSystem->getVaoManager();

        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual(
                    "My DynamicMesh_" + Ogre::StringConverter::toString(idx),
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

        Ogre::SubMesh *subMesh = mesh->createSubMesh();

        Ogre::VertexElement2Vec vertexElements;
        vertexElements.push_back( Ogre::VertexElement2( Ogre::VET_FLOAT3, Ogre::VES_POSITION ) );
        vertexElements.push_back( Ogre::VertexElement2( Ogre::VET_FLOAT3, Ogre::VES_NORMAL ) );

        //Pass cubeVertices to have initialized values. However you can safely use a null pointer.
        CubeVertices cubeVertices[8];
        memcpy( cubeVertices, c_originalVertices, sizeof(CubeVertices) * 8 );

        Ogre::VertexBufferPacked *vertexBuffer = 0;
        vertexBuffer = vaoManager->createVertexBuffer( vertexElements, 8,
                                                       Ogre::BT_DYNAMIC_PERSISTENT,
                                                       cubeVertices, false );

        //Now the Vao
        Ogre::VertexBufferPackedVec vertexBuffers;
        vertexBuffers.push_back( vertexBuffer );
        Ogre::IndexBufferPacked *indexBuffer = createIndexBuffer();
        Ogre::VertexArrayObject *vao = vaoManager->createVertexArrayObject(
                    vertexBuffers, indexBuffer, Ogre::OT_TRIANGLE_LIST );

        subMesh->mVao[Ogre::VpNormal].push_back( vao );
        //Use the same geometry for shadow casting.
        subMesh->mVao[Ogre::VpShadow].push_back( vao );

        //Set the bounds to get frustum culling and LOD to work correctly.
        mesh->_setBounds( Ogre::Aabb( Ogre::Vector3::ZERO, Ogre::Vector3::UNIT_SCALE ), false );
        mesh->_setBoundingSphereRadius( 1.732f );

        return std::pair<Ogre::MeshPtr, Ogre::VertexBufferPacked*>( mesh, vertexBuffer );
    }
    //-----------------------------------------------------------------------------------
    void DynamicGeometryGameState::createScene01(void)
    {
        //Create all four types of meshes.
        mStaticMesh  = createStaticMesh( false );
        mPartialMesh = createStaticMesh( true );

        for( size_t i=0; i<2; ++i )
        {
            std::pair<Ogre::MeshPtr, Ogre::VertexBufferPacked*> dynamicMesh;
            dynamicMesh = createDynamicMesh( i );
            mDynamicMesh[i]         = dynamicMesh.first;
            mDynamicVertexBuffer[i] = dynamicMesh.second;
        }

        //Initialize the scene (items)
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        Ogre::Item *item = sceneManager->createItem( mStaticMesh, Ogre::SCENE_DYNAMIC );
        Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                createChildSceneNode( Ogre::SCENE_DYNAMIC );
        sceneNode->attachObject( item );
        sceneNode->setPosition( -6, 0, 0 );

        item = sceneManager->createItem( mPartialMesh, Ogre::SCENE_DYNAMIC );
        sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                createChildSceneNode( Ogre::SCENE_DYNAMIC );
        sceneNode->attachObject( item );
        sceneNode->setPosition( -2, 0, 0 );

        item = sceneManager->createItem( mDynamicMesh[0], Ogre::SCENE_DYNAMIC );
        sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                createChildSceneNode( Ogre::SCENE_DYNAMIC );
        sceneNode->attachObject( item );
        sceneNode->setPosition( 2, 0, 0 );

        item = sceneManager->createItem( mDynamicMesh[1], Ogre::SCENE_DYNAMIC );
        sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                createChildSceneNode( Ogre::SCENE_DYNAMIC );
        sceneNode->attachObject( item );
        sceneNode->setPosition( 6, 0, 0 );

        Ogre::Light *light = sceneManager->createLight();
        Ogre::SceneNode *lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
        lightNode->attachObject( light );
        light->setPowerScale( Ogre::Math::PI ); //Since we don't do HDR, counter the PBS' division by PI
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        mCameraController = new CameraController( mGraphicsSystem, false );

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void DynamicGeometryGameState::destroyScene()
    {
        for( size_t i=0; i<2; ++i )
        {
            //Permanently unmap persistent mapped buffers
            if( mDynamicVertexBuffer[i] &&
                mDynamicVertexBuffer[i]->getMappingState() != Ogre::MS_UNMAPPED )
            {
                mDynamicVertexBuffer[i]->unmap( Ogre::UO_UNMAP_ALL );
            }
        }

        //If we don't do this, the smart pointers will try to
        //delete memory after Ogre has shutdown (and crash)
        mStaticMesh.setNull();
        mPartialMesh.setNull();
        for( size_t i=0; i<2; ++i )
            mDynamicMesh[i].setNull();
    }
    //-----------------------------------------------------------------------------------
    void DynamicGeometryGameState::updateDynamicBuffer01( float *cubeVertices, const CubeVertices originalVerts[8],
                                                          size_t start, size_t end ) const
    {
        const float cosAlpha = cosf( mRotationTime );
        const float sinAlpha = sinf( mRotationTime );

        for( size_t i=start; i<end; ++i )
        {
            cubeVertices[0] = originalVerts[i].px * cosAlpha - originalVerts[i].pz * sinAlpha;
            cubeVertices[1] = originalVerts[i].py;
            cubeVertices[2] = originalVerts[i].px * sinAlpha + originalVerts[i].pz * cosAlpha;

            cubeVertices[3] = originalVerts[i].nx * cosAlpha - originalVerts[i].nz * sinAlpha;
            cubeVertices[4] = originalVerts[i].ny;
            cubeVertices[5] = originalVerts[i].nx * sinAlpha + originalVerts[i].nz * cosAlpha;

            cubeVertices += 6;
        }
    }
    //-----------------------------------------------------------------------------------
    void DynamicGeometryGameState::update( float timeSinceLast )
    {
        mRotationTime += timeSinceLast;
        mRotationTime = fmod( mRotationTime, Ogre::Math::PI * 2.0f );

        const float cosAlpha = cosf( mRotationTime );
        const float sinAlpha = sinf( mRotationTime );

        {
            //Partial update the buffer's 2nd vertex.
            Ogre::VertexBufferPacked *partialVertexBuffer = mPartialMesh->getSubMesh( 0 )->
                    mVao[Ogre::VpNormal][0]->getVertexBuffers()[0];
            CubeVertices newVertex( c_originalVertices[2] );
            newVertex.px += cosAlpha;
            partialVertexBuffer->upload( &newVertex, 2, 1 );
        }


        //----------------------------------------------------------------
        //First dynamic buffer example.

        //Dynamic buffers assume you will be fully uploading the entire buffer's contents
        //every time you map them.
        //"Partially" mapping or filling the buffer will not result in desired results
        //(data uploaded in previous frames will get mixed with with the
        //new data you're uploading)

        //You should NEVER read from cubeVertices pointer. Beware that something as innocent as
        //++(*cubeVertices) or cubeVertices[0] += 1; or cubesVertices[1] = cubesVertices[0];
        //implies reading from the mapped memory.
        //
        //Reading from this memory may return garbage, may return old data (from previous frames)
        //and will probably be *very* slow since the memory region is often write combined.
        //Sometimes you need to check the assembly to see the compiler isn't reading from
        //that memory even though the C++ code doesn't.
        float * RESTRICT_ALIAS cubeVertices = reinterpret_cast<float*RESTRICT_ALIAS>(
                    mDynamicVertexBuffer[0]->map( 0, mDynamicVertexBuffer[0]->getNumElements() ) );

        for( size_t i=0; i<8; ++i )
        {
            cubeVertices[0] = c_originalVertices[i].px * cosAlpha - c_originalVertices[i].py * sinAlpha;
            cubeVertices[1] = c_originalVertices[i].px * sinAlpha + c_originalVertices[i].py * cosAlpha;
            cubeVertices[2] = c_originalVertices[i].pz;

            cubeVertices[3] = c_originalVertices[i].nx * cosAlpha - c_originalVertices[i].ny * sinAlpha;
            cubeVertices[4] = c_originalVertices[i].nx * sinAlpha + c_originalVertices[i].ny * cosAlpha;
            cubeVertices[5] = c_originalVertices[i].nz;

            cubeVertices += 6;
        }

        mDynamicVertexBuffer[0]->unmap( Ogre::UO_KEEP_PERSISTENT );

        //----------------------------------------------------------------
        //Second dynamic buffer example.
        //  Update the cube mapping multiple times per frame.
        //  Every time you map, you 'advance' the buffer for the next frame.
        //  If you want to map it again within the same frame, you first
        //  need to 'regress' the buffer back to its original state,
        //  while being carefull that:
        //      1. Once you're done with all your maps, and by the time rendering starts,
        //         the buffer has advanced ONLY once.
        //      2. You do not write to a memory region that you have already written to,
        //         unless you know you haven't issued draw calls that are using this region yet.

        //The last 'false' indicates the buffer not to advance forward.
        cubeVertices = reinterpret_cast<float*RESTRICT_ALIAS>(
                    mDynamicVertexBuffer[1]->map( 0, 2, false ) );
        updateDynamicBuffer01( cubeVertices, c_originalVertices, 0, 2 );
        mDynamicVertexBuffer[1]->unmap( Ogre::UO_KEEP_PERSISTENT );

        //We do not regress the frame, because we haven't advanced yet.
        cubeVertices = reinterpret_cast<float*RESTRICT_ALIAS>(
                    mDynamicVertexBuffer[1]->map( 2, 4, true ) );
        updateDynamicBuffer01( cubeVertices, c_originalVertices, 2, 4 );
        mDynamicVertexBuffer[1]->unmap( Ogre::UO_KEEP_PERSISTENT );

        //We regress the frame, because the previous map had advanced automatically by passing 'true'.
        mDynamicVertexBuffer[1]->regressFrame();
        cubeVertices = reinterpret_cast<float*RESTRICT_ALIAS>(
                    mDynamicVertexBuffer[1]->map( 4, 6, false ) );
        updateDynamicBuffer01( cubeVertices, c_originalVertices, 4, 6 );
        mDynamicVertexBuffer[1]->unmap( Ogre::UO_KEEP_PERSISTENT );
        mDynamicVertexBuffer[1]->advanceFrame();    //Make sure we advance when we're done and
                                                    //draw calls may start afterwards.

        // ... hypothetical draw calls issued ...

        //We regress the frame, because the previous map had advanced it; and the frame isn't over yet.
        mDynamicVertexBuffer[1]->regressFrame();
        cubeVertices = reinterpret_cast<float*RESTRICT_ALIAS>(
                    mDynamicVertexBuffer[1]->map( 6, 8, false ) );
        updateDynamicBuffer01( cubeVertices, c_originalVertices, 6, 8 );
        mDynamicVertexBuffer[1]->unmap( Ogre::UO_KEEP_PERSISTENT );
        mDynamicVertexBuffer[1]->advanceFrame();    //Make sure we advance when we're done and
                                                    //draw calls may start afterwards.

        TutorialGameState::update( timeSinceLast );
    }
}
