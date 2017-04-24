/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2016 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include "OgrePrerequisites.h"
#include <iostream>

#include "System/MainEntryPoints.h"

#include "GraphicsSystem.h"
#include "LogicSystem.h"
#include "GameState.h"
#include "SdlInputHandler.h"

#include "Threading/YieldTimer.h"

#include "OgreRenderWindow.h"
#include "OgreTimer.h"

#include "Threading/OgreThreads.h"
#include "Threading/OgreBarrier.h"

#include "Threading/OgreThreads.h"

using namespace Demo;

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
INT WINAPI Demo::MainEntryPoints::mainAppMultiThreaded( HINSTANCE hInst, HINSTANCE hPrevInstance,
                                                        LPSTR strCmdLine, INT nCmdShow )
#else
int Demo::MainEntryPoints::mainAppMultiThreaded( int argc, const char *argv[] )
#endif
{
    GameState *graphicsGameState = 0;
    GraphicsSystem *graphicsSystem = 0;
    GameState *logicGameState = 0;
    LogicSystem *logicSystem = 0;

    Ogre::Barrier barrier( 2 );

    MainEntryPoints::createSystems( &graphicsGameState, &graphicsSystem,
                                    &logicGameState, &logicSystem );

    GameEntityManager gameEntityManager( graphicsSystem, logicSystem );

    ThreadData threadData;
    threadData.graphicsSystem   = graphicsSystem;
    threadData.logicSystem      = logicSystem;
    threadData.barrier          = &barrier;

    Ogre::ThreadHandlePtr threadHandles[2];
    threadHandles[0] = Ogre::Threads::CreateThread( THREAD_GET( renderThread ), 0, &threadData );
    threadHandles[1] = Ogre::Threads::CreateThread( THREAD_GET( logicThread ), 1, &threadData );

    Ogre::Threads::WaitForThreads( 2, threadHandles );

    MainEntryPoints::destroySystems( graphicsGameState, graphicsSystem,
                                     logicGameState, logicSystem );

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

    #if OGRE_USE_SDL2
    //Do this after creating the scene for easier the debugging (the mouse doesn't hide itself)
    SdlInputHandler *inputHandler = graphicsSystem->getInputHandler();
    inputHandler->setGrabMousePointer( true );
    inputHandler->setMouseVisible( false );
    #endif

    Ogre::RenderWindow *renderWindow = graphicsSystem->getRenderWindow();

    Ogre::Timer timer;

    unsigned long startTime = timer.getMicroseconds();

    double timeSinceLast = 1.0 / 60.0;

    while( !graphicsSystem->getQuit() )
    {
        graphicsSystem->beginFrameParallel();
        graphicsSystem->update( static_cast<float>( timeSinceLast ) );
        graphicsSystem->finishFrameParallel();

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
        logicSystem->update( static_cast<float>( MainEntryPoints::Frametime ) );
        logicSystem->finishFrameParallel();

        logicSystem->finishFrame();

        if( !renderWindow->isVisible() )
        {
            //Don't burn CPU cycles unnecessary when we're minimized.
            Ogre::Threads::Sleep( 500 );
        }

        //YieldTimer will wait until the current time is greater than startTime + cFrametime
        startTime = yieldTimer.yield( MainEntryPoints::Frametime, startTime );
    }

    barrier->sync();

    logicSystem->destroyScene();
    barrier->sync();

    logicSystem->deinitialize();
    barrier->sync();

    return 0;
}
