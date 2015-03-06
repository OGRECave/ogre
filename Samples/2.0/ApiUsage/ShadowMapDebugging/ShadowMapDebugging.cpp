
#include "GraphicsSystem.h"
#include "ShadowMapDebuggingGameState.h"
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
    class ShadowMapDebuggingGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                    "ShadowMapDebuggingWorkspace", true );
        }

    public:
        ShadowMapDebuggingGraphicsSystem( GameState *gameState ) :
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
    ShadowMapDebuggingGameState ShadowMapDebuggingGameState(
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
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/ShadowMapDebugging.compositor" );
    ShadowMapDebuggingGraphicsSystem graphicsSystem( &ShadowMapDebuggingGameState );

    ShadowMapDebuggingGameState._notifyGraphicsSystem( &graphicsSystem );

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
