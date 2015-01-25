
#ifndef _Demo_MyGameState_H_
#define _Demo_MyGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Ogre
{
    namespace v1
    {
        class TextAreaOverlayElement;
    }
}

namespace Demo
{
    class GraphicsSystem;

    class MyGameState : public TutorialGameState
    {
        float               mDisplacement;

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

    public:
        MyGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );

        virtual void keyReleased (const SDL_KeyboardEvent &arg );
    };
}

#endif
