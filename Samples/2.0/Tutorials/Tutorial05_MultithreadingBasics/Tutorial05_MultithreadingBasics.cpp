
#include "GraphicsSystem.h"
#include "LogicSystem.h"
#include "GraphicsGameState.h"

#include "Threading/YieldTimer.h"

#include "OgreRenderWindow.h"
#include "OgreTimer.h"

#include "Threading/OgreThreads.h"
#include "Threading/OgreBarrier.h"

#include <iostream>

using namespace Demo;

extern const double cFrametime;
const double cFrametime = 1.0 / 25.0;

extern bool gFakeFrameskip;
bool gFakeFrameskip = false;

extern bool gFakeSlowmo;
bool gFakeSlowmo = false;

unsigned long renderThread( Ogre::ThreadHandle *threadHandle );
unsigned long logicThread( Ogre::ThreadHandle *threadHandle );
THREAD_DECLARE( renderThread );
THREAD_DECLARE( logicThread );

struct ThreadData
{
    GraphicsSystem  *graphicsSystem;
    LogicSystem     *logicSystem;
    Ogre::Barrier   *barrier;
};

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main()
#endif
{
    GraphicsGameState graphicsGameState(
                "This tutorial shows how to setup two update loops: One for Graphics, another for\n"
                "Logic, each in its own thread. We don't render anything because we will now need\n"
                "to do a robust synchronization for creating, destroying and updating Entities,\n"
                "which is too complex to show in just one tutorial step.\n"
                "\n"
                "The key concept is that Graphic's createScene01 runs in parallel to Logic's\n"
                "createScene01. But we guarantee that createScene02 will be called after all\n"
                "createScene01s have been called. In other words, createScene is divided in\n"
                "two stages and each stage runs in parallel.\n"
                "\n"
                "This means that Logic will be creating the entities in stage 01; and Graphics\n"
                "will see the request to create the Ogre objects (e.g. Item, SceneNode) in\n"
                "stage 02. Meanwhile Graphics may dedicate the time in stage 01 to preload some\n"
                "meshes, overlays, and other resources that will always be needed.\n"
                "Logic in stage 02 will be idle, so it may dedicate that time to load non-\n"
                "graphics related data (like physics representations).\n" );
    GraphicsSystem graphicsSystem( &graphicsGameState );
    GameState logicGameState; //Dummy
    LogicSystem logicSystem( &logicGameState );
    Ogre::Barrier barrier( 2 );

    graphicsGameState._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem._notifyLogicSystem( &logicSystem );
    logicSystem._notifyGraphicsSystem( &graphicsSystem );

    ThreadData threadData;
    threadData.graphicsSystem   = &graphicsSystem;
    threadData.logicSystem      = &logicSystem;
    threadData.barrier          = &barrier;

    Ogre::ThreadHandlePtr threadHandles[2];
    threadHandles[0] = Ogre::Threads::CreateThread( THREAD_GET( renderThread ), 0, &threadData );
    threadHandles[1] = Ogre::Threads::CreateThread( THREAD_GET( logicThread ), 1, &threadData );

    Ogre::Threads::WaitForThreads( 2, threadHandles );

    return 0;
}


//---------------------------------------------------------------------
unsigned long renderThreadApp( Ogre::ThreadHandle *threadHandle )
{
    ThreadData *threadData = reinterpret_cast<ThreadData*>( threadHandle->getUserParam() );
    GraphicsSystem *graphicsSystem  = threadData->graphicsSystem;
    Ogre::Barrier *barrier          = threadData->barrier;

    graphicsSystem->initialize( "Tutorial 05: Multithreading Basics" );
    barrier->sync();

    if( graphicsSystem->getQuit() )
    {
        graphicsSystem->deinitialize();
        return 0; //User cancelled config
    }

    graphicsSystem->createScene01();
    barrier->sync();

    graphicsSystem->createScene02();
    barrier->sync();

    Ogre::RenderWindow *renderWindow = graphicsSystem->getRenderWindow();

    Ogre::Timer timer;

    unsigned long startTime = timer.getMicroseconds();

    double timeSinceLast = 1.0 / 60.0;

    while( !graphicsSystem->getQuit() )
    {
        graphicsSystem->beginFrameParallel();
        graphicsSystem->update( timeSinceLast );
        graphicsSystem->finishFrameParallel();

        if( !renderWindow->isVisible() )
        {
            //Don't burn CPU cycles unnecessary when we're minimized.
            Ogre::Threads::Sleep( 500 );
        }

        if( gFakeFrameskip )
            Ogre::Threads::Sleep( 120 );

        unsigned long endTime = timer.getMicroseconds();
        timeSinceLast = (endTime - startTime) / 1000000.0;
        timeSinceLast = std::min( 1.0, timeSinceLast ); //Prevent from going haywire.
        startTime = endTime;
    }

    barrier->sync();

    graphicsSystem->destroyScene();
    barrier->sync();

    graphicsSystem->deinitialize();
    barrier->sync();

    return 0;
}
unsigned long renderThread( Ogre::ThreadHandle *threadHandle )
{
    unsigned long retVal = -1;

    try
    {
        retVal = renderThreadApp( threadHandle );
    }
    catch( Ogre::Exception& e )
    {
   #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBoxA( NULL, e.getFullDescription().c_str(), "An exception has occured!",
                     MB_OK | MB_ICONERROR | MB_TASKMODAL );
   #else
        std::cerr << "An exception has occured: " <<
                     e.getFullDescription().c_str() << std::endl;
   #endif

        abort();
    }

    return retVal;
}
//---------------------------------------------------------------------
unsigned long logicThread( Ogre::ThreadHandle *threadHandle )
{
    ThreadData *threadData = reinterpret_cast<ThreadData*>( threadHandle->getUserParam() );
    GraphicsSystem *graphicsSystem  = threadData->graphicsSystem;
    LogicSystem *logicSystem        = threadData->logicSystem;
    Ogre::Barrier *barrier          = threadData->barrier;

    logicSystem->initialize();
    barrier->sync();

    if( graphicsSystem->getQuit() )
    {
        logicSystem->deinitialize();
        return 0; //Render thread cancelled early
    }

    logicSystem->createScene01();
    barrier->sync();

    logicSystem->createScene02();
    barrier->sync();

    Ogre::RenderWindow *renderWindow = graphicsSystem->getRenderWindow();

    Ogre::Timer timer;
    YieldTimer yieldTimer( &timer );

    unsigned long startTime = timer.getMicroseconds();

    while( !graphicsSystem->getQuit() )
    {
        logicSystem->beginFrameParallel();
        logicSystem->update( static_cast<float>( cFrametime ) );
        logicSystem->finishFrameParallel();

        logicSystem->finishFrame();

        if( gFakeSlowmo )
            Ogre::Threads::Sleep( 120 );

        if( !renderWindow->isVisible() )
        {
            //Don't burn CPU cycles unnecessary when we're minimized.
            Ogre::Threads::Sleep( 500 );
        }

        //YieldTimer will wait until the current time is greater than startTime + cFrametime
        startTime = yieldTimer.yield( cFrametime, startTime );
    }

    barrier->sync();

    logicSystem->destroyScene();
    barrier->sync();

    logicSystem->deinitialize();
    barrier->sync();

    return 0;
}
