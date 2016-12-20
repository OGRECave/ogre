
#include "GraphicsSystem.h"
#include "DynamicGeometryGameState.h"

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
        DynamicGeometryGameState *gfxGameState = new DynamicGeometryGameState(
        "Shows how to create a Mesh programmatically from code and update it.\n"
        "None of the cubes were loaded from disk.\n"
        "All of the animation is performed by uploading new vertex data from CPU.\n"
        "The cubes, from left to right.\n"
        "   1. It is a BT_IMMUTABLE buffer example.\n"
        "   2. Uses BT_DEFAULT buffer and updates a single vertex.\n"
        "   3. Uses BT_DYNAMIC_* and uploads all the vertices again on every map\n"
        "      (dynamic buffers are required to reupload all data after mapping).\n"
        "   4. Same as 3., but shows how to perform more than one map per frame\n"
        "      (advanced GPU memory management)." );

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
        return "Dynamic Buffers Example";
    }
}
