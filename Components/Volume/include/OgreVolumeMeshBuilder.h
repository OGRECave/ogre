/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef __Ogre_Volume_MeshBuilder_H__
#define __Ogre_Volume_MeshBuilder_H__

#include <vector>
#include "OgreManualObject.h"
#include "OgreVector.h"
#include "OgreAxisAlignedBox.h"
#include "OgreVolumePrerequisites.h"

namespace Ogre {
namespace Volume {
    /** \addtogroup Optional
    *  @{
    */
    /** \addtogroup Volume
    *  @{
    */
    /** Lightweight struct to represent a mesh vertex.
    */
    typedef struct _OgreVolumeExport Vertex
    {
        /// X coordinate of the position
        Real x;
        
        /// Y coordinate of the position
        Real y;
        
        /// Z coordinate of the position
        Real z;
        
        /// X component of the normal
        Real nX;
        
        /// Y component of the normal
        Real nY;
        
        /// Z component of the normal
        Real nZ;

        /** Convenience constructor.
        @param v
            The vertex position.
        @param n
            The vertex normal.
        */
        Vertex(const Vector3 &v, const Vector3 &n) :
            x(v.x), y(v.y), z(v.z),
            nX(n.x), nY(n.y), nZ(n.z)
        {
        }
        Vertex()
        {
        }
    } Vertex;

    /** == operator for two vertices.
    @param a
        The first vertex to test.
    @param b
        The second vertex to test.
    */
    bool _OgreVolumeExport operator==(Vertex const& a, Vertex const& b);
    
    /** A less operator. 
    @note
        This operator is needed so that Vertex can serve as the key in a map structrue 
    @param a
        The first vertex to test.
    @param b
        The second vertex to test.
    */
    bool _OgreVolumeExport operator<(const Vertex& a, const Vertex& b);

    /** To hold vertices.
    */
    typedef std::vector<Vertex> VecVertex;

    /** To hold indices.
    */
    typedef std::vector<uint32> VecIndices;

    /** Callback class when the user needs information about the triangles of
        chunks of a LOD level.
    */
    class _OgreVolumeExport MeshBuilderCallback
    {
    public:
        virtual ~MeshBuilderCallback() {}

        /** To be called with the callback function of a MeshBuilder.
        @param simpleRenderable
            Contains the SimpleRenderable for which the triangles were built.
        @param vertices
            Contains the vertices of the triangles.
        @param indices
            Contains the indices of the triangles.
        @param level
            The LOD level of this mesh.
        @param inProcess
            The amount of other meshes/LOD-Chunks still to be loaded.
        */
        virtual void ready(const SimpleRenderable *simpleRenderable, const VecVertex &vertices, const VecIndices &indices, size_t level, int inProcess) = 0;
    };

    /** Class to build up a mesh with vertices and indices.
    */
    class _OgreVolumeExport MeshBuilder : public UtilityAlloc
    {
    protected:

        /// The buffer binding.
        static const unsigned short MAIN_BINDING;

        /// Map to get a vertex index.
        typedef std::map<Vertex, uint32> UMapVertexIndex;
        UMapVertexIndex mIndexMap;

         /// Holds the vertices of the mesh.
        VecVertex mVertices;

        /// Holds the indices of the mesh.
        VecIndices mIndices;

        /// Holds the bounding box.
        AxisAlignedBox mBox;

        /// Holds whether the initial bounding box has been set
        bool mBoxInit;
        
        /** Adds a vertex to the data structure, reusing the index if it is already known.
        @param v
            The vertex.
        */
        inline void addVertex(const Vertex &v)
        {
            uint32 i = 0;
            if (mIndexMap.find(v) == mIndexMap.end())
            {
                i = mVertices.size();
                mIndexMap[v] = i;
                mVertices.push_back(v);

                // Update bounding box
                mBox.merge(Vector3(v.x, v.y, v.z));
            }
            else
            {
                i = mIndexMap[v];
            }
            mIndices.push_back(i);
        }

    public:
        
