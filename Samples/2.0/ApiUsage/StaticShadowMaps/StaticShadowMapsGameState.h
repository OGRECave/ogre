
#ifndef _Demo_StaticShadowMapsGameState_H_
#define _Demo_StaticShadowMapsGameState_H_

#include "OgrePrerequisites.h"
#include "OgreOverlayPrerequisites.h"
#include "OgreOverlay.h"
#include "TutorialGameState.h"

namespace Ogre
{
    class CompositorShadowNode;
}

namespace Demo
{
    class StaticShadowMapsGameState : public TutorialGameState
    {
        Ogre::SceneNode     *mSceneNode[16];

        Ogre::SceneNode     *mLightNodes[3];

        bool                mAnimateObjects;
        bool                mUpdateShadowMaps;
        Ogre::CompositorShadowNode  *mShadowNode;

        Ogre::v1::Overlay *mDebugOverlayPSSM;
        Ogre::v1::Overlay *mDebugOverlaySpotlights;

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

        void createShadowMapDebugOverlays(void);

    public:
        StaticShadowMapsGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );

        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
