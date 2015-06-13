
#ifndef _Demo_PostprocessingGameState_H_
#define _Demo_PostprocessingGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

#include "OgreStringVector.h"
#include "OgreIdString.h"

namespace Demo
{
    class PostprocessingGameState : public TutorialGameState
    {
        Ogre::StringVector          mCompositorNames;
        size_t                      mCurrentPage;

        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

        void importV1Mesh( const Ogre::String &meshName );

        /// Creates textures needed by some of the postprocessing effects.
        void createCustomTextures(void);

        /// Creates hard coded postfilter effects from code instead of scripts.
        /// Just to show how to do it. Needs to be called before or inside
        /// setupCompositor; since setupCompositor modifies the workspace
        /// definition to add all the postprocessing nodes beforehand, but
        /// disabled.
        void createExtraEffectsFromCode(void);

        /// Shows two of the many possible ways to toggle a postprocess FX
        /// on and off in real time.
        void togglePostprocess( Ogre::IdString nodeName );

    public:
        PostprocessingGameState( const Ogre::String &helpDescription );

        Ogre::CompositorWorkspace* setupCompositor();

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );

        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
