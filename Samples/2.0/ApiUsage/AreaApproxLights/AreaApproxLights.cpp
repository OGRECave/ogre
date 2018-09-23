
#include "GraphicsSystem.h"
#include "AreaApproxLightsGameState.h"

#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "OgreConfigFile.h"
#include "Compositor/OgreCompositorManager2.h"

#include "OgreHlmsManager.h"
#include "OgreHlmsPbs.h"

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
    class AreaApproxLightsGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            mWorkspace = compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                          "ShadowMapDebuggingWorkspace", true );
            return mWorkspace;
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

        virtual void loadResources(void)
        {
            GraphicsSystem::loadResources();

            Ogre::Hlms *hlms = mRoot->getHlmsManager()->getHlms( Ogre::HLMS_PBS );
            OGRE_ASSERT_HIGH( dynamic_cast<Ogre::HlmsPbs*>( hlms ) );
            Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlms );
            hlmsPbs->loadLtcMatrix();
        }

    public:
        AreaApproxLightsGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        AreaApproxLightsGameState *gfxGameState = new AreaApproxLightsGameState(
        "Shows how to setup texture masks for the Area Light fake approximations.\n"
        "using Overlays and the Unlit Hlms implementation.\n"
        "Also shows photorealistic area lights using Linearly Transformed Cosines (LTC).\n"
        "LTC area lights currently do not support textures. Approximation does.\n"
        "Please note area lights use regular Forward (not Forward+) and cannot have\n"
        "shadow mapping.\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/ShadowMapDebugging.compositor" );

        GraphicsSystem *graphicsSystem = new AreaApproxLightsGraphicsSystem( gfxGameState );

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
        return "Area Lights (Fake / Approximation & Realistic / LTC)";
    }
}
