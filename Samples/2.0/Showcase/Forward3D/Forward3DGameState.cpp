
#include "Forward3DGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"
#include "OgreForward3D.h"

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
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorShadowNode.h"

#include "OgreHlmsPbs.h"
#include "OgreHlmsPbsDatablock.h"

#include "OgreOverlayManager.h"
#include "OgreOverlayContainer.h"
#include "OgreOverlay.h"

using namespace Demo;

namespace Demo
{
    struct Presets
    {
        Ogre::uint32 width;
        Ogre::uint32 height;
        Ogre::uint32 numSlices;
        Ogre::uint32 lightsPerCell;
        float minDistance;
        float maxDistance;
    };

    const Presets c_presets[] =
    {
        { 4, 4, 5, 96, 3.0f, 200.0f },
        { 4, 4, 4, 96, 3.0f, 100.0f },
        { 4, 4, 4, 64, 3.0f, 200.0f },
        { 4, 4, 4, 32, 3.0f, 200.0f },
        { 4, 4, 7, 64, 3.0f, 150.0f },
        { 4, 4, 3, 128, 3.0f, 200.0f },
    };

    Forward3DGameState::Forward3DGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mAnimateObjects( true ),
        mCurrentForward3DPreset( 0 ),
        mNumLights( 128 ),
        mLightRadius( 10.0f ),
        mLowThreshold( false )
    {
        mDisplayHelpMode        = 2;
        mNumDisplayHelpModes    = 3;
        memset( mSceneNode, 0, sizeof(mSceneNode) );
    }
    //-----------------------------------------------------------------------------------
    void Forward3DGameState::createScene01(void)
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        mGraphicsSystem->getCamera()->setPosition( Ogre::Vector3( 0, 30, 100 ) );

        sceneManager->setForwardClustered( true, 16, 8, 24, 96, 0, 5, 500 );

