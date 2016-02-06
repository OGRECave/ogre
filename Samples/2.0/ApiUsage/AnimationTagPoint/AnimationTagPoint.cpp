
#include "GraphicsSystem.h"
#include "AnimationTagPointGameState.h"
#include "SdlInputHandler.h"

#include "OgreTimer.h"
#include "Threading/OgreThreads.h"
#include "OgreRenderWindow.h"
#include "OgreConfigFile.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"

using namespace Demo;

namespace Demo
{
    class AnimationTagPointGraphicsSystem : public GraphicsSystem
    {
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

            const char *c_locations[1] =
            {
                "Models",
            };

            for( size_t i=0; i<1; ++i )
            {
                Ogre::String dataFolder = originalDataFolder + c_locations[i];
                addResourceLocation( dataFolder, "FileSystem", "General" );
            }
        }

    public:
        AnimationTagPointGraphicsSystem( GameState *gameState ) :
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
    AnimationTagPointGameState animationTagPointGameState(
        "TagPoints are much more powerful in 2.1 than they were in 1.x\n"
        "The 1.x TagPoints have many issues which won't be described here.\n"
        "In 2.1; TagPoints are for all intent and purposes, superior SceneNodes\n"
        "Those wanting full flexibility may choose to only use TagPoints instead of.\n"
        "SceneNodes, thus your nodes will always be able to be attached and detached\n"
        "to/from bones at will without worrying about proper downcasting or keeping track\n"
        "of which nodes are TagPoints and which are regular SceneNodes.\n"
        "Note however TagPoints consume a little more RAM per node than SceneNodes.\n\n"
        "This sample shows multiple ways in which TagPoints are used to attach to bones\n" );
    AnimationTagPointGraphicsSystem graphicsSystem( &animationTagPointGameState );

    animationTagPointGameState._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem.initialize( "Using TagPoints to attach nodes to Skeleton Bones" );

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
