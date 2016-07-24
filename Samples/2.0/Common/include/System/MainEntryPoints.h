
#ifndef _Demo_MainEntryPoints_H_
#define _Demo_MainEntryPoints_H_

#include "OgrePrerequisites.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
    #define WIN32_LEAN_AND_MEAN
    #define VC_EXTRALEAN
    #define NOMINMAX
    #include <windows.h>
#endif

namespace Demo
{
    class GameState;
    class GraphicsSystem;
    class LogicSystem;
}

namespace Demo
{
    /** Most of our demos use the same basic setup for game loop. If you want to understand
        how it works in depth, see Tutorial01_Initialization through Tutorial06_Multithreading
    @par
        We believe the developer should be in charge of their entry points. Problem is,
        not all platforms are the same:
            * On Linux you should use int main( int argc, const char *argv[] );
            * On Windows you should use WinMain (windows apps) or int main like in Linux (console)
            * On iOS you must declare int main, but Apple really want you to use an AppDelegate.
            * On Android you the entry point is loaded then executed from your Java code.

        Some frameworks address this problem by forcing you to derive from some arbitrary class,
        or using macros.
        We don't force you to do anything, but we provide utility functions if you don't
        want to deal with issue.
    @par
        If you are a beginner or just want to go straight to coding an Ogre app:
            Follow exactly what most samples do (e.g. StereoRendering,
            DynamicGeometry, CustomRenderable):
            1. #include "MainEntryPointHelper.h"
            2. #include "System/MainEntryPoints.h"
            3. Define createSystems (& destroySystems) where you allocate the systems this
               framework needs. You can subclass these systems to override functionality.
               GraphicsSystem::update and GameState::update get called every frame.
            4. Declare int mainApp (or INT WINAPI WinMainApp) where you either choose to
               delegate to:
                    MainEntryPoints::mainAppSingleThreaded
                        This entry point will behave exactly the same as
                        Tutorial04_InterpolationLoop. If you just want variable framerate, you
                        could just avoid allocating LogicSystem.
                    MainEntryPoints::mainAppMultiThreaded
                        This entry point will behave exactly the same as
                        Tutorial06_Multithreading. You must allocate LogicSystem if you
                        use this one.
            5. Overload GraphicsSystem, LogicSystem (optional), GameState (for graphics)
               and GameState (for Logic, optional) to do your work.
        If in doubt, just copy-paste a simple sample like StereoRendering or CustomRenderable
        and work from there. Or ShadowMapDebugging/PbsMaterials if you like more advanced
        samples but still easy to grasp.
    @par
        If you're a power user and want full control:
            The header "MainEntryPointHelper.h" declares the actual main/WinMain on desktop OSes
            We will do some minor maintenance work there, then call mainApp.
            You can either avoid including MainEntryPointHelper.h entirely and define the entry
            point yourself, or work on mainApp instead. Or you can modify the code
            MainEntryPointHelper.h, it's not an integral part of Ogre. The choice is up to you.

            The members createSystems, destroySystems, getWindowTitle & Frametime are used by
            the functionality provided in mainAppSingleThreaded & mainAppMultiThreaded.
            If you decide to not use one of these two functions, then the mentioned members
            (createSystems & co.) are useless to you.

            On iOS, we provide an AppDelegate and our main() implementation will try to
            instantiate it. These two (AppDelegate + main) are located in
            Samples/2.0/Common/src/System/iOS. If you need a different AppDelegate,
            change those sources.
            The files Info.plist & Main.storyboard under the folder
            Samples/2.0/Common/src/System/iOS/Resources are configured to load GameViewController
            which is also declared in one of our .mm files. If you wish to use your own
            controller, tweak the plist and storyboard files.
            Or just modify GameViewController.mm instead.
    */
    class MainEntryPoints
    {
    public:
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
        static void createSystems( GameState **outGraphicsGameState, GraphicsSystem **outGraphicsSystem,
                                   GameState **outLogicGameState, LogicSystem **outLogicSystem );

        /// Destroys the systems created via createSystems. Implementation should check for null pointers.
        static void destroySystems( GameState *graphicsGameState, GraphicsSystem *graphicsSystem,
                                    GameState *logicGameState, LogicSystem *logicSystem );

        static const char* getWindowTitle(void);

        /// Time in seconds a frame should last in fixed timestep (e.g. to simulate
        /// physics at 60hz; set it to Frametime = 1 / 60.0). The default is 60hz
        static double Frametime;

    #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        static INT WINAPI mainAppSingleThreaded( HINSTANCE hInst, HINSTANCE hPrevInstance,
                                                 LPSTR strCmdLine, INT nCmdShow );
    #else
        static int mainAppSingleThreaded( int argc, const char *argv[] );
    #endif
    #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        static INT WINAPI mainAppMultiThreaded( HINSTANCE hInst, HINSTANCE hPrevInstance,
                                                LPSTR strCmdLine, INT nCmdShow );
    #else
        static int mainAppMultiThreaded( int argc, const char *argv[] );
    #endif
    };
}

#endif

