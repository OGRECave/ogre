
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

#ifndef _LodCollapseCost_H__
#define _LodCollapseCost_H__

#include "OgreLodPrerequisites.h"
#include "OgreLodData.h"

namespace Ogre
{
/** \addtogroup Optional
*  @{
*/
/** \addtogroup MeshLodGenerator
*  @{
*/
class _OgreLodExport LodCollapseCost {
public:
    virtual ~LodCollapseCost() {}
    /// This is called after the LodInputProvider has initialized LodData.
    virtual void initCollapseCosts(LodData* data);
    /// Called from initCollapseCosts for every edge.
    virtual void initVertexCollapseCost(LodData* data, LodData::Vertex* vertex);
    /// Called when edge cost gets invalid.
    virtual void updateVertexCollapseCost(LodData* data, LodData::Vertex* vertex);
    /// Called by initVertexCollapseCost and updateVertexCollapseCost, when the vertex minimal cost needs to be updated.
    virtual void computeVertexCollapseCost(LodData* data, LodData::Vertex* vertex, Real& collapseCost, LodData::Vertex*& collapseTo);
    /// Returns the collapse cost of the given edge. 
    virtual Real computeEdgeCollapseCost(LodData* data, LodData::Vertex* src, LodData::Edge* dstEdge) = 0;
protected:
    // Helper functions:
    bool isBorderVertex(const LodData::Vertex* vertex) const;
};
/** @} */
/** @} */
}
#endif


