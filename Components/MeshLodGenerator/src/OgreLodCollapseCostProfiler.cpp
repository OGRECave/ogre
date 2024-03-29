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

    Real LodCollapseCostProfiler::computeEdgeCollapseCost( LodData* data, LodData::Vertex* src, LodData::Edge* dstEdge )
    {
        OgreAssert(0, "Only computeVertexCollapseCost should call this function.");
        return 0;
    }

    void LodCollapseCostProfiler::computeVertexCollapseCost( LodData* data, LodData::Vertex* vertex, Real& collapseCost, LodData::Vertex*& collapseTo )
    {
        if(!mHasProfile[LodData::getVectorIDFromPointer(data->mVertexList, vertex)]){
            for (auto& e : vertex->edges) {
                e.collapseCost = mCostCalculator->computeEdgeCollapseCost(data, vertex, &e);
                if (collapseCost > e.collapseCost) {
                    collapseCost = e.collapseCost;
                    collapseTo = e.dst;
                }
            }
        } else {
            std::pair<ProfileLookup::iterator, ProfileLookup::iterator> ret = mProfileLookup.equal_range(vertex);
            for (auto& e : vertex->edges) {
                e.collapseCost = LodData::UNINITIALIZED_COLLAPSE_COST;
                for(ProfileLookup::iterator it = ret.first; it != ret.second; ++it){
                    if(it->second.dst == e.dst ){
                        e.collapseCost = it->second.cost;
                        break;
                    }
                }
                if(e.collapseCost == LodData::UNINITIALIZED_COLLAPSE_COST){
                    e.collapseCost = mCostCalculator->computeEdgeCollapseCost(data, vertex, &e);
                }
                if (collapseCost > e.collapseCost) {
                    collapseCost = e.collapseCost;
                    collapseTo = e.dst;
                }
            }
        }
    }

    void LodCollapseCostProfiler::injectProfile( LodData* data)
    {
        mHasProfile.clear();
        mHasProfile.resize(data->mVertexList.size(), false);
        for(auto& p : mProfile){
            LodData::Vertex v;
            v.position = p.src;
            LodData::UniqueVertexSet::iterator src = data->mUniqueVertexSet.find(&v);
            OgreAssert(src != data->mUniqueVertexSet.end(), "Invalid vertex position in Lod profile");
            mHasProfile[LodData::getVectorIDFromPointer(data->mVertexList, *src)] = true;
            v.position = p.dst;
            LodData::UniqueVertexSet::iterator dst = data->mUniqueVertexSet.find(&v);
            OgreAssert(dst != data->mUniqueVertexSet.end(), "Invalid vertex position in Lod profile");
            ProfiledEdge e;
            e.dst = *dst;
            e.cost = p.cost;
            OgreAssert(e.cost >= 0 && e.cost != LodData::UNINITIALIZED_COLLAPSE_COST, "Invalid collapse cost");
            mProfileLookup.emplace(*src, e);
        }
    }

    void LodCollapseCostProfiler::initCollapseCosts( LodData* data )
    {
        injectProfile(data);
        mProfile.clear();
        mCostCalculator->initCollapseCosts(data);
    }

}

