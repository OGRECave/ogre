#include "OgreCameraMan.h"
#include "OgreSceneManager.h"

namespace OgreBites {

CameraMan::CameraMan(Ogre::Camera *cam)
    : mCamera(0)
    , mTarget(0)
    , mOrbiting(false)
    , mZooming(false)
    , mTopSpeed(150)
    , mVelocity(Ogre::Vector3::ZERO)
    , mGoingForward(false)
    , mGoingBack(false)
    , mGoingLeft(false)
    , mGoingRight(false)
    , mGoingUp(false)
    , mGoingDown(false)
    , mFastMove(false)
{

    setCamera(cam);
    setStyle(CS_FREELOOK);
}

void CameraMan::setCamera(Ogre::Camera *cam)
{
    mCamera = cam;
}

void CameraMan::setTarget(Ogre::SceneNode *target)
{
    if (target != mTarget)
    {
        mTarget = target;
        if(target)
        {
            setYawPitchDist(Ogre::Degree(0), Ogre::Degree(15), 150);
            mCamera->setAutoTracking(true, mTarget);
        }
        else
        {
            mCamera->setAutoTracking(false);
        }

    }


}

void CameraMan::setYawPitchDist(Ogre::Radian yaw, Ogre::Radian pitch, Ogre::Real dist)
{
    mCamera->setPosition(mTarget->_getDerivedPosition());
    mCamera->setOrientation(mTarget->_getDerivedOrientation());
    mCamera->yaw(yaw);
    mCamera->pitch(-pitch);
    mCamera->moveRelative(Ogre::Vector3(0, 0, dist));
}

void CameraMan::setStyle(CameraStyle style)
{
    if (mStyle != CS_ORBIT && style == CS_ORBIT)
    {
        setTarget(mTarget ? mTarget : mCamera->getSceneManager()->getRootSceneNode());
        mCamera->setFixedYawAxis(true);
        manualStop();
        setYawPitchDist(Ogre::Degree(0), Ogre::Degree(15), 150);
    }
    else if (mStyle != CS_FREELOOK && style == CS_FREELOOK)
    {
        mCamera->setAutoTracking(false);
        mCamera->setFixedYawAxis(true);
    }
    else if (mStyle != CS_MANUAL && style == CS_MANUAL)
    {
        mCamera->setAutoTracking(false);
        manualStop();
    }
    mStyle = style;

}

void CameraMan::manualStop()
{
    if (mStyle == CS_FREELOOK)
    {
        mGoingForward = false;
        mGoingBack = false;
        mGoingLeft = false;
        mGoingRight = false;
        mGoingUp = false;
        mGoingDown = false;
        mVelocity = Ogre::Vector3::ZERO;
    }
}

bool CameraMan::frameRenderingQueued(const Ogre::FrameEvent &evt)
{
    if (mStyle == CS_FREELOOK)
    {
        // build our acceleration vector based on keyboard input composite
        Ogre::Vector3 accel = Ogre::Vector3::ZERO;
        if (mGoingForward) accel += mCamera->getDirection();
        if (mGoingBack) accel -= mCamera->getDirection();
        if (mGoingRight) accel += mCamera->getRight();
        if (mGoingLeft) accel -= mCamera->getRight();
        if (mGoingUp) accel += mCamera->getUp();
        if (mGoingDown) accel -= mCamera->getUp();

        // if accelerating, try to reach top speed in a certain time
        Ogre::Real topSpeed = mFastMove ? mTopSpeed * 20 : mTopSpeed;
        if (accel.squaredLength() != 0)
        {
            accel.normalise();
            mVelocity += accel * topSpeed * evt.timeSinceLastFrame * 10;
        }
        // if not accelerating, try to stop in a certain time
        else mVelocity -= mVelocity * evt.timeSinceLastFrame * 10;

        Ogre::Real tooSmall = std::numeric_limits<Ogre::Real>::epsilon();

        // keep camera velocity below top speed and above epsilon
        if (mVelocity.squaredLength() > topSpeed * topSpeed)
        {
            mVelocity.normalise();
            mVelocity *= topSpeed;
        }
        else if (mVelocity.squaredLength() < tooSmall * tooSmall)
            mVelocity = Ogre::Vector3::ZERO;

        if (mVelocity != Ogre::Vector3::ZERO) mCamera->move(mVelocity * evt.timeSinceLastFrame);
    }

    return true;
}

void CameraMan::injectKeyDown(const KeyboardEvent &evt)
{
    if (mStyle == CS_FREELOOK)
    {
        Keycode key = evt.keysym.scancode;
        if (key == SDL_SCANCODE_W || key == SDL_SCANCODE_UP) mGoingForward = true;
        else if (key == SDL_SCANCODE_S || key == SDL_SCANCODE_DOWN) mGoingBack = true;
        else if (key == SDL_SCANCODE_A || key == SDL_SCANCODE_LEFT) mGoingLeft = true;
        else if (key == SDL_SCANCODE_D || key == SDL_SCANCODE_RIGHT) mGoingRight = true;
        else if (key == SDL_SCANCODE_PAGEUP) mGoingUp = true;
        else if (key == SDL_SCANCODE_PAGEDOWN) mGoingDown = true;
        else if (key == SDL_SCANCODE_LSHIFT) mFastMove = true;
    }
}

void CameraMan::injectKeyUp(const KeyboardEvent &evt)
{
    if (mStyle == CS_FREELOOK)
    {
        Keycode key = evt.keysym.scancode;
        if (key == SDL_SCANCODE_W || key == SDL_SCANCODE_UP) mGoingForward = false;
        else if (key == SDL_SCANCODE_S || key == SDL_SCANCODE_DOWN) mGoingBack = false;
        else if (key == SDL_SCANCODE_A || key == SDL_SCANCODE_LEFT) mGoingLeft = false;
        else if (key == SDL_SCANCODE_D || key == SDL_SCANCODE_RIGHT) mGoingRight = false;
        else if (key == SDL_SCANCODE_PAGEUP) mGoingUp = false;
        else if (key == SDL_SCANCODE_PAGEDOWN) mGoingDown = false;
        else if (key == SDL_SCANCODE_LSHIFT) mFastMove = false;
    }
}

void CameraMan::injectMouseMove(const MouseMotionEvent &evt)
{
    if (mStyle == CS_ORBIT)
    {
        Ogre::Real dist = (mCamera->getPosition() - mTarget->_getDerivedPosition()).length();

        if (mOrbiting)   // yaw around the target, and pitch locally
        {
            mCamera->setPosition(mTarget->_getDerivedPosition());

            mCamera->yaw(Ogre::Degree(-evt.xrel * 0.25f));
            mCamera->pitch(Ogre::Degree(-evt.yrel * 0.25f));

            mCamera->moveRelative(Ogre::Vector3(0, 0, dist));

            // don't let the camera go over the top or around the bottom of the target
        }
        else if (mZooming)  // move the camera toward or away from the target
        {
            // the further the camera is, the faster it moves
            mCamera->moveRelative(Ogre::Vector3(0, 0, evt.yrel * 0.004f * dist));
        }
    }
    else if (mStyle == CS_FREELOOK)
    {
        mCamera->yaw(Ogre::Degree(-evt.xrel * 0.15f));
        mCamera->pitch(Ogre::Degree(-evt.yrel * 0.15f));
    }
}

void CameraMan::injectMouseWheel(const MouseWheelEvent &evt) {
    if (mStyle == CS_ORBIT && evt.y != 0)
    {
        Ogre::Real dist = (mCamera->getPosition() - mTarget->_getDerivedPosition()).length();
        mCamera->moveRelative(Ogre::Vector3(0, 0, -evt.y * 0.08f * dist));
    }
}

void CameraMan::injectMouseDown(const MouseButtonEvent &evt)
{
    if (mStyle == CS_ORBIT)
    {
        if (evt.button == BUTTON_LEFT) mOrbiting = true;
        else if (evt.button == BUTTON_RIGHT) mZooming = true;
    }
}

void CameraMan::injectMouseUp(const MouseButtonEvent &evt)
{
    if (mStyle == CS_ORBIT)
    {
        if (evt.button == BUTTON_LEFT) mOrbiting = false;
        else if (evt.button == BUTTON_RIGHT) mZooming = false;
    }
}

}