        /** Adds a cube to a manual object rendering lines. Corner numeration:
             4 5
            7 6
             0 1
            3 2
        @param manual
            The manual for the cube lines.
        @param c0
            The corner 0.
        @param c1
            The corner 1.
        @param c2
            The corner 2.
        @param c3
            The corner 3.
        @param c4
            The corner 4.
        @param c5
            The corner 5.
        @param c6
            The corner 6.
        @param c7
            The corner 7.
        @param baseIndex
            The next free index of this manual object.
            Is incremented by 8 in this function.
        */
        static inline void addCubeToManualObject(
            ManualObject *manual,
            const Vector3 &c0,
            const Vector3 &c1,
            const Vector3 &c2,
            const Vector3 &c3,
            const Vector3 &c4,
            const Vector3 &c5,
            const Vector3 &c6,
            const Vector3 &c7,
            uint32 &baseIndex
            )
        {
            manual->position(c0);
            manual->position(c1);
            manual->position(c2);
            manual->position(c3);
            manual->position(c4);
            manual->position(c5);
            manual->position(c6);
            manual->position(c7);

            manual->index(baseIndex + 0); manual->index(baseIndex + 1);
            manual->index(baseIndex + 1); manual->index(baseIndex + 2);
            manual->index(baseIndex + 2); manual->index(baseIndex + 3);
            manual->index(baseIndex + 3); manual->index(baseIndex + 0);
        
            manual->index(baseIndex + 4); manual->index(baseIndex + 5);
            manual->index(baseIndex + 5); manual->index(baseIndex + 6);
            manual->index(baseIndex + 6); manual->index(baseIndex + 7);
            manual->index(baseIndex + 7); manual->index(baseIndex + 4);
        
            manual->index(baseIndex + 0); manual->index(baseIndex + 4);
            manual->index(baseIndex + 1); manual->index(baseIndex + 5);
            manual->index(baseIndex + 2); manual->index(baseIndex + 6);
            manual->index(baseIndex + 3); manual->index(baseIndex + 7);
            baseIndex += 8;
        }

        /** Constructor.
        */
        MeshBuilder(void);
        
        /** Adds a triangle to the mesh with reusing already existent vertices via their index.
        @param v0
            The first vertex of the triangle.
        @param n0
            The normal of the first vertex.
        @param v1
            The second vertex of the triangle.
        @param n1
            The normal of the second vertex.
        @param v2
            The third vertex of the triangle.
        @param n2
            The normal of the third vertex.
        */
        inline void addTriangle(const Vector3 &v0, const Vector3 &n0, const Vector3 &v1, const Vector3 &n1, const Vector3 &v2, const Vector3 &n2)
        {
            addVertex(Vertex(v0, n0));
            addVertex(Vertex(v1, n1));
            addVertex(Vertex(v2, n2));
        }

        /** Generates the vertex- and indexbuffer of this mesh on the given
            RenderOperation.
        @param operation
            The RenderOperation for the buffers.
        @return
            The amount of generated triangles.
        */
        size_t generateBuffers(RenderOperation &operation);

        /** Generates an entity via a ManualObject.
        @param sceneManager
            The creating sceneManager.
        @param name
            The name for the entity.
        @param material
            The material to use.
        @return
            The created entity.
        */
        Entity* generateWithManualObject(SceneManager *sceneManager, const String &name, const String &material);

        /** Gets the bounding box of the mesh.
        @return
            The bounding box.
        */
        AxisAlignedBox getBoundingBox(void);

        /** Executes a MeshBuilderCallback on this instance.
        @param callback
            The callback to execute.
        @param simpleRenderable
            Contains the SimpleRenderable for which the triangles were built.
        @param level
            The LOD level of this mesh.
        @param inProcess
            The amount of other meshes/LOD-Chunks still to be loaded.
        */
        void executeCallback(MeshBuilderCallback *callback, const SimpleRenderable *simpleRenderable, size_t level, int inProcess) const;

    };
    /** @} */
    /** @} */
}
}

#endif
