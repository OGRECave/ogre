/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

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
#ifndef __Ogre_Volume_DualGridGenerator_H__
#define __Ogre_Volume_DualGridGenerator_H__

#include <vector>

#include "OgreSceneManager.h"

#include "OgreVolumeOctreeNode.h"
#include "OgreVolumeDualCell.h"
#include "OgreVolumePrerequisites.h"

namespace Ogre {
namespace Volume {

    /** Class for the generation of the DualGrid.
    */
    class _OgreVolumeExport DualGridGenerator : public UtilityAlloc
    {
    protected:
        
        /// To give the debug manual object an unique name.
        static size_t mDualGridI;
        
        /// The entity for the debug visualization of the grid.
        Entity* mDualGrid;

        typedef vector<DualCell>::type VecDualCell;
        /// Holds the generated dual cells of the grid.
        VecDualCell mDualCells;

        /// Starting node to generate the grid from.
        OctreeNode const* mRoot;
        
        /** Adds a dualcell.
         @param c0
            The first corner.
         @param c1
            The second corner.
         @param c2
            The third corner.
         @param c3
            The fourth corner.
         @param c4
            The fifth corner.
         @param c5
            The sixth corner.
         @param c6
            The seventh corner.
         */
        inline void addDualCell(const Vector3 &c0, const Vector3 &c1, const Vector3 &c2, const Vector3 &c3, const Vector3 &c4, const Vector3 &c5, const Vector3 &c6, const Vector3 &c7)
        {
            mDualCells.push_back(DualCell(c0, c1, c2, c3, c4, c5, c6, c7));
        }
        
        /** Adds a dualcell with precalculated values.
         @param c0
            The first corner.
         @param c1
            The second corner.
         @param c2
            The third corner.
         @param c3
            The fourth corner.
         @param c4
            The fifth corner.
         @param c5
            The sixth corner.
         @param c6
            The seventh corner.
         @param c7
            The eighth corner.
         @param v0
            The first of the cell, one Vector4 consists of gradient (x, y, z) and density (w).
         @param v1
            The second of the cell, one Vector4 consists of gradient (x, y, z) and density (w).
         @param v2
            The third of the cell, one Vector4 consists of gradient (x, y, z) and density (w).
         @param v3
            The fourth of the cell, one Vector4 consists of gradient (x, y, z) and density (w).
         @param v4
            The fifth of the cell, one Vector4 consists of gradient (x, y, z) and density (w).
         @param v5
            The sixth of the cell, one Vector4 consists of gradient (x, y, z) and density (w).
         @param v6
            The seventh of the cell, one Vector4 consists of gradient (x, y, z) and density (w).
         @param v7
            The eighth of the cell, one Vector4 consists of gradient (x, y, z) and density (w).
         */
        inline void addDualCell(const Vector3 &c0, const Vector3 &c1, const Vector3 &c2, const Vector3 &c3, const Vector3 &c4, const Vector3 &c5, const Vector3 &c6, const Vector3 &c7,
            const Vector4 &v0, const Vector4 &v1, const Vector4 &v2, const Vector4 &v3, const Vector4 &v4, const Vector4 &v5, const Vector4 &v6, const Vector4 &v7)
        {
            mDualCells.push_back(DualCell(c0, c1, c2, c3, c4, c5, c6, c7, v0, v1, v2, v3, v4, v5, v6, v7));
        }

        /* Startpoint for the creation recursion.
        @param n
            The node to start with.
        */
        void nodeProc(const OctreeNode *n);

        /* faceProc with variing X and Y of the nodes, see the paper for faceProc().
            Direction of parameters: Z+ (n0 and n3 for example of parent cell)
        @param n0
            The first node.
        @param n1
            The second node.
        */
        void faceProcXY(const OctreeNode *n0, const OctreeNode *n1);

