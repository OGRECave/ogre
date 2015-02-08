
//---------------------------------------------------------------------------------------
// This tutorial shows nothing fancy. Only how to setup Ogre to render to
// a window, rolling your own game loop, single threaded.
//
// You only need the basics from the Common framework. Just derive from GameState to
// perform your own scene and update it accordingly.
// Scene setup is divided in two stages (createScene01 & createScene02) because it
// is prepared for multithreading; although it is not strictly necessary in this case.
// See the multithreading tutorial for a better explanation
//---------------------------------------------------------------------------------------

#include "GraphicsSystem.h"
#include "GameState.h"

#include "OgreRenderWindow.h"
#include "OgreTimer.h"

#include "Threading/OgreThreads.h"

using namespace Demo;

namespace Demo
{
    class MyGraphicsSystem : public GraphicsSystem
    {
        // No resources. They're not needed and a potential point of failure.
        // This is a very simple project
        virtual void setupResources(void) {}
    public:
        MyGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState ) {}
    };
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main()
#endif
{
    GameState gameState;
    MyGraphicsSystem graphicsSystem( &gameState );

    graphicsSystem.initialize( "Tutorial 01: Initialization" );

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
