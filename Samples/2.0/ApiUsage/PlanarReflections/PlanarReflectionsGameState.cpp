
#include "PlanarReflectionsGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"

#include "OgreCamera.h"
#include "OgreRenderWindow.h"

#include "OgreHlmsUnlitDatablock.h"
#include "OgreHlmsSamplerblock.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlms.h"
#include "OgreHlmsPbs.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorShadowNode.h"

#include "OgreOverlayManager.h"
#include "OgreOverlayContainer.h"
#include "OgreOverlay.h"

#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspaceListener.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassScene.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h"

#include "OgreHlmsCompute.h"
#include "OgreHlmsComputeJob.h"
#include "Utils/MiscUtils.h"

#include "OgrePlanarReflections.h"

using namespace Demo;

namespace Demo
{
    class PlanarReflectionsWorkspaceListener : public Ogre::CompositorWorkspaceListener
    {
        Ogre::PlanarReflections *mPlanarReflections;

    public:
        PlanarReflectionsWorkspaceListener( Ogre::PlanarReflections *planarReflections ) :
            mPlanarReflections( planarReflections ) {}
        virtual ~PlanarReflectionsWorkspaceListener() {}

        virtual void workspacePreUpdate( Ogre::CompositorWorkspace *workspace )
        {
            mPlanarReflections->beginFrame();
        }

        virtual void passEarlyPreExecute( Ogre::CompositorPass *pass )
        {
            //Ignore non-scene passes
            if( pass->getType() != Ogre::PASS_SCENE )
                return;
            assert( dynamic_cast<const Ogre::CompositorPassSceneDef*>( pass->getDefinition() ) );
            const Ogre::CompositorPassSceneDef *passDef =
                    static_cast<const Ogre::CompositorPassSceneDef*>( pass->getDefinition() );

            //Ignore scene passes that belong to a shadow node.
            if( passDef->mShadowNodeRecalculation == Ogre::SHADOW_NODE_CASTER_PASS )
                return;

            //Ignore scene passes we haven't specifically tagged to receive reflections
            if( passDef->mIdentifier != 25001 )
                return;

            Ogre::CompositorPassScene *passScene = static_cast<Ogre::CompositorPassScene*>( pass );
            Ogre::Camera *camera = passScene->getCamera();

            //Note: The Aspect Ratio must match that of the camera we're reflecting.
            mPlanarReflections->update( camera, camera->getAspectRatio() );
        }
    };

