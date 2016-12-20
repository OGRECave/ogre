
#include "GraphicsSystem.h"
#include "V2MeshGameState.h"

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
        V2MeshGameState *gfxGameState = new V2MeshGameState(
        "The mesh file format for v2 objects (i.e. Items) is currently WIP. You can easily\n"
        "import V1 meshes into the V2 format.\n"
        "There are a couple considerations to take care:\n"
        "   * You must explicitly import the mesh before using it for v2 objects.\n"
        "   * Doing so will make two copies in GPU VRAM; one for each version. If you won't\n"
        "     be using the v1 version, you can unload it to free memory.\n"
        "\n"
        "The import process can optionally perform the following tasks for you:\n"
        "   * Convert Position to 16-bit half floating point format. Recommended for desktop.\n"
        "     Most of the time you don't need the extra precision for local coordinates.\n"
        "   * Convert UVs to 16-bit half floating point. Same recommendations as position.\n"
        "   * Generate QTangents, stored as 16-bit normalized signed short. Recommended on most\n"
        "     platforms.\n"
        "\n"
        "QTangents store normal tangent and reflection information as a special quaternion (the\n"
        "sign of 'q.w' stores the reflection information). They're used for normal mapping and\n"
        "are storage efficient. They only require 8 bytes per vertex whereas the traditional\n"
        "encoding needs 28 bytes, but require slightly more ALU ops for decoding inside the\n"
        "vertex shader. QTangents can't be used if tangents can't be generated (i.e. the mesh\n"
        "doesn't contain normals or one set of UV); and will automatically generate the tangent\n"
        "and reflection data for you, before converting to QTangent if tangents weren't originally\n"
        "present.\n"
        "Hlms automatically handles QTangents. Even if you won't be using normal mapping, a\n"
        "QTangent requires 8 bytes whereas a normal needs 12 bytes. Whether the higher ALU\n"
        "cost is worth it needs profiling on mobile; which are usually very bandwidth limited.\n"
        "\n"
        "With all options turned on, a 1.9 vertex format weighting 48 bytes per vertex\n"
        "(3x Position, 3x Normals, 1x reflection, 2x UV) can be stored with just 20 bytes,\n"
        "halving the video memory size and bandwidth required" );

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
        return "Importing V1 meshes into V2 objects (convert v1 objects to v2 format)";
    }
}
