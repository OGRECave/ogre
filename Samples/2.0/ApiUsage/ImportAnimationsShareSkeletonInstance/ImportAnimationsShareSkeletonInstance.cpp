
#include "GraphicsSystem.h"
#include "ImportAnimationsShareSkeletonInstanceGameState.h"

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
        ImportAnimationsShareSkeletonInstanceGameState *gfxGameState =
                new ImportAnimationsShareSkeletonInstanceGameState(
        "This sample shows how to directly import animation clips from multiple .skeleton files \n"
        "directly into a single skeleton from a v2Mesh\n"
        "And also how to share the same skeleton instance between components of the same\n"
        "actor/character. For example, an RPG player wearing armour, boots, helmets, etc.\n"
        "In this sample, the feet, hands, head, legs and torso are all separate items using\n"
        "the same skeleton." );

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
        return "Import animations & share skeleton";
    }
}
