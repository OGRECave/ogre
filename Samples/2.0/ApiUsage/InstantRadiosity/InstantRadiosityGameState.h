
#ifndef _Demo_InstantRadiosityGameState_H_
#define _Demo_InstantRadiosityGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

#include "OgreLight.h"

#include "SdlEmulationLayer.h"
#if OGRE_USE_SDL2
    #include "SDL_keyboard.h"
#endif

namespace Ogre
{
    class InstantRadiosity;
    class HlmsPbsDatablock;
    class IrradianceVolume;
}

namespace Demo
{
    class InstantRadiosityGameState : public TutorialGameState
    {
        Ogre::SceneNode     *mLightNode;
        Ogre::Light         *mLight;
        Ogre::Light::LightTypes  mCurrentType;

        Ogre::InstantRadiosity          *mInstantRadiosity;
        Ogre::IrradianceVolume          *mIrradianceVolume;
        Ogre::Real                      mIrradianceCellSize;

        std::map<SDL_Keycode, SDL_Keysym> mKeysHold;

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

        void createLight(void);
        void updateIrradianceVolume(void);

    public:
        InstantRadiosityGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);
        virtual void destroyScene(void);

        virtual void update( float timeSinceLast );

        virtual void keyPressed( const SDL_KeyboardEvent &arg );
        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
