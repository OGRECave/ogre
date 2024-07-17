/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreVolumeDualGridGenerator.h"
#include "OgreManualObject.h"
#include "OgreSceneManager.h"
#include "OgreVolumeMeshBuilder.h"

namespace Ogre {
namespace Volume {

    size_t DualGridGenerator::mDualGridI = 0;

    //-----------------------------------------------------------------------

    void DualGridGenerator::nodeProc(const OctreeNode *n)
    {
        if (n->isSubdivided())
        {

            const OctreeNode *c0 = n->getChild(0);
            const OctreeNode *c1 = n->getChild(1);
            const OctreeNode *c2 = n->getChild(2);
            const OctreeNode *c3 = n->getChild(3);
            const OctreeNode *c4 = n->getChild(4);
            const OctreeNode *c5 = n->getChild(5);
            const OctreeNode *c6 = n->getChild(6);
            const OctreeNode *c7 = n->getChild(7);

            nodeProc(c0);
            nodeProc(c1);
            nodeProc(c2);
            nodeProc(c3);
            nodeProc(c4);
            nodeProc(c5);
            nodeProc(c6);
            nodeProc(c7);

            faceProcXY(c0, c3);
            faceProcXY(c1, c2);
            faceProcXY(c4, c7);
            faceProcXY(c5, c6);

            faceProcZY(c0, c1);
            faceProcZY(c3, c2);
            faceProcZY(c4, c5);
            faceProcZY(c7, c6);

            faceProcXZ(c4, c0);
            faceProcXZ(c5, c1);
            faceProcXZ(c7, c3);
            faceProcXZ(c6, c2);

            edgeProcX(c0, c3, c7, c4);
            edgeProcX(c1, c2, c6, c5);

            edgeProcY(c0, c1, c2, c3);
            edgeProcY(c4, c5, c6, c7);

            edgeProcZ(c7, c6, c2, c3);
            edgeProcZ(c4, c5, c1, c0);

            vertProc(c0, c1, c2, c3, c4, c5, c6, c7);
        }
    }

    //-----------------------------------------------------------------------

    void DualGridGenerator::faceProcXY(const OctreeNode *n0, const OctreeNode *n1)
    {
        const bool n0Subdivided = n0->isSubdivided();
        const bool n1Subdivided = n1->isSubdivided();

        if (n0Subdivided || n1Subdivided)
        {
            const OctreeNode *c0 = n0Subdivided ? n0->getChild(3) : n0;
            const OctreeNode *c1 = n0Subdivided ? n0->getChild(2) : n0;
            const OctreeNode *c2 = n1Subdivided ? n1->getChild(1) : n1;
            const OctreeNode *c3 = n1Subdivided ? n1->getChild(0) : n1;

            const OctreeNode *c4 = n0Subdivided ? n0->getChild(7) : n0;
            const OctreeNode *c5 = n0Subdivided ? n0->getChild(6) : n0;
            const OctreeNode *c6 = n1Subdivided ? n1->getChild(5) : n1;
            const OctreeNode *c7 = n1Subdivided ? n1->getChild(4) : n1;

            faceProcXY(c0, c3);
            faceProcXY(c1, c2);
            faceProcXY(c4, c7);
            faceProcXY(c5, c6);

            edgeProcX(c0, c3, c7, c4);
            edgeProcX(c1, c2, c6, c5);
            edgeProcY(c0, c1, c2, c3);
            edgeProcY(c4, c5, c6, c7);

            vertProc(c0, c1, c2, c3, c4, c5, c6, c7);
        }
    }

    //-----------------------------------------------------------------------

