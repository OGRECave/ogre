
#include "GraphicsSystem.h"
#include "Tutorial_DistortionGameState.h"
#include "SdlInputHandler.h"

#include "OgreTimer.h"
#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "OgreConfigFile.h"
#include "Compositor/OgreCompositorManager2.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"

using namespace Demo;

namespace Demo
{
	class DistortionGraphicsSystem : public GraphicsSystem
	{
		virtual Ogre::CompositorWorkspace* setupCompositor()
		{
			Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
			return compositorManager->addWorkspace(mSceneManager, mRenderWindow, mCamera,
				"DistortionWorkspace", true);
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

			const char *c_locations[5] =
			{
				"2.0/scripts/materials/Common",
				"2.0/scripts/materials/Common/GLSL",
				"2.0/scripts/materials/Common/HLSL",
				"2.0/scripts/materials/PbsMaterials",
				"2.0/scripts/materials/Distortion"
			};


			for (size_t i = 0; i<5; ++i)
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
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMainApp(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
int mainApp()
#endif
{
	DistortionGameState distortionGameState(
		"This example shows how to make compositing setup that renders different parts of the scene to different textures.\n"
		"Here we will render distortion pass to its own texture and use shader to compose the scene and distortion pass.\n"
		"Distortion setup can be used to create blastwave effects, mix with fire particle effects to get heat distortion etc.\n"
		"You can use this setup with all kind of objects but in this example we are using only textured simple spheres\n"
		"For proper use, you should use particle systems to get better results.\n"
		);
	DistortionGraphicsSystem graphicsSystem(&distortionGameState);

	distortionGameState._notifyGraphicsSystem(&graphicsSystem);

	graphicsSystem.initialize("Distortion Sample");

	if (graphicsSystem.getQuit())
	{
		graphicsSystem.deinitialize();
		return 0; //User cancelled config
	}

	Ogre::RenderWindow *renderWindow = graphicsSystem.getRenderWindow();

	graphicsSystem.createScene01();
	graphicsSystem.createScene02();

	//Do this after creating the scene for easier the debugging (the mouse doesn't hide itself)
	SdlInputHandler *inputHandler = graphicsSystem.getInputHandler();
	inputHandler->setGrabMousePointer(true);
	inputHandler->setMouseVisible(false);

	Ogre::Timer timer;
	unsigned long startTime = timer.getMicroseconds();

	double timeSinceLast = 1.0 / 60.0;

	while (!graphicsSystem.getQuit())
	{
		graphicsSystem.beginFrameParallel();
		graphicsSystem.update(static_cast<float>(timeSinceLast));
		graphicsSystem.finishFrameParallel();
		graphicsSystem.finishFrame();

		if (!renderWindow->isVisible())
		{
			//Don't burn CPU cycles unnecessary when we're minimized.
			Ogre::Threads::Sleep(500);
		}

		unsigned long endTime = timer.getMicroseconds();
		timeSinceLast = (endTime - startTime) / 1000000.0;
		timeSinceLast = std::min(1.0, timeSinceLast); //Prevent from going haywire.
		startTime = endTime;
	}

	graphicsSystem.destroyScene();
	graphicsSystem.deinitialize();

	return 0;
}
