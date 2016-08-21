
#include "GraphicsSystem.h"
#include "Tutorial_SSAOGameState.h"
#include "SdlInputHandler.h"

#include "OgreTimer.h"
#include "Threading/OgreThreads.h"
#include "OgreRenderWindow.h"

#include "OgreRoot.h"
#include "Compositor/OgreCompositorManager2.h"
#include "OgreConfigFile.h"
#include "OgreArchiveManager.h"
#include "OgreArchive.h"

#include "OgreHlms.h"
#include "OgreHlmsPbsPrerequisites.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsDatablock.h"
#include "OgreHlmsManager.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"

using namespace Demo;

namespace Demo
{
    class Tutorial_SSAOGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                    "SSAOWorkspace", true );
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

            const char *c_locations[7] =
			{
				"2.0/scripts/materials/Tutorial_SSAO",
                "2.0/scripts/materials/Tutorial_SSAO/GLSL",
                "2.0/scripts/materials/Tutorial_SSAO/HLSL",
				"2.0/scripts/materials/PbsMaterials",
				"2.0/scripts/materials/Common",
				"2.0/scripts/materials/Common/GLSL",
				"2.0/scripts/materials/Common/HLSL"
			};

            for (size_t i = 0; i<7; ++i)
			{
				Ogre::String dataFolderFull = dataFolder + c_locations[i];
				addResourceLocation(dataFolderFull, "FileSystem", "General");
			}

        }

    public:
        Tutorial_SSAOGraphicsSystem( GameState *gameState ) :
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
    Tutorial_SSAOGameState tutorial_SSAOGameState(
		"This tutorial shows how to make depth-only horizontal based amibent occlusion (HBAO).\n"
        "The sample uses the compositor feature 'quad_normals camera_far_corners_view_space'\n"
        "in combination with a special shader so that we can unproject the depth sampled\n"
        "from the depth buffer and combine it with the frustum corners to get the view space\n"
        "projection.\n"
        "There is also example in the shader how to reconstruct view space normals from fragment position.\n"
        "Normals are used in SSAO to rotate hemisphere sample kernel pointing outwards\n"
		"from fragment's face.\n"
		"There are three steps: calculate occlusion (SSAO_HS_ps.glsl/hlsl),\n"
		"blur (SSAO_BlurV/H_ps.glsl/hlsl) and apply (SSAO_Apply_ps.glsl/hlsl).\n"
	);
    Tutorial_SSAOGraphicsSystem graphicsSystem( &tutorial_SSAOGameState );

    tutorial_SSAOGameState._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem.initialize( "SSAO Demo" );

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
