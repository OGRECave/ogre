
#ifndef _Demo_TutorialSky_PostprocessGameState_H_
#define _Demo_TutorialSky_PostprocessGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"
#include "OgreMesh2.h"

namespace Demo
{
    struct CubeVertices
    {
        float px, py, pz;   //Position
        float nx, ny, nz;   //Normals

        CubeVertices() {}
        CubeVertices( float _px, float _py, float _pz,
                      float _nx, float _ny, float _nz ) :
            px( _px ), py( _py ), pz( _pz ),
            nx( _nx ), ny( _ny ), nz( _nz )
        {
        }
    };

    class TutorialSky_PostprocessGameState : public TutorialGameState
    {
    public:
        TutorialSky_PostprocessGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);
    };
}

#endif
