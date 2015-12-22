
#ifndef _Demo_AnimationTagPointGameState_H_
#define _Demo_AnimationTagPointGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Ogre
{
    class SkeletonAnimation;
}

namespace Demo
{
    class AnimationTagPointGameState : public TutorialGameState
    {
        Ogre::SceneNode *mSphereNodes[4];
        Ogre::SceneNode *mCubesNode;
        Ogre::SceneNode *mCubeNodes[4];

        Ogre::SkeletonAnimation *mWalkAnimation;

    public:
        AnimationTagPointGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);
        virtual void update( float timeSinceLast );
    };
}

#endif
