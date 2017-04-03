
#include "GraphicsSystem.h"
#include "InstantRadiosityGameState.h"

#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "OgreConfigFile.h"
#include "Compositor/OgreCompositorManager2.h"

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
    class InstantRadiosityGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                    "ShadowMapDebuggingWorkspace", true );
        }

    public:
        InstantRadiosityGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        InstantRadiosityGameState *gfxGameState = new InstantRadiosityGameState(
        "This sample shows how to use 'Instant Radiosity' (IR for short).\n"
        "IR traces rays in CPU and creates a VPL (Virtual Point Light) at every hit point to mimic\n"
        "the effect of Global Illumination. A few highlights:\n"
        "   * As GI solution, IR is neither too fast nor too slow. Somewhere in the middle.\n"
        "   * The build process may be expensive enough for it to not be real time.\n"
        "   * The advantage of IR is that it works on dynamic objects.\n"
        "   * VPLs get averaged and clustered based on cluster size to improve speed.\n"
        "There's several ways to fight light leaking:\n"
        "   * Smaller cluster sizes are more accurate, but are much slower (because of more VPLs)\n"
        "   * Smaller mVplMaxRange is faster and leaks less, but its illumination reach is lower,\n"
        "     causing them to be less accurate\n"
        "   * mBias pushes the placement of the VPL away from its true position. This is not physically\n"
        "     accurate but allows to reduce mVplMaxRange and fight light leaking at the same time.\n"
        "   * mVplThreshold helps removing very weak VPLs, improving performance\n"
        "\n" );

        GraphicsSystem *graphicsSystem = new InstantRadiosityGraphicsSystem( gfxGameState );

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
        return "Global Illumination using Instant Radiosity";
    }
}
