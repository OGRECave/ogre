
#include "GraphicsSystem.h"
#include "MyGameState.h"

#include "OgreRenderWindow.h"
#include "OgreTimer.h"

#include "Threading/OgreThreads.h"

using namespace Demo;

int main()
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
        "Keep going through the Tutorials for superior methods of updating the rendering loop." );
    GraphicsSystem graphicsSystem( &myGameState );

    myGameState._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem.initialize();

    Ogre::RenderWindow *renderWindow = graphicsSystem.getRenderWindow();

    graphicsSystem.createScene01();
    graphicsSystem.createScene02();

    Ogre::Timer timer;
    unsigned long startTime = timer.getMicroseconds();

    double timeSinceLast = 1.0 / 60.0;

    while( !graphicsSystem.getQuit() )
    {
        graphicsSystem.update( static_cast<float>( timeSinceLast ) );

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
