
#ifndef _Demo_StereoRenderingGameState_H_
#define _Demo_StereoRenderingGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Demo
{
    class StereoRenderingGameState : public TutorialGameState
    {
        Ogre::SceneNode     *mSceneNode[16];

    public:
        StereoRenderingGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );
    };
}

#endif
