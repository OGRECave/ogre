
#include "GraphicsSystem.h"
#include "LogicSystem.h"
#include "GraphicsGameState.h"
#include "LogicGameState.h"

#include "OgreRenderWindow.h"
#include "OgreTimer.h"

#include "Threading/OgreThreads.h"

using namespace Demo;

extern const double cFrametime;
const double cFrametime = 1.0 / 25.0;

extern bool gFakeSlowmo;
bool gFakeSlowmo = false;

extern bool gFakeFrameskip;
bool gFakeFrameskip = false;

int main()
{
    GraphicsGameState graphicsGameState(
               "This tutorial combines fixed and variable framerate: Logic is executed at 25hz, while\n"
               "graphics are being rendered at a variable rate, interpolating between frames to\n"
               "achieve a smooth result.\n"
               "When OGRE or the GPU is taking too long, you will see a 'frame skip' effect, when\n"
               "the CPU is taking too long to process the Logic code, you will see a 'slow motion'\n"
               "effect.\n"
               "This combines the best of both worlds and is the recommended approach in serious\n"
               "games\n"
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

    graphicsSystem.initialize();
    logicSystem.initialize();

    Ogre::RenderWindow *renderWindow = graphicsSystem.getRenderWindow();

    graphicsSystem.createScene01();
    logicSystem.createScene01();
    graphicsSystem.createScene02();
    logicSystem.createScene02();

    Ogre::Timer timer;

    unsigned long startTime = timer.getMilliseconds();
    double accumulator = cFrametime;

    double timeSinceLast = 1.0 / 60.0;

    while( !graphicsSystem.getQuit() )
    {
        while( accumulator >= cFrametime )
        {
            logicSystem.update( static_cast<float>( cFrametime ) );
            accumulator -= cFrametime;

            if( gFakeSlowmo )
                Ogre::Threads::Sleep( 40 );
        }

        graphicsSystem.update( timeSinceLast );

        if( !renderWindow->isVisible() )
        {
            //Don't burn CPU cycles unnecessary when we're minimized.
            Ogre::Threads::Sleep( 500 );
        }

        if( gFakeFrameskip )
            Ogre::Threads::Sleep( 40 );

        unsigned long endTime = timer.getMilliseconds();
        timeSinceLast = (endTime - startTime) / 1000.0;
        timeSinceLast = std::min( 1.0, timeSinceLast ); //Prevent from going haywire.
        accumulator += timeSinceLast;
        startTime = endTime;
    }

    graphicsSystem.destroyScene();
    graphicsSystem.deinitialize();

    return 0;
}
