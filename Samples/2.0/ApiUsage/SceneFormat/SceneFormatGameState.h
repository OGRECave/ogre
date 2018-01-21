
#ifndef _Demo_SceneFormatGameState_H_
#define _Demo_SceneFormatGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Ogre
{
    class InstantRadiosity;
    class IrradianceVolume;
}

namespace Demo
{
    class SceneFormatGameState : public TutorialGameState
    {
        Ogre::InstantRadiosity  *mInstantRadiosity;
        Ogre::IrradianceVolume  *mIrradianceVolume;

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

        void destroyInstantRadiosity(void);

        void generateScene(void);
        void exportScene(void);
        void importScene(void);

    public:
        SceneFormatGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);
        virtual void destroyScene(void);

        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
