
#include "GraphicsSystem.h"
#include "Tutorial_ReconstructPosFromDepthGameState.h"
#include "SdlInputHandler.h"

#include "OgreTimer.h"
#include "Threading/OgreThreads.h"
#include "OgreRenderWindow.h"

#include "OgreRoot.h"
#include "Compositor/OgreCompositorManager2.h"
#include "OgreConfigFile.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"

using namespace Demo;

namespace Demo
{
    class Tutorial_ReconstructPosFromDepthGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                    "Tutorial_ReconstructPosFromDepthWorkspace", true );
        }

        virtual void setupResources(void)
        {
            GraphicsSystem::setupResources();

            Ogre::ConfigFile cf;
            cf.load(mResourcePath + "resources2.cfg");

            Ogre::String dataFolder = cf.getSetting( "DoNotUseAsResource", "Hlms", "" );

            if( dataFolder.empty() )
                dataFolder = "./";
            else if( *(dataFolder.end() - 1) != '/' )
                dataFolder += "/";

            dataFolder += "2.0/scripts/materials/Tutorial_ReconstructPosFromDepth";

            addResourceLocation( dataFolder, "FileSystem", "General" );
        }

    public:
        Tutorial_ReconstructPosFromDepthGraphicsSystem( GameState *gameState ) :
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
    Tutorial_ReconstructPosFromDepthGameState tutorial_ReconstructPosFromDepthGameState(
        "Shows how to create a sky as simple postprocess effect.\n"
        "The vertex shader ensures the depth is always = 1, and the pixel shader\n"
        "takes a cubemap texture.\n"
        "The magic is in the compositor feature 'quad_normals camera_direction' which\n"
        "sends the data needed by the cubemap lookup via normals. This data can also\n"
        "be used for realtime analytical atmosphere scattering shaders.\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/Tutorial_ReconstructPosFromDepth.compositor\n"
        "   * Samples/Media/2.0/materials/Tutorial_ReconstructPosFromDepth/*.*\n"
        "\n"
        "LEGAL: Uses Saint Peter's Basilica (C) by Emil Persson under CC Attrib 3.0 Unported\n"
        "See Samples/Media/materials/textures/Cubemaps/License.txt for more information." );
    Tutorial_ReconstructPosFromDepthGraphicsSystem graphicsSystem( &tutorial_ReconstructPosFromDepthGameState );

    tutorial_ReconstructPosFromDepthGameState._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem.initialize( "Dynamic Buffers Example" );

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
