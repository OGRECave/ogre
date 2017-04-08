
#include "GraphicsSystem.h"
#include "ShadowMapDebuggingGameState.h"

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
    class ShadowMapDebuggingGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            mWorkspace = compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                          "ShadowMapDebuggingWorkspace", true );
            return mWorkspace;
        }

    public:
        ShadowMapDebuggingGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        ShadowMapDebuggingGameState *gfxGameState = new ShadowMapDebuggingGameState(
        "Shows how to render the shadow map textures from a shadow node (compositor),\n"
        "using Overlays and the Unlit Hlms implementation.\n"
        "This is very useful for debugging shadow map artifacts, bugs or glitches.\n"
        "Advanced users may even want to temporarily modify the PBS template shaders\n"
        "(Samples/Media/Hlms/Pbs/GLSL/PixelShader_ps.glsl) to colour the regions\n"
        "in which the PSSM splits start and end.\n"
        "\n"
        "This is also useful for the Ogre devs to test any regression in shadow mapping.\n"
        "Note: The spotlights' maps may switch places as you move the camera, because\n"
        "the closest light to the camera always goes first.\n"
        "\n"
        "The first 3 maps are the directional light (3 split PSSM) and the other 2 are\n"
        "the 2 spotlights\n"
        "\n"
        "The ESM filtering mode is much more expensive, however we can achieve convincing\n"
        "results with much lower resolution and less splits to get competing performance "
        "\nback. When in ESM, we use 2 splits for the PSSM, and 2 shadow maps for the\n"
        "spotlights. We also lowered the resolution.\n"
        "\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/ShadowMapDebugging.compositor" );

        GraphicsSystem *graphicsSystem = new ShadowMapDebuggingGraphicsSystem( gfxGameState );

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
        return "Shadow map debugging";
    }
}
