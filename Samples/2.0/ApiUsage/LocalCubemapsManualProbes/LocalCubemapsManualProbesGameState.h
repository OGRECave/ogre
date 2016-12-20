
#ifndef _Demo_LocalCubemapsManualProbesGameState_H_
#define _Demo_LocalCubemapsManualProbesGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

#include "OgreTexture.h"

namespace Ogre
{
    class ParallaxCorrectedCubemap;
    class HlmsPbsDatablock;
}

namespace Demo
{
    class LocalCubemapsManualProbesGameState : public TutorialGameState
    {
        Ogre::SceneNode     *mLightNodes[3];

        Ogre::ParallaxCorrectedCubemap  *mParallaxCorrectedCubemap;
        Ogre::HlmsPbsDatablock          *mMaterials[4*4];

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

        void setupParallaxCorrectCubemaps(void);

    public:
        LocalCubemapsManualProbesGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);
        virtual void destroyScene(void);

        virtual void update( float timeSinceLast );

        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
