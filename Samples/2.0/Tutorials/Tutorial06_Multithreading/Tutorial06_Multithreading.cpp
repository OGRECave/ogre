
#include "GraphicsSystem.h"
#include "LogicSystem.h"
#include "GameEntityManager.h"
#include "GraphicsGameState.h"
#include "LogicGameState.h"

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
                "This is an advanced multithreading tutorial. For the simple version see Tutorial 05\n"
                "We introduce the 'GameEntity' structure which encapsulates a game object data:\n"
                "It contains its graphics (i.e. Entity and SceneNode) and its physics/logic data\n"
                "(a transform, the hkpEntity/btRigidBody pointers, etc)\n"
                "GameEntity is created via the GameEntityManager; which is responsible for telling\n"
                "the render thread to create the graphics; and delays deleting the GameEntity until\n"
                "all threads are done using it.\n"
                "Position/Rot./Scale is updated via a ring buffer, which ensures that the Logic \n"
                "thread is never writing to the transforms while being read by the Render thread\n"
                "You could gain some performance and memory by purposedly not caring and leaving\n"
                "a race condition (Render thread reading the transforms while Logic may be \n"
                "updating it) if you don't mind a very occasional flickering.\n"
                "\n"
                "The Logic thread is in charge of simulating the transforms and ultimately, updating\n"
                "the transforms.\n"
                "The Render thread is in charge of interpolating these transforms and leaving the \n"
                "rendering to Ogre (culling, updating the scene graph, skeletal animations, rendering)\n"
                "\n"
                "Render-split multithreaded rendering is very powerful and scales well to two cores\n"
                "but it requires a different way of thinking. You don't directly create Ogre objects.\n"
                "You request them via messages that need first to bake all necesary information (do \n"
                "you want an Entity, an Item, a particle FX?), and they get created asynchronously.\n"
                "A unit may be created in a logic frame, but may still not be rendered yet, and may\n"
                "take one or two render frames to appear on screen.\n"
                "\n"
                "Skeletal animation is not covered by this tutorial, but the same principle applies.\n"
                "First define a few baked structures about the animations you want to use, and then\n"
                "send messages for synchronizing it (i.e. play X animation, jump to time Y, set blend\n"
                "weight Z, etc)" );
    GraphicsSystem graphicsSystem( &graphicsGameState );
    LogicGameState logicGameState;
    LogicSystem logicSystem( &logicGameState );
    Ogre::Barrier barrier( 2 );

    graphicsGameState._notifyGraphicsSystem( &graphicsSystem );
    logicGameState._notifyLogicSystem( &logicSystem );

    graphicsSystem._notifyLogicSystem( &logicSystem );
    logicSystem._notifyGraphicsSystem( &graphicsSystem );

    GameEntityManager gameEntityManager( &graphicsSystem, &logicSystem );

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

    graphicsSystem->initialize( "Tutorial 06: Multithreading" );
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
