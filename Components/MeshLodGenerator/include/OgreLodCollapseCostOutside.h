
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

#ifndef _LodCollapseCostOutside_H__
#define _LodCollapseCostOutside_H__

#include "OgreLodPrerequisites.h"
#include "OgreLodCollapseCost.h"
#include "OgreLodData.h"
#include "OgreSharedPtr.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{

class _OgreLodExport LodCollapseCostOutside :
    public LodCollapseCost
{
public:
    LodCollapseCostOutside(LodCollapseCostPtr costCalculator, Real outsideWeight, Real outsideWalkAngle);
    ~LodCollapseCostOutside();
    virtual void initCollapseCosts(LodData* data);
    virtual Real computeEdgeCollapseCost(LodData* data, LodData::Vertex* src, LodData::Edge* dstEdge);
private:

    Real mOutsideWeight;
    Real mOutsideWalkAngle;

    // Result of this collapse cost algorithm will be modified, if it is outside.
    LodCollapseCostPtr mCostCalculator;
    LodOutsideMarker* mOutsideMarker;
};

}

#include "OgreHeaderSuffix.h"

#endif


