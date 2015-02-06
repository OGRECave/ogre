
#include "GraphicsSystem.h"
#include "StereoRenderingGameState.h"

#include "OgreTimer.h"
#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "Compositor/OgreCompositorManager2.h"

using namespace Demo;

namespace Demo
{
    class StereoGraphicsSystem : public GraphicsSystem
    {
        Ogre::SceneNode             *mCameraNode;
        Ogre::Camera                *mEyeCameras[2];
        Ogre::CompositorWorkspace   *mEyeWorkspaces[2];

        //-------------------------------------------------------------------------------
        virtual void createCamera(void)
        {
            //Use one node to control both cameras
            Ogre::SceneNode *camerasNode = mSceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                    createChildSceneNode( Ogre::SCENE_DYNAMIC );
            camerasNode->setName( "Cameras Node" );

            camerasNode->setPosition( 0, 5, 15 );

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
                camerasNode->attachObject( mEyeCameras[i] );
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
                                                                 true, -1,
                                                                 vpOffsetScale,
                                                                 vpModifierMask,
                                                                 executionMask );

            vpModifierMask  = 0x02;
            executionMask   = 0x02;
            vpOffsetScale   = Ogre::Vector4( 0.5f, 0.0f, 0.5f, 1.0f );
            mEyeWorkspaces[1] = compositorManager->addWorkspace( mSceneManager, mRenderWindow,
                                                                 mEyeCameras[1], workspaceName,
                                                                 true, -1,
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
}

int main()
{
    StereoRenderingGameState stereoGameState(
        "Shows the flexibility of the Compositor. Creates one workspace for each eye, using\n"
        "execution and viewport modifier masks to control which passes are rendered in which\n"
        "eyes, and also which ones are shown on the entire screen (i.e. clear, HUD)\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/StereoRendering.compositor\n"
        "For more information about stereo rendering and 'Stereo and Split-Screen Rendering'\n"
        "section of the 2.0 guide in the Docs/2.0 folder." );
    StereoGraphicsSystem graphicsSystem( &stereoGameState );

    stereoGameState._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem.initialize();

    Ogre::RenderWindow *renderWindow = graphicsSystem.getRenderWindow();

    graphicsSystem.createScene01();
    graphicsSystem.createScene02();

    Ogre::Timer timer;
    unsigned long startTime = timer.getMicroseconds();

    double timeSinceLast = 1.0 / 60.0;

    while( !graphicsSystem.getQuit() )
    {
        graphicsSystem.beginFrameParallel();
        graphicsSystem.update( static_cast<float>( timeSinceLast ) );
        graphicsSystem.finishFrameParallel();
        graphicsSystem.finishFrame();

        if( !renderWindow->isVisible() )
        {
            //Don't burn CPU cycles unnecessary when we're minimized.
            Ogre::Threads::Sleep( 500 );
        }

        unsigned long endTime = timer.getMicroseconds();
        timeSinceLast = (endTime - startTime) / 1000000.0;
        timeSinceLast = std::min( 1.0, timeSinceLast ); //Prevent from going haywire.
        startTime = endTime;
    }

    graphicsSystem.destroyScene();
    graphicsSystem.deinitialize();

    return 0;
}
