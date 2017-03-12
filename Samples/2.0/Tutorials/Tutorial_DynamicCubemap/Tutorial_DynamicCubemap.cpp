
#include "GraphicsSystem.h"
#include "Tutorial_DynamicCubemapGameState.h"

#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "OgreConfigFile.h"
#include "Compositor/OgreCompositorManager2.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"
#include "System/MainEntryPoints.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMainApp( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR strCmdLine, INT nCmdShow )
#else
int mainApp( int argc, const char *argv[] )
#endif
{
    return Demo::MainEntryPoints::mainAppSingleThreaded( DEMO_MAIN_ENTRY_PARAMS );
}

namespace Demo
{
    class DynamicCubemapGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            //Delegate compositor creation to the game state. We need cubemap's texture
            //to be passed to the compositor so Ogre can insert the proper barriers.
            assert( dynamic_cast<DynamicCubemapGameState*>(mCurrentGameState) );
            return static_cast<DynamicCubemapGameState*>(mCurrentGameState)->setupCompositor();
        }

        virtual void setupResources(void)
        {
            GraphicsSystem::setupResources();

            Ogre::ConfigFile cf;
            cf.load(mResourcePath + "resources2.cfg");

            Ogre::String originalDataFolder = cf.getSetting( "DoNotUseAsResource", "Hlms", "" );

            if( originalDataFolder.empty() )
                originalDataFolder = "./";
            else if( *(originalDataFolder.end() - 1) != '/' )
                originalDataFolder += "/";

            const char *c_locations[2] =
            {
                "2.0/scripts/materials/TutorialSky_Postprocess",
                "2.0/scripts/materials/PbsMaterials"
            };

            for( size_t i=0; i<2; ++i )
            {
                Ogre::String dataFolder = originalDataFolder + c_locations[i];
                addResourceLocation( dataFolder, "FileSystem", "General" );
            }
        }

    public:
        DynamicCubemapGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        DynamicCubemapGameState *gfxGameState = new DynamicCubemapGameState(
        "Shows how to setup dynamic cubemapping via the compositor, so that it can\n"
        "be used for dynamic reflections.\n"
        "The most important elements are in the compositor script Tutorial_DynamicCubemap.compositor\n"
        "and in DynamicCubemapGameState::setupCompositor.\n"
        "It's worth noting the compositor expects you to 'expose' the cubemap in the passes\n"
        "it's going to be used in, for Vulkan & D3D12 compatibility (also GL if the texture is\n"
        "an UAV). How to connect the results of one compositor workspace into another is shown.\n"
        "in setupCompositor.\n"
        "\n"
        "This sample is using only one cubemap probe in the middle of the scene.\n"
        "\n"
        "Hlms PBS automatically detects hazards (i.e. using the cubemap as texture while\n"
        "rendering to it) and will remove the cubemap from being used as texture for that pass.\n"
        "\n"
        "This hazard detection prevents multiple bounces (i.e. mirrors inside mirrors).\n"
        "Such feature is not provided out of the box yet.\n"
        "\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/Tutorial_DynamicCubemap.compositor\n"
        "   * Samples/Media/2.0/materials/PbsMaterials/PbsMaterials.material\n"
        "\n" );

        GraphicsSystem *graphicsSystem = new DynamicCubemapGraphicsSystem( gfxGameState );

        gfxGameState->_notifyGraphicsSystem( graphicsSystem );

        *outGraphicsGameState = gfxGameState;
        *outGraphicsSystem = graphicsSystem;
    }

    void MainEntryPoints::destroySystems( GameState *graphicsGameState,
                                          GraphicsSystem *graphicsSystem,
                                          GameState *logicGameState,
                                          LogicSystem *logicSystem )
    {
        delete graphicsSystem;
        delete graphicsGameState;
    }

    const char* MainEntryPoints::getWindowTitle(void)
    {
        return "Dynamic Cubemap Tutorial";
    }
}
