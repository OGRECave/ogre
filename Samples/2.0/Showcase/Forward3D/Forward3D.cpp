
#include "GraphicsSystem.h"
#include "Forward3DGameState.h"
#include "SdlInputHandler.h"

#include "OgreTimer.h"
#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "OgreConfigFile.h"
#include "Compositor/OgreCompositorManager2.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"

using namespace Demo;

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
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMainApp( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int mainApp()
#endif
{
    Forward3DGameState forward3DGameState(
        "Forward3D is a technique capable of rendering many lights. It is required in order\n"
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
        "Forward3D has many parameters, and this demo shows different presets that often trade\n"
        "quality for speed; but sometimes some presets work better on some scenarios than others.\n"
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
    Forward3DGraphicsSystem graphicsSystem( &forward3DGameState );

    forward3DGameState._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem.initialize( "Shadow map debugging" );

    if( graphicsSystem.getQuit() )
    {
        graphicsSystem.deinitialize();
        return 0; //User cancelled config
    }

    Ogre::RenderWindow *renderWindow = graphicsSystem.getRenderWindow();

    graphicsSystem.createScene01();
    graphicsSystem.createScene02();

    //Do this after creating the scene for easier the debugging (the mouse doesn't hide itself)
    SdlInputHandler *inputHandler = graphicsSystem.getInputHandler();
    inputHandler->setGrabMousePointer( true );
    inputHandler->setMouseVisible( false );

    Ogre::Timer timer;
    unsigned long startTime = timer.getMicroseconds();

    double timeSinceLast = 1.0 / 60.0;

    while( !graphicsSystem.getQuit() )
    {
        graphicsSystem.beginFrameParallel();
        graphicsSystem.update( static_cast<float>( timeSinceLast ) );
        graphicsSystem.finishFrameParallel();
        graphicsSystem.finishFrame();

        if( !renderWindow->isVisible() )
        {
            //Don't burn CPU cycles unnecessary when we're minimized.
            Ogre::Threads::Sleep( 500 );
        }

        unsigned long endTime = timer.getMicroseconds();
        timeSinceLast = (endTime - startTime) / 1000000.0;
        timeSinceLast = std::min( 1.0, timeSinceLast ); //Prevent from going haywire.
        startTime = endTime;
    }

    graphicsSystem.destroyScene();
    graphicsSystem.deinitialize();

    return 0;
}
