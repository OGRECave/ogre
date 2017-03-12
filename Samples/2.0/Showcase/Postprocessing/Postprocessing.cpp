
#include "GraphicsSystem.h"
#include "PostprocessingGameState.h"

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
    class PostprocessingGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            //Delegate compositor creation to the game state. It could be done here,
            //but we would later have to inform the game state about some data.
            assert( dynamic_cast<PostprocessingGameState*>(mCurrentGameState) );
            return static_cast<PostprocessingGameState*>(mCurrentGameState)->setupCompositor();
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

            const char *c_locations[6] =
            {
                "2.0/scripts/materials/TutorialSky_Postprocess",
                "2.0/scripts/materials/Postprocessing",
                "2.0/scripts/materials/Postprocessing/GLSL",
                "2.0/scripts/materials/Postprocessing/HLSL",
                "2.0/scripts/materials/Postprocessing/Metal",
                "2.0/scripts/materials/Postprocessing/SceneAssets",
            };

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE && OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
            Ogre::String dataFolder = originalDataFolder + "packs/cubemapsJS.zip";
#else
            Ogre::String dataFolder = originalDataFolder + "cubemapsJS.zip";
#endif
            addResourceLocation( dataFolder, "Zip", "General" );

            for( size_t i=0; i<6; ++i )
            {
                Ogre::String dataFolder = originalDataFolder + c_locations[i];
                addResourceLocation( dataFolder, "FileSystem", "General" );
            }
        }

    public:
        PostprocessingGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        PostprocessingGameState *gfxGameState = new PostprocessingGameState(
        "Shows how to use the compositor for postprocessing effects.\n"
        "Use the numbers in the keyboard to toggle effects on/off\n"
        "\nThere is no 'right way' to setup a compositor. This sample\n"
        "shows two ways to do it.\n"
        "The majority of effects boil down to two RenderTextures ping-ponging\n"
        "each other (A writes to B, B writes to A, B writes to A).\n\n"
        "We take advantage of that by creating two RTTs in the starting\n"
        "rendering node (PostprocessingSampleStdRenderer) and then sending\n"
        "the two textures to the postprocessing nodes, alternating rt0 and rt1\n"
        "output channels every time; so that if 'Bloom' reads from rt0 and writes\n"
        "to rt1, then 'Radial Blur' will read from rt1 and write to rt0\n"
        "It's a simple but effective setup that maximizes resource usage and.\n"
        "avoid useless copies.\n\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/materials/Common/*.*\n"
        "   * Samples/Media/2.0/materials/Postprocessing/*.*\n"
        "   * Samples/Media/2.0/scripts/materials/TutorialSky_Postprocess/*.*\n"
        "   * Samples/Media/packs/cubemapsJS.zip\n" );

        GraphicsSystem *graphicsSystem = new PostprocessingGraphicsSystem( gfxGameState );

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
        return "Postprocessing Sample";
    }
}

