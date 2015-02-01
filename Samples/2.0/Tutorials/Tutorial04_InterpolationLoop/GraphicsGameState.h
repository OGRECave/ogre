
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
        Ogre::SceneNode *mSceneNode;
        Ogre::Vector3 mLastPosition;
        Ogre::Vector3 mCurrentPosition;

        bool        mEnableInterpolation;

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

    public:
        GraphicsGameState( const Ogre::String &helpDescription );

        Ogre::Vector3& _getLastPositionRef(void)        { return mLastPosition; }
        Ogre::Vector3& _getCurrentPositionRef(void)      { return mCurrentPosition; }

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );

        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
