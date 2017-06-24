
#include "GraphicsSystem.h"
#include "PlanarReflectionsGameState.h"

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
    class PlanarReflectionsGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            mWorkspace = compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                          "PlanarReflectionStandardsWorkspace", true );
            return mWorkspace;
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

            const char *c_locations[1] =
            {
                "2.0/scripts/materials/PlanarReflections"
            };

            for( size_t i=0; i<1; ++i )
            {
                Ogre::String dataFolder = originalDataFolder + c_locations[i];
                addResourceLocation( dataFolder, "FileSystem", "General" );
            }
        }

    public:
        PlanarReflectionsGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        PlanarReflectionsGameState *gfxGameState = new PlanarReflectionsGameState(
        "Shows how to use Planar Reflections.\n"
        "Planar Reflections can be used with both Unlit and PBS, but they're setup\n"
        "differently. Unlit is very fast, but also very basic. It's mostly useful for\n"
        "perfect mirrors. An unlit datablock is tied to a particular Actor (reflection plane).\n"
        "PBS on the other hand, can dynamically assign Renderables to the closest actor that\n"
        "aligns with the Renderable's (predominant) normal, regardless of the pbs datablock\n"
        "it uses. If the actor is close enough but not an exact match, PBS will attempt to\n"
        "project the reflection in an attempt to correct this, however it's an approximation.\n"
        "If you want perfect reflections, the reflection plane of the actor must match and align\n"
        "exactly the surface being drawn; or rely on a fallback (Local Cubemaps, SSR, etc).\n"
        "Furthermore, PBS will automatically disable reflections while rendering reflections\n"
        "(but Unlit won't do this, so it's your job to leave it out of rendering i.e.\n"
        "via visibility masks).\n"
        "\n"
        "Actors are culled against the camera, thus if they're no longer visible Ogre will\n"
        "stop updating those actors, improving performance.\n"
        "\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/PlanarReflections.compositor\n"
        "   * Samples/Media/2.0/scripts/materials/PlanarReflections/*.*\n" );

        GraphicsSystem *graphicsSystem = new PlanarReflectionsGraphicsSystem( gfxGameState );

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
        return "Planar Reflections";
    }
}
