
#include "Tutorial_ReconstructPosFromDepthGameState.h"
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

#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgrePass.h"

using namespace Demo;

namespace Demo
{
    Tutorial_ReconstructPosFromDepthGameState::Tutorial_ReconstructPosFromDepthGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription )
    {
    }
    //-----------------------------------------------------------------------------------
    void Tutorial_ReconstructPosFromDepthGameState::createScene01(void)
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        Ogre::v1::MeshPtr planeMeshV1 = Ogre::v1::MeshManager::getSingleton().createPlane( "Plane v1",
                                            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            Ogre::Plane( Ogre::Vector3::UNIT_Y, 1.0f ), 50.0f, 50.0f,
                                            1, 1, true, 1, 4.0f, 4.0f, Ogre::Vector3::UNIT_Z,
                                            Ogre::v1::HardwareBuffer::HBU_STATIC,
                                            Ogre::v1::HardwareBuffer::HBU_STATIC );

        Ogre::MeshPtr planeMesh = Ogre::MeshManager::getSingleton().createManual(
                    "Plane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

        planeMesh->importV1( planeMeshV1.get(), true, true, true );

        {
            Ogre::Item *item = sceneManager->createItem( planeMesh, Ogre::SCENE_DYNAMIC );
            Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                                                    createChildSceneNode( Ogre::SCENE_DYNAMIC );
            sceneNode->setPosition( 0, -1, 0 );
            sceneNode->attachObject( item );
        }

        float armsLength = 2.5f;

        for( int i=0; i<4; ++i )
        {
            for( int j=0; j<4; ++j )
            {
                Ogre::Item *item = sceneManager->createItem( "Cube_d.mesh",
                                                             Ogre::ResourceGroupManager::
                                                             AUTODETECT_RESOURCE_GROUP_NAME,
                                                             Ogre::SCENE_DYNAMIC );

                size_t idx = i * 4 + j;

                Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                        createChildSceneNode( Ogre::SCENE_DYNAMIC );

                sceneNode->setPosition( (i - 1.5f) * armsLength,
                                        2.0f,
                                        (j - 1.5f) * armsLength );
                sceneNode->setScale( 0.65f, 0.65f, 0.65f );

                sceneNode->roll( Ogre::Radian( (Ogre::Real)idx ) );

                sceneNode->attachObject( item );
            }
        }

        Ogre::SceneNode *rootNode = sceneManager->getRootSceneNode();

        Ogre::Light *light = sceneManager->createLight();
        Ogre::SceneNode *lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setPowerScale( 1.0f );
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        mCameraController = new CameraController( mGraphicsSystem, false );

        TutorialGameState::createScene01();

        //We need to set the parameters based on camera to the
        //shader so that the un-projection works as expected
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().load(
                    "ReconstructPosFromDepth",
                    Ogre::ResourceGroupManager::
                    AUTODETECT_RESOURCE_GROUP_NAME ).staticCast<Ogre::Material>();

        Ogre::Pass *pass = material->getTechnique(0)->getPass(0);
        Ogre::GpuProgramParametersSharedPtr psParams = pass->getFragmentProgramParameters();

        Ogre::Camera *camera = mGraphicsSystem->getCamera();
        Ogre::Real projectionA = camera->getFarClipDistance() /
                                    (camera->getFarClipDistance() - camera->getNearClipDistance());
        Ogre::Real projectionB = (-camera->getFarClipDistance() * camera->getNearClipDistance()) /
                                    (camera->getFarClipDistance() - camera->getNearClipDistance());
        //The division will keep "linearDepth" in the shader in the [0; 1] range.
        projectionB /= camera->getFarClipDistance();
        psParams->setNamedConstant( "projectionParams", Ogre::Vector4( projectionA, projectionB, 0, 0 ) );
    }
}
