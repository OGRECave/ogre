
#include "GraphicsSystem.h"
#include "ScreenSpaceReflectionsGameState.h"

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
    class ScreenSpaceReflectionsGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            const bool useMsaa = mRenderWindow->getFSAA() > 1;

            if( useMsaa && mRoot->getRenderSystem()->getName() == "OpenGL 3+ Rendering Subsystem" )
            {
                OGRE_EXCEPT( Ogre::Exception::ERR_NOT_IMPLEMENTED,
                             "MSAA + OpenGL is not yet implemented due to missing MSAA + MRT. "
                             "You'll have to wait until the texture refactor. It works OK on D3D11. "
                             "Sorry.",
                             "ScreenSpaceReflectionsGraphicsSystem" );
            }

            ScreenSpaceReflections::setupSSR( useMsaa, true, compositorManager );

            Ogre::String compositorName = "ScreenSpaceReflectionsWorkspace";
            if( useMsaa )
                compositorName = "ScreenSpaceReflectionsWorkspaceMsaa";

            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                    compositorName, true );
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

            const char *c_locations[4] =
            {
                "2.0/scripts/materials/ScreenSpaceReflections",
                "2.0/scripts/materials/ScreenSpaceReflections/GLSL",
                "2.0/scripts/materials/ScreenSpaceReflections/HLSL",
                "2.0/scripts/materials/ScreenSpaceReflections/Metal",
            };

            for( size_t i=0; i<4; ++i )
            {
                Ogre::String dataFolder = originalDataFolder + c_locations[i];
                addResourceLocation( dataFolder, "FileSystem", "General" );
            }
        }

    public:
        ScreenSpaceReflectionsGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        ScreenSpaceReflectionsGameState *gfxGameState = new ScreenSpaceReflectionsGameState(
        "TBD\n"
        "\n" );

        GraphicsSystem *graphicsSystem = new ScreenSpaceReflectionsGraphicsSystem( gfxGameState );

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
        return "Screen Space Reflections";
    }
}
