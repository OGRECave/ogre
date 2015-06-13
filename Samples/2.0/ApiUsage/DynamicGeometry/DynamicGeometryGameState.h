
#ifndef _Demo_DynamicGeometryGameState_H_
#define _Demo_DynamicGeometryGameState_H_

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

    class DynamicGeometryGameState : public TutorialGameState
    {
        Ogre::MeshPtr               mStaticMesh;
        Ogre::MeshPtr               mPartialMesh;
        Ogre::MeshPtr               mDynamicMesh[2];
        Ogre::VertexBufferPacked    *mDynamicVertexBuffer[2];

        float mRotationTime;

        /// Helper function to create an index buffer.
        Ogre::IndexBufferPacked* createIndexBuffer(void);

        /// Creates the MeshPtr needed by mStaticMesh & mPartialMesh
        Ogre::MeshPtr createStaticMesh( bool partialMesh );

        /// Creates a dynamic buffer. The idx parameter is needed for the mesh' name
        std::pair<Ogre::MeshPtr, Ogre::VertexBufferPacked*> createDynamicMesh( size_t idx );

        /// Helper function to fill the 2nd dynamic vertex buffer
        void updateDynamicBuffer01( float *cubeVertices, const CubeVertices originalVerts[8],
                                    size_t start, size_t end ) const;
    public:
        DynamicGeometryGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);
        virtual void destroyScene(void);

        virtual void update( float timeSinceLast );
    };
}

#endif
