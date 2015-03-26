
#ifndef _Demo_V2ManualObjectGameState_H_
#define _Demo_V2ManualObjectGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Demo
{
    class V2ManualObjectGameState : public TutorialGameState
    {
    public:
        V2ManualObjectGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

        virtual void update( float timeSinceLast );

        virtual void keyReleased( const SDL_KeyboardEvent & arg );

    private:
        Ogre::ManualObject * mManualObject;
        bool mFirstFrame;
        std::vector<Ogre::Vector3> mVertices;
        float mAccumulator;
        bool mAnimate;

        void fillBuffer(float uvOffset);
    };
}

#endif