    void DualGridGenerator::faceProcZY(const OctreeNode *n0, const OctreeNode *n1)
    {
        const bool n0Subdivided = n0->isSubdivided();
        const bool n1Subdivided = n1->isSubdivided();

        if (n0Subdivided || n1Subdivided)
        {
            const OctreeNode *c0 = n0Subdivided ? n0->getChild(1) : n0;
            const OctreeNode *c1 = n1Subdivided ? n1->getChild(0) : n1;
            const OctreeNode *c2 = n1Subdivided ? n1->getChild(3) : n1;
            const OctreeNode *c3 = n0Subdivided ? n0->getChild(2) : n0;

            const OctreeNode *c4 = n0Subdivided ? n0->getChild(5) : n0;
            const OctreeNode *c5 = n1Subdivided ? n1->getChild(4) : n1;
            const OctreeNode *c6 = n1Subdivided ? n1->getChild(7) : n1;
            const OctreeNode *c7 = n0Subdivided ? n0->getChild(6) : n0;

            faceProcZY(c0, c1);
            faceProcZY(c3, c2);
            faceProcZY(c4, c5);
            faceProcZY(c7, c6);

            edgeProcY(c0, c1, c2, c3);
            edgeProcY(c4, c5, c6, c7);
            edgeProcZ(c7, c6, c2, c3);
            edgeProcZ(c4, c5, c1, c0);

            vertProc(c0, c1, c2, c3, c4, c5, c6, c7);
        }
    }

    //-----------------------------------------------------------------------

    void DualGridGenerator::faceProcXZ(const OctreeNode *n0, const OctreeNode *n1)
    {
        const bool n0Subdivided = n0->isSubdivided();
        const bool n1Subdivided = n1->isSubdivided();

        if (n0Subdivided || n1Subdivided)
        {
            const OctreeNode *c0 = n1Subdivided ? n1->getChild(4) : n1;
            const OctreeNode *c1 = n1Subdivided ? n1->getChild(5) : n1;
            const OctreeNode *c2 = n1Subdivided ? n1->getChild(6) : n1;
            const OctreeNode *c3 = n1Subdivided ? n1->getChild(7) : n1;

            const OctreeNode *c4 = n0Subdivided ? n0->getChild(0) : n0;
            const OctreeNode *c5 = n0Subdivided ? n0->getChild(1) : n0;
            const OctreeNode *c6 = n0Subdivided ? n0->getChild(2) : n0;
            const OctreeNode *c7 = n0Subdivided ? n0->getChild(3) : n0;

            faceProcXZ(c4, c0);
            faceProcXZ(c5, c1);
            faceProcXZ(c7, c3);
            faceProcXZ(c6, c2);

            edgeProcX(c0, c3, c7, c4);
            edgeProcX(c1, c2, c6, c5);
            edgeProcZ(c7, c6, c2, c3);
            edgeProcZ(c4, c5, c1, c0);

            vertProc(c0, c1, c2, c3, c4, c5, c6, c7);
        }
    }

    //-----------------------------------------------------------------------

    void DualGridGenerator::edgeProcX(const OctreeNode *n0, const OctreeNode *n1, const OctreeNode *n2, const OctreeNode *n3)
    {
        const bool n0Subdivided = n0->isSubdivided();
        const bool n1Subdivided = n1->isSubdivided();
        const bool n2Subdivided = n2->isSubdivided();
        const bool n3Subdivided = n3->isSubdivided();

        if (n0Subdivided || n1Subdivided || n2Subdivided || n3Subdivided)
        {
            const OctreeNode *c0 = n0Subdivided ? n0->getChild(7) : n0;
            const OctreeNode *c1 = n0Subdivided ? n0->getChild(6) : n0;
            const OctreeNode *c2 = n1Subdivided ? n1->getChild(5) : n1;
            const OctreeNode *c3 = n1Subdivided ? n1->getChild(4) : n1;
            const OctreeNode *c4 = n3Subdivided ? n3->getChild(3) : n3;
            const OctreeNode *c5 = n3Subdivided ? n3->getChild(2) : n3;
            const OctreeNode *c6 = n2Subdivided ? n2->getChild(1) : n2;
            const OctreeNode *c7 = n2Subdivided ? n2->getChild(0) : n2;

            edgeProcX(c0, c3, c7, c4);
            edgeProcX(c1, c2, c6, c5);

            vertProc(c0, c1, c2, c3, c4, c5, c6, c7);
        }
    }

