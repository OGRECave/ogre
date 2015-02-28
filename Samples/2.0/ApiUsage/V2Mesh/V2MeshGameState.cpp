
#include "V2MeshGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"
#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreMesh2.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2Serializer.h"

#include "OgreCamera.h"
#include "OgreRenderWindow.h"

using namespace Demo;

namespace Demo
{
    V2MeshGameState::V2MeshGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription )
    {
    }
    //-----------------------------------------------------------------------------------
    void V2MeshGameState::createScene01(void)
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        Ogre::v1::MeshPtr v1Mesh;
        Ogre::MeshPtr v2Mesh;

        //---------------------------------------------------------------------------------------
        //Import Athene to v2 and render it without saving to disk.
        //---------------------------------------------------------------------------------------

        //Load the v1 mesh. Notice the v1 namespace
        //Also notice the HBU_STATIC flag; since the HBU_WRITE_ONLY
        //bit would prohibit us from reading the data for importing.
        v1Mesh = Ogre::v1::MeshManager::getSingleton().load(
                    "athene.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                    Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC );

        //Create a v2 mesh to import to, with a different name (arbitrary).
        v2Mesh = Ogre::MeshManager::getSingleton().createManual(
                    "athene.mesh Imported", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

        bool halfPosition   = true;
        bool halfUVs        = true;
        bool useQtangents   = true;

        //Import the v1 mesh to v2
        v2Mesh->importV1( v1Mesh.get(), halfPosition, halfUVs, useQtangents );

        //We don't need the v1 mesh. Free CPU memory, get it out of the GPU.
        //Leave it loaded if you want to use athene with v1 Entity.
        v1Mesh->unload();

        //Create an Item with the model we just imported.
        //Notice we use the name of the imported model. We could also use the overload
        //with the mesh pointer:
        //  item = sceneManager->createItem( v2Mesh, Ogre::SCENE_DYNAMIC );
        Ogre::Item *item = sceneManager->createItem( "athene.mesh Imported",
                                                     Ogre::ResourceGroupManager::
                                                     AUTODETECT_RESOURCE_GROUP_NAME,
                                                     Ogre::SCENE_DYNAMIC );
        Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                createChildSceneNode( Ogre::SCENE_DYNAMIC );
        sceneNode->attachObject( item );
        sceneNode->scale( 0.1f, 0.1f, 0.1f );
        //---------------------------------------------------------------------------------------
        //
        //---------------------------------------------------------------------------------------

        //---------------------------------------------------------------------------------------
        //Import Barrel to save it to disk.
        //---------------------------------------------------------------------------------------
        v1Mesh = Ogre::v1::MeshManager::getSingleton().load(
                    "Barrel.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                    Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC );
        //Create a v2 mesh to import to, with a different name.
        v2Mesh = Ogre::MeshManager::getSingleton().createManual(
                    "Barrel Imported", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

        v2Mesh->importV1( v1Mesh.get(), halfPosition, halfUVs, useQtangents );
        v1Mesh->unload();

        //Save the v2 mesh to disk (optional)
        //The VaoManager argument is only required when importing a mesh.
        //You can get it from the RenderSystem
        Ogre::MeshSerializer meshSerializer( 0 );
        meshSerializer.exportMesh( v2Mesh.get(), "BarrelExportedModel_inV2.mesh" );
        //---------------------------------------------------------------------------------------
        //
        //---------------------------------------------------------------------------------------

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
