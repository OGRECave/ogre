
#include "CameraController.h"

#include "GraphicsSystem.h"

#include "OgreCamera.h"
#include "OgreRenderWindow.h"

using namespace Demo;

namespace Demo
{
    CameraController::CameraController( GraphicsSystem *graphicsSystem, bool useSceneNode ) :
        mUseSceneNode( useSceneNode ),
        mSpeedMofifier( false ),
        mCameraYaw( 0 ),
        mCameraPitch( 0 ),
        mCameraBaseSpeed( 10 ),
        mCameraSpeedBoost( 5 ),
        mGraphicsSystem( graphicsSystem )
    {
        memset( mWASD, 0, sizeof(mWASD) );
        memset( mSlideUpDown, 0, sizeof(mSlideUpDown) );
    }
    //-----------------------------------------------------------------------------------
    void CameraController::update( float timeSinceLast )
    {
        Ogre::Camera *camera = mGraphicsSystem->getCamera();

        if( mCameraYaw || mCameraPitch )
        {
            if( mUseSceneNode )
            {
                Ogre::Node *cameraNode = camera->getParentNode();

                // Update now as yaw needs the derived orientation.
                cameraNode->_getFullTransformUpdated();
                cameraNode->yaw( Ogre::Radian( mCameraYaw ), Ogre::Node::TS_WORLD );
                cameraNode->pitch( Ogre::Radian( mCameraPitch ) );
            }
            else
            {
                camera->yaw( Ogre::Radian( mCameraYaw ) );
                camera->pitch( Ogre::Radian( mCameraPitch ) );
            }

            mCameraYaw   = 0.0f;
            mCameraPitch = 0.0f;
        }

        int camMovementZ = mWASD[2] - mWASD[0];
        int camMovementX = mWASD[3] - mWASD[1];
        int slideUpDown = mSlideUpDown[0] - mSlideUpDown[1];

        if( camMovementZ || camMovementX || slideUpDown )
        {
            Ogre::Vector3 camMovementDir( camMovementX, slideUpDown, camMovementZ );
            camMovementDir.normalise();
            camMovementDir *= timeSinceLast * mCameraBaseSpeed * (1 + mSpeedMofifier * mCameraSpeedBoost);

            if( mUseSceneNode )
            {
                Ogre::Node *cameraNode = camera->getParentNode();
                cameraNode->translate( camMovementDir, Ogre::Node::TS_LOCAL );
            }
            else
            {
                camera->moveRelative( camMovementDir );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    bool CameraController::keyPressed( const SDL_KeyboardEvent &arg )
    {
        if( arg.keysym.sym == SDLK_LSHIFT )
            mSpeedMofifier = true;

        if( arg.keysym.sym == SDLK_w )
            mWASD[0] = true;
        else if( arg.keysym.sym == SDLK_a )
            mWASD[1] = true;
        else if( arg.keysym.sym == SDLK_s )
            mWASD[2] = true;
        else if( arg.keysym.sym == SDLK_d )
            mWASD[3] = true;
        else if( arg.keysym.sym == SDLK_PAGEUP )
            mSlideUpDown[0] = true;
        else if( arg.keysym.sym == SDLK_PAGEDOWN )
            mSlideUpDown[1] = true;
        else
            return false;

        return true;
    }
    //-----------------------------------------------------------------------------------
    bool CameraController::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( arg.keysym.sym == SDLK_LSHIFT )
            mSpeedMofifier = false;

        if( arg.keysym.sym == SDLK_w )
            mWASD[0] = false;
        else if( arg.keysym.sym == SDLK_a )
            mWASD[1] = false;
        else if( arg.keysym.sym == SDLK_s )
            mWASD[2] = false;
        else if( arg.keysym.sym == SDLK_d )
            mWASD[3] = false;
        else if( arg.keysym.sym == SDLK_PAGEUP )
            mSlideUpDown[0] = false;
        else if( arg.keysym.sym == SDLK_PAGEDOWN )
            mSlideUpDown[1] = false;
        else
            return false;

        return true;
    }
    //-----------------------------------------------------------------------------------
    void CameraController::mouseMoved( const SDL_Event &arg )
    {
        float width  = static_cast<float>( mGraphicsSystem->getRenderWindow()->getWidth() );
        float height = static_cast<float>( mGraphicsSystem->getRenderWindow()->getHeight() );

        mCameraYaw   += -arg.motion.xrel / width;
        mCameraPitch += -arg.motion.yrel / height;
    }
}