        Ogre::v1::MeshPtr planeMeshV1 = Ogre::v1::MeshManager::getSingleton().createPlane( "Plane v1",
                                            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            Ogre::Plane( Ogre::Vector3::UNIT_Y, 1.0f ), 5000.0f, 5000.0f,
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

        for( int i=0; i<64; ++i )
        {
            Ogre::Item *item = sceneManager->createItem( "Cube_d.mesh",
                                                         Ogre::ResourceGroupManager::
                                                         AUTODETECT_RESOURCE_GROUP_NAME,
                                                         Ogre::SCENE_STATIC );

            Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
                    createChildSceneNode( Ogre::SCENE_STATIC );

            Ogre::Vector3 vScale( Ogre::Math::RangeRandom( 0.5f, 10.0f ),
                                  Ogre::Math::RangeRandom( 5.0f, 15.0f ),
                                  Ogre::Math::RangeRandom( 0.5f, 10.0f ) );

            sceneNode->setPosition( Ogre::Math::RangeRandom( -250, 250 ),
                                    vScale.y * 0.5f - 1.0f,
                                    Ogre::Math::RangeRandom( -250, 250 ) );
            sceneNode->setScale( vScale );
            sceneNode->attachObject( item );
        }

        Ogre::SceneNode *rootNode = sceneManager->getRootSceneNode();

        Ogre::Light *light = sceneManager->createLight();
        Ogre::SceneNode *lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setPowerScale( 1.0f );
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        mLightNodes[0] = lightNode;

        sceneManager->setAmbientLight( Ogre::ColourValue( 0.3f, 0.5f, 0.7f ) * 0.1f * 0.75f,
                                       Ogre::ColourValue( 0.6f, 0.45f, 0.3f ) * 0.065f * 0.75f,
                                       -light->getDirection() + Ogre::Vector3::UNIT_Y * 0.2f );

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

        generateLights();

        mCameraController = new CameraController( mGraphicsSystem, false );

        //This sample is too taxing on pixel shaders. Use the fastest PCF filter.
        Ogre::Hlms *hlms = mGraphicsSystem->getRoot()->getHlmsManager()->getHlms( Ogre::HLMS_PBS );
        assert( dynamic_cast<Ogre::HlmsPbs*>( hlms ) );
        Ogre::HlmsPbs *pbs = static_cast<Ogre::HlmsPbs*>( hlms );
        pbs->setShadowSettings( Ogre::HlmsPbs::PCF_2x2 );

        //Change the roughness of the default datablock to something prettier.
        static_cast<Ogre::HlmsPbsDatablock*>( pbs->getDefaultDatablock() )->setRoughness( 0.1f );

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void Forward3DGameState::changeForward3DPreset( bool goForward )
    {
        const Ogre::uint32 numPresets = sizeof( c_presets ) / sizeof( c_presets[0] );

        if( goForward )
            mCurrentForward3DPreset = (mCurrentForward3DPreset + 1) % numPresets;
        else
            mCurrentForward3DPreset = (mCurrentForward3DPreset + numPresets - 1) % numPresets;

        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        Ogre::ForwardPlusBase *forwardPlus = sceneManager->getForwardPlus();
        const bool wasInDebugMode = forwardPlus->getDebugMode();

        const Presets &preset = c_presets[mCurrentForward3DPreset];
        sceneManager->setForward3D( true, preset.width, preset.height,
                                    preset.numSlices, preset.lightsPerCell,
                                    preset.minDistance, preset.maxDistance );

        forwardPlus = sceneManager->getForwardPlus();
        forwardPlus->setDebugMode( wasInDebugMode );
    }
    //-----------------------------------------------------------------------------------
    void Forward3DGameState::generateLights(void)
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        Ogre::LightArray::const_iterator itor = mGeneratedLights.begin();
        Ogre::LightArray::const_iterator end  = mGeneratedLights.end();

        while( itor != end )
        {
            Ogre::SceneNode *sceneNode = (*itor)->getParentSceneNode();
            sceneNode->getParentSceneNode()->removeAndDestroyChild( sceneNode );
            sceneManager->destroyLight( *itor );
            ++itor;
        }

        mGeneratedLights.clear();

        Ogre::SceneNode *rootNode = sceneManager->getRootSceneNode();

        //Deterministic randomness
        srand( 101 );

        for( Ogre::uint32 i=0; i<mNumLights; ++i )
        {
            Ogre::Light *light = sceneManager->createLight();
            Ogre::SceneNode *lightNode = rootNode->createChildSceneNode();
            lightNode->attachObject( light );

            if( i & 1 )
            {
                light->setDiffuseColour( 0.2f, 0.4f, 0.8f ); //Cold
                light->setSpecularColour( 0.2f, 0.4f, 0.8f );
            }
            else
            {
                light->setDiffuseColour( 0.8f, 0.4f, 0.2f ); //Warm
                light->setSpecularColour( 0.8f, 0.4f, 0.2f );
            }

            light->setPowerScale( Ogre::Math::PI );
            light->setCastShadows( false );
            light->setType( Ogre::Light::LT_POINT );
            lightNode->setPosition( Ogre::Math::RangeRandom( -250, 250 ),
                                    Ogre::Math::RangeRandom( 2.0f, 10.0f ),
                                    Ogre::Math::RangeRandom( -250, 250 ) );
            /*light->setDirection( Ogre::Vector3( -1, -1, 1 ).normalisedCopy() );
            light->setDirection( Ogre::Vector3( Ogre::Math::RangeRandom( -1, 1 ),
                                                Ogre::Math::RangeRandom( -1, -0.5 ),
                                                Ogre::Math::RangeRandom( -1, 1 ) ).normalisedCopy() );*/
            light->setAttenuationBasedOnRadius( mLightRadius, mLowThreshold ? 0.00192f : 0.0192f );

            mGeneratedLights.push_back( light );
        }
    }
    //-----------------------------------------------------------------------------------
    void Forward3DGameState::update( float timeSinceLast )
    {
        if( mAnimateObjects )
        {
            for( int i=0; i<16; ++i )
                mSceneNode[i]->yaw( Ogre::Radian(timeSinceLast * i * 0.125f) );
        }

        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void Forward3DGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );

