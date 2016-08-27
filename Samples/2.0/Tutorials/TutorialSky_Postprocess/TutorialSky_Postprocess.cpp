
#include "GraphicsSystem.h"
#include "TutorialSky_PostprocessGameState.h"

#include "OgreRenderWindow.h"

#include "OgreRoot.h"
#include "Compositor/OgreCompositorManager2.h"
#include "OgreConfigFile.h"

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
    class TutorialSky_PostprocessGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                    "TutorialSky_PostprocessWorkspace", true );
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

            dataFolder += "2.0/scripts/materials/TutorialSky_Postprocess";

            addResourceLocation( dataFolder, "FileSystem", "General" );
        }

    public:
        TutorialSky_PostprocessGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };
    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        TutorialSky_PostprocessGameState *gfxGameState = new TutorialSky_PostprocessGameState(
        "Shows how to create a sky as simple postprocess effect.\n"
        "The vertex shader ensures the depth is always = 1, and the pixel shader\n"
        "takes a cubemap texture.\n"
        "The magic is in the compositor feature 'quad_normals camera_direction' which\n"
        "sends the data needed by the cubemap lookup via normals. This data can also\n"
        "be used for realtime analytical atmosphere scattering shaders.\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/TutorialSky_Postprocess.compositor\n"
        "   * Samples/Media/2.0/materials/TutorialSky_Postprocess/*.*\n"
        "\n"
        "LEGAL: Uses Saint Peter's Basilica (C) by Emil Persson under CC Attrib 3.0 Unported\n"
        "See Samples/Media/materials/textures/Cubemaps/License.txt for more information." );

        TutorialSky_PostprocessGraphicsSystem *graphicsSystem =
                new TutorialSky_PostprocessGraphicsSystem( gfxGameState );

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
        return "Rendering Sky as a postprocess with a single shader";
    }
}
