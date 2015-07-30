
#include "V1InterfacesGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreEntity.h"
#include "OgreMesh.h"
#include "OgreMeshManager.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlms.h"

using namespace Demo;

namespace Demo
{
    V1InterfacesGameState::V1InterfacesGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription )
    {
    }
    //-----------------------------------------------------------------------------------
    void V1InterfacesGameState::createScene01(void)
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        Ogre::v1::MeshManager::getSingleton().createPlane( "Plane v1",
                                            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            Ogre::Plane( Ogre::Vector3::UNIT_Y, 1.0f ), 50.0f, 50.0f,
                                            1, 1, true, 1, 4.0f, 4.0f, Ogre::Vector3::UNIT_Z );

        {
            Ogre::v1::Entity *entity = sceneManager->createEntity(
                        "Plane v1", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                        Ogre::SCENE_DYNAMIC );

            entity->setCastShadows( false );

            //The plane mesh won't try to set any material. **Therefore it will crash
            //inside Ogre** if we don't specify any material (we use the default one here).
            Ogre::Hlms *hlms = mGraphicsSystem->getRoot()->getHlmsManager()->getHlms( Ogre::HLMS_PBS );
            entity->setDatablock( hlms->getDefaultDatablock() );

            Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                                                    createChildSceneNode( Ogre::SCENE_DYNAMIC );
            sceneNode->setPosition( 0, -1, 0 );
            sceneNode->attachObject( entity );
        }

        /*Ogre::v1::MeshPtr v1Mesh;

        //Load the v1 mesh. Notice the v1 namespace
        //Also notice the HBU_STATIC flag; since the HBU_WRITE_ONLY
        //bit would prohibit us from reading the data for importing.
        v1Mesh = Ogre::v1::MeshManager::getSingleton().load(
                    "athene.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                    Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC );

        bool halfPosition   = true;
        bool halfUVs        = true;
        bool useQtangents   = true;

        //Import the v1 mesh to v2
        V1Interfaces->importV1( v1Mesh.get(), halfPosition, halfUVs, useQtangents );

        //We don't need the v1 mesh. Free CPU memory, get it out of the GPU.
        //Leave it loaded if you want to use athene with v1 Entity.
        v1Mesh->unload();*/

        Ogre::v1::Entity *entity = sceneManager->createEntity( "athene.mesh",
                                                               Ogre::ResourceGroupManager::
                                                               AUTODETECT_RESOURCE_GROUP_NAME,
                                                               Ogre::SCENE_DYNAMIC );
        Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                createChildSceneNode( Ogre::SCENE_DYNAMIC );
        sceneNode->attachObject( entity );
        sceneNode->setPosition( 0.0f, 1.6f, 0.0f );
        sceneNode->scale( 0.02f, 0.02f, 0.02f );

        Ogre::Light *light = sceneManager->createLight();
        Ogre::SceneNode *lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
        lightNode->attachObject( light );
        light->setPowerScale( Ogre::Math::PI ); //Since we don't do HDR, counter the PBS' division by PI
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        mCameraController = new CameraController( mGraphicsSystem, false );

        TutorialGameState::createScene01();
    }
}
