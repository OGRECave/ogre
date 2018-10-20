
#ifndef _Demo_SceneFormatGameState_H_
#define _Demo_SceneFormatGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Ogre
{
    class InstantRadiosity;
    class IrradianceVolume;
    class ParallaxCorrectedCubemap;
}

namespace Demo
{
    class SceneFormatGameState : public TutorialGameState
    {
        Ogre::String            mFullpathToFile;
        Ogre::InstantRadiosity  *mInstantRadiosity;
        Ogre::IrradianceVolume  *mIrradianceVolume;
        Ogre::ParallaxCorrectedCubemap *mParallaxCorrectedCubemap;

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

        void resetScene(void);
        void setupParallaxCorrectCubemaps(void);
        void destroyInstantRadiosity(void);
        void destroyParallaxCorrectCubemaps(void);

        Ogre::TexturePtr createRawDecalDiffuseTex();
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
