
#include "GraphicsSystem.h"
#include "TutorialUav02_SetupGameState.h"
#include "SdlInputHandler.h"

#include "OgreTimer.h"
#include "Threading/OgreThreads.h"
#include "OgreRenderWindow.h"

#include "OgreRoot.h"
#include "Compositor/OgreCompositorManager2.h"
#include "OgreConfigFile.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"

using namespace Demo;

namespace Demo
{
    class TutorialUav02_SetupGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                    "TutorialUav02_SetupWorkspace", true );
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
                "2.0/scripts/materials/Common",
                "2.0/scripts/materials/Common/GLSL",
                "2.0/scripts/materials/Common/HLSL",
                "2.0/scripts/materials/TutorialUav02_Setup",
                "2.0/scripts/materials/TutorialUav02_Setup/GLSL",
                "2.0/scripts/materials/TutorialUav02_Setup/HLSL",
            };

            for( size_t i=0; i<6; ++i )
            {
                Ogre::String dataFolder = originalDataFolder + c_locations[i];
                addResourceLocation( dataFolder, "FileSystem", "General" );
            }
        }

    public:
        TutorialUav02_SetupGraphicsSystem( GameState *gameState ) :
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
    TutorialUav02_SetupGameState tutorialUav02_SetupGameState(
        "This sample is exactly as TutorialUav01_Setup, except that it shows\n"
        "reading from the UAVs as an UAV (e.g. use imageLoad) instead of using\n"
        "it as a texture for reading. This time, we need to tell the compositor\n"
        "we'll be using this UAV for reading so that Ogre can issue the right barriers.\n"
        "For more general information, see TutorialUav01_Setup.\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/TutorialUav02_Setup.compositor\n"
        "   * Samples/Media/2.0/materials/TutorialUav02_Setup/*.*\n"
        "\n" );
    TutorialUav02_SetupGraphicsSystem graphicsSystem( &tutorialUav02_SetupGameState );

    tutorialUav02_SetupGameState._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem.initialize( "UAV Setup Example" );

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
