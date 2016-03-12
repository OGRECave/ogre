
#ifndef _Demo_TutorialCompute02_UavBufferGameState_H_
#define _Demo_TutorialCompute02_UavBufferGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Demo
{
    class TutorialCompute02_UavBufferGameState : public TutorialGameState
    {
        Ogre::SceneNode     *mSceneNode;
        float               mDisplacement;

    public:
        TutorialCompute02_UavBufferGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );
    };
}

#endif
