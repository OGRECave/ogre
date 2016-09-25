
#include "GraphicsSystem.h"
#include "StereoRenderingGameState.h"

#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreRenderWindow.h"
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
    class StereoGraphicsSystem : public GraphicsSystem
    {
        Ogre::SceneNode             *mCamerasNode;
        Ogre::Camera                *mEyeCameras[2];
        Ogre::CompositorWorkspace   *mEyeWorkspaces[2];

        //-------------------------------------------------------------------------------
        virtual void createCamera(void)
        {
            //Use one node to control both cameras
            mCamerasNode = mSceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                    createChildSceneNode( Ogre::SCENE_DYNAMIC );
            mCamerasNode->setName( "Cameras Node" );

            mCamerasNode->setPosition( 0, 5, 15 );

            mEyeCameras[0] = mSceneManager->createCamera( "Left Eye" );
            mEyeCameras[1] = mSceneManager->createCamera( "Right Eye" );

            const Ogre::Real eyeDistance        = 0.5f;
            const Ogre::Real eyeFocusDistance   = 0.45f;

            for( int i=0; i<2; ++i )
            {
                const Ogre::Vector3 camPos( eyeDistance * (i * 2 - 1), 0, 0 );
                mEyeCameras[i]->setPosition( camPos );

                Ogre::Vector3 lookAt( eyeFocusDistance * (i * 2 - 1), -5, -15 );
                //Ogre::Vector3 lookAt( 0, 0, 0 );

                // Look back along -Z
                mEyeCameras[i]->lookAt( lookAt );
                mEyeCameras[i]->setNearClipDistance( 0.2f );
                mEyeCameras[i]->setFarClipDistance( 1000.0f );
                mEyeCameras[i]->setAutoAspectRatio( true );

                //By default cameras are attached to the Root Scene Node.
                mEyeCameras[i]->detachFromParent();
                mCamerasNode->attachObject( mEyeCameras[i] );
            }

            mCamera = mEyeCameras[0];
        }

        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::uint8 vpModifierMask, executionMask;
            Ogre::Vector4 vpOffsetScale;

            const Ogre::IdString workspaceName( "StereoRenderingWorkspace" );
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();

            vpModifierMask  = 0x01;
            executionMask   = 0x01;
            vpOffsetScale   = Ogre::Vector4( 0.0f, 0.0f, 0.5f, 1.0f );
            mEyeWorkspaces[0] = compositorManager->addWorkspace( mSceneManager, mRenderWindow,
                                                                 mEyeCameras[0], workspaceName,
                                                                 true, -1, (Ogre::UavBufferPackedVec*)0,
                                                                 (Ogre::ResourceLayoutMap*)0,
                                                                 (Ogre::ResourceAccessMap*)0,
                                                                 vpOffsetScale,
                                                                 vpModifierMask,
                                                                 executionMask );

            vpModifierMask  = 0x02;
            executionMask   = 0x02;
            vpOffsetScale   = Ogre::Vector4( 0.5f, 0.0f, 0.5f, 1.0f );
            mEyeWorkspaces[1] = compositorManager->addWorkspace( mSceneManager, mRenderWindow,
                                                                 mEyeCameras[1], workspaceName,
                                                                 true, -1, (Ogre::UavBufferPackedVec*)0,
                                                                 (Ogre::ResourceLayoutMap*)0,
                                                                 (Ogre::ResourceAccessMap*)0,
                                                                 vpOffsetScale,
                                                                 vpModifierMask,
                                                                 executionMask);
            return mEyeWorkspaces[0];
        }

    public:
        StereoGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        StereoRenderingGameState *gfxGameState = new StereoRenderingGameState(
        "This tutorial demonstrates the most basic rendering loop: Variable framerate.\n"
        "Variable framerate means the application adapts to the current frame rendering\n"
        "performance and boosts or decreases the movement speed of objects to maintain\n"
        "the appearance that objects are moving at a constant velocity.\n"
        "When framerate is low, it looks 'frame skippy'; when framerate is high,\n"
        "it looks very smooth.\n"
        "Note: If you can't exceed 60 FPS, it's probably because of VSync being turned on.\n"
        "\n"
        "Despite what it seems, this is the most basic form of updating, and a horrible way\n"
        "to update your objects if you want to do any kind of serious game development.\n"
        "Keep going through the Tutorials for superior methods of updating the rendering loop.\n"
        "\n"
        "Note: The cube is black because there is no lighting. We are not focusing on that." );

        GraphicsSystem *graphicsSystem = new StereoGraphicsSystem( gfxGameState );

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
        return "Stereo Rendering Sample";
    }
}
