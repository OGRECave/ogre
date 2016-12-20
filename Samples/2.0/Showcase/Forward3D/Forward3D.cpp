
#include "GraphicsSystem.h"
#include "Forward3DGameState.h"

#include "OgreRoot.h"
#include "OgreRenderWindow.h"
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
    class Forward3DGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            //We reuse the ShadowMapDebugging sample's workspace. Pretty handful.
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                    "ShadowMapDebuggingWorkspace", true );
        }

    public:
        Forward3DGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        Forward3DGameState *gfxGameState = new Forward3DGameState(
        "Forward3D & Clustered are techniques capable of rendering many lights. It is required in order\n"
        "to render non-shadow casting non-directional lights with the PBS implementation.\n"
        "Deferred shading has a lot of problems (transparency, antialiasing, multiple BDRF). Besides,\n"
        "it uses a lot of bandwidth. Forward+/Forward2.5 is great, but requires DX11 HW (needs UAV) and\n"
        "a Z-Prepass. This Z-prepass is often a turn off for many (even though in some cases it may\n"
        "improve performance, i.e. if you’re heavily pixel shader or ROP [Raster OPeration] bound).\n"
        "It’s not superior on all accounts, but it can work on DX10 hardware and doesn’t require a\n"
        "Z-prepass. The result is a nice generic algorithm that can run on a lot of hardware and can\n"
        "handle a lot of lights. Whether it performs better or worse than Deferred or Forward+ depends\n"
        "on the scene.\n"
        "\n\n"
        "Like its alternatives Defered & Forward+, it works best with many small lights, or few big\n"
        "lights.\n"
        "Forward3D & Clustered have many parameters, and this demo shows different presets that often\n"
        "trade quality for speed; but sometimes some presets work better on some scenarios than others.\n"
        "This demo stresses the implementation, showing you both its strengths and limitations\n"
        "   1. Increase the number of lights to show the artifacts.\n"
        "   2. Decrease the lights' radius to alleviate the artifacts.\n"
        "   3. Switch profiles to see the differences. Often profile 0 & 5 work best.\n"
        "   4. Repeat steps 1 through 3 using a High threshold.\n"
        "   5. It may be better to have fewer but bigger lights in outdoor scenes\n"
        "   6. Lights are 'lodded' based on distance to camera as a side effect of how the algorithm works\n"
        "It is quite unrealistic that all lights to have the same parameters. Heterogenous\n"
        "light parameters will give you better results.\n"
        "Also avoid keeping too many lights tight in the same place. If we increase the distance\n"
        "between each light in this sample, the artifacts begin to disappear.\n"
        "Light's size has a direct impact on quality. Theoretically all lights have an unlimited\n"
        "range. However we cut it off after certain threshold for performance reasons. Very low\n"
        "thresholds stress the F3D system, but very high thresholds will cut the light too early.\n"
        "\n"
        "Forward+ and Deferred as alternative implementations are planned in the future.\n"
        "\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/ShadowMapDebugging.compositor" );

        Forward3DGraphicsSystem *graphicsSystem = new Forward3DGraphicsSystem( gfxGameState );

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
        return "Forward+ Lights";
    }
}