    //-----------------------------------------------------------------------

    void DualGridGenerator::edgeProcY(const OctreeNode *n0, const OctreeNode *n1, const OctreeNode *n2, const OctreeNode *n3)
    {
        const bool n0Subdivided = n0->isSubdivided();
        const bool n1Subdivided = n1->isSubdivided();
        const bool n2Subdivided = n2->isSubdivided();
        const bool n3Subdivided = n3->isSubdivided();

        if (n0Subdivided || n1Subdivided || n2Subdivided || n3Subdivided)
        {
            const OctreeNode *c0 = n0Subdivided ? n0->getChild(2) : n0;
            const OctreeNode *c1 = n1Subdivided ? n1->getChild(3) : n1;
            const OctreeNode *c2 = n2Subdivided ? n2->getChild(0) : n2;
            const OctreeNode *c3 = n3Subdivided ? n3->getChild(1) : n3;
            const OctreeNode *c4 = n0Subdivided ? n0->getChild(6) : n0;
            const OctreeNode *c5 = n1Subdivided ? n1->getChild(7) : n1;
            const OctreeNode *c6 = n2Subdivided ? n2->getChild(4) : n2;
            const OctreeNode *c7 = n3Subdivided ? n3->getChild(5) : n3;

            edgeProcY(c0, c1, c2, c3);
            edgeProcY(c4, c5, c6, c7);

            vertProc(c0, c1, c2, c3, c4, c5, c6, c7);
        }
    }

    //-----------------------------------------------------------------------

    void DualGridGenerator::edgeProcZ(const OctreeNode *n0, const OctreeNode *n1, const OctreeNode *n2, const OctreeNode *n3)
    {
        const bool n0Subdivided = n0->isSubdivided();
        const bool n1Subdivided = n1->isSubdivided();
        const bool n2Subdivided = n2->isSubdivided();
        const bool n3Subdivided = n3->isSubdivided();

        if (n0Subdivided || n1Subdivided || n2Subdivided || n3Subdivided)
        {
            const OctreeNode *c0 = n3Subdivided ? n3->getChild(5) : n3;
            const OctreeNode *c1 = n2Subdivided ? n2->getChild(4) : n2;
            const OctreeNode *c2 = n2Subdivided ? n2->getChild(7) : n2;
            const OctreeNode *c3 = n3Subdivided ? n3->getChild(6) : n3;
            const OctreeNode *c4 = n0Subdivided ? n0->getChild(1) : n0;
            const OctreeNode *c5 = n1Subdivided ? n1->getChild(0) : n1;
            const OctreeNode *c6 = n1Subdivided ? n1->getChild(3) : n1;
            const OctreeNode *c7 = n0Subdivided ? n0->getChild(2) : n0;

            edgeProcZ(c7, c6, c2, c3);
            edgeProcZ(c4, c5, c1, c0);

            vertProc(c0, c1, c2, c3, c4, c5, c6, c7);
        }
    }

    //-----------------------------------------------------------------------

