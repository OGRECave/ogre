
#include "GraphicsSystem.h"
#include "LocalCubemapsManualProbesGameState.h"

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
    class LocalCubemapsManualProbesGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            //Delegate compositor creation to the game state. We need cubemap's texture
            //to be passed to the compositor so Ogre can insert the proper barriers.
//            assert( dynamic_cast<LocalCubemapsManualProbesGameState*>(mCurrentGameState) );
//            return static_cast<LocalCubemapsManualProbesGameState*>(mCurrentGameState)->setupCompositor();

            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                    "LocalCubemapsWorkspace", true );
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

            const char *c_locations[9] =
            {
                "2.0/scripts/materials/Common",
                "2.0/scripts/materials/Common/GLSL",
                "2.0/scripts/materials/Common/HLSL",
                "2.0/scripts/materials/Common/Metal",
                "2.0/scripts/materials/LocalCubemaps/",
                "2.0/scripts/materials/LocalCubemaps/GLSL",
                "2.0/scripts/materials/LocalCubemaps/HLSL",
                "2.0/scripts/materials/LocalCubemaps/Metal",
                "2.0/scripts/materials/TutorialSky_Postprocess"
            };

            for( size_t i=0; i<9; ++i )
            {
                Ogre::String dataFolder = originalDataFolder + c_locations[i];
                addResourceLocation( dataFolder, "FileSystem", "General" );
            }
        }

    public:
        LocalCubemapsManualProbesGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        LocalCubemapsManualProbesGameState *gfxGameState = new LocalCubemapsManualProbesGameState(
        "This sample shows how to use Parallax Reflect Cubemaps for accurate local reflections.\n"
        "First, you'll have to create several probes with parametrized sizes, and the system\n"
        "will transition between the areas as you move around\n"
        "\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/LocalCubemaps.compositor\n"
        "   * Samples/Media/2.0/materials/LocalCubemaps/*\n"
        "\n" );

        GraphicsSystem *graphicsSystem = new LocalCubemapsManualProbesGraphicsSystem( gfxGameState );

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
        return "Local Reflections using Parallax Corrected Cubemaps";
    }
}
