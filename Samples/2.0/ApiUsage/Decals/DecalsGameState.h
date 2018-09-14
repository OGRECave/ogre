
#ifndef _Demo_DecalsGameState_H_
#define _Demo_DecalsGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Demo
{
    class DecalsGameState : public TutorialGameState
    {
        Ogre::SceneNode     *mSceneNode[16];

        Ogre::SceneNode     *mLightNodes[3];

        bool                mAnimateObjects;

        size_t          mNumSpheres;
        Ogre::uint8     mTransparencyMode;
        float           mTransparencyValue;

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

        void setTransparencyToMaterials(void);

    public:
        DecalsGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );

        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
