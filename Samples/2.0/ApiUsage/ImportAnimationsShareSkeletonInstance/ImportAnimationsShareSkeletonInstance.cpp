
#include "GraphicsSystem.h"
#include "ImportAnimationsShareSkeletonInstanceGameState.h"
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
    class ImportAnimationsShareSkeletonInstanceGraphicsSystem : public GraphicsSystem
    {
    public:
        ImportAnimationsShareSkeletonInstanceGraphicsSystem(GameState *gameState) :
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
    ImportAnimationsShareSkeletonInstanceGameState importAnimationsShareSkeletonInstanceGameState(
        "This sample shows how to directly import animation clips from multiple .skeleton files \n"
        "directly into a single skeleton from a v2Mesh\n"
        "And also how to share the same skeleton instance between components of the same\n"
        "actor/character. For example, an RPG player wearing armour, boots, helmets, etc.\n"
        "In this sample, the feet, hands, head, legs and torso are all separate items using\n"
        "the same skeleton." );
    ImportAnimationsShareSkeletonInstanceGraphicsSystem graphicsSystem(
                &importAnimationsShareSkeletonInstanceGameState );

    importAnimationsShareSkeletonInstanceGameState._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem.initialize( "Import animations & share skeleton" );

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
