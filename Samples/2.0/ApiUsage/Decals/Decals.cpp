
#include "GraphicsSystem.h"
#include "DecalsGameState.h"

#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "OgreConfigFile.h"
#include "Compositor/OgreCompositorManager2.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsTextureManager.h"

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
    class DecalsGraphicsSystem : public GraphicsSystem
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

        void reserveDecalTextures(void)
        {
            /*
            Decals support having up to 3 texture arrays, one for diffuse, normal and emissive maps.
            That means that all your diffuse texture must share the same resolution & format.

            You CAN create the texture normally via TextureManager by creating a TEX_TYPE_2D_ARRAY
            texture and managing the slices yourself. There is no need to depend on the
            HlmsTextureManager, other than being a convenience.

            We must reserve the textures and load them first, before initialiseAllResourceGroups
            loads materials that may end up putting the textures in a different array.

            If we do not do this, then we have to assign them a different alias to load it twice
            on RAM, e.g. call
            hlmsTextureManager->createOrRetrieveTexture( "different_name_floor_bump",
                                                         "floor_bump.PNG",... );
            */
            Ogre::HlmsManager *hlmsManager = mRoot->getHlmsManager();
            Ogre::HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();
            /*
            These pool IDs must be unique per texture array, to ensure textures go into the
            right array.
            However you'll see here that both decalDiffuseId & decalNormalId are the same.
            This is because normal maps, being TEXTURE_TYPE_NORMALS and of a different format
            (RG8_SNORM instead of RGBA8_UNORM) will end up in different arrays anyway.
            So the reasons they will end up in different pools anyway is because:
                1. One uses TEXTURE_TYPE_DIFFUSE, the other TEXTURE_TYPE_NORMALS
                2. One uses RGBA8_UNORM, the other RG8_SNORM

            For example if you want to load diffuse decals into one array, and textures for
            area lights into a different array, then you would use different pool IDs:
                decalDiffuseId = 1;
                areaLightsId = 2;
            */
            const Ogre::uint32 decalDiffuseId = 1;
            const Ogre::uint32 decalNormalId = 1;

            hlmsTextureManager->reservePoolId( decalDiffuseId, Ogre::HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,
                                               512u, 512u, 8u, 9u, Ogre::PF_A8R8G8B8, false, true );
            hlmsTextureManager->reservePoolId( decalNormalId, Ogre::HlmsTextureManager::TEXTURE_TYPE_NORMALS,
                                               512u, 512u, 8u, 9u, Ogre::PF_R8G8_SNORM, true, false );

            /*
                Create a blank diffuse & normal map textures, so we can use index 0 to "disable" them
                if we want them disabled for a particular Decal.
                This is not necessary if you intend to have all your decals using both diffuse
                and normals.
            */
            Ogre::uint8 *blackBuffer = reinterpret_cast<Ogre::uint8*>(
                                           OGRE_MALLOC( 512u * 512u * 4u, Ogre::MEMCATEGORY_RESOURCE ) );
            memset( blackBuffer, 0, 512u * 512u * 4u );
            Ogre::Image blackImage;
            blackImage.loadDynamicImage( blackBuffer, 512u, 512u, 1u, Ogre::PF_A8R8G8B8, true );
            hlmsTextureManager->createOrRetrieveTexture( "decals_disabled_diffuse",
                                                         "decals_disabled_diffuse",
                                                         Ogre::HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,
                                                         decalDiffuseId, &blackImage );
            //128 == 0 in Normal maps, because of signedness
            blackImage.freeMemory();
            blackBuffer = reinterpret_cast<Ogre::uint8*>(
                              OGRE_MALLOC( 512u * 512u * 2u, Ogre::MEMCATEGORY_RESOURCE ) );
            memset( blackBuffer, 0, 512u * 512u * 2u );
            blackImage.loadDynamicImage( blackBuffer, 512u, 512u, 1u, Ogre::PF_R8G8_SNORM, true );
            hlmsTextureManager->createOrRetrieveTexture( "decals_disabled_normals",
                                                         "decals_disabled_normals",
                                                         Ogre::HlmsTextureManager::TEXTURE_TYPE_NORMALS,
                                                         decalNormalId, &blackImage );

            /*
                Now actually load the decals we want into the array.
                Note aliases are all lowercase! Ogre automatically aliases
                all resources as lowercase, thus we need to do that too, or else
                the texture will end up being loaded twice
            */
            hlmsTextureManager->createOrRetrieveTexture( "floor_diffuse.png", "floor_diffuse.PNG",
                                                         Ogre::HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,
                                                         decalDiffuseId );
            hlmsTextureManager->createOrRetrieveTexture( "floor_bump.png", "floor_bump.PNG",
                                                         Ogre::HlmsTextureManager::TEXTURE_TYPE_NORMALS,
                                                         decalNormalId );
        }

        virtual void loadResources(void)
        {
            registerHlms();

            loadHlmsDiskCache();

            reserveDecalTextures();

            // Initialise, parse scripts etc
            Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups( true );
        }

    public:
        DecalsGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        DecalsGameState *gfxGameState = new DecalsGameState(
        ""
        "\n"
        "LEGAL: Uses Saint Peter's Basilica (C) by Emil Persson under CC Attrib 3.0 Unported\n"
        "See Samples/Media/materials/textures/Cubemaps/License.txt for more information." );

        GraphicsSystem *graphicsSystem = new DecalsGraphicsSystem( gfxGameState );

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
        return "Screen Space Decals";
    }
}
