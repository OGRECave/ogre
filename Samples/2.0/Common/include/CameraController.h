
#ifndef _Demo_CameraController_H_
#define _Demo_CameraController_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Demo
{
    class CameraController
    {
        bool                mUseSceneNode;

        bool                mSpeedMofifier;
        bool                mWASD[4];
        bool                mSlideUpDown[2];
        float               mCameraYaw;
        float               mCameraPitch;
        public: float       mCameraBaseSpeed;
        public: float       mCameraSpeedBoost;

    private:
        GraphicsSystem      *mGraphicsSystem;

    public:
        CameraController( GraphicsSystem *graphicsSystem, bool useSceneNode=false );

        void update( float timeSinceLast );

        /// Returns true if we've handled the event
        bool keyPressed( const SDL_KeyboardEvent &arg );
        /// Returns true if we've handled the event
        bool keyReleased( const SDL_KeyboardEvent &arg );

        void mouseMoved( const SDL_Event &arg );
    };
}

#endif
