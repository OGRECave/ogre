
#ifndef _CameraController_H_
#define _CameraController_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Demo
{
    class CameraController
    {
        bool                mUseSceneNode;

        bool                mWASD[4];
        float               mCameraYaw;
        float               mCameraPitch;

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
