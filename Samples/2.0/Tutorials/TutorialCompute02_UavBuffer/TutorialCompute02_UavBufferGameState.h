
#ifndef _Demo_TutorialCompute02_UavBufferGameState_H_
#define _Demo_TutorialCompute02_UavBufferGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"
#include "OgreMaterial.h"

namespace Demo
{
    class TutorialCompute02_UavBufferGameState : public TutorialGameState
    {
        Ogre::SceneNode     *mSceneNode;
        float               mDisplacement;

        Ogre::MaterialPtr       mDrawFromUavBufferMat;
        Ogre::HlmsComputeJob    *mComputeJob;

        Ogre::uint32 mLastWindowWidth;
        Ogre::uint32 mLastWindowHeight;

    public:
        TutorialCompute02_UavBufferGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);
        virtual void destroyScene(void);

        virtual void update( float timeSinceLast );
    };
}

#endif
