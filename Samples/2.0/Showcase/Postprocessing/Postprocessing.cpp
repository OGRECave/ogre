
#include "GraphicsSystem.h"
#include "PostprocessingGameState.h"
#include "SdlInputHandler.h"

#include "OgreTimer.h"
#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "OgreConfigFile.h"
#include "Compositor/OgreCompositorManager2.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"

using namespace Demo;

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

            const char *c_locations[8] =
            {
                "2.0/scripts/materials/Common",
                "2.0/scripts/materials/Common/GLSL",
                "2.0/scripts/materials/Common/HLSL",
                "2.0/scripts/materials/TutorialSky_Postprocess",
                "2.0/scripts/materials/Postprocessing",
                "2.0/scripts/materials/Postprocessing/GLSL",
                "2.0/scripts/materials/Postprocessing/HLSL",
                "2.0/scripts/materials/Postprocessing/SceneAssets",
            };

            Ogre::String dataFolder = originalDataFolder + "packs/cubemapsJS.zip";
            addResourceLocation( dataFolder, "Zip", "General" );

            for( size_t i=0; i<8; ++i )
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
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMainApp( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int mainApp()
#endif
{
    PostprocessingGameState postprocessingGameState(
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
    PostprocessingGraphicsSystem graphicsSystem( &postprocessingGameState );

    postprocessingGameState._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem.initialize( "Postprocessing Sample" );

    if( graphicsSystem.getQuit() )
    {
        graphicsSystem.deinitialize();
        return 0; //User cancelled config
    }

    Ogre::RenderWindow *renderWindow = graphicsSystem.getRenderWindow();

    graphicsSystem.createScene01();
    graphicsSystem.createScene02();

    //Do this after creating the scene for easier the debugging (the mouse doesn't hide itself)
    SdlInputHandler *inputHandler = graphicsSystem.getInputHandler();
    inputHandler->setGrabMousePointer( true );
    inputHandler->setMouseVisible( false );

    Ogre::Timer timer;
    unsigned long startTime = timer.getMicroseconds();

    double timeSinceLast = 1.0 / 60.0;

    while( !graphicsSystem.getQuit() )
    {
        graphicsSystem.beginFrameParallel();
        graphicsSystem.update( static_cast<float>( timeSinceLast ) );
        graphicsSystem.finishFrameParallel();
        graphicsSystem.finishFrame();

        if( !renderWindow->isVisible() )
        {
            //Don't burn CPU cycles unnecessary when we're minimized.
            Ogre::Threads::Sleep( 500 );
        }

        unsigned long endTime = timer.getMicroseconds();
        timeSinceLast = (endTime - startTime) / 1000000.0;
        timeSinceLast = std::min( 1.0, timeSinceLast ); //Prevent from going haywire.
        startTime = endTime;
    }

    graphicsSystem.destroyScene();
    graphicsSystem.deinitialize();

    return 0;
}
