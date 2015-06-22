
#ifndef _Demo_Tutorial_ReconstructPosFromDepthGameState_H_
#define _Demo_Tutorial_ReconstructPosFromDepthGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"
#include "OgreMesh2.h"

namespace Demo
{
    class Tutorial_ReconstructPosFromDepthGameState : public TutorialGameState
    {
    public:
        Tutorial_ReconstructPosFromDepthGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);
    };
}

#endif
