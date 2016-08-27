
#include "GraphicsSystem.h"
#include "LogicSystem.h"
#include "GraphicsGameState.h"
#include "LogicGameState.h"

#include "OgreRenderWindow.h"
#include "OgreTimer.h"

#include "Threading/OgreThreads.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"

using namespace Demo;

extern const double cFrametime;
const double cFrametime = 1.0 / 25.0;

extern bool gFakeSlowmo;
bool gFakeSlowmo = false;

extern bool gFakeFrameskip;
bool gFakeFrameskip = false;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMainApp( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR strCmdLine, INT nCmdShow )
#else
int mainApp( int argc, const char *argv[] )
#endif
{
    GraphicsGameState graphicsGameState(
               "This tutorial combines fixed and variable framerate: Logic is executed at 25hz, while\n"
               "graphics are being rendered at a variable rate, interpolating between frames to\n"
               "achieve a smooth result.\n"
               "When OGRE or the GPU is taking too long, you will see a 'frame skip' effect, when\n"
               "the CPU is taking too long to process the Logic code, you will see a 'slow motion'\n"
               "effect.\n"
               "This combines the best of both worlds and is the recommended approach for serious\n"
               "game development.\n"
               "\n"
               "The only two disadvantages from this technique are:\n"
               " * We introduce 1 frame of latency.\n"
               " * Teleporting may be shown as very fast movement; as the graphics state will try to\n"
               "   blend between the last and current position. This can be solved though, by writing\n"
               "   to both the previous and current position in case of teleportation. We purposedly\n"
               "   don't do this to show the effect/'glitch'.\n"
               "\n"
               "This approach needs to copy all the transform data from the logic side to the\n"
               "graphics side (i.e. Ogre SceneNodes). This is very common however, since a physics\n"
               "engine will use its own data structures to store its transforms (i.e. Bullet, Havok,\n"
               "ODE, PhysX)\n"
               "\n"
               "The next Tutorials will show how to run the logic and physics in its own thread, while\n"
               "OGRE renders in its own thread." );
    GraphicsSystem graphicsSystem( &graphicsGameState );
    LogicGameState logicGameState;
    LogicSystem logicSystem( &logicGameState );

    logicGameState._notifyGraphicsGameState( &graphicsGameState );
    graphicsGameState._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem._notifyLogicSystem( &logicSystem );
    logicSystem._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem.initialize( "Tutorial 04: Interpolation Loop" );
    logicSystem.initialize();

    if( graphicsSystem.getQuit() )
    {
        logicSystem.deinitialize();
        graphicsSystem.deinitialize();
        return 0; //User cancelled config
    }

    Ogre::RenderWindow *renderWindow = graphicsSystem.getRenderWindow();

    graphicsSystem.createScene01();
    logicSystem.createScene01();
    graphicsSystem.createScene02();
    logicSystem.createScene02();

    Ogre::Timer timer;

    unsigned long startTime = timer.getMicroseconds();
    double accumulator = cFrametime;

    double timeSinceLast = 1.0 / 60.0;

    while( !graphicsSystem.getQuit() )
    {
        while( accumulator >= cFrametime )
        {
            graphicsSystem.beginFrameParallel();

            logicSystem.beginFrameParallel();
            logicSystem.update( static_cast<float>( cFrametime ) );
            logicSystem.finishFrameParallel();

            graphicsSystem.finishFrameParallel();

            logicSystem.finishFrame();
            graphicsSystem.finishFrame();

            accumulator -= cFrametime;

            if( gFakeSlowmo )
                Ogre::Threads::Sleep( 40 );
        }

        graphicsSystem.beginFrameParallel();
        graphicsSystem.update( timeSinceLast );
        graphicsSystem.finishFrameParallel();

        if( !renderWindow->isVisible() )
        {
            //Don't burn CPU cycles unnecessary when we're minimized.
            Ogre::Threads::Sleep( 500 );
        }

        if( gFakeFrameskip )
            Ogre::Threads::Sleep( 40 );

        unsigned long endTime = timer.getMicroseconds();
        timeSinceLast = (endTime - startTime) / 1000000.0;
        timeSinceLast = std::min( 1.0, timeSinceLast ); //Prevent from going haywire.
        accumulator += timeSinceLast;
        startTime = endTime;
    }

    graphicsSystem.destroyScene();
    logicSystem.destroyScene();
    logicSystem.deinitialize();
    graphicsSystem.deinitialize();

    return 0;
}