    void DualGridGenerator::vertProc(const OctreeNode *n0, const OctreeNode *n1, const OctreeNode *n2, const OctreeNode *n3, const OctreeNode *n4, const OctreeNode *n5, const OctreeNode *n6, const OctreeNode *n7)
    {
        const bool n0Subdivided = n0->isSubdivided();
        const bool n1Subdivided = n1->isSubdivided();
        const bool n2Subdivided = n2->isSubdivided();
        const bool n3Subdivided = n3->isSubdivided();
        const bool n4Subdivided = n4->isSubdivided();
        const bool n5Subdivided = n5->isSubdivided();
        const bool n6Subdivided = n6->isSubdivided();
        const bool n7Subdivided = n7->isSubdivided();

        if (n0Subdivided || n1Subdivided || n2Subdivided || n3Subdivided ||
            n4Subdivided || n5Subdivided || n6Subdivided || n7Subdivided)
        {
            const OctreeNode *c0 = n0Subdivided ? n0->getChild(6) : n0;
            const OctreeNode *c1 = n1Subdivided ? n1->getChild(7) : n1;
            const OctreeNode *c2 = n2Subdivided ? n2->getChild(4) : n2;
            const OctreeNode *c3 = n3Subdivided ? n3->getChild(5) : n3;
            const OctreeNode *c4 = n4Subdivided ? n4->getChild(2) : n4;
            const OctreeNode *c5 = n5Subdivided ? n5->getChild(3) : n5;
            const OctreeNode *c6 = n6Subdivided ? n6->getChild(0) : n6;
            const OctreeNode *c7 = n7Subdivided ? n7->getChild(1) : n7;

            vertProc(c0, c1, c2, c3, c4, c5, c6, c7);
        }
        else
        {

            if (!n0->isIsoSurfaceNear() && !n1->isIsoSurfaceNear() && !n2->isIsoSurfaceNear() && !n3->isIsoSurfaceNear() &&
                !n4->isIsoSurfaceNear() && !n5->isIsoSurfaceNear() && !n6->isIsoSurfaceNear() && !n7->isIsoSurfaceNear())
            {
                return;
            }

            Vector4 values[8];
            values[0] = n0->getCenterValue();
            values[1] = n1->getCenterValue();
            values[2] = n2->getCenterValue();
            values[3] = n3->getCenterValue();
            values[4] = n4->getCenterValue();
            values[5] = n5->getCenterValue();
            values[6] = n6->getCenterValue();
            values[7] = n7->getCenterValue();
            addDualCell(n0->getCenter(), n1->getCenter(), n2->getCenter(), n3->getCenter(),
                n4->getCenter(), n5->getCenter(), n6->getCenter(), n7->getCenter(), values);
            createBorderCells(n0, n1, n2, n3, n4, n5, n6, n7);
        }
    }

    //-----------------------------------------------------------------------

