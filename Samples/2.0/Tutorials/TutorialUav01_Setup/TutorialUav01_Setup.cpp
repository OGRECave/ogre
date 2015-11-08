
#include "GraphicsSystem.h"
#include "TutorialUav01_SetupGameState.h"
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
    class TutorialUav01_SetupGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                    "TutorialUav01_SetupWorkspace", true );
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
                "2.0/scripts/materials/TutorialUav01_Setup",
                "2.0/scripts/materials/TutorialUav01_Setup/GLSL",
                "2.0/scripts/materials/TutorialUav01_Setup/HLSL",
            };

            for( size_t i=0; i<6; ++i )
            {
                Ogre::String dataFolder = originalDataFolder + c_locations[i];
                addResourceLocation( dataFolder, "FileSystem", "General" );
            }
        }

    public:
        TutorialUav01_SetupGraphicsSystem( GameState *gameState ) :
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
    TutorialUav01_SetupGameState tutorialUav01_SetupGameState(
        "Shows how to setup an UAV (Unordered Access View).\n"
        "UAVs are complex and for advanced users, but they're very powerful and\n"
        "enable a whole new level of features and possibilities.\n"
        "This sample first fills an UAV with some data, then renders to screen\n"
        "sampling from it as a texture.\n"
        "We perform a RenderTarget-less render to the UAV via a helper PF_NULL\n"
        "texture that has a width and a height, but doesn't really occupy memory.\n"
        "When 'testTexture' is used as an UAV, you need to tell Ogre via uses_uav\n"
        "in the pass so Ogre can bake and issue the proper memory barriers\n"
        "(to avoid hazards).\n"
        "When it is used as a texture, there is no need as Ogre will automatically\n"
        "infer this information and issue the appropiate barrier.\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/TutorialUav01_Setup.compositor\n"
        "   * Samples/Media/2.0/materials/TutorialUav01_Setup/*.*\n"
        "\n" );
    TutorialUav01_SetupGraphicsSystem graphicsSystem( &tutorialUav01_SetupGameState );

    tutorialUav01_SetupGameState._notifyGraphicsSystem( &graphicsSystem );

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
