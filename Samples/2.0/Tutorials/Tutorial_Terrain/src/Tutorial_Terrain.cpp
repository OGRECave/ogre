
#include "GraphicsSystem.h"
#include "Tutorial_TerrainGameState.h"

#include "OgreRenderWindow.h"

#include "OgreRoot.h"
#include "Compositor/OgreCompositorManager2.h"
#include "OgreConfigFile.h"

#include "Terra/Hlms/OgreHlmsTerra.h"
#include "OgreHlmsManager.h"
#include "OgreArchiveManager.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"
#include "System/MainEntryPoints.h"

namespace Demo
{
    class Tutorial_TerrainGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            //Delegate compositor creation to the game state. We need terra's shadow texture
            //to be passed to the compositor so Ogre can insert the proper barriers.
            assert( dynamic_cast<Tutorial_TerrainGameState*>(mCurrentGameState) );
            return static_cast<Tutorial_TerrainGameState*>(mCurrentGameState)->setupCompositor();
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
                "2.0/scripts/materials/Tutorial_Terrain",
                "2.0/scripts/materials/Tutorial_Terrain/GLSL",
                "2.0/scripts/materials/Tutorial_Terrain/HLSL",
                "2.0/scripts/materials/Postprocessing/SceneAssets"
            };

            for( size_t i=0; i<4; ++i )
            {
                Ogre::String dataFolder = originalDataFolder + c_locations[i];
                addResourceLocation( dataFolder, "FileSystem", "General" );
            }
        }

        virtual void registerHlms(void)
        {
            GraphicsSystem::registerHlms();

            Ogre::ConfigFile cf;
            cf.load(mResourcePath + "resources2.cfg");

            Ogre::String dataFolder = cf.getSetting( "DoNotUseAsResource", "Hlms", "" );

            if( dataFolder.empty() )
                dataFolder = "./";
            else if( *(dataFolder.end() - 1) != '/' )
                dataFolder += "/";

            Ogre::RenderSystem *renderSystem = mRoot->getRenderSystem();

            Ogre::String shaderSyntax = "GLSL";
            if( renderSystem->getName() == "Direct3D11 Rendering Subsystem" )
                shaderSyntax = "HLSL";

            Ogre::Archive *archiveLibrary = Ogre::ArchiveManager::getSingletonPtr()->load(
                            dataFolder + "Hlms/Common/" + shaderSyntax,
                            "FileSystem", true );
            Ogre::Archive *archiveLibraryAny = Ogre::ArchiveManager::getSingletonPtr()->load(
                            dataFolder + "Hlms/Common/Any",
                            "FileSystem", true );
            Ogre::Archive *archivePbsLibraryAny = Ogre::ArchiveManager::getSingletonPtr()->load(
                            dataFolder + "Hlms/Pbs/Any",
                            "FileSystem", true );
            Ogre::Archive *pbsLibrary = Ogre::ArchiveManager::getSingletonPtr()->load(
                            dataFolder + "Hlms/Pbs/" + shaderSyntax,
                            "FileSystem", true );

            Ogre::ArchiveVec library;
            library.push_back( archiveLibrary );
            library.push_back( archiveLibraryAny );
            library.push_back( archivePbsLibraryAny );
            library.push_back( pbsLibrary );

            Ogre::Archive *archiveTerra = Ogre::ArchiveManager::getSingletonPtr()->load(
                            dataFolder + "Hlms/Terra/" + shaderSyntax,
                            "FileSystem", true );
            Ogre::HlmsTerra *hlmsTerra = OGRE_NEW Ogre::HlmsTerra( archiveTerra, &library );
            Ogre::HlmsManager *hlmsManager = mRoot->getHlmsManager();
            hlmsManager->registerHlms( hlmsTerra );

            //Add Terra's piece files that customize the PBS implementation.
            //These pieces are coded so that they will be activated when
            //we set the HlmsPbsTerraShadows listener and there's an active Terra
            //(see Tutorial_TerrainGameState::createScene01)
            Ogre::Hlms *hlmsPbs = hlmsManager->getHlms( Ogre::HLMS_PBS );
            Ogre::Archive *archivePbs = hlmsPbs->getDataFolder();
            Ogre::ArchiveVec libraryPbs = hlmsPbs->getPiecesLibraryAsArchiveVec();
            libraryPbs.push_back( Ogre::ArchiveManager::getSingletonPtr()->load(
                                      dataFolder + "Hlms/Terra/" + shaderSyntax + "/PbsTerraShadows",
                                      "FileSystem", true ) );
            hlmsPbs->reloadFrom( archivePbs, &libraryPbs );
        }

    public:
        Tutorial_TerrainGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }

        virtual void createScene01()
        {
            GraphicsSystem::createScene01();
            //The first time setupCompositor got called, Terra wasn't ready yet.
            //Create the workspace again (will destroy previous workspace).
            mWorkspace = setupCompositor();
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        Tutorial_TerrainGameState *gfxGameState = new Tutorial_TerrainGameState(
        "--- !!! NOTE: THIS SAMPLE IS A WORK IN PROGRESS !!! ---\n"
        "This is an advanced tutorial that shows several things working together:\n"
        "   * Own Hlms implementation to render the terrain\n"
        "   * Vertex buffer-less rendering: The terrain is generated purely using SV_VertexID "
        "tricks and a heightmap texture\n"
        "   * Hlms customizations to PBS to make terrain shadows affect regular objects\n"
        "   * Compute Shaders to generate terrain shadows every frame\n"
        "   * Common terrain functionality such as loading the heightmap, generating normals, LOD.\n"
        "The Terrain system is called 'Terra' and has been isolated under the Terra folder like\n"
        "a component for easier copy-pasting into your own projects.\n\n"
        "Because every project has its own needs regarding terrain rendering, we're not providing\n"
        "Terra as an official Component, but rather as a tutorial; where users can copy paste our\n"
        "implementation and use it as is, or make their own tweaks or enhancements.\n\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/Tutorial_Terrain.compositor\n"
        "   * Samples/Media/2.0/materials/Tutorial_Terrain/*.*\n"
        "   * Samples/Media/2.0/materials/Common/GLSL/GaussianBlurBase_cs.glsl\n"
        "   * Samples/Media/2.0/materials/Common/HLSL/GaussianBlurBase_cs.hlsl\n"
        "   * Samples/Media/Hlms/Terra/*.*\n" );

        Tutorial_TerrainGraphicsSystem *graphicsSystem =
                new Tutorial_TerrainGraphicsSystem( gfxGameState );

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
        return "Tutorial: Terrain";
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