    void DualGridGenerator::createBorderCells(const OctreeNode *n0, const OctreeNode *n1, const OctreeNode *n2, const OctreeNode *n3, const OctreeNode *n4, const OctreeNode *n5, const OctreeNode *n6, const OctreeNode *n7)
    {
        if (n0->isBorderBack(*mRoot) && n1->isBorderBack(*mRoot) && n4->isBorderBack(*mRoot) && n5->isBorderBack(*mRoot))
        {
            addDualCell(n0->getCenterBack(), n1->getCenterBack(), n1->getCenter(), n0->getCenter(),
                n4->getCenterBack(), n5->getCenterBack(), n5->getCenter(), n4->getCenter());
            // Generate back edge border cells
            if (n4->isBorderTop(*mRoot) && n5->isBorderTop(*mRoot))
            {
                addDualCell(n4->getCenterBack(), n5->getCenterBack(), n5->getCenter(), n4->getCenter(),
                    n4->getCenterBackTop(), n5->getCenterBackTop(), n5->getCenterTop(), n4->getCenterTop());
                // Generate back top corner cells
                if (n4->isBorderLeft(*mRoot))
                {
                    addDualCell(n4->getCenterBackLeft(), n4->getCenterBack(), n4->getCenter(), n4->getCenterLeft(),
                        n4->getCorner4(), n4->getCenterBackTop(), n4->getCenterTop(), n4->getCenterLeftTop());
                }
                if (n5->isBorderRight(*mRoot))
                {
                    addDualCell(n5->getCenterBack(), n5->getCenterBackRight(), n5->getCenterRight(), n5->getCenter(),
                        n5->getCenterBackTop(), n5->getCorner5(), n5->getCenterRightTop(), n5->getCenterTop());
                }
            }
            if (n0->isBorderBottom(*mRoot) && n1->isBorderBottom(*mRoot))
            {
                addDualCell(n0->getCenterBackBottom(), n1->getCenterBackBottom(), n1->getCenterBottom(), n0->getCenterBottom(),
                    n0->getCenterBack(), n1->getCenterBack(), n1->getCenter(), n0->getCenter());
                // Generate back bottom corner cells
                if (n0->isBorderLeft(*mRoot))
                {
                    addDualCell(n0->getFrom(), n0->getCenterBackBottom(), n0->getCenterBottom(), n0->getCenterLeftBottom(),
                        n0->getCenterBackLeft(), n0->getCenterBack(), n0->getCenter(), n0->getCenterLeft());
                }
                if (n1->isBorderRight(*mRoot))
                {
                    addDualCell(n1->getCenterBackBottom(), n1->getCorner1(), n1->getCenterRightBottom(), n1->getCenterBottom(),
                        n1->getCenterBack(), n1->getCenterBackRight(), n1->getCenterRight(), n1->getCenter());
                }
            }
        }
        if (n2->isBorderFront(*mRoot) && n3->isBorderFront(*mRoot) && n6->isBorderFront(*mRoot) && n7->isBorderFront(*mRoot))
        {
            addDualCell(n3->getCenter(), n2->getCenter(), n2->getCenterFront(), n3->getCenterFront(),
                n7->getCenter(), n6->getCenter(), n6->getCenterFront(), n7->getCenterFront());
            // Generate front edge border cells
            if (n6->isBorderTop(*mRoot) && n7->isBorderTop(*mRoot))
            {
                addDualCell(n7->getCenter(), n6->getCenter(), n6->getCenterFront(), n7->getCenterFront(),
                    n7->getCenterTop(), n6->getCenterTop(), n6->getCenterFrontTop(), n7->getCenterFrontTop());
                // Generate back bottom corner cells
                if (n7->isBorderLeft(*mRoot))
                {
                    addDualCell(n7->getCenterLeft(), n7->getCenter(), n7->getCenterFront(), n7->getCenterFrontLeft(),
                        n7->getCenterLeftTop(), n7->getCenterTop(), n7->getCenterFrontTop(), n7->getCorner7());
                }
                if (n6->isBorderRight(*mRoot))
                {
                    addDualCell(n6->getCenter(), n6->getCenterRight(), n6->getCenterFrontRight(), n6->getCenterFront(),
                        n6->getCenterTop(), n6->getCenterRightTop(), n6->getTo(), n6->getCenterFrontTop());
                }
            }
            if (n3->isBorderBottom(*mRoot) && n2->isBorderBottom(*mRoot))
            {
                addDualCell(n3->getCenterBottom(), n2->getCenterBottom(), n2->getCenterFrontBottom(), n3->getCenterFrontBottom(),
                    n3->getCenter(), n2->getCenter(), n2->getCenterFront(), n3->getCenterFront());
                // Generate back bottom corner cells
                if (n3->isBorderLeft(*mRoot))
                {
                    addDualCell(n3->getCenterLeftBottom(), n3->getCenterBottom(), n3->getCenterFrontBottom(), n3->getCorner3(),
                        n3->getCenterLeft(), n3->getCenter(), n3->getCenterFront(), n3->getCenterFrontLeft());
                }
                if (n2->isBorderRight(*mRoot))
                {
                    addDualCell(n2->getCenterBottom(), n2->getCenterRightBottom(), n2->getCorner2(), n2->getCenterFrontBottom(),
                        n2->getCenter(), n2->getCenterRight(), n2->getCenterFrontRight(), n2->getCenterFront());
                }
            }
        }
        if (n0->isBorderLeft(*mRoot) && n3->isBorderLeft(*mRoot) && n4->isBorderLeft(*mRoot) && n7->isBorderLeft(*mRoot))
        {
            addDualCell(n0->getCenterLeft(), n0->getCenter(), n3->getCenter(), n3->getCenterLeft(),
                n4->getCenterLeft(), n4->getCenter(), n7->getCenter(), n7->getCenterLeft());
            // Generate left edge border cells
            if (n4->isBorderTop(*mRoot) && n7->isBorderTop(*mRoot))
            {
                addDualCell(n4->getCenterLeft(), n4->getCenter(), n7->getCenter(), n7->getCenterLeft(),
                    n4->getCenterLeftTop(), n4->getCenterTop(), n7->getCenterTop(), n7->getCenterLeftTop());
            }
            if (n0->isBorderBottom(*mRoot) && n3->isBorderBottom(*mRoot))
            {
                addDualCell(n0->getCenterLeftBottom(), n0->getCenterBottom(), n3->getCenterBottom(), n3->getCenterLeftBottom(),
                    n0->getCenterLeft(), n0->getCenter(), n3->getCenter(), n3->getCenterLeft());
            }
            if (n0->isBorderBack(*mRoot) && n4->isBorderBack(*mRoot))
            {
                addDualCell(n0->getCenterBackLeft(), n0->getCenterBack(), n0->getCenter(), n0->getCenterLeft(),
                    n4->getCenterBackLeft(), n4->getCenterBack(), n4->getCenter(), n4->getCenterLeft());
            }
            if (n3->isBorderFront(*mRoot) && n7->isBorderFront(*mRoot))
            {
                addDualCell(n3->getCenterLeft(), n3->getCenter(), n3->getCenterFront(), n3->getCenterFrontLeft(),
                    n7->getCenterLeft(), n7->getCenter(), n7->getCenterFront(), n7->getCenterFrontLeft());
            }
        }
        if (n1->isBorderRight(*mRoot) && n2->isBorderRight(*mRoot) && n5->isBorderRight(*mRoot) && n6->isBorderRight(*mRoot))
        {
            addDualCell(n1->getCenter(), n1->getCenterRight(), n2->getCenterRight(), n2->getCenter(),
                n5->getCenter(), n5->getCenterRight(), n6->getCenterRight(), n6->getCenter());
            // Generate right edge border cells
            if (n5->isBorderTop(*mRoot) && n6->isBorderTop(*mRoot))
            {
                addDualCell(n5->getCenter(), n5->getCenterRight(), n6->getCenterRight(), n6->getCenter(),
                    n5->getCenterTop(), n5->getCenterRightTop(), n6->getCenterRightTop(), n6->getCenterTop());
            }
            if (n1->isBorderBottom(*mRoot) && n2->isBorderBottom(*mRoot))
            {
                addDualCell(n1->getCenterBottom(), n1->getCenterRightBottom(), n2->getCenterRightBottom(), n2->getCenterBottom(),
                    n1->getCenter(), n1->getCenterRight(), n2->getCenterRight(), n2->getCenter());
            }
            if (n1->isBorderBack(*mRoot) && n5->isBorderBack(*mRoot))
            {
                addDualCell(n1->getCenterBack(), n1->getCenterBackRight(), n1->getCenterRight(), n1->getCenter(),
                    n5->getCenterBack(), n5->getCenterBackRight(), n5->getCenterRight(), n5->getCenter());
            }
            if (n2->isBorderFront(*mRoot) && n6->isBorderFront(*mRoot))
            {
                addDualCell(n2->getCenter(), n2->getCenterRight(), n2->getCenterFrontRight(), n2->getCenterFront(),
                    n6->getCenter(), n6->getCenterRight(), n6->getCenterFrontRight(), n6->getCenterFront());
            }
        }
        if (n4->isBorderTop(*mRoot) && n5->isBorderTop(*mRoot) && n6->isBorderTop(*mRoot) && n7->isBorderTop(*mRoot))
        {
            addDualCell(n4->getCenter(), n5->getCenter(), n6->getCenter(), n7->getCenter(),
                n4->getCenterTop(), n5->getCenterTop(), n6->getCenterTop(), n7->getCenterTop());
        }
        if (n0->isBorderBottom(*mRoot) && n1->isBorderBottom(*mRoot) && n2->isBorderBottom(*mRoot) && n3->isBorderBottom(*mRoot))
        {
            addDualCell(n0->getCenterBottom(), n1->getCenterBottom(), n2->getCenterBottom(), n3->getCenterBottom(),
                n0->getCenter(), n1->getCenter(), n2->getCenter(), n3->getCenter());
        }
    }