    PlanarReflectionsGameState::PlanarReflectionsGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mAnimateObjects( true )
    {
        memset( mSceneNode, 0, sizeof(mSceneNode) );
    }
    //-----------------------------------------------------------------------------------
    void PlanarReflectionsGameState::createReflectiveSurfaces(void)
    {
        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        bool useComputeMipmaps = false;
#if !OGRE_NO_JSON
        useComputeMipmaps =
                root->getRenderSystem()->getCapabilities()->hasCapability( Ogre::RSC_COMPUTE_PROGRAM );
#endif

        //Setup PlanarReflections
        mPlanarReflections = new Ogre::PlanarReflections( sceneManager, root->getCompositorManager2(),
                                                          1.0, 0 );
        mWorkspaceListener = new PlanarReflectionsWorkspaceListener( mPlanarReflections );
        {
            Ogre::CompositorWorkspace *workspace = mGraphicsSystem->getCompositorWorkspace();
            workspace->setListener( mWorkspaceListener );
        }

        //The perfect mirror doesn't need mipmaps.
        mPlanarReflections->setMaxActiveActors( 1u, "PlanarReflectionsReflectiveWorkspace",
                                                true, 512, 512, false,
                                                Ogre::PF_R8G8B8A8, useComputeMipmaps );
        //The rest of the reflections do.
        mPlanarReflections->setMaxActiveActors( 2u, "PlanarReflectionsReflectiveWorkspace",
                                                true, 512, 512, true,
                                                Ogre::PF_R8G8B8A8, useComputeMipmaps );
        const Ogre::Vector2 mirrorSize( 10.0f, 10.0f );

        //Create the plane mesh
        //Note that we create the plane to look towards +Z; so that sceneNode->getOrientation
        //matches the orientation for the PlanarReflectionActor
        Ogre::v1::MeshPtr planeMeshV1 = Ogre::v1::MeshManager::getSingleton().createPlane(
                    "Plane Mirror Unlit",
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                    Ogre::Plane( Ogre::Vector3::UNIT_Z, 0.0f ),
                    mirrorSize.x, mirrorSize.y,
                    1, 1, true, 1, 1.0f, 1.0f, Ogre::Vector3::UNIT_Y,
                    Ogre::v1::HardwareBuffer::HBU_STATIC,
                    Ogre::v1::HardwareBuffer::HBU_STATIC );
        Ogre::MeshPtr planeMesh = Ogre::MeshManager::getSingleton().createManual(
                    "Plane Mirror Unlit", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

        planeMesh->importV1( planeMeshV1.get(), true, true, true );

        //---------------------------------------------------------------------
        //Setup mirror for Unlit.
        //---------------------------------------------------------------------
        Ogre::Item *item = sceneManager->createItem( planeMesh, Ogre::SCENE_DYNAMIC );
        Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                createChildSceneNode( Ogre::SCENE_DYNAMIC );
        sceneNode->setPosition( 5, 5, 0 );
        sceneNode->setOrientation( Ogre::Quaternion( Ogre::Radian( -Ogre::Math::HALF_PI ),
                                                     Ogre::Vector3::UNIT_Y ) );
        sceneNode->attachObject( item );
        //item->setCastShadows( false );
        item->setVisibilityFlags( 1u ); // Do not render this plane during the reflection phase.

        Ogre::PlanarReflectionActor *actor =
                mPlanarReflections->addActor( Ogre::PlanarReflectionActor(
                                                 sceneNode->getPosition(), mirrorSize,
                                                 sceneNode->getOrientation() ) );

        Ogre::Hlms *hlmsUnlit = root->getHlmsManager()->getHlms( Ogre::HLMS_UNLIT );

        Ogre::HlmsMacroblock macroblock;
        Ogre::HlmsBlendblock blendblock;
        Ogre::String datablockName( "Mirror_Unlit" );
        Ogre::HlmsUnlitDatablock *mirror = static_cast<Ogre::HlmsUnlitDatablock*>(
                    hlmsUnlit->createDatablock( datablockName, datablockName,
                                                macroblock, blendblock, Ogre::HlmsParamVec() ) );
        mPlanarReflections->reserve( 0, actor );
        //Make sure it's always activated (i.e. always win against other actors)
        //unless it's not visible by the camera.
        actor->mActivationPriority = 0;
        mirror->setTexture( 0, 0, mPlanarReflections->getTexture( 0 ) );
        mirror->setEnablePlanarReflection( 0, true );
        item->setDatablock( mirror );

        //---------------------------------------------------------------------
        //Setup mirror for PBS.
        //---------------------------------------------------------------------
        Ogre::Hlms *hlms = root->getHlmsManager()->getHlms( Ogre::HLMS_PBS );
        assert( dynamic_cast<Ogre::HlmsPbs*>( hlms ) );
        Ogre::HlmsPbs *pbs = static_cast<Ogre::HlmsPbs*>( hlms );
        pbs->setPlanarReflections( mPlanarReflections );

        item = sceneManager->createItem( planeMesh, Ogre::SCENE_DYNAMIC );
        item->setDatablock( "GlassRoughness" );
        sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                createChildSceneNode( Ogre::SCENE_DYNAMIC );
        sceneNode->setPosition( -5, 2.5f, 0 );
        sceneNode->setOrientation( Ogre::Quaternion( Ogre::Radian( Ogre::Math::HALF_PI ),
                                                     Ogre::Vector3::UNIT_Y ) );
        sceneNode->setScale( Ogre::Vector3( 0.75f, 0.5f, 1.0f ) );
        sceneNode->attachObject( item );

        actor = mPlanarReflections->addActor( Ogre::PlanarReflectionActor(
                                                  sceneNode->getPosition(),
                                                  mirrorSize * Ogre::Vector2( 0.75f, 0.5f ),
                                                  sceneNode->getOrientation() ) );

        Ogre::PlanarReflections::TrackedRenderable trackedRenderable(
                    item->getSubItem(0), item,
                    Ogre::Vector3::UNIT_Z, Ogre::Vector3( 0, 0, 0 ) );
        mPlanarReflections->addRenderable( trackedRenderable );
    }
    //-----------------------------------------------------------------------------------
    void PlanarReflectionsGameState::createScene01(void)
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

        Ogre::Item *mainPlane = 0;

        {
            Ogre::Item *item = sceneManager->createItem( planeMesh, Ogre::SCENE_DYNAMIC );
            Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                                                    createChildSceneNode( Ogre::SCENE_DYNAMIC );
            sceneNode->setPosition( 0, -1, 0 );
            sceneNode->attachObject( item );

            mainPlane = item;
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

                mSceneNode[idx] = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                        createChildSceneNode( Ogre::SCENE_DYNAMIC );

                mSceneNode[idx]->setPosition( (i - 1.5f) * armsLength,
                                              2.0f,
                                              (j - 1.5f) * armsLength );
                mSceneNode[idx]->setScale( 0.65f, 0.65f, 0.65f );

                mSceneNode[idx]->roll( Ogre::Radian( (Ogre::Real)idx ) );

                mSceneNode[idx]->attachObject( item );
            }
        }

