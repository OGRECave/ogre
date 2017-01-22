
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

#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"

#include "OgreTextureManager.h"

#include "OgreForward3D.h"

using namespace Demo;

namespace Demo
{
    ScreenSpaceReflectionsGameState::ScreenSpaceReflectionsGameState(
            const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mScreenSpaceReflections( 0 )
    {
        mDisplayHelpMode        = 2;
        mNumDisplayHelpModes    = 3;
    }
    //-----------------------------------------------------------------------------------
    void ScreenSpaceReflectionsGameState::createScene01(void)
    {
//        Ogre::TexturePtr globalCubemap = Ogre::TextureManager::getSingleton().load(
//                    "SaintPetersBasilica.dds", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
//                    Ogre::TEX_TYPE_CUBE_MAP );
        //mScreenSpaceReflections = new ScreenSpaceReflections( globalCubemap );
        mScreenSpaceReflections = new ScreenSpaceReflections( Ogre::TexturePtr() );

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

        sceneManager->setAmbientLight( Ogre::ColourValue( 0.2, 0.4, 0.8 ) * 0.2,
                                       Ogre::ColourValue( 0.6, 0.5, 0.4 ) * 0.2,
                                       Ogre::Vector3::UNIT_Y );

        mCameraController = new CameraController( mGraphicsSystem, false );
        mCameraController->mCameraBaseSpeed = 1.0f;
        mCameraController->mCameraSpeedBoost = 10.0f;

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void ScreenSpaceReflectionsGameState::destroyScene(void)
    {
        delete mScreenSpaceReflections;
        mScreenSpaceReflections = 0;
    }
    //-----------------------------------------------------------------------------------
    void ScreenSpaceReflectionsGameState::update( float timeSinceLast )
    {
        mScreenSpaceReflections->update( mGraphicsSystem->getCamera() );
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

        static float tmpValue = 1;

        if( arg.keysym.sym == SDLK_SPACE )
        {
            Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().load(
                        "SSR/ScreenSpaceReflectionsVectors",
                        Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ).
                    staticCast<Ogre::Material>();

            Ogre::Pass *pass = material->getTechnique(0)->getPass(0);

            Ogre::GpuProgramParametersSharedPtr psParams = pass->getFragmentProgramParameters();

            if( arg.keysym.mod & KMOD_LSHIFT )
            {
                tmpValue -= 20;
                tmpValue = Ogre::max( tmpValue, 0 );
            }
            else
                tmpValue += 20;
            psParams->setNamedConstant( "p_maxSteps", tmpValue );
        }

        TutorialGameState::keyReleased( arg );
    }
}
