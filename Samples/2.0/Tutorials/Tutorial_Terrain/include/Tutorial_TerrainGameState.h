
#ifndef _Demo_Tutorial_TerrainGameState_H_
#define _Demo_Tutorial_TerrainGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Ogre
{
    class Terra;
}

namespace Demo
{
    class Tutorial_TerrainGameState : public TutorialGameState
    {
        float mTimeOfDay;
        float mAzimuth;
        Ogre::Terra *mTerra;
        Ogre::Light *mSunLight;

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
