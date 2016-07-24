
#ifndef _Demo_MainEntryPointHelper_H_
#define _Demo_MainEntryPointHelper_H_

#include "OgrePrerequisites.h"

namespace Demo
{
    class GameState;
    class GraphicsSystem;
    class LogicSystem;
}

namespace Demo
{
    /** Creates the system for the helper functions in Common framework to use in
        mainAppVarible & co.
    @param outGraphicsGameState [out]
        Pointer to newly allocated GameState used by GraphicsSystem.
    @param outGraphicsSystem [out]
        Pointer to newly allocated GraphicsSystem.
    @param outLogicGameState [out]
        Pointer to newly allocated GameState used by LogicSystem. Can be null.
        If null, outLogicSystem must be null too
    @param outLogicSystem [out]
        Pointer to newly allocated LogicSystem. Can be null.
        If null, outLogicGameState must be null too
    */
    void createSystems( GameState **outGraphicsGameState, GraphicsSystem **outGraphicsSystem,
                        GameState **outLogicGameState, LogicSystem **outLogicSystem );

    /// Destroys the systems created via createSystems. Implementation should check for null pointers.
    void destroySystems( GameState *graphicsGameState, GraphicsSystem *graphicsSystem,
                         GameState *logicGameState, LogicSystem *logicSystem );

    const char* getWindowTitle(void);

    extern double gFrametime;

    #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        INT WINAPI mainAppSingleThreaded( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT );
    #else
        int mainAppSingleThreaded( int argc, const char *argv[] );
    #endif
    #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        INT WINAPI mainAppMultiThreaded( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT );
    #else
        int mainAppMultiThreaded( int argc, const char *argv[] );
    #endif
}

#endif

