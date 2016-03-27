
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
        Ogre::Terra *mTerra;
        Ogre::Light *mSunLight;

    public:
        Tutorial_TerrainGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);
        virtual void destroyScene(void);

        virtual void update( float timeSinceLast );
    };
}

#endif
