
#include "GraphicsSystem.h"
#include "DynamicGeometryGameState.h"
#include "SdlInputHandler.h"

#include "OgreTimer.h"
#include "Threading/OgreThreads.h"
#include "OgreRenderWindow.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"

using namespace Demo;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMainApp( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int mainApp()
#endif
{
    DynamicGeometryGameState dynamicGeometryGameState(
        "Shows how to create a Mesh programmatically from code and update it.\n"
        "None of the cubes were loaded from disk.\n"
        "All of the animation is performed by uploading new vertex data from CPU.\n"
        "The cubes, from left to right.\n"
        "   1. It is a BT_IMMUTABLE buffer example.\n"
        "   2. Uses BT_DEFAULT buffer and updates a single vertex.\n"
        "   3. Uses BT_DYNAMIC_* and uploads all the vertices again on every map\n"
        "      (dynamic buffers are required to reupload all data after mapping).\n"
        "   4. Same as 3., but shows how to perform more than one map per frame\n"
        "      (advanced GPU memory management)." );
    GraphicsSystem graphicsSystem( &dynamicGeometryGameState );

    dynamicGeometryGameState._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem.initialize( "Dynamic Buffers Example" );

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
