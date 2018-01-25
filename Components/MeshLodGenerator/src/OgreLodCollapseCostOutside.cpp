/*
 * -----------------------------------------------------------------------------
 * This source file is part of OGRE
 * (Object-oriented Graphics Rendering Engine)
 * For the latest info, see http://www.ogre3d.org/
 *
 * Copyright (c) 2000-2014 Torus Knot Software Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * -----------------------------------------------------------------------------
 */

#include "OgreMeshLodPrecompiledHeaders.h"

namespace Ogre
{
    LodCollapseCostOutside::LodCollapseCostOutside( LodCollapseCostPtr costCalculator, Real outsideWeight, Real outsideWalkAngle ) :
        mOutsideWeight(outsideWeight),
        mOutsideWalkAngle(outsideWalkAngle),
        mCostCalculator(costCalculator),
        mOutsideMarker(NULL)
    {
    }

    LodCollapseCostOutside::~LodCollapseCostOutside()
    {
        delete mOutsideMarker;
    }

    void LodCollapseCostOutside::initCollapseCosts( LodData* data )
    {
        OgreAssert(mOutsideWeight != 0.0, "");

        delete mOutsideMarker;
        mOutsideMarker = new LodOutsideMarker(data->mVertexList, data->mMeshBoundingSphereRadius, mOutsideWalkAngle);
        mOutsideMarker->markOutside();


        mCostCalculator->initCollapseCosts(data);
    }

    Real LodCollapseCostOutside::computeEdgeCollapseCost( LodData* data, LodData::Vertex* src, LodData::Edge* dstEdge )
    {
        Real cost = mCostCalculator->computeEdgeCollapseCost(data, src, dstEdge);
        if(mOutsideMarker->isVertexOutside(src) || mOutsideMarker->isVertexOutside(dstEdge->dst)) {
            if(mOutsideWeight != LodData::NEVER_COLLAPSE_COST && cost != LodData::NEVER_COLLAPSE_COST) {
                cost *= std::max<Real>(0.0078125f, mOutsideWeight * 8.0f);
            } else {
                return LodData::NEVER_COLLAPSE_COST;
            }
        }
        OgreAssert(cost >= 0 && cost != LodData::UNINITIALIZED_COLLAPSE_COST, "Invalid collapse cost");
        return cost;
    }
}

