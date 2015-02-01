
#ifndef _Demo_GraphicsGameState_H_
#define _Demo_GraphicsGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

#include "OgreVector3.h"

namespace Demo
{
    class GraphicsSystem;

    class GraphicsGameState : public TutorialGameState
    {
        bool        mEnableInterpolation;

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

    public:
        GraphicsGameState( const Ogre::String &helpDescription );

        virtual void update( float timeSinceLast );

        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
