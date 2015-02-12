
#ifndef _Demo_ShadowMapDebuggingGameState_H_
#define _Demo_ShadowMapDebuggingGameState_H_

#include "OgrePrerequisites.h"
#include "OgreOverlayPrerequisites.h"
#include "TutorialGameState.h"

namespace Demo
{
    class ShadowMapDebuggingGameState : public TutorialGameState
    {
        Ogre::SceneNode     *mSceneNode[16];

        Ogre::SceneNode     *mLightNodes[3];

        bool                mAnimateObjects;

        Ogre::v1::Overlay *mDebugOverlayPSSM;
        Ogre::v1::Overlay *mDebugOverlaySpotlights;

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

        void createShadowMapDebugOverlays(void);

    public:
        ShadowMapDebuggingGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );

        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
