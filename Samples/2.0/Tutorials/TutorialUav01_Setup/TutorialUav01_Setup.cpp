
#include "GraphicsSystem.h"
#include "TutorialUav01_SetupGameState.h"

#include "OgreRenderWindow.h"

#include "OgreRoot.h"
#include "Compositor/OgreCompositorManager2.h"
#include "OgreConfigFile.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"
#include "System/MainEntryPoints.h"

namespace Demo
{
    class TutorialUav01_SetupGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                    "TutorialUav01_SetupWorkspace", true );
        }

        virtual void setupResources(void)
        {
            GraphicsSystem::setupResources();

            Ogre::ConfigFile cf;
            cf.load(mResourcePath + "resources2.cfg");

            Ogre::String originalDataFolder = cf.getSetting( "DoNotUseAsResource", "Hlms", "" );

            if( originalDataFolder.empty() )
                originalDataFolder = "./";
            else if( *(originalDataFolder.end() - 1) != '/' )
                originalDataFolder += "/";

            const char *c_locations[3] =
            {
                "2.0/scripts/materials/TutorialUav01_Setup",
                "2.0/scripts/materials/TutorialUav01_Setup/GLSL",
                "2.0/scripts/materials/TutorialUav01_Setup/HLSL",
            };

            for( size_t i=0; i<3; ++i )
            {
                Ogre::String dataFolder = originalDataFolder + c_locations[i];
                addResourceLocation( dataFolder, "FileSystem", "General" );
            }
        }

    public:
        TutorialUav01_SetupGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        TutorialUav01_SetupGameState *gfxGameState = new TutorialUav01_SetupGameState(
        "Shows how to setup an UAV (Unordered Access View).\n"
        "UAVs are complex and for advanced users, but they're very powerful and\n"
        "enable a whole new level of features and possibilities.\n"
        "This sample first fills an UAV with some data, then renders to screen\n"
        "sampling from it as a texture.\n"
        "We perform a RenderTarget-less render to the UAV via a helper PF_NULL\n"
        "texture that has a width and a height, but doesn't really occupy memory.\n"
        "When 'testTexture' is used as an UAV, you need to tell Ogre via uses_uav\n"
        "in the pass so Ogre can bake and issue the proper memory barriers\n"
        "(to avoid hazards).\n"
        "When it is used as a texture, there is no need as Ogre will automatically\n"
        "infer this information and issue the appropiate barrier.\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/TutorialUav01_Setup.compositor\n"
        "   * Samples/Media/2.0/materials/TutorialUav01_Setup/*.*\n"
        "\n" );

        GraphicsSystem *graphicsSystem = new TutorialUav01_SetupGraphicsSystem( gfxGameState );

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
        return "UAV Setup Example";
    }
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMainApp( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR strCmdLine, INT nCmdShow )
#else
int mainApp( int argc, const char *argv[] )
#endif
{
    return Demo::MainEntryPoints::mainAppSingleThreaded( DEMO_MAIN_ENTRY_PARAMS );
}
