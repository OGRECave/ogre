
#ifndef _Demo_TutorialUav01_SetupGameState_H_
#define _Demo_TutorialUav01_SetupGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"
#include "OgreMesh2.h"

namespace Demo
{
    class TutorialUav01_SetupGameState : public TutorialGameState
    {
    public:
        TutorialUav01_SetupGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);
    };
}

#endif
