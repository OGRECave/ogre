
#include "GraphicsSystem.h"
#include "V2ManualObjectGameState.h"

#include "OgreRenderWindow.h"
#include "OgreRoot.h"
#include "OgreConfigFile.h"
#include "Compositor/OgreCompositorManager2.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"
#include "System/MainEntryPoints.h"

namespace Demo
{
    class ManualObjectGraphicsSystem : public GraphicsSystem
    {
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
        ManualObjectGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        V2ManualObjectGameState *gfxGameState = new V2ManualObjectGameState(
        "Original ManualObject allowed users to simply create custom meshes in runtime.\n"
        "The new one does the same thing, except some limitations arising from the method\n"
        "used to update vertex and index buffers. Previously you were able to change the data layout\n"
        "after calling beginUpdate(). This is no longer possible, as the buffers are bound in a persistent\n"
        "fashion and updated in-place, which means you need to keep the exact same data layout and size.\n"
        "Not complying to this would cause buffer overflow and most likely crash the application.\n"
        "To change the data layout or buffer size, call clear() and create the buffer from scratch by\n"
        "calling begin() again.");

        GraphicsSystem *graphicsSystem = new ManualObjectGraphicsSystem( gfxGameState );

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
        return "v2 ManualObject usage";
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