        /* faceProc with variing Z and Y of the nodes, see the paper for faceProc().
            Direction of parameters: X+ (n0 and n1 for example of parent cell)
        @param n0
            The first node.
        @param n1
            The second node.
        */
        void faceProcZY(const OctreeNode *n0, const OctreeNode *n1);

        /* faceProc with variing X and Z of the nodes, see the paper for faceProc().
            Direction of parameters: Y- (n4 and n0 for example of parent cell)
        @param n0
            The first node.
        @param n1
            The second node.
        */
        void faceProcXZ(const OctreeNode *n0, const OctreeNode *n1);

        /** edgeProc with variing X of the nodes, see the paper for edgeProc().
            Direction of parameters: Z+, Y around the clock (n0, n3, n7, n4 for example of the parent cell)
        @param n0
            The first node.
        @param n1
            The second node.
        @param n2
            The third node.
        @param n3
            The fourth node.
        */
        void edgeProcX(const OctreeNode *n0, const OctreeNode *n1, const OctreeNode *n2, const OctreeNode *n3);

        /** edgeProc with variing Y of the nodes, see the paper for edgeProc().
            Direction of parameters: X+, Z around the clock (n0, n1, n2, n3 for example of the parent cell)
        @param n0
            The first node.
        @param n1
            The second node.
        @param n2
            The third node.
        @param n3
            The fourth node.
        */
        void edgeProcY(const OctreeNode *n0, const OctreeNode *n1, const OctreeNode *n2, const OctreeNode *n3);

        /** edgeProc with variing Z of the nodes, see the paper for edgeProc().
            Direction of parameters: X+, Y around the clock (n7, n6, n2, n3 for example of the parent cell)
        @param n0
            The first node.
        @param n1
            The second node.
        @param n2
            The third node.
        @param n3
            The fourth node.
        */
        void edgeProcZ(const OctreeNode *n0, const OctreeNode *n1, const OctreeNode *n2, const OctreeNode *n3);

        /* vertProc of the nodes, see the paper for vertProc. Difference to the paper: The cells to the
            octree border are already created here and not with another traversion.
        @param n0
            The first node.
        @param n1
            The second node.
        @param n3
            The third node.
        @param n4
            The fourth node.
        @param n5
            The fifth node.
        @param n6
            The sixth node.
        @param n7
            The seventh node.
        */
        void vertProc(const OctreeNode *n0, const OctreeNode *n1, const OctreeNode *n2, const OctreeNode *n3, const OctreeNode *n4, const OctreeNode *n5, const OctreeNode *n6, const OctreeNode *n7);
        
        /* Creates the bordercells.
        @param n0
            The first node.
        @param n1
            The second node.
        @param n3
            The third node.
        @param n4
            The fourth node.
        @param n5
            The fifth node.
        @param n6
            The sixth node.
        @param n7
            The seventh node.
        */
        void createBorderCells(const OctreeNode *n0, const OctreeNode *n1, const OctreeNode *n2, const OctreeNode *n3, const OctreeNode *n4, const OctreeNode *n5, const OctreeNode *n6, const OctreeNode *n7);

   public:

        /** Constructor.
        */
        DualGridGenerator(void);

        /** Generates the dualgrid of the given octree root node.
        @param root
            The octree root node.
        */
        void generateDualGrid(const OctreeNode *root);

        /** Gets the lazily created entity of the dualgrid debug visualization.
        @param sceneManager
            The scenemanager creating the entity.
        @return
            The entity. Might be null if no dualcells are available.
        */
        Entity* getDualGrid(SceneManager *sceneManager);

        /** Gets the amount of generated dual cells.
        @return
            The amount of generated dual cells.
        */
        inline size_t getDualCellCount(void) const
        {
            return mDualCells.size();
        }

        /** Gets a dual cell.
        @param i
            The index of the wanted dual cell.
        @return
            The dual cell.
        */
        inline DualCell getDualCell(size_t i) const
        {
            return mDualCells[i];
        }
    };
}
}

#endif