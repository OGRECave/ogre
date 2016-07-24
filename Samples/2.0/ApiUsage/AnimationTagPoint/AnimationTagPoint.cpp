
#include "GraphicsSystem.h"
#include "AnimationTagPointGameState.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"
#include "System/MainEntryPoints.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMainApp( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR strCmdLine, INT nCmdShow )
#else
int mainApp( int argc, const char *argv[] )
#endif
{
    return Demo::MainEntryPoints::mainAppSingleThreaded( DEMO_MAIN_ENTRY_PARAMS );
}

namespace Demo
{
    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        AnimationTagPointGameState *gfxGameState = new AnimationTagPointGameState(
        "TagPoints are much more powerful in 2.1 than they were in 1.x\n"
        "The 1.x TagPoints have many issues which won't be described here.\n"
        "In 2.1; TagPoints are for all intent and purposes, superior SceneNodes\n"
        "Those wanting full flexibility may choose to only use TagPoints instead of.\n"
        "SceneNodes, thus your nodes will always be able to be attached and detached\n"
        "to/from bones at will without worrying about proper downcasting or keeping track\n"
        "of which nodes are TagPoints and which are regular SceneNodes.\n"
        "Note however TagPoints consume a little more RAM per node than SceneNodes.\n\n"
        "This sample shows multiple ways in which TagPoints are used to attach to bones\n" );

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

    const char* MainEntryPoints::getWindowTitle(void)
    {
        return "Using TagPoints to attach nodes to Skeleton Bones";
    }
}
