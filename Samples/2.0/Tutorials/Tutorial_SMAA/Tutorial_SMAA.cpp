
#include "GraphicsSystem.h"
#include "Tutorial_SMAAGameState.h"
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
#include "System/MainEntryPoints.h"

namespace Demo
{
    class Tutorial_SMAAGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                    "TutorialSMAA_Workspace", true );
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

            const char *c_locations[5] =
            {
                "2.0/scripts/materials/Tutorial_SMAA",
                "2.0/scripts/materials/Tutorial_SMAA/GLSL",
                "2.0/scripts/materials/Tutorial_SMAA/HLSL",
                "2.0/scripts/materials/Tutorial_SMAA/Metal",
                "2.0/scripts/materials/Tutorial_SMAA/TutorialCompositorScript",
            };

            for (size_t i = 0; i<5; ++i)
            {
                Ogre::String dataFolderFull = dataFolder + c_locations[i];
                addResourceLocation(dataFolderFull, "FileSystem", "General");
            }

        }

    public:
        Tutorial_SMAAGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        Tutorial_SMAAGameState *gfxGameState = new Tutorial_SMAAGameState(
        "TBD" );

        GraphicsSystem *graphicsSystem = new Tutorial_SMAAGraphicsSystem( gfxGameState );

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
        return "SMAA (Enhanced Subpixel Morphological Antialiasing) Demo";
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
