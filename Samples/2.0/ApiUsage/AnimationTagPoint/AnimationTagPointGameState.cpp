
#include "AnimationTagPointGameState.h"
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
    AnimationTagPointGameState::AnimationTagPointGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mWalkAnimation( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    void AnimationTagPointGameState::createScene01(void)
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        Ogre::v1::MeshPtr v1Mesh;
        Ogre::MeshPtr v2Mesh;

        //Load the v1 mesh. Notice the v1 namespace
        //Also notice the HBU_STATIC flag; since the HBU_WRITE_ONLY
        //bit would prohibit us from reading the data for importing.
        v1Mesh = Ogre::v1::MeshManager::getSingleton().load(
                    "Stickman.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                    Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC );

        v2Mesh = Ogre::MeshManager::getSingleton().createManual(
                    "Stickman.mesh", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

        bool halfPosition   = true;
        bool halfUVs        = true;
        bool useQtangents   = false;

        //Import the v1 mesh to v2
        v2Mesh->importV1( v1Mesh.get(), halfPosition, halfUVs, useQtangents );

        //We don't need the v1 mesh. Free CPU memory, get it out of the GPU.
        //Leave it loaded if you want to use athene with v1 Entity.
        v1Mesh->unload();

        Ogre::Item *stickmanItem = 0;
        {
            stickmanItem = sceneManager->createItem( "Stickman.mesh",
                                                     Ogre::ResourceGroupManager::
                                                     AUTODETECT_RESOURCE_GROUP_NAME,
                                                     Ogre::SCENE_DYNAMIC );
            Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                    createChildSceneNode( Ogre::SCENE_DYNAMIC );
            sceneNode->attachObject( stickmanItem );
        }

        for( int i=0; i<5; ++i )
        {
            Ogre::Item *item = sceneManager->createItem( "Sphere1000.mesh",
                                                         Ogre::ResourceGroupManager::
                                                         AUTODETECT_RESOURCE_GROUP_NAME,
                                                         Ogre::SCENE_DYNAMIC );

            Ogre::SkeletonInstance *skeletonInstance = stickmanItem->getSkeletonInstance();
            Ogre::Bone *bone = skeletonInstance->getBone( "Hand_L" );

            Ogre::TagPoint *tagPoint = sceneManager->createTagPoint();
            tagPoint->attachObject( item );

            const float angleFraction = i / 5.0f * Ogre::Math::TWO_PI;
            tagPoint->setPosition( sinf( angleFraction ) * 0.1f, 0.0f, cosf( angleFraction ) * 0.1f );
            tagPoint->scale( 0.1f, 0.1f, 0.1f );

            bone->addTagPoint( tagPoint );
            mSphereNodes[i] = tagPoint;
        }

        {
            Ogre::SkeletonInstance *skeletonInstance = stickmanItem->getSkeletonInstance();
            Ogre::Bone *bone = skeletonInstance->getBone( "Hand_R" );
            Ogre::TagPoint *tagPoint = sceneManager->createTagPoint();
            bone->addTagPoint( tagPoint );
            mCubesNode = tagPoint;
        }

        for( int i=0; i<5; ++i )
        {
            Ogre::Item *item = sceneManager->createItem( "Cube_d.mesh",
                                                         Ogre::ResourceGroupManager::
                                                         AUTODETECT_RESOURCE_GROUP_NAME,
                                                         Ogre::SCENE_DYNAMIC );
            Ogre::SceneNode *sceneNode = mCubesNode->createChildSceneNode();

            const float angleFraction = i / 5.0f * Ogre::Math::TWO_PI;
            sceneNode->setPosition( sinf( angleFraction ) * 0.1f, 0.0f, cosf( angleFraction ) * 0.1f );
            sceneNode->scale( 0.045f, 0.045f, 0.045f );

            sceneNode->attachObject( item );
            mCubeNodes[i] = sceneNode;
        }

        {
            Ogre::SkeletonInstance *skeletonInstance = stickmanItem->getSkeletonInstance();
            mWalkAnimation = skeletonInstance->getAnimation( "Idle" );
            mWalkAnimation->setEnabled( true );
        }

        Ogre::Light *light = sceneManager->createLight();
        Ogre::SceneNode *lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
        lightNode->attachObject( light );
        light->setPowerScale( Ogre::Math::PI ); //Since we don't do HDR, counter the PBS' division by PI
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        mCameraController = new CameraController( mGraphicsSystem, false );
        Ogre::Camera *camera = mGraphicsSystem->getCamera();
        camera->setPosition( Ogre::Vector3( 0, 2.5, 4 ) );

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void AnimationTagPointGameState::update( float timeSinceLast )
    {
        for( int i=0; i<5; ++i )
        {
            const float angleFraction = (i + mGraphicsSystem->getAccumTimeSinceLastLogicFrame()) /
                    4.0f * Ogre::Math::TWO_PI;
            mSphereNodes[i]->setPosition( sinf( angleFraction ) * 0.1f,
                                          0.0f,
                                          cosf( angleFraction ) * 0.1f );

            mCubeNodes[i]->roll( Ogre::Radian( timeSinceLast * 1.51f ) );
        }

        mCubesNode->yaw( Ogre::Radian( timeSinceLast ) );

        mWalkAnimation->addTime( timeSinceLast );

        TutorialGameState::update( timeSinceLast );
    }
}
