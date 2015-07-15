
#ifndef _Demo_HdrGameState_H_
#define _Demo_HdrGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Demo
{
    class HdrGameState : public TutorialGameState
    {
        Ogre::SceneNode     *mSceneNode[16];

        Ogre::SceneNode     *mLightNodes[3];

        bool                mAnimateObjects;
        Ogre::uint32        mCurrentPreset;

        Ogre::String mPresetName;
        float mExposure;
        float mMinAutoExposure;
        float mMaxAutoExposure;
        float mBloomFullThreshold;

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

        void switchPreset( int direction=1 );

    public:
        HdrGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );

        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
