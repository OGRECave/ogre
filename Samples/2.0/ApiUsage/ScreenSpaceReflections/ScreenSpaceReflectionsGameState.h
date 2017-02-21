
#ifndef _Demo_ScreenSpaceReflectionsGameState_H_
#define _Demo_ScreenSpaceReflectionsGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

#include "Utils/ScreenSpaceReflections.h"

#include "SdlEmulationLayer.h"
#if OGRE_USE_SDL2
    #include "SDL_keyboard.h"
#endif

namespace Ogre
{
    class HlmsPbsDatablock;
}

namespace Demo
{
    class ScreenSpaceReflectionsGameState : public TutorialGameState
    {
        ScreenSpaceReflections  *mScreenSpaceReflections;
        Ogre::HlmsPbsDatablock  *mMaterials[4];

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

    public:
        ScreenSpaceReflectionsGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);
        virtual void destroyScene(void);

        virtual void update( float timeSinceLast );

        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
