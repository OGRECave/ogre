
#ifndef _Demo_Tutorial_TerrainGameState_H_
#define _Demo_Tutorial_TerrainGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Ogre
{
    class Terra;
    class HlmsPbsTerraShadows;
}

namespace Demo
{
    class Tutorial_TerrainGameState : public TutorialGameState
    {
        bool mLockCameraToGround;
        float mTimeOfDay;
        float mAzimuth;
        Ogre::Terra *mTerra;
        Ogre::Light *mSunLight;

        /// Listener to make PBS objects also be affected by terrain's shadows
        Ogre::HlmsPbsTerraShadows *mHlmsPbsTerraShadows;

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

    public:
        Tutorial_TerrainGameState( const Ogre::String &helpDescription );

        Ogre::CompositorWorkspace* setupCompositor();

        virtual void createScene01(void);
        virtual void destroyScene(void);

        virtual void update( float timeSinceLast );

        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
