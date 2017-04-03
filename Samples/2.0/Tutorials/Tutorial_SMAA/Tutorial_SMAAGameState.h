
#ifndef _Demo_Tutorial_SMAAGameState_H_
#define _Demo_Tutorial_SMAAGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"
#include "OgreMesh2.h"

namespace Demo
{
    class Tutorial_SMAAGameState : public TutorialGameState
    {
	private:
		Ogre::SceneNode     *mSceneNode[16];
		Ogre::SceneNode     *mLightNodes[3];

		bool                mAnimateObjects;

		virtual void generateDebugText(float timeSinceLast, Ogre::String &outText);

    public:
        Tutorial_SMAAGameState( const Ogre::String &helpDescription );

		virtual void createScene01(void);

		virtual void update(float timeSinceLast);

		virtual void keyReleased(const SDL_KeyboardEvent &arg);

    };
}

#endif
