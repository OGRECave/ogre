
#ifndef _Demo_StereoRenderingGameState_H_
#define _Demo_StereoRenderingGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Demo
{
    class StereoRenderingGameState : public TutorialGameState
    {
        Ogre::SceneNode     *mSceneNode[16];

        bool                mWASD[4];

    public:
        StereoRenderingGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );

        virtual void keyPressed( const SDL_KeyboardEvent &arg );
        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
