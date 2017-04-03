
#include "GraphicsSystem.h"
#include "StaticShadowMapsGameState.h"

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
    class StaticShadowMapsGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                    "StaticShadowMapsWorkspace", true );
        }

    public:
        StaticShadowMapsGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        StaticShadowMapsGameState *gfxGameState = new StaticShadowMapsGameState(
        "First see ShadowMapDebugging sample if you haven't already.\n"
        "This sample shows how to use static/fixed shadow maps to increase performance.\n"
        "The spotlights are static, and will only update explicitly. Try animating the\n"
        "cubes (F2) and disable updating the shadows (F3) to see the performance impact.\n"
        "Static shadows are faster and thus allow higher resolutions (which is what we\n"
        "did in this demo) for the same framerate budget. Note that for simple scenes.\n"
        "you will still see a framerate drop by increasing resolution due to worsensed\n"
        "texture cache utilization. But this becomes less important for complex scenes.\n"
        "\n"
        "Aside from performance, static shadow maps fix a light to a given shadow map,\n"
        "which means the spotlight maps won't switch places as you move the camera like\n"
        "they do in ShadowMapDebugging sample. This gives the artist the ability to\n"
        "have better control over a scene's shadows, which can result in an improved\n"
        "experience.\n"
        "This demo is not exactly the same as ShadowMapDebugging:\n"
        "   * The resolution for the static shadow maps is much higher.\n"
        "   * We use 1 point & 1 spot lights; instead of 2 spot lights.\n"
        "\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/StaticShadowMaps.compositor" );

        GraphicsSystem *graphicsSystem = new StaticShadowMapsGraphicsSystem( gfxGameState );

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
