
#ifndef _Demo_EmptyProjectGameState_H_
#define _Demo_EmptyProjectGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Demo
{
	class EmptyProjectGameState : public TutorialGameState
    {
        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

    public:
		EmptyProjectGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );

        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
