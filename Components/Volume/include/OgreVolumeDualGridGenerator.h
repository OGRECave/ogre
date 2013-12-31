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
#ifndef __Ogre_Volume_DualGridGenerator_H__
#define __Ogre_Volume_DualGridGenerator_H__

#include <vector>

#include "OgreSceneManager.h"

#include "OgreVolumeOctreeNode.h"
#include "OgreVolumePrerequisites.h"
#include "OgreVolumeIsoSurface.h"

namespace Ogre {
namespace Volume {

    /** To store the generated dual cells in a vector.
    */
    typedef struct _OgreVolumeExport DualCell
    {
    public:
        Vector3 mC0;
        Vector3 mC1;
        Vector3 mC2;
        Vector3 mC3;
        Vector3 mC4;
        Vector3 mC5;
        Vector3 mC6;
        Vector3 mC7;
        DualCell(const Vector3 &c0, const Vector3 &c1, const Vector3 &c2, const Vector3 &c3, const Vector3 &c4, const Vector3 &c5, const Vector3 &c6, const Vector3 &c7) :
            mC0(c0), mC1(c1), mC2(c2), mC3(c3), mC4(c4), mC5(c5), mC6(c6), mC7(c7)
        {
        }
    } DualCell;
    
    /** To hold dual cells.
    */
    typedef vector<DualCell>::type VecDualCell;

    /** Class for the generation of the DualGrid.
    */
    class _OgreVolumeExport DualGridGenerator : public UtilityAlloc
    {
    protected:
        
        /// To give the debug manual object an unique name.
        static size_t mDualGridI;
        
        /// The entity for the debug visualization of the grid.
        Entity* mDualGrid;
        
        /// Starting node to generate the grid from.
        OctreeNode const* mRoot;
        
        /// Holds the generated dual cells of the grid.
        VecDualCell mDualCells;

        /// Whether to store the dualcells for later visualization.
        bool mSaveDualCells;

        /// To contour the dualcells.
        IsoSurface *mIs;

        /// To store the triangles of the contour.
        MeshBuilder *mMb;

        /// The maximum distance where to generate the skirts.
        Real mMaxMSDistance;

        /// The global from.
        Vector3 mTotalFrom;

        /// The total to.
        Vector3 mTotalTo;

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
            addDualCell(c0, c1, c2, c3, c4, c5, c6, c7, 0);
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
         @param values
            The (possible) values at the corners.
         */
        inline void addDualCell(const Vector3 &c0, const Vector3 &c1, const Vector3 &c2, const Vector3 &c3, const Vector3 &c4, const Vector3 &c5, const Vector3 &c6, const Vector3 &c7,
            Vector4 *values)
        {

            if (mSaveDualCells)
            {
                mDualCells.push_back(DualCell(c0, c1, c2, c3, c4, c5, c6, c7));
            }

            Vector3 corners[8];
            corners[0] = c0;
            corners[1] = c1;
            corners[2] = c2;
            corners[3] = c3;
            corners[4] = c4;
            corners[5] = c5;
            corners[6] = c6;
            corners[7] = c7;
            mIs->addMarchingCubesTriangles(corners, values, mMb);
            Vector3 from = mRoot->getFrom();
            Vector3 to = mRoot->getTo();
            if (corners[0].z == from.z && corners[0].z != mTotalFrom.z)
            {
                mIs->addMarchingSquaresTriangles(corners, values, IsoSurface::MS_CORNERS_BACK, mMaxMSDistance, mMb);
            }
            if (corners[2].z == to.z && corners[2].z != mTotalTo.z)
            {
                mIs->addMarchingSquaresTriangles(corners, values, IsoSurface::MS_CORNERS_FRONT, mMaxMSDistance, mMb);
            }
            if (corners[0].x == from.x && corners[0].x != mTotalFrom.x)
            {
                mIs->addMarchingSquaresTriangles(corners, values, IsoSurface::MS_CORNERS_LEFT, mMaxMSDistance, mMb);
            }
            if (corners[1].x == to.x && corners[1].x != mTotalTo.x)
            {
                mIs->addMarchingSquaresTriangles(corners, values, IsoSurface::MS_CORNERS_RIGHT, mMaxMSDistance, mMb);
            }
            if (corners[5].y == to.y && corners[5].y != mTotalTo.y)
            {
                mIs->addMarchingSquaresTriangles(corners, values, IsoSurface::MS_CORNERS_TOP, mMaxMSDistance, mMb);
            }
            if (corners[0].y == from.y && corners[0].y != mTotalFrom.y)
            {
                mIs->addMarchingSquaresTriangles(corners, values, IsoSurface::MS_CORNERS_BOTTOM, mMaxMSDistance, mMb);
            }
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
        @param is
            To contour the dualcells.
        @param mb
            To store the triangles of the contour.
        @param maxMSDistance
            The maximum distance to the isosurface where to generate skirts.
        @param totalFrom
            The global from.
        @param totalTo
            The global to.
        @param saveDualCells
            Whether to save the generated dualcells of the generated dual cells.
        */
        void generateDualGrid(const OctreeNode *root, IsoSurface *is, MeshBuilder *mb, Real maxMSDistance, const Vector3 &totalFrom, const Vector3 &totalTo, bool saveDualCells);

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