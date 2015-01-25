
#include "GraphicsSystem.h"
#include "MyGameState.h"
#include "Threading/YieldTimer.h"

#include "OgreRenderWindow.h"
#include "OgreTimer.h"

#include "Threading/OgreThreads.h"

using namespace Demo;

const double cFrametime[2] = { 1.0 / 25.0, 1.0 / 60.0 };

extern int gCurrentFrameTimeIdx;
int gCurrentFrameTimeIdx = 0;

extern bool gFakeSlowmo;
bool gFakeSlowmo = false;

int main()
{
    MyGameState myGameState(
               "This tutorial is very similar to Tutorial02; however it uses a fixed framerate instead\n"
               "of a variable one. This means timeSinceLast will always be 1/25 (or whatever value you\n"
               "set); and the update will always be capped at 25hz / 25 FPS.\n"
               "We chose a value of 25hz to demonstrate the big difference. A value of 60hz will look\n"
               "smooth.\n"
               "\n"
               "There are many reasons to using a fixed framerate instead of a variable one:\n"
               " * It is more stable. High framerates (i.e. 10000 fps) cause floating point precision\n"
               "   issues in 'timeSinceLast' as it becomes very small. The value may even eventually\n"
               "   round to 0!\n"
               " * Physics stability, physics and logic simulations don't like variable framerate.\n"
               " * Determinism (given the same input, every run of the program will always return the\n"
               "  same output). Variable framerate and determinism don't play together.\n"
               "\n"
               "This also means that if your CPU/GPU aren't fast enough to render the frame in\n"
               "40ms, the game simulation will start looking in 'slow motion'; unlike variable\n"
               "framerate where the speed is maintained, but it looks a bit more 'frame skippy'.\n"
               "\n"
               "If the frame was rendered in less time than 40ms; The 'YieldTimer' class will tell\n"
               "the OS to yield this thread to give a chance to other threads and processes to process\n"
               "their own stuff.\n"
               "Ideally we would call sleep; but on many OSes (particularly Windows!) the granularity\n"
               "of sleep is very low: calling Sleep(1) may end up sleeping 15ms!!!\n"
               "\n"
               "Notice that the slow motion problem (when CPU/GPU can't cope with our app) is much more\n"
               "noticeable when rendering at 60hz than at 25hz\n"
               "\n"
               "For more information, see Fix Your TimeStep!\n"
               " http://gafferongames.com/game-physics/fix-your-timestep/" );
    GraphicsSystem graphicsSystem( &myGameState );

    myGameState._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem.initialize();

    Ogre::RenderWindow *renderWindow = graphicsSystem.getRenderWindow();

    graphicsSystem.createScene01();
    graphicsSystem.createScene02();

    Ogre::Timer timer;
    YieldTimer yieldTimer( &timer );

    unsigned long startTime = timer.getMilliseconds();

    while( !graphicsSystem.getQuit() )
    {
        graphicsSystem.update( static_cast<float>( cFrametime[gCurrentFrameTimeIdx] ) );

        if( !renderWindow->isVisible() )
        {
            //Don't burn CPU cycles unnecessary when we're minimized.
            Ogre::Threads::Sleep( 500 );
        }

        if( gFakeSlowmo )
            Ogre::Threads::Sleep( 50 );

        //YieldTimer will wait until the current time is greater than startTime + cFrametime
        startTime = yieldTimer.yield( cFrametime[gCurrentFrameTimeIdx], startTime );
    }

    graphicsSystem.destroyScene();
    graphicsSystem.deinitialize();

    return 0;
}
