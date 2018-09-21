
#ifndef _Demo_DecalsGameState_H_
#define _Demo_DecalsGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Demo
{
    struct DebugDecalVisual
    {
        Ogre::Item  *plane;
        Ogre::Item  *cube;
        Ogre::SceneNode *sceneNode;
    };

    class DecalsGameState : public TutorialGameState
    {
        Ogre::SceneNode     *mSceneNode[16];

        Ogre::SceneNode     *mLightNodes[3];

        bool                mAnimateObjects;

        size_t          mNumSpheres;
        Ogre::uint8     mTransparencyMode;
        float           mTransparencyValue;

        DebugDecalVisual *mDecalDebugVisual;

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

        void createDecalDebugData(void);
        DebugDecalVisual* attachDecalDebugHelper( Ogre::SceneNode *decalNode );
        void destroyDecalDebugHelper( DebugDecalVisual *decalDebugVisual );
        void setTransparencyToMaterials(void);

    public:
        DecalsGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);
        virtual void destroyScene(void);

        virtual void update( float timeSinceLast );

        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
