
#ifndef _Demo_Compute01GameState_H_
#define _Demo_Compute01GameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Demo
{
    class Compute01GameState : public TutorialGameState
    {
        Ogre::SceneNode     *mSceneNode;
        float               mDisplacement;

    public:
        Compute01GameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );
    };
}

#endif
