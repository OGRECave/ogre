
#ifndef _Demo_Tutorial_SSAOGameState_H_
#define _Demo_Tutorial_SSAOGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"
#include "OgreMesh2.h"

namespace Demo
{
    class Tutorial_SSAOGameState : public TutorialGameState
    {
	private:
		Ogre::SceneNode     *mSceneNode[16];

		Ogre::SceneNode     *mLightNodes[3];

		bool                mAnimateObjects;

		size_t          mNumSpheres;

		Ogre::Pass* mSSAOPass;
		Ogre::Pass* mApplyPass;

		float mKernelRadius;
		float mPowerScale;

		virtual void generateDebugText(float timeSinceLast, Ogre::String &outText);

    public:
        Tutorial_SSAOGameState( const Ogre::String &helpDescription );

		virtual void createScene01(void);

		virtual void update(float timeSinceLast);

		virtual void keyReleased(const SDL_KeyboardEvent &arg);

    };
}

#endif
