#include "OgreCameraMan.h"
#include "OgreSceneManager.h"

namespace OgreBites {

CameraMan::CameraMan(Ogre::SceneNode *cam)
    : mYawSpace(Ogre::Node::TS_PARENT)
    , mCamera(0)
    , mStyle(CS_MANUAL)
    , mTarget(0)
    , mOrbiting(false)
    , mMoving(false)
    , mTopSpeed(150)
    , mVelocity(Ogre::Vector3::ZERO)
    , mGoingForward(false)
    , mGoingBack(false)
    , mGoingLeft(false)
    , mGoingRight(false)
    , mGoingUp(false)
    , mGoingDown(false)
    , mFastMove(false)
    , mOffset(0, 0, 0)
{

    setCamera(cam);
    setStyle(CS_FREELOOK);
}

void CameraMan::setCamera(Ogre::SceneNode *cam)
{
    mCamera = cam;
}

void CameraMan::setTarget(Ogre::SceneNode *target)
{
    if (target == mTarget)
        return;

    mTarget = target;
}

void CameraMan::setYawPitchDist(const Ogre::Radian&  yaw, const Ogre::Radian& pitch, Ogre::Real dist)
{
    OgreAssert(mTarget, "no target set");

    mOffset = Ogre::Vector3::ZERO;
    mCamera->setPosition(mTarget->_getDerivedPosition());
    mCamera->setOrientation(mTarget->_getDerivedOrientation());
    mCamera->yaw(yaw);
    mCamera->pitch(-pitch);
    mCamera->translate(Ogre::Vector3(0, 0, dist), Ogre::Node::TS_LOCAL);
}

void CameraMan::setStyle(CameraStyle style)
{
    if (mStyle != CS_ORBIT && style == CS_ORBIT)
    {
        setTarget(mTarget ? mTarget : mCamera->getCreator()->getRootSceneNode());
        // fix the yaw axis if requested
        mCamera->setFixedYawAxis(mYawSpace == Ogre::Node::TS_PARENT);
        manualStop();

        // try to replicate the camera configuration
        Ogre::Real dist = getDistToTarget();
        const Ogre::Quaternion& q = mCamera->getOrientation();
        setYawPitchDist(q.getYaw(), q.getPitch(), dist == 0 ? 150 : dist); // enforce some distance
    }
    else if (mStyle != CS_FREELOOK && style == CS_FREELOOK)
    {
        mCamera->setFixedYawAxis(true); // also fix axis with lookAt calls
    }
    else if (mStyle != CS_MANUAL && style == CS_MANUAL)
    {
        manualStop();
    }
    mStyle = style;
    mCamera->setAutoTracking(false);
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

void CameraMan::frameRendered(const Ogre::FrameEvent &evt)
{
    if (mStyle == CS_FREELOOK)
    {
        // build our acceleration vector based on keyboard input composite
        Ogre::Vector3 accel = Ogre::Vector3::ZERO;
        Ogre::Matrix3 axes = mCamera->getLocalAxes();
        if (mGoingForward) accel -= axes.GetColumn(2);
        if (mGoingBack) accel += axes.GetColumn(2);
        if (mGoingRight) accel += axes.GetColumn(0);
        if (mGoingLeft) accel -= axes.GetColumn(0);
        if (mGoingUp) accel += axes.GetColumn(1);
        if (mGoingDown) accel -= axes.GetColumn(1);

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

        if (mVelocity != Ogre::Vector3::ZERO) mCamera->translate(mVelocity * evt.timeSinceLastFrame);
    }
}

bool CameraMan::keyPressed(const KeyboardEvent &evt)
{
    if (mStyle == CS_FREELOOK)
    {
        Keycode key = evt.keysym.sym;
        if (key == 'w' || key == SDLK_UP) mGoingForward = true;
        else if (key == 's' || key == SDLK_DOWN) mGoingBack = true;
        else if (key == 'a' || key == SDLK_LEFT) mGoingLeft = true;
        else if (key == 'd' || key == SDLK_RIGHT) mGoingRight = true;
        else if (key == SDLK_PAGEUP) mGoingUp = true;
        else if (key == SDLK_PAGEDOWN) mGoingDown = true;
        else if (key == SDLK_LSHIFT) mFastMove = true;
    }

    return InputListener::keyPressed(evt);
}

bool CameraMan::keyReleased(const KeyboardEvent &evt)
{
    if (mStyle == CS_FREELOOK)
    {
        Keycode key = evt.keysym.sym;
        if (key == 'w' || key == SDLK_UP) mGoingForward = false;
        else if (key == 's' || key == SDLK_DOWN) mGoingBack = false;
        else if (key == 'a' || key == SDLK_LEFT) mGoingLeft = false;
        else if (key == 'd' || key == SDLK_RIGHT) mGoingRight = false;
        else if (key == SDLK_PAGEUP) mGoingUp = false;
        else if (key == SDLK_PAGEDOWN) mGoingDown = false;
        else if (key == SDLK_LSHIFT) mFastMove = false;
    }

    return InputListener::keyReleased(evt);
}

Ogre::Real CameraMan::getDistToTarget()
{
    Ogre::Vector3 offset = mCamera->getPosition() - mTarget->_getDerivedPosition() - mOffset;
    return offset.length();
}

void CameraMan::setPivotOffset(const Ogre::Vector3& pivot)
{
    Ogre::Real dist = getDistToTarget();
    mOffset = pivot;
    mCamera->setPosition(mTarget->_getDerivedPosition() + mOffset);
    mCamera->translate(Ogre::Vector3(0, 0, dist), Ogre::Node::TS_LOCAL);
}

bool CameraMan::mouseMoved(const MouseMotionEvent &evt)
{
    if (mStyle == CS_ORBIT)
    {
        Ogre::Real dist = getDistToTarget();

        if (mOrbiting)   // yaw around the target, and pitch locally
        {
            mCamera->setPosition(mTarget->_getDerivedPosition() + mOffset);

            mCamera->yaw(Ogre::Degree(-evt.xrel * 0.25f), mYawSpace);
            mCamera->pitch(Ogre::Degree(-evt.yrel * 0.25f));

            mCamera->translate(Ogre::Vector3(0, 0, dist), Ogre::Node::TS_LOCAL);
            // don't let the camera go over the top or around the bottom of the target
        }
        else if (mMoving)  // move the camera along the image plane
        {
            Ogre::Vector3 delta = mCamera->getOrientation() * Ogre::Vector3(-evt.xrel, evt.yrel, 0);
            // the further the camera is, the faster it moves
            delta *= dist / 1000.0f;
            mOffset += delta;
            mCamera->translate(delta);
        }
    }
    else if (mStyle == CS_FREELOOK)
    {
        mCamera->yaw(Ogre::Degree(-evt.xrel * 0.15f), Ogre::Node::TS_PARENT);
        mCamera->pitch(Ogre::Degree(-evt.yrel * 0.15f));
    }

    return InputListener::mouseMoved(evt);
}

bool CameraMan::mouseWheelRolled(const MouseWheelEvent &evt) {
    if (mStyle == CS_ORBIT && evt.y != 0)
    {
        Ogre::Real dist = (mCamera->getPosition() - mTarget->_getDerivedPosition()).length();
        mCamera->translate(Ogre::Vector3(0, 0, -evt.y * 0.08f * dist), Ogre::Node::TS_LOCAL);
    }

    return InputListener::mouseWheelRolled(evt);
}

bool CameraMan::mousePressed(const MouseButtonEvent &evt)
{
    if (mStyle == CS_ORBIT)
    {
        if (evt.button == BUTTON_LEFT) mOrbiting = true;
        else if (evt.button == BUTTON_RIGHT) mMoving = true;
    }

    return InputListener::mousePressed(evt);
}

bool CameraMan::mouseReleased(const MouseButtonEvent &evt)
{
    if (mStyle == CS_ORBIT)
    {
        if (evt.button == BUTTON_LEFT) mOrbiting = false;
        else if (evt.button == BUTTON_RIGHT) mMoving = false;
    }

    return InputListener::mouseReleased(evt);
}

}