        Ogre::SceneNode *rootNode = sceneManager->getRootSceneNode();

        Ogre::Light *light = sceneManager->createLight();
        Ogre::SceneNode *lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setPowerScale( 1.0f );
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        mLightNodes[0] = lightNode;

        light = sceneManager->createLight();
        lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setDiffuseColour( 0.8f, 0.4f, 0.2f ); //Warm
        light->setSpecularColour( 0.8f, 0.4f, 0.2f );
        light->setPowerScale( Ogre::Math::PI );
        light->setType( Ogre::Light::LT_SPOTLIGHT );
        lightNode->setPosition( -10.0f, 10.0f, 10.0f );
        light->setDirection( Ogre::Vector3( 1, -1, -1 ).normalisedCopy() );
        light->setAttenuationBasedOnRadius( 10.0f, 0.01f );

        mLightNodes[1] = lightNode;

        light = sceneManager->createLight();
        lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setDiffuseColour( 0.2f, 0.4f, 0.8f ); //Cold
        light->setSpecularColour( 0.2f, 0.4f, 0.8f );
        light->setPowerScale( Ogre::Math::PI );
        light->setType( Ogre::Light::LT_SPOTLIGHT );
        lightNode->setPosition( 10.0f, 10.0f, -10.0f );
        light->setDirection( Ogre::Vector3( -1, -1, 1 ).normalisedCopy() );
        light->setAttenuationBasedOnRadius( 10.0f, 0.01f );

        mLightNodes[2] = lightNode;

        mCameraController = new CameraController( mGraphicsSystem, false );

        createReflectiveSurfaces();

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void PlanarReflectionsGameState::destroyScene(void)
    {
        Ogre::CompositorWorkspace *workspace = mGraphicsSystem->getCompositorWorkspace();
        workspace->setListener( (Ogre::CompositorWorkspaceListener*)0 );
        delete mWorkspaceListener;
        mWorkspaceListener = 0;
        delete mPlanarReflections;
        mPlanarReflections = 0;
    }
    //-----------------------------------------------------------------------------------
    void PlanarReflectionsGameState::update( float timeSinceLast )
    {
        if( mAnimateObjects )
        {
            for( int i=0; i<16; ++i )
                mSceneNode[i]->yaw( Ogre::Radian(timeSinceLast * i * 0.125f) );
        }

        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void PlanarReflectionsGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );
        outText += "\nPress F2 to toggle animation. ";
        outText += mAnimateObjects ? "[On]" : "[Off]";
    }
    //-----------------------------------------------------------------------------------
    void PlanarReflectionsGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS)) != 0 )
        {
            TutorialGameState::keyReleased( arg );
            return;
        }

        if( arg.keysym.sym == SDLK_F2 )
        {
            mAnimateObjects = !mAnimateObjects;
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
