
#include "ScreenSpaceReflectionsGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"

#include "OgreCamera.h"

#include "OgreHlmsPbsDatablock.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsTextureManager.h"
#include "OgreHlmsPbs.h"

#include "OgreLwString.h"

#include "../LocalCubemaps/LocalCubemapScene.h"

#include "OgreForward3D.h"

using namespace Demo;

namespace Demo
{
    ScreenSpaceReflectionsGameState::ScreenSpaceReflectionsGameState(
            const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription )
    {
        mDisplayHelpMode        = 2;
        mNumDisplayHelpModes    = 3;
    }
    //-----------------------------------------------------------------------------------
    void ScreenSpaceReflectionsGameState::createScene01(void)
    {
        //Setup a scene similar to that of PBS sample, except
        //we apply the cubemap to everything via C++ code
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        {
            Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
            assert( dynamic_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );
            Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );

            Ogre::HlmsBlendblock blendblock;
            Ogre::HlmsMacroblock macroblock;

            struct DemoMaterials
            {
                Ogre::String matName;
                Ogre::ColourValue colour;
            };

            DemoMaterials materials[4] =
            {
                { "Red", Ogre::ColourValue::Red },
                { "Green", Ogre::ColourValue::Green },
                { "Blue", Ogre::ColourValue::Blue },
                { "Cream", Ogre::ColourValue::White },
            };

            for( int i=0; i<4; ++i )
            {
                Ogre::String finalName = materials[i].matName;

                Ogre::HlmsPbsDatablock *datablock;
                datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                            hlmsPbs->createDatablock( finalName, finalName,
                                                      macroblock, blendblock,
                                                      Ogre::HlmsParamVec() ) );
                datablock->setBackgroundDiffuse( materials[i].colour );
                datablock->setFresnel( Ogre::Vector3( 0.1f ), false );
                datablock->setRoughness( 0.02 );
            }
        }

        generateScene( sceneManager );

        Ogre::SceneNode *rootNode = sceneManager->getRootSceneNode();

        Ogre::Light *light = sceneManager->createLight();
        Ogre::SceneNode *lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setPowerScale( 1.0f );
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        mCameraController = new CameraController( mGraphicsSystem, false );
        mCameraController->mCameraBaseSpeed = 1.0f;
        mCameraController->mCameraSpeedBoost = 10.0f;

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void ScreenSpaceReflectionsGameState::destroyScene(void)
    {
    }
    //-----------------------------------------------------------------------------------
    void ScreenSpaceReflectionsGameState::update( float timeSinceLast )
    {
        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void ScreenSpaceReflectionsGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );

        Ogre::Camera *camera = mGraphicsSystem->getCamera();
        outText += "\nCamera: ";
        outText += Ogre::StringConverter::toString( camera->getPosition().x ) + ", " +
                Ogre::StringConverter::toString( camera->getPosition().y ) + ", " +
                Ogre::StringConverter::toString( camera->getPosition().z );
    }
    //-----------------------------------------------------------------------------------
    void ScreenSpaceReflectionsGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS|KMOD_LSHIFT|KMOD_RSHIFT)) != 0 )
        {
            TutorialGameState::keyReleased( arg );
            return;
        }

        TutorialGameState::keyReleased( arg );
    }
}
