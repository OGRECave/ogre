
#ifndef _Demo_StencilTestGameState_H_
#define _Demo_StencilTestGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Demo
{
    class StencilTestGameState : public TutorialGameState
    {
        Ogre::SceneNode     *mSceneNode;

    public:
        StencilTestGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );
    };
}

#endif
