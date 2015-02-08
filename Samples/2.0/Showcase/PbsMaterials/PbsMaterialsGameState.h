
#ifndef _Demo_PbsMaterialsGameState_H_
#define _Demo_PbsMaterialsGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Demo
{
    class PbsMaterialsGameState : public TutorialGameState
    {
        Ogre::SceneNode     *mSceneNode[16];

        Ogre::SceneNode     *mLightNodes[3];

    public:
        PbsMaterialsGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );
    };
}

#endif
