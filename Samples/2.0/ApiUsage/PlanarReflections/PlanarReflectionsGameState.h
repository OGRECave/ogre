
#ifndef _Demo_PlanarReflectionsGameState_H_
#define _Demo_PlanarReflectionsGameState_H_

#include "OgrePrerequisites.h"
#include "OgreOverlayPrerequisites.h"
#include "TutorialGameState.h"

namespace Ogre
{
    class PlanarReflections;
}

namespace Demo
{
    class PlanarReflectionsWorkspaceListener;

    class PlanarReflectionsGameState : public TutorialGameState
    {
        Ogre::SceneNode     *mSceneNode[16];

        Ogre::SceneNode     *mLightNodes[3];

        bool                mAnimateObjects;

        Ogre::PlanarReflections             *mPlanarReflections;
        PlanarReflectionsWorkspaceListener  *mWorkspaceListener;

        void createReflectiveSurfaces(void);

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

    public:
        PlanarReflectionsGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);
        virtual void destroyScene(void);

        virtual void update( float timeSinceLast );

        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