    //-----------------------------------------------------------------------

    DualGridGenerator::DualGridGenerator(): mDualGrid(0), mRoot(0), mSaveDualCells(0), mIs(0), mMb(0), mMaxMSDistance(0)
    {
    }

    //-----------------------------------------------------------------------

    void DualGridGenerator::generateDualGrid(const OctreeNode *root, IsoSurface *is, MeshBuilder *mb, Real maxMSDistance, const Vector3 &totalFrom, const Vector3 &totalTo, bool saveDualCells)
    {
        mRoot = root;
        mIs = is;
        mMb = mb;
        mMaxMSDistance = maxMSDistance;
        mTotalFrom = totalFrom;
        mTotalTo = totalTo;
        mSaveDualCells = saveDualCells;

        nodeProc(root);

        // Build up a minimal dualgrid for octrees without children.
        if (!root->isSubdivided())
        {
            addDualCell(root->getFrom(), root->getCenterBackBottom(), root->getCenterBottom(), root->getCenterLeftBottom(),
                root->getCenterBackLeft(), root->getCenterBack(), root->getCenter(), root->getCenterLeft());
            addDualCell(root->getCenterBackBottom(), root->getCorner1(), root->getCenterRightBottom(), root->getCenterBottom(),
                root->getCenterBack(), root->getCenterBackRight(), root->getCenterRight(), root->getCenter());
            addDualCell(root->getCenterBottom(), root->getCenterRightBottom(), root->getCorner2(), root->getCenterFrontBottom(),
                root->getCenter(), root->getCenterRight(), root->getCenterFrontRight(), root->getCenterFront());
            addDualCell(root->getCenterLeftBottom(), root->getCenterBottom(), root->getCenterFrontBottom(), root->getCorner3(),
                root->getCenterLeft(), root->getCenter(), root->getCenterFront(), root->getCenterFrontLeft());

            addDualCell(root->getCenterBackLeft(), root->getCenterBack(), root->getCenter(), root->getCenterLeft(),
                root->getCorner4(), root->getCenterBackTop(), root->getCenterTop(), root->getCenterLeftTop());
            addDualCell(root->getCenterBack(), root->getCenterBackRight(), root->getCenterRight(), root->getCenter(),
                root->getCenterBackTop(), root->getCorner5(), root->getCenterRightTop(), root->getCenterTop());
            addDualCell(root->getCenter(), root->getCenterRight(), root->getCenterFrontRight(), root->getCenterFront(),
                root->getCenterTop(), root->getCenterRightTop(), root->getTo(), root->getCenterFrontTop());
            addDualCell(root->getCenterLeft(), root->getCenter(), root->getCenterFront(), root->getCenterFrontLeft(),
                root->getCenterLeftTop(), root->getCenterTop(), root->getCenterFrontTop(), root->getCorner7());
        }
    }

    //-----------------------------------------------------------------------

    Entity* DualGridGenerator::getDualGrid(SceneManager *sceneManager)
    {
        if (!mDualGrid && mDualCells.size() > 0)
        {
            ManualObject* manual = sceneManager->createManualObject();
            manual->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_LIST);
            manual->colour((Real)0.0, (Real)1.0, (Real)0.0);
            manual->estimateVertexCount(mDualCells.size() * 8);
            manual->estimateIndexCount(mDualCells.size() * 24);

            uint32 baseIndex = 0;
            for (const auto& c : mDualCells)
            {
                MeshBuilder::addCubeToManualObject(
                    manual,
                    c.mC0,
                    c.mC1,
                    c.mC2,
                    c.mC3,
                    c.mC4,
                    c.mC5,
                    c.mC6,
                    c.mC7,
                    baseIndex);
            }

            manual->end();
            mDualGridI++;
            StringStream meshName;
            meshName << "VolumeDualGridGridMesh" << mDualGridI;
            manual->convertToMesh(meshName.str());
            StringStream entityName;
            entityName << "VolumeDualGrid" << mDualGridI;
            mDualGrid = sceneManager->createEntity(entityName.str(), meshName.str());
        }
        return mDualGrid;
    }

}
}