        if( mDisplayHelpMode == 2 )
        {
            Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
            Ogre::ForwardPlusBase *forwardPlus = sceneManager->getForwardPlus();

            outText += "\nF2 to toggle animation. ";
            outText += mAnimateObjects ? "[On]" : "[Off]";
            outText += "\nF3 to use a low/high threshold for radius. ";
            outText += mLowThreshold ? "[Low]" : "[High]";
            outText += "\nF4 to use atten. range approximation. ";
            outText += forwardPlus->getFadeAttenuationRange() ? "[On]" : "[Off]";
            outText += "\nF5/F6 to increase/reduce number of lights. ";
            outText += "[" + Ogre::StringConverter::toString( mNumLights ) + "]";
            outText += "\nF7/F8 to increase/reduce light's radius. ";
            outText += "[" + Ogre::StringConverter::toString( mLightRadius ) + "]";
            outText += "\nF9 Debug Mode. ";
            outText += forwardPlus->getDebugMode() ? "[On]" : "[Off]";
            outText += "\nF10 Switch between Forward3D and Clustered Forward. ";
            outText += forwardPlus->getForwardPlusMethod() == Ogre::ForwardPlusBase::MethodForward3D ?
                        "[F3D]" : "[Clustered]";

            if( forwardPlus->getForwardPlusMethod() == Ogre::ForwardPlusBase::MethodForward3D )
            {
                outText += "\n[Shift+] SPACE to switch Forward3D settings back and forth. ";
                outText += "Preset: [";
                outText += Ogre::StringConverter::toString( mCurrentForward3DPreset ) + "]";

                const Presets &preset = c_presets[mCurrentForward3DPreset];
                outText += "\n\nWidth  " + Ogre::StringConverter::toString( preset.width );
                outText += "\nHeight " + Ogre::StringConverter::toString( preset.height );
                outText += "\nSlices " + Ogre::StringConverter::toString( preset.numSlices );
                outText += "\nLights p/ Cell " + Ogre::StringConverter::toString( preset.lightsPerCell );
                outText += "\nMin Distance   " + Ogre::StringConverter::toString( preset.minDistance );
                outText += "\nMax Distance  "  + Ogre::StringConverter::toString( preset.maxDistance );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void Forward3DGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        Ogre::ForwardPlusBase *forwardPlus = sceneManager->getForwardPlus();

        if( arg.keysym.sym == SDLK_SPACE &&
            forwardPlus->getForwardPlusMethod() == Ogre::ForwardPlusBase::MethodForward3D )
        {
            changeForward3DPreset( !(arg.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT)) );
        }
        else if( (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS)) != 0 )
        {
            TutorialGameState::keyReleased( arg );
            return;
        }

        if( arg.keysym.sym == SDLK_F2 )
        {
            mAnimateObjects = !mAnimateObjects;
        }
        else if( arg.keysym.sym == SDLK_F3 )
        {
            mLowThreshold = !mLowThreshold;
            generateLights();
        }
        else if( arg.keysym.sym == SDLK_F4 )
        {
            forwardPlus->setFadeAttenuationRange( !forwardPlus->getFadeAttenuationRange() );
        }
        else if( arg.keysym.sym == SDLK_F5 || arg.keysym.sym == SDLK_F6 )
        {
            if( arg.keysym.sym == SDLK_F5 )
                mNumLights = mNumLights >> 1;
            else
                mNumLights = mNumLights << 1;

            mNumLights = Ogre::Math::Clamp<Ogre::uint32>( mNumLights, 1, 4096 );
            generateLights();
        }
        else if( arg.keysym.sym == SDLK_F7 || arg.keysym.sym == SDLK_F8 )
        {
            if( arg.keysym.sym == SDLK_F7 )
                mLightRadius = mLightRadius - 1.0f;
            else
                mLightRadius = mLightRadius + 1.0f;

            mLightRadius = Ogre::Math::Clamp( mLightRadius, 0.5f, 100.0f );
            generateLights();
        }
        else if( arg.keysym.sym == SDLK_F9 )
        {
            forwardPlus->setDebugMode( !forwardPlus->getDebugMode() );
        }
        else if( arg.keysym.sym == SDLK_F10 )
        {
            const bool wasInDebugMode = forwardPlus->getDebugMode();

            if( forwardPlus->getForwardPlusMethod() == Ogre::ForwardPlusBase::MethodForwardClustered )
            {
                const Presets &preset = c_presets[mCurrentForward3DPreset];
                sceneManager->setForward3D( true, preset.width, preset.height,
                                            preset.numSlices, preset.lightsPerCell,
                                            preset.minDistance, preset.maxDistance );
            }
            else
            {
                sceneManager->setForwardClustered( true, 16, 8, 24, 96, 0, 5, 500 );
            }

            forwardPlus = sceneManager->getForwardPlus();
            forwardPlus->setDebugMode( wasInDebugMode );
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
