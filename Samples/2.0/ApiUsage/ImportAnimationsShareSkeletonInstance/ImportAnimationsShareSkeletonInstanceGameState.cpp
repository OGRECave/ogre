
#include "ImportAnimationsShareSkeletonInstanceGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"
#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreMesh2.h"
#include "OgreMeshManager2.h"

#include "Animation/OgreSkeletonInstance.h"
#include "Animation/OgreTagPoint.h"
#include "Animation/OgreSkeletonAnimation.h"

#include "OgreCamera.h"
#include "OgreRenderWindow.h"

using namespace Demo;

namespace Demo
{
    ImportAnimationsShareSkeletonInstanceGameState::ImportAnimationsShareSkeletonInstanceGameState(const Ogre::String &helpDescription) :
        TutorialGameState(helpDescription),
        mAnyAnimation(0)
    {
    }
    //-----------------------------------------------------------------------------------
    void ImportAnimationsShareSkeletonInstanceGameState::createScene01(void)
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();        

        Ogre::v1::MeshPtr v1Mesh;
        Ogre::MeshPtr v2Mesh;

        bool halfPosition = true;
        bool halfUVs = true;
        bool useQtangents = false;

        {   // Prepare char_reference mesh
            v1Mesh = Ogre::v1::MeshManager::getSingleton().load(
                "char_reference.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC);
            v2Mesh = Ogre::MeshManager::getSingleton().createManual(
                "char_reference.mesh", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            v2Mesh->importV1(v1Mesh.get(), halfPosition, halfUVs, useQtangents);
            v1Mesh->unload();
        }

        size_t numParts = 5;
        const Ogre::String parts[] =
        {
            "char_feet.mesh", "char_hands.mesh", "char_head.mesh", "char_legs.mesh", "char_torso.mesh"
        };

        // Prepare parts
        for( size_t i = 0; i< numParts; ++i )
        {
            v1Mesh = Ogre::v1::MeshManager::getSingleton().load(
                        parts[i], Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                        Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC );
            v2Mesh = Ogre::MeshManager::getSingleton().createManual(
                        parts[i], Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
            v2Mesh->importV1( v1Mesh.get(), halfPosition, halfUVs, useQtangents );
            v1Mesh->unload();
        }

        Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                createChildSceneNode( Ogre::SCENE_DYNAMIC );

        //Create the master.
        Ogre::Item *charItem = sceneManager->createItem( "char_reference.mesh",
                                                         Ogre::ResourceGroupManager::
                                                         AUTODETECT_RESOURCE_GROUP_NAME,
                                                         Ogre::SCENE_DYNAMIC );
        sceneNode->attachObject( charItem );

        for( size_t i = 0; i<numParts; ++i )
        {
            //Create the slaves, and make them use the skeleton from the master.
            Ogre::Item* charPart = sceneManager->createItem( parts[i],
                                                             Ogre::ResourceGroupManager::
                                                             AUTODETECT_RESOURCE_GROUP_NAME,
                                                             Ogre::SCENE_DYNAMIC );
            sceneNode->attachObject( charPart );
            charPart->useSkeletonInstanceFrom( charItem );
        }

        {
            //Import animation from char_mining.skeleton
            Ogre::SkeletonInstance *skeletonInstance = charItem->getSkeletonInstance();
            skeletonInstance->addAnimationsFromSkeleton( "char_mining.skeleton",
                                                         Ogre::ResourceGroupManager::
                                                         AUTODETECT_RESOURCE_GROUP_NAME );
            mAnyAnimation = skeletonInstance->getAnimation( "char_mining" );
            mAnyAnimation->setEnabled( true );
        }

        Ogre::Light *light = sceneManager->createLight();
        Ogre::SceneNode *lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
        lightNode->attachObject( light );
        light->setPowerScale( Ogre::Math::PI ); //Since we don't do HDR, counter the PBS' division by PI
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        light->setDirection( Ogre::Vector3(-1, -1, -1).normalisedCopy() );

        mCameraController = new CameraController( mGraphicsSystem, false );
        Ogre::Camera *camera = mGraphicsSystem->getCamera();
        camera->setPosition( Ogre::Vector3( 0, 2.5, 4 ) );

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void ImportAnimationsShareSkeletonInstanceGameState::update( float timeSinceLast )
    {        
        mAnyAnimation->addTime( timeSinceLast );

        TutorialGameState::update( timeSinceLast );
    }
}
