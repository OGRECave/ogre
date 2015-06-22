
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
        "This tutorial shows how to reconstruct the position from only the depth buffer in\n"
        "a very efficient way. This is very useful for Deferred Shading, SSAO, etc.\n"
        "The sample uses the compositor feature 'quad_normals camera_far_corners_view_space'\n"
        "in combination with a special shader so that we can unproject the depth sampled\n"
        "from the depth buffer and combine it with the frustum corners to get the view space\n"
        "projection. The code (commented out) also shows how to get the world-space position\n"
        "which requires only slight adjustments.\n\n"
        "Additionally, we show how to avoid depth buffer decompression using the compositor.\n"
        "See section '4.1.4.2 Depth Textures' of the manual for an explanation.\n\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/Tutorial_ReconstructPosFromDepth.compositor\n"
        "   * Samples/Media/2.0/materials/Tutorial_ReconstructPosFromDepth/*.*\n"
        "\n"
        "For the technical explanation, see 'Reconstructing Position From Depth':\n"
        "   Part I: http://mynameismjp.wordpress.com/2009/03/10/reconstructing-position-from-depth/\n"
        "   Part II: http://mynameismjp.wordpress.com/2009/05/05/reconstructing-position-from-depth-continued/\n"
        "   Part III: http://mynameismjp.wordpress.com/2010/09/05/position-from-depth-3/\n");
    Tutorial_ReconstructPosFromDepthGraphicsSystem graphicsSystem( &tutorial_ReconstructPosFromDepthGameState );

    tutorial_ReconstructPosFromDepthGameState._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem.initialize( "Reconstructing Position from Depth" );

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
