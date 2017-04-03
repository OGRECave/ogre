
#include "HdrGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"

#include "OgreCamera.h"
#include "OgreRenderWindow.h"

#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsSamplerblock.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsTextureManager.h"
#include "OgreHlmsPbs.h"

#include "Utils/HdrUtils.h"

using namespace Demo;

namespace Demo
{
    HdrGameState::HdrGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mAnimateObjects( true ),
        mCurrentPreset( -1 ),
        mExposure( 0.0f ),
        mMinAutoExposure( -2.5f ),
        mMaxAutoExposure( 2.5f ),
        mBloomFullThreshold( 5.0f )
    {
        mDisplayHelpMode = 2;
        mNumDisplayHelpModes = 3;
        memset( mSceneNode, 0, sizeof(mSceneNode) );
    }
    //-----------------------------------------------------------------------------------
    void HdrGameState::createScene01(void)
    {
        HdrUtils::init( (Ogre::uint8)mGraphicsSystem->getRenderWindow()->getFSAA() );

        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        const float armsLength = 2.5f;

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
            item->setDatablock( "Marble" );
            Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                                                    createChildSceneNode( Ogre::SCENE_DYNAMIC );
            sceneNode->setPosition( 0, -1, 0 );
            sceneNode->attachObject( item );

            //Change the addressing mode of the roughness map to wrap via code.
            //Detail maps default to wrap, but the rest to clamp.
            assert( dynamic_cast<Ogre::HlmsPbsDatablock*>( item->getSubItem(0)->getDatablock() ) );
            Ogre::HlmsPbsDatablock *datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                                                            item->getSubItem(0)->getDatablock() );
            //Make a hard copy of the sampler block
            Ogre::HlmsSamplerblock samplerblock( *datablock->getSamplerblock( Ogre::PBSM_ROUGHNESS ) );
            samplerblock.mU = Ogre::TAM_WRAP;
            samplerblock.mV = Ogre::TAM_WRAP;
            samplerblock.mW = Ogre::TAM_WRAP;
            //Set the new samplerblock. The Hlms system will
            //automatically create the API block if necessary
            datablock->setSamplerblock( Ogre::PBSM_ROUGHNESS, samplerblock );
        }

        for( int i=0; i<4; ++i )
        {
            for( int j=0; j<4; ++j )
            {
                Ogre::String meshName;

                if( i == j )
                    meshName = "Sphere1000.mesh";
                else
                    meshName = "Cube_d.mesh";

                Ogre::Item *item = sceneManager->createItem( meshName,
                                                             Ogre::ResourceGroupManager::
                                                             AUTODETECT_RESOURCE_GROUP_NAME,
                                                             Ogre::SCENE_DYNAMIC );
                if( i % 2 == 0 )
                    item->setDatablock( "Rocks" );
                else
                    item->setDatablock( "Marble" );

                item->setVisibilityFlags( 0x000000001 );

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

        {
            size_t numItems = 0;
            Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
            Ogre::HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();

            assert( dynamic_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );

            Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );

            const int numX = 8;
            const int numZ = 8;

            const float armsLength = 1.0f;
            const float startX = (numX-1) / 2.0f;
            const float startZ = (numZ-1) / 2.0f;

            for( int x=0; x<numX; ++x )
            {
                for( int z=0; z<numZ; ++z )
                {
                    Ogre::String datablockName = "Test" + Ogre::StringConverter::toString( numItems++ );
                    Ogre::HlmsPbsDatablock *datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                                hlmsPbs->createDatablock( datablockName,
                                                          datablockName,
                                                          Ogre::HlmsMacroblock(),
                                                          Ogre::HlmsBlendblock(),
                                                          Ogre::HlmsParamVec() ) );

                    Ogre::HlmsTextureManager::TextureLocation texLocation = hlmsTextureManager->
                            createOrRetrieveTexture( "SaintPetersBasilica.dds",
                                                     Ogre::HlmsTextureManager::TEXTURE_TYPE_ENV_MAP );

                    datablock->setTexture( Ogre::PBSM_REFLECTION, texLocation.xIdx, texLocation.texture );
                    datablock->setDiffuse( Ogre::Vector3( 0.0f, 1.0f, 0.0f ) );

                    datablock->setRoughness( std::max( 0.02f, x / Ogre::max( 1, (float)(numX-1) ) ) );
                    datablock->setFresnel( Ogre::Vector3( z / Ogre::max( 1, (float)(numZ-1) ) ), false );

                    Ogre::Item *item = sceneManager->createItem( "Sphere1000.mesh",
                                                                 Ogre::ResourceGroupManager::
                                                                 AUTODETECT_RESOURCE_GROUP_NAME,
                                                                 Ogre::SCENE_DYNAMIC );
                    item->setDatablock( datablock );
                    item->setVisibilityFlags( 0x000000002 );

                    Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                            createChildSceneNode( Ogre::SCENE_DYNAMIC );
                    sceneNode->setPosition( Ogre::Vector3( armsLength * x - startX,
                                                           1.0f,
                                                           armsLength * z - startZ ) );
                    sceneNode->attachObject( item );
                }
            }
        }

        Ogre::SceneNode *rootNode = sceneManager->getRootSceneNode();

        Ogre::Light *light = sceneManager->createLight();
        Ogre::SceneNode *lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setPowerScale( 97.0f );
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        mLightNodes[0] = lightNode;

        sceneManager->setAmbientLight( Ogre::ColourValue( 0.3f, 0.5f, 0.7f ) * 0.1f * 0.75f * 60.0f,
                                       Ogre::ColourValue( 0.6f, 0.45f, 0.3f ) * 0.065f * 0.75f * 60.0f,
                                       -light->getDirection() + Ogre::Vector3::UNIT_Y * 0.2f );

        light = sceneManager->createLight();
        lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setDiffuseColour( 0.8f, 0.4f, 0.2f ); //Warm
        light->setSpecularColour( 0.8f, 0.4f, 0.2f );
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
        light->setType( Ogre::Light::LT_SPOTLIGHT );
        lightNode->setPosition( 10.0f, 10.0f, -10.0f );
        light->setDirection( Ogre::Vector3( -1, -1, 1 ).normalisedCopy() );
        light->setAttenuationBasedOnRadius( 10.0f, 0.01f );

        mLightNodes[2] = lightNode;

        mCameraController = new CameraController( mGraphicsSystem, false );

        switchPreset();

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void HdrGameState::update( float timeSinceLast )
    {
        if( mAnimateObjects )
        {
            for( int i=0; i<16; ++i )
                mSceneNode[i]->yaw( Ogre::Radian(timeSinceLast * i * 0.125f) );
        }

        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void HdrGameState::switchPreset( int direction )
    {
        struct Preset
        {
            const char *name;
            Ogre::ColourValue skyColour;
            Ogre::ColourValue ambUpperHemisphere;
            Ogre::ColourValue ambLowerHemisphere;
            float lightPower[3];
            float exposure;
            float minAutoExposure;
            float maxAutoExposure;
            float bloomThreshold;
            float envmapScale;
        };

        //Our HDR is calibrated to multiply by 1024 (-10 stops)
        //(this is due to limited range in 16-bit float RenderTargets).
        //Direct sunlight on a perpendicular surface is ~100.000 lumens
        //100.000 / 1024 = ~97.0f
        //
        //  1 lux = 1 lumen per square meter.
        //
        // According to http://lumennow.org/lumens-vs-watts/
        //  40-watt incandescent bulb = 450 lumens
        //  60-watt incandescent bulb = 800 lumens
        //  100-watt incandescent bulb = 1600 lumens
        //
        // According to http://www.greenbusinesslight.com/page/119/lux-lumens-and-watts
        //  Outdoor average sunlight ranges from 32 000 to 100 000 lux
        //  Warehouse aisles are lit to approx 100-200 lux
        //  A bright office requires about 400 lux of illumination
        //  At sunset and sunrise (with a clear sky), ambient outdoor light is also about 400 lux
        //  Building corridors can be lit adequately at around 100 lux
        //  Moonlight represents about 1 lux
        //
        //  A 400W Metal Halide lamp - used for high bay lighting in warehouses: 95 lumens/watt
        //  A 100W Incandescent bulb - used for general task lighting applications: 17 lumens/watt
        //  A 32W T5 or T8 Fluorescent tube - used for general office ceiling lighting - 50 lumens/watt
        //  A 150W High pressure sodium bulb - used for street and area - lighting 80 lumens/watt
        //
        // How we calculated the presets?
        //  Starting by kwnowing the sun's power in a bright day, we use the EV values from:
        //      https://en.wikipedia.org/wiki/Exposure_value#Tabulated_exposure_values
        //  We now know that at a bright day is between EV 14-16 A heavy overcast scene is EV 12.
        //  This means we take the 97.000 lumens from the sun and divide it by
        //  2^(16-12) = 2^4 = 16 => 97.000 / 16 = 6062.5 lumens.
        //  Now we know the sun in an overcast day is around 6062.5 lumens.
        //  6062.5 lumens / 1024 = 5.92. Note that the sample uses 6.0625, which is a little more.
        //  And we then apply the same process to all the parameters.
        //  Not all the parameters were scaled exactly the same because we're not
        //  really simulating Global Illumination but rather faking it.
        //  We hand-tweak a few adjustments based on obvervation. Furthermore note that
        //  "A bright day" is between EV 14-16; not a perfect EV 16. You may divide some parameters by
        //  16, some by 8 and some by 32... or 10 (not power of 2). This isn't an exact science, but it
        //  is very accurate. As long as you don't stray too much from the reference you will probably
        //  get very convincing results.
        //
        //  Exposure values were adjusted by hand based on observation.
        const Preset c_presets[] =
        {
            {
                "Bright, sunny day",
                Ogre::ColourValue( 0.2f, 0.4f, 0.6f )  * 60.0f, //Sky
                Ogre::ColourValue( 0.3f, 0.50f, 0.7f ) * 4.5f,
                Ogre::ColourValue( 0.6f, 0.45f, 0.3f ) * 2.925f,
                97.0f,              //Sun power
                1.5f, 1.5f,         //Lights
                0.0f, -1.0f, 2.5f,  //Exposure
                5.0f,               //Bloom
                16.0f               //Env. map scale
            },
            {
                "Average, slightly hazy day",
                Ogre::ColourValue( 0.2f, 0.4f, 0.6f ) * 32.0f, //Sky
                Ogre::ColourValue( 0.3f, 0.50f, 0.7f ) * 3.15f,
                Ogre::ColourValue( 0.6f, 0.45f, 0.3f ) * 2.0475f,
                48.0f, //Sun power
                1.5f, 1.5f,         //Lights
                0.0f, -2.0f, 2.5f,  //Exposure
                5.0f,               //Bloom
                8.0f                //Env. map scale
            },
            {
                "Heavy overcast day",
                Ogre::ColourValue( 0.4f, 0.4f, 0.4f ) * 4.5f, //Sky
                Ogre::ColourValue( 0.5f, 0.5f, 0.5f ) * 0.4f,
                Ogre::ColourValue( 0.5f, 0.5f, 0.5f ) * 0.365625f,
                6.0625f,            //Sun power
                1.5f, 1.5f,         //Lights
                0.0f, -2.5f, 1.0f,  //Exposure
                5.0f,               //Bloom
                0.5f                //Env. map scale
            },
            {
                "Gibbous moon night",
                Ogre::ColourValue( 0.27f, 0.3f, 0.6f ) * 0.01831072f, //Sky
                Ogre::ColourValue( 0.5f, 0.5f, 0.50f ) * 0.003f,
                Ogre::ColourValue( 0.4f, 0.5f, 0.65f ) * 0.00274222f,
                0.0009251f,         //Sun power
                1.5f, 1.5f,         //Lights
                0.65f, -2.5f, 3.0f, //Exposure
                5.0f,               //Bloom
                0.0152587890625f    //Env. map scale
            },
            {
                "Gibbous moon night w/ powerful spotlights",
                Ogre::ColourValue( 0.27f, 0.3f, 0.6f ) * 0.01831072f, //Sky
                Ogre::ColourValue( 0.5f, 0.5f, 0.50f ) * 0.003f,
                Ogre::ColourValue( 0.4f, 0.5f, 0.65f ) * 0.00274222f,
                0.0009251f,         //Sun power
                6.5f, 6.5f,         //Lights
                0.65f, -2.5f, 3.0f, //Exposure
                5.0f,               //Bloom
                0.0152587890625f    //Env. map scale
            },
            {
                "JJ Abrams style",
                Ogre::ColourValue( 0.2f, 0.4f, 0.6f )  * 6.0f, //Sky
                Ogre::ColourValue( 0.3f, 0.50f, 0.7f ) * 0.1125f,
                Ogre::ColourValue( 0.6f, 0.45f, 0.3f ) * 0.073125f,
                4.0f,               //Sun power
                17.05f, 17.05f,     //Lights
                0.5f, 1.0f, 2.5f,   //Exposure
                3.0f,               //Bloom
                1.0f,               //Env. map scale
            },
        };

        {
            const Ogre::uint32 numPresets = sizeof(c_presets) / sizeof(c_presets[0]);

            if( direction >= 0 )
                mCurrentPreset = (mCurrentPreset + 1) % numPresets;
            else
                mCurrentPreset = (mCurrentPreset + numPresets - 1) % numPresets;
        }

        const Preset &preset = c_presets[mCurrentPreset];

        mPresetName         = preset.name;
        mExposure           = preset.exposure;
        mMinAutoExposure    = preset.minAutoExposure;
        mMaxAutoExposure    = preset.maxAutoExposure;
        mBloomFullThreshold = preset.bloomThreshold;

        HdrUtils::setSkyColour( preset.skyColour, 1.0f );
        HdrUtils::setExposure( mExposure, mMinAutoExposure, mMaxAutoExposure );
        HdrUtils::setBloomThreshold( Ogre::max( mBloomFullThreshold - 2.0f, 0.0f ),
                                     Ogre::max( mBloomFullThreshold, 0.01f ) );
        for( int i=0; i<3; ++i )
        {
            assert( dynamic_cast<Ogre::Light*>( mLightNodes[i]->getAttachedObject( 0 ) ) );
            Ogre::Light *light = static_cast<Ogre::Light*>( mLightNodes[i]->getAttachedObject( 0 ) );
            light->setPowerScale( preset.lightPower[i] );
        }

        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        sceneManager->setAmbientLight( preset.ambLowerHemisphere,
                                       preset.ambUpperHemisphere,
                                       sceneManager->getAmbientLightHemisphereDir(),
                                       preset.envmapScale );
    }
    //-----------------------------------------------------------------------------------
    void HdrGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );

        if( mDisplayHelpMode == 2 )
        {
            outText += "\nHold SHIFT to decrease values\n[SPACE] Preset: ";
            outText += mPresetName;
            outText += "\nF5 Exposure = ";
            outText += Ogre::StringConverter::toString( mExposure );
            outText += "\nF6 Min Auto Exposure = ";
            outText += Ogre::StringConverter::toString( mMinAutoExposure );
            outText += "\nF7 Max Auto Exposure = ";
            outText += Ogre::StringConverter::toString( mMaxAutoExposure );
            outText += "\nF8 Bloom Threshold = ";
            outText += Ogre::StringConverter::toString( mBloomFullThreshold );
        }
        else if( mDisplayHelpMode == 1 )
        {
            Ogre::uint32 visibilityMask = mGraphicsSystem->getSceneManager()->getVisibilityMask();

            outText += "\nPress F2 to toggle animation. ";
            outText += mAnimateObjects ? "[On]" : "[Off]";
            outText += "\nPress F3 to show/hide animated objects. ";
            outText += (visibilityMask & 0x000000001) ? "[On]" : "[Off]";
            outText += "\nPress F4 to show/hide palette of spheres. ";
            outText += (visibilityMask & 0x000000002) ? "[On]" : "[Off]";
        }
    }
    //-----------------------------------------------------------------------------------
    void HdrGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS|KMOD_LSHIFT|KMOD_RSHIFT)) != 0 )
        {
            TutorialGameState::keyReleased( arg );
            return;
        }

        if( arg.keysym.sym == SDLK_F5 )
        {
            if( arg.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT) )
                mExposure -= 0.5;
            else
                mExposure += 0.5;

            HdrUtils::setExposure( mExposure, mMinAutoExposure, mMaxAutoExposure );
        }
        else if( arg.keysym.sym == SDLK_F6 )
        {
            if( arg.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT) )
                mMinAutoExposure -= 0.5;
            else
            {
                mMinAutoExposure += 0.5;
                if( mMinAutoExposure > mMaxAutoExposure )
                    mMaxAutoExposure = mMinAutoExposure;
            }

            HdrUtils::setExposure( mExposure, mMinAutoExposure, mMaxAutoExposure );
        }
        else if( arg.keysym.sym == SDLK_F7 )
        {
            if( arg.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT) )
            {
                mMaxAutoExposure -= 0.5;
                if( mMaxAutoExposure < mMinAutoExposure )
                    mMinAutoExposure = mMaxAutoExposure;
            }
            else
            {
                mMaxAutoExposure += 0.5;
            }

            HdrUtils::setExposure( mExposure, mMinAutoExposure, mMaxAutoExposure );
        }
        else if( arg.keysym.sym == SDLK_F8 )
        {
            if( arg.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT) )
            {
                mBloomFullThreshold *= 0.5f;
                if( mBloomFullThreshold < 0 )
                    mBloomFullThreshold = 0;
            }
            else
            {
                mBloomFullThreshold *= 2.0f;
            }

            HdrUtils::setBloomThreshold( Ogre::max( mBloomFullThreshold - 2.0f, 0.0f ),
                                         Ogre::max( mBloomFullThreshold, 0.01f ) );
        }
        else if( arg.keysym.sym == SDLK_SPACE )
        {
            switchPreset( (arg.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT)) ? -1 : 1 );
        }
        if( arg.keysym.sym == SDLK_F2 )
        {
            mAnimateObjects = !mAnimateObjects;
        }
        else if( arg.keysym.sym == SDLK_F3 )
        {
            Ogre::uint32 visibilityMask = mGraphicsSystem->getSceneManager()->getVisibilityMask();
            bool showMovingObjects = (visibilityMask & 0x00000001);
            showMovingObjects = !showMovingObjects;
            visibilityMask &= ~0x00000001;
            visibilityMask |= (Ogre::uint32)showMovingObjects;
            mGraphicsSystem->getSceneManager()->setVisibilityMask( visibilityMask );
        }
        else if( arg.keysym.sym == SDLK_F4 )
        {
            Ogre::uint32 visibilityMask = mGraphicsSystem->getSceneManager()->getVisibilityMask();
            bool showPalette = (visibilityMask & 0x00000002) != 0;
            showPalette = !showPalette;
            visibilityMask &= ~0x00000002;
            visibilityMask |= (Ogre::uint32)(showPalette) << 1;
            mGraphicsSystem->getSceneManager()->setVisibilityMask( visibilityMask );
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
