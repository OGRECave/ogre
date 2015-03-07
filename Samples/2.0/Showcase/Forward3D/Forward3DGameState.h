
#ifndef _Demo_Forward3DGameState_H_
#define _Demo_Forward3DGameState_H_

#include "OgrePrerequisites.h"
#include "OgreOverlayPrerequisites.h"
#include "TutorialGameState.h"

#include "OgreCommon.h"

namespace Demo
{
    class Forward3DGameState : public TutorialGameState
    {
        Ogre::SceneNode     *mSceneNode[16];

        Ogre::SceneNode     *mLightNodes[3];

        bool                mAnimateObjects;

        Ogre::uint32        mCurrentForward3DPreset;

        Ogre::LightArray    mGeneratedLights;
        Ogre::uint32        mNumLights;
        float               mLightRadius;
        bool                mLowThreshold;

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

        void changeForward3DPreset( bool goForward );

        void generateLights(void);

    public:
        Forward3DGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );

        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
