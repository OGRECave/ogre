
#include "GraphicsSystem.h"
#include "Tutorial_DistortionGameState.h"

#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "OgreConfigFile.h"
#include "Compositor/OgreCompositorManager2.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"

#include "System/MainEntryPoints.h"

namespace Demo
{
    class DistortionGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                    "DistortionWorkspace", true );
        }

        virtual void setupResources(void)
        {
            GraphicsSystem::setupResources();

            Ogre::ConfigFile cf;
            cf.load(mResourcePath + "resources2.cfg");

            Ogre::String originalDataFolder = cf.getSetting("DoNotUseAsResource", "Hlms", "");

            if (originalDataFolder.empty())
                originalDataFolder = "./";
            else if (*(originalDataFolder.end() - 1) != '/')
                originalDataFolder += "/";

            const char *c_locations[2] =
            {
                "2.0/scripts/materials/PbsMaterials",
                "2.0/scripts/materials/Distortion"
            };

            for (size_t i = 0; i<2; ++i)
            {
                Ogre::String dataFolder = originalDataFolder + c_locations[i];
                addResourceLocation(dataFolder, "FileSystem", "General");
            }
        }

    public:
        DistortionGraphicsSystem(GameState *gameState) :
            GraphicsSystem(gameState)
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        DistortionGameState *gfxGameState = new DistortionGameState(
        "This example shows how to make compositing setup that renders different parts of the scene to different textures.\n"
        "Here we will render distortion pass to its own texture and use shader to compose the scene and distortion pass.\n"
        "Distortion setup can be used to create blastwave effects, mix with fire particle effects to get heat distortion etc.\n"
        "You can use this setup with all kind of objects but in this example we are using only textured simple spheres\n"
        "For proper use, you should use particle systems to get better results.\n" );

        GraphicsSystem *graphicsSystem = new DistortionGraphicsSystem( gfxGameState );

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
        return "Distortion Sample";
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
