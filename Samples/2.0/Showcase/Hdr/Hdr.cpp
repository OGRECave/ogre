
#include "GraphicsSystem.h"
#include "HdrGameState.h"

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
    class HdrGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            Ogre::RenderSystem *renderSystem = mRoot->getRenderSystem();
            const Ogre::RenderSystemCapabilities *caps = renderSystem->getCapabilities();

            Ogre::String compositorName = "HdrWorkspace";
            if( mRenderWindow->getFSAA() > 1u && caps->hasCapability( Ogre::RSC_EXPLICIT_FSAA_RESOLVE ) )
                compositorName = "HdrWorkspaceMsaa";

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

            const char *c_locations[5] =
            {
                "2.0/scripts/materials/HDR",
                "2.0/scripts/materials/HDR/GLSL",
                "2.0/scripts/materials/HDR/HLSL",
                "2.0/scripts/materials/HDR/Metal",
                "2.0/scripts/materials/PbsMaterials"
            };

            for( size_t i=0; i<5; ++i )
            {
                Ogre::String dataFolder = originalDataFolder + c_locations[i];
                addResourceLocation( dataFolder, "FileSystem", "General" );
            }
        }

    public:
        HdrGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        HdrGameState *gfxGameState = new HdrGameState(
        "This samples shows the HDR (High Dynamic Range) pipeline in action\n"
        "HDR combined with PBR let us use real world values as input for our lighting and\n"
        "a real world scale such as lumen, lux and EV Stops (photography, in log2 space)\n"
        "The parameters for the presets have been taken from Wikipedia:\n"
        "   1. https://en.wikipedia.org/wiki/Exposure_value#Tabulated_exposure_values\n"
        "   2. https://en.wikipedia.org/wiki/Sunlight#Summary\n"
        "   3. http://lumennow.org/lumens-vs-watts/\n"
        "   4. http://www.greenbusinesslight.com/page/119/lux-lumens-and-watts\n"
        "\n"
        "We still need to tweak the parameters a little because we don't do real\n"
        "Global Illumination. However real world parameters are a great starting point.\n\n"
        "HDR can of course, also be used with artistic purposes.\n"
        "This sample lets you control the exposure, auto-exposure, and bloom parameters.\n\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/materials/Common/Copyback.material (Copyback_1xFP32_ps)\n"
        "   * Samples/Media/2.0/materials/HDR/*.*\n"
        "\n"
        "Additionally, this sample shows how to do high quality MSAA by using a reversible \n"
        "tonemap operator with a custom resolve. More information can be found at:\n"
        "   * https://mynameismjp.wordpress.com/2012/10/24/msaa-overview/\n"
        "   * http://graphicrants.blogspot.com.ar/2013/12/tone-mapping.html\n"
        "   * http://gpuopen.com/optimized-reversible-tonemapper-for-resolve/\n"
        "\n"
        "\n"
        "LEGAL: Uses Saint Peter's Basilica (C) by Emil Persson under CC Attrib 3.0 Unported\n"
        "See Samples/Media/materials/textures/Cubemaps/License.txt for more information.");

        GraphicsSystem *graphicsSystem = new HdrGraphicsSystem( gfxGameState );

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
        return "High Dynamic Range (HDR) Sample";
    }
}
