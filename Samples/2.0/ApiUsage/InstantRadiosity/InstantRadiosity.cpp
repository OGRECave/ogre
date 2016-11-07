
#include "GraphicsSystem.h"
#include "InstantRadiosityGameState.h"

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
    class InstantRadiosityGraphicsSystem : public GraphicsSystem
    {
//        virtual Ogre::CompositorWorkspace* setupCompositor()
//        {
//            //Delegate compositor creation to the game state. We need cubemap's texture
//            //to be passed to the compositor so Ogre can insert the proper barriers.
////            assert( dynamic_cast<InstantRadiosityGameState*>(mCurrentGameState) );
////            return static_cast<InstantRadiosityGameState*>(mCurrentGameState)->setupCompositor();

//            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
//            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
//                                                    "InstantRadiosityWorkspace", true );
//        }

        /*virtual void setupResources(void)
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
                "2.0/scripts/materials/InstantRadiosity/",
                "2.0/scripts/materials/InstantRadiosity/GLSL",
                "2.0/scripts/materials/InstantRadiosity/HLSL",
                "2.0/scripts/materials/InstantRadiosity/Metal",
                "2.0/scripts/materials/TutorialSky_Postprocess"
            };

            for( size_t i=0; i<9; ++i )
            {
                Ogre::String dataFolder = originalDataFolder + c_locations[i];
                addResourceLocation( dataFolder, "FileSystem", "General" );
            }
        }*/

    public:
        InstantRadiosityGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
            mAlwaysAskForConfig = false;
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        InstantRadiosityGameState *gfxGameState = new InstantRadiosityGameState(
        "TBD\n"
        "\n" );

        GraphicsSystem *graphicsSystem = new InstantRadiosityGraphicsSystem( gfxGameState );

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
        return "Global Illumination using Instant Radiosity";
    }
}
