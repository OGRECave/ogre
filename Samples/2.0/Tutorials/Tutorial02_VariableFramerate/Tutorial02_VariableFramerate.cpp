
#include "GraphicsSystem.h"
#include "MyGameState.h"

#include "OgreRenderWindow.h"
#include "OgreTimer.h"

#include "Threading/OgreThreads.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"
#include "System/MainEntryPoints.h"

namespace Demo
{
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        MyGameState *gfxGameState = new MyGameState( "See desktop's version notes for more details." );
        GraphicsSystem *graphicsSystem = new GraphicsSystem( gfxGameState );

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
#endif
}

using namespace Demo;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMainApp( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR strCmdLine, INT nCmdShow )
#else
int mainApp( int argc, const char *argv[] )
#endif
{
    MyGameState myGameState(
        "This tutorial demonstrates the most basic rendering loop: Variable framerate.\n"
        "Variable framerate means the application adapts to the current frame rendering\n"
        "performance and boosts or decreases the movement speed of objects to maintain\n"
        "the appearance that objects are moving at a constant velocity.\n"
        "When framerate is low, it looks 'frame skippy'; when framerate is high,\n"
        "it looks very smooth.\n"
        "Note: If you can't exceed 60 FPS, it's probably because of VSync being turned on.\n"
        "\n"
        "Despite what it seems, this is the most basic form of updating, and a horrible way\n"
        "to update your objects if you want to do any kind of serious game development.\n"
        "Keep going through the Tutorials for superior methods of updating the rendering loop.\n"
        "\n"
        "Note: The cube is black because there is no lighting. We are not focusing on that." );
    GraphicsSystem graphicsSystem( &myGameState );

    myGameState._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem.initialize( "Tutorial 02: Variable Framerate" );

    if( graphicsSystem.getQuit() )
    {
        graphicsSystem.deinitialize();
        return 0; //User cancelled config
    }

    Ogre::RenderWindow *renderWindow = graphicsSystem.getRenderWindow();

    graphicsSystem.createScene01();
    graphicsSystem.createScene02();

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
