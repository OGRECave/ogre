
#include "GraphicsSystem.h"
#include "PbsMaterialsGameState.h"

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
    class PbsMaterialsGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                    "PbsMaterialsWorkspace", true );
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

            dataFolder += "2.0/scripts/materials/PbsMaterials";

            addResourceLocation( dataFolder, "FileSystem", "General" );
        }

    public:
        PbsMaterialsGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        PbsMaterialsGameState *gfxGameState = new PbsMaterialsGameState(
        "Shows how to use the PBS material system. There's nothing really fancy,\n"
        "it's just programmer art. The PBS materials can be setup from script or\n"
        "code. This sample does both. At the time being, not all settings from the\n"
        "PBS implementation can be tweaked with scripts. See PbsDatablock::PbsDatablock\n"
        "constructor documentation. Also see the Hlms section of the porting guide in\n"
        "the Docs/2.0 folder.\n"
        "\n"
        "The sphere palette shows what happens when tweaking the roughness around the\n"
        "X axis; and the fresnel term around the Z axis.\n"
        "The scene is being lit by a white directional light (3-split PSSM) and two spot\n"
        "lights, one of warm colour, one cold. Both are also shadowed."
        "\n"
        "Of all the features supported by the PBS implementation, perhaps the hardest to\n"
        "to understand is the Detail Weight Map. It allows you to 'paint' the detail maps\n"
        "over the mesh, by controlling weight of each of the 4 maps via the RGBA channels\n"
        "of the weight map. 'R' controls the detail map 0, 'G' the detail map 1,\n"
        "'B' the detail map 2, and 'A' the detail map 3.\n"
        "\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/PbsMaterials.compositor\n"
        "   * Samples/Media/2.0/materials/PbsMaterials/PbsMaterials.material\n"
        "\n"
        "Known issues:\n"
        " * Non shadow casting point & spot lights require Forward3D to be enabled (on desktop).\n"
        "   This is by design (more implementations will come: Forward+ & Deferred; for now the\n"
        "   only ones working are Forward3D and Forward Clustered).\n"
        "\n"
        "LEGAL: Uses Saint Peter's Basilica (C) by Emil Persson under CC Attrib 3.0 Unported\n"
        "See Samples/Media/materials/textures/Cubemaps/License.txt for more information." );

        GraphicsSystem *graphicsSystem = new PbsMaterialsGraphicsSystem( gfxGameState );

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
        return "PBS Materials Sample";
    }
}
