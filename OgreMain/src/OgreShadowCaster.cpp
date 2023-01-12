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
#include "OgreStableHeaders.h"
#include "OgreEdgeListBuilder.h"
#include "OgreOptimisedUtil.h"

namespace Ogre {
    ShadowRenderable::ShadowRenderable(MovableObject* parent, const HardwareIndexBufferSharedPtr& indexBuffer,
                                   const VertexData* vertexData, bool createSeparateLightCap,
                                   bool isLightCap)
    : mLightCap(0), mParent(parent)
    {
        // Initialise render op
        mRenderOp.indexData = OGRE_NEW IndexData();
        mRenderOp.indexData->indexBuffer = indexBuffer;
        mRenderOp.indexData->indexStart = 0;
        // index start and count are sorted out later

        // Create vertex data which just references position component (and 2 component)
        mRenderOp.vertexData = OGRE_NEW VertexData();
        // Map in position data
        mRenderOp.vertexData->vertexDeclaration->addElement(0,0,VET_FLOAT3, VES_POSITION);
        ushort origPosBind =
            vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION)->getSource();
        mPositionBuffer = vertexData->vertexBufferBinding->getBuffer(origPosBind);
        mRenderOp.vertexData->vertexBufferBinding->setBinding(0, mPositionBuffer);
        // Map in w-coord buffer (if present)
        if(vertexData->hardwareShadowVolWBuffer)
        {
            mRenderOp.vertexData->vertexDeclaration->addElement(1,0,VET_FLOAT1, VES_TEXTURE_COORDINATES, 0);
            mWBuffer = vertexData->hardwareShadowVolWBuffer;
            mRenderOp.vertexData->vertexBufferBinding->setBinding(1, mWBuffer);
        }
        // Use same vertex start as input
        mRenderOp.vertexData->vertexStart = vertexData->vertexStart;

        if (isLightCap)
        {
            // Use original vertex count, no extrusion
            mRenderOp.vertexData->vertexCount = vertexData->vertexCount;
        }
        else
        {
            // Vertex count must take into account the doubling of the buffer,
            // because second half of the buffer is the extruded copy
            mRenderOp.vertexData->vertexCount = vertexData->vertexCount * 2;

            if (createSeparateLightCap)
            {
                // Create child light cap
                mLightCap = OGRE_NEW ShadowRenderable(parent, indexBuffer, vertexData, false, true);
            }
        }
    }
    ShadowRenderable::~ShadowRenderable()
    {
        delete mLightCap;
        delete mRenderOp.indexData;
        delete mRenderOp.vertexData;
    }
    void ShadowRenderable::rebindIndexBuffer(const HardwareIndexBufferSharedPtr& indexBuffer)
    {
        mRenderOp.indexData->indexBuffer = indexBuffer;
        if (mLightCap) mLightCap->rebindIndexBuffer(indexBuffer);
    }
    void ShadowRenderable::getWorldTransforms(Matrix4* xform) const
    {
        *xform = mParent->_getParentNodeFullTransform();
    }
    const LightList& ShadowRenderable::getLights(void) const 
    {
        // return empty
        static LightList ll;
        return ll;
    }
    // ------------------------------------------------------------------------
    void ShadowCaster::clearShadowRenderableList(ShadowRenderableList& shadowRenderables)
    {
        for(auto & shadowRenderable : shadowRenderables)
        {
            OGRE_DELETE shadowRenderable;
            shadowRenderable = 0;
        }
        shadowRenderables.clear();
    }
    // ------------------------------------------------------------------------
    void ShadowCaster::updateEdgeListLightFacing(EdgeData* edgeData, 
        const Vector4& lightPos)
    {
        edgeData->updateTriangleLightFacing(lightPos);
    }
    // ------------------------------------------------------------------------
    static bool isBoundOkForMcGuire(const AxisAlignedBox& lightCapBounds, const Ogre::Vector3& lightPosition)
    {
        // If light position is inside light cap bound then extrusion could be in opposite directions
        // and McGuire cap could intersect near clip plane of camera frustum without being noticed
        if(lightCapBounds.contains(lightPosition))
            return false;

        // If angular size of object is too high then extrusion could be in almost opposite directions,
        // interpolated points would be extruded by shorter distance, and strange geometry of McGuire cap
        // could be visible even for well tesselated meshes. As a heuristic we will avoid McGuire cap if
        // angular size is larger than 60 degrees - it guarantees that interpolated points would be
        // extruded by at least cos(60deg/2) ~ 86% of the original extrusion distance.
        if(lightCapBounds.getHalfSize().length() / (lightCapBounds.getCenter() - lightPosition).length() > 0.5) // if boundingSphereAngularSize > 60deg
        {
            // Calculate angular size one more time using edge corners angular distance comparision,
            // Determine lit sides of the bound, store in mask
            enum { L = 1, R = 2, B = 4, T = 8, F = 16, N = 32 }; // left, right, bottom, top, far, near
            unsigned lightSidesMask = 
                (lightPosition.x < lightCapBounds.getMinimum().x ? L : 0) | // left
                (lightPosition.x > lightCapBounds.getMaximum().x ? R : 0) | // right
                (lightPosition.y < lightCapBounds.getMinimum().y ? B : 0) | // bottom
                (lightPosition.y > lightCapBounds.getMaximum().y ? T : 0) | // top
                (lightPosition.z < lightCapBounds.getMinimum().z ? F : 0) | // far
                (lightPosition.z > lightCapBounds.getMaximum().z ? N : 0);  // near
            
            // find corners on lit/unlit edge (should not be more than 6 simultaneously, but better be safe than sorry)
            Ogre::Vector3 edgeCorners[8]; 
            unsigned edgeCornersCount = 0;
            std::pair<unsigned, AxisAlignedBox::CornerEnum> cornerMap[8] = {
                { F|L|B, AxisAlignedBox::FAR_LEFT_BOTTOM }, { F|R|B, AxisAlignedBox::FAR_RIGHT_BOTTOM },
                { F|L|T, AxisAlignedBox::FAR_LEFT_TOP },    { F|R|T, AxisAlignedBox::FAR_RIGHT_TOP },
                { N|L|B, AxisAlignedBox::NEAR_LEFT_BOTTOM },{ N|R|B, AxisAlignedBox::NEAR_RIGHT_BOTTOM },
                { N|L|T, AxisAlignedBox::NEAR_LEFT_TOP },   { N|R|T, AxisAlignedBox::NEAR_RIGHT_TOP }};
            for(auto& c : cornerMap)
                if((lightSidesMask & c.first) != 0 && (lightSidesMask & c.first) != c.first) // if adjacent sides not all lit or all unlit
                    edgeCorners[edgeCornersCount++] = lightCapBounds.getCorner(c.second);
            
            // find max angular size in range [0..pi] by finding min cos of angular size, range [1..-1]
            Real cosAngle = 1.0;
            for(unsigned i0 = 0; i0 + 1 < edgeCornersCount; ++i0)
                for(unsigned i1 = i0 + 1; i1 < edgeCornersCount; ++i1)
                {
                    // 4~6 edge corners, 6~15 angular distance calculations
                    Vector3 a = (edgeCorners[i0] - lightPosition).normalisedCopy();
                    Vector3 b = (edgeCorners[i1] - lightPosition).normalisedCopy();
                    Real cosAB = a.dotProduct(b);
                    if(cosAngle > cosAB)
                        cosAngle  = cosAB;
                }
            
            if(cosAngle < 0.5) // angularSize > 60 degrees
                return false;
        }

        return true;
    }
    // ------------------------------------------------------------------------
    void ShadowCaster::generateShadowVolume(EdgeData* edgeData, 
        const HardwareIndexBufferSharedPtr& indexBuffer, size_t& indexBufferUsedSize, 
        const Light* light, ShadowRenderableList& shadowRenderables, unsigned long flags)
    {
        // Edge groups should be 1:1 with shadow renderables
        assert(edgeData->edgeGroups.size() == shadowRenderables.size());

        Light::LightTypes lightType = light->getType();

        // Whether to use the McGuire method, a triangle fan covering all silhouette
        // This won't work properly with multiple separate edge groups (should be one fan per group, not implemented)
        // or when light position is too close to light cap bound.
        bool useMcGuire = edgeData->edgeGroups.size() <= 1 && 
            (lightType == Light::LT_DIRECTIONAL || isBoundOkForMcGuire(getLightCapBounds(), light->getDerivedPosition()));
        ShadowRenderableList::const_iterator si;

        // pre-count the size of index data we need since it makes a big perf difference
        // to GL in particular if we lock a smaller area of the index buffer
        size_t preCountIndexes = 0;

        si = shadowRenderables.begin();
        for (auto& eg : edgeData->edgeGroups)
        {
            bool  firstDarkCapTri = true;
            for (auto& edge :  eg.edges)
            {
                // Silhouette edge, when two tris has opposite light facing, or
                // degenerate edge where only tri 1 is valid and the tri light facing
                char lightFacing = edgeData->triangleLightFacings[edge.triIndex[0]];
                if ((edge.degenerate && lightFacing) ||
                    (!edge.degenerate && (lightFacing != edgeData->triangleLightFacings[edge.triIndex[1]])))
                {

                    preCountIndexes += 3;

                    // Are we extruding to infinity?
                    if (!(lightType == Light::LT_DIRECTIONAL &&
                        flags & SRF_EXTRUDE_TO_INFINITY))
                    {
                        preCountIndexes += 3;
                    }

                    if(useMcGuire)
                    {
                        // Do dark cap tri
                        // Use McGuire et al method, a triangle fan covering all silhouette
                        // edges and one point (taken from the initial tri)
                        if (flags & SRF_INCLUDE_DARK_CAP)
                        {
                            if (firstDarkCapTri)
                            {
                                firstDarkCapTri = false;
                            }
                            else
                            {
                                preCountIndexes += 3;
                            }
                        }
                    }
                }

            }

            if(useMcGuire)
            {
                // Do light cap
                if (flags & SRF_INCLUDE_LIGHT_CAP) 
                {
                    // Iterate over the triangles which are using this vertex set
                    EdgeData::TriangleList::const_iterator ti, tiend;
                    EdgeData::TriangleLightFacingList::const_iterator lfi;
                    ti = edgeData->triangles.begin() + eg.triStart;
                    tiend = ti + eg.triCount;
                    lfi = edgeData->triangleLightFacings.begin() + eg.triStart;
                    for ( ; ti != tiend; ++ti, ++lfi)
                    {
                        assert(ti->vertexSet == eg.vertexSet);
                        // Check it's light facing
                        if (*lfi)
                        {
                            preCountIndexes += 3;
                        }
                    }

                }
            }
            else
            {
                // Do both caps
                int increment = ((flags & SRF_INCLUDE_DARK_CAP) ? 3 : 0) + ((flags & SRF_INCLUDE_LIGHT_CAP) ? 3 : 0);
                if(increment != 0)
                {
                    // Iterate over the triangles which are using this vertex set
                    EdgeData::TriangleList::const_iterator ti, tiend;
                    EdgeData::TriangleLightFacingList::const_iterator lfi;
                    ti = edgeData->triangles.begin() + eg.triStart;
                    tiend = ti + eg.triCount;
                    lfi = edgeData->triangleLightFacings.begin() + eg.triStart;
                    for ( ; ti != tiend; ++ti, ++lfi)
                    {
                        assert(ti->vertexSet == eg.vertexSet);
                        // Check it's light facing
                        if (*lfi)
                            preCountIndexes += increment;
                    }
                }
            }
            ++si;
        }
        // End pre-count
        
        //Check if index buffer is to small 
        if (preCountIndexes > indexBuffer->getNumIndexes())
        {
            LogManager::getSingleton().logWarning(
                "shadow index buffer size to small. Auto increasing buffer size to" +
                StringConverter::toString(sizeof(unsigned short) * preCountIndexes));

            SceneManager* pManager = Root::getSingleton()._getCurrentSceneManager();
            if (pManager)
            {
                pManager->setShadowIndexBufferSize(preCountIndexes);
            }
            
            //Check that the index buffer size has actually increased
            if (preCountIndexes > indexBuffer->getNumIndexes())
            {
                //increasing index buffer size has failed
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                    "Lock request out of bounds.",
                    "ShadowCaster::generateShadowVolume");
            }
        }
        else if(indexBufferUsedSize + preCountIndexes > indexBuffer->getNumIndexes())
        {
            indexBufferUsedSize = 0;
        }

        // Lock index buffer for writing, just enough length as we need
        HardwareBufferLockGuard indexLock(indexBuffer,
            sizeof(unsigned short) * indexBufferUsedSize, sizeof(unsigned short) * preCountIndexes,
            indexBufferUsedSize == 0 ? HardwareBuffer::HBL_DISCARD : HardwareBuffer::HBL_NO_OVERWRITE);
        unsigned short* pIdx = static_cast<unsigned short*>(indexLock.pData);
        size_t numIndices = indexBufferUsedSize;
        
        // Iterate over the groups and form renderables for each based on their
        // lightFacing
        si = shadowRenderables.begin();
        for (auto& eg : edgeData->edgeGroups)
        {
            // Initialise the index start for this shadow renderable
            IndexData* indexData = (*si)->getRenderOperationForUpdate()->indexData;

            if (indexData->indexBuffer != indexBuffer)
            {
                (*si)->rebindIndexBuffer(indexBuffer);
                indexData = (*si)->getRenderOperationForUpdate()->indexData;
            }

            indexData->indexStart = numIndices;
            // original number of verts (without extruded copy)
            size_t originalVertexCount = eg.vertexData->vertexCount;
            bool  firstDarkCapTri = true;
            unsigned short darkCapStart = 0;

            for (auto& edge : eg.edges)
            {
                // Silhouette edge, when two tris has opposite light facing, or
                // degenerate edge where only tri 1 is valid and the tri light facing
                char lightFacing = edgeData->triangleLightFacings[edge.triIndex[0]];
                if ((edge.degenerate && lightFacing) ||
                    (!edge.degenerate && (lightFacing != edgeData->triangleLightFacings[edge.triIndex[1]])))
                {
                    size_t v0 = edge.vertIndex[0];
                    size_t v1 = edge.vertIndex[1];
                    if (!lightFacing)
                    {
                        // Inverse edge indexes when t1 is light away
                        std::swap(v0, v1);
                    }

                    /* Note edge(v0, v1) run anticlockwise along the edge from
                    the light facing tri so to point shadow volume tris outward,
                    light cap indexes have to be backwards

                    We emit 2 tris if light is a point light, 1 if light 
                    is directional, because directional lights cause all
                    points to converge to a single point at infinity.

                    First side tri = near1, near0, far0
                    Second tri = far0, far1, near1

                    'far' indexes are 'near' index + originalVertexCount
                    because 'far' verts are in the second half of the 
                    buffer
                    */
                    assert(v1 < 65536 && v0 < 65536 && (v0 + originalVertexCount) < 65536 &&
                        "Vertex count exceeds 16-bit index limit!");
                    *pIdx++ = static_cast<unsigned short>(v1);
                    *pIdx++ = static_cast<unsigned short>(v0);
                    *pIdx++ = static_cast<unsigned short>(v0 + originalVertexCount);
                    numIndices += 3;

                    // Are we extruding to infinity?
                    if (!(lightType == Light::LT_DIRECTIONAL &&
                        flags & SRF_EXTRUDE_TO_INFINITY))
                    {
                        // additional tri to make quad
                        *pIdx++ = static_cast<unsigned short>(v0 + originalVertexCount);
                        *pIdx++ = static_cast<unsigned short>(v1 + originalVertexCount);
                        *pIdx++ = static_cast<unsigned short>(v1);
                        numIndices += 3;
                    }

                    if(useMcGuire)
                    {
                        // Do dark cap tri
                        // Use McGuire et al method, a triangle fan covering all silhouette
                        // edges and one point (taken from the initial tri)
                        if (flags & SRF_INCLUDE_DARK_CAP)
                        {
                            if (firstDarkCapTri)
                            {
                                darkCapStart = static_cast<unsigned short>(v0 + originalVertexCount);
                                firstDarkCapTri = false;
                            }
                            else
                            {
                                *pIdx++ = darkCapStart;
                                *pIdx++ = static_cast<unsigned short>(v1 + originalVertexCount);
                                *pIdx++ = static_cast<unsigned short>(v0 + originalVertexCount);
                                numIndices += 3;
                            }

                        }
                    }
                }

            }

            if(!useMcGuire)
            {
                // Do dark cap
                if (flags & SRF_INCLUDE_DARK_CAP) 
                {
                    // Iterate over the triangles which are using this vertex set
                    EdgeData::TriangleList::const_iterator ti, tiend;
                    EdgeData::TriangleLightFacingList::const_iterator lfi;
                    ti = edgeData->triangles.begin() + eg.triStart;
                    tiend = ti + eg.triCount;
                    lfi = edgeData->triangleLightFacings.begin() + eg.triStart;
                    for ( ; ti != tiend; ++ti, ++lfi)
                    {
                        const EdgeData::Triangle& t = *ti;
                        assert(t.vertexSet == eg.vertexSet);
                        // Check it's light facing
                        if (*lfi)
                        {
                            assert(t.vertIndex[0] < 65536 && t.vertIndex[1] < 65536 &&
                                t.vertIndex[2] < 65536 && 
                                "16-bit index limit exceeded!");
                            *pIdx++ = static_cast<unsigned short>(t.vertIndex[1] + originalVertexCount);
                            *pIdx++ = static_cast<unsigned short>(t.vertIndex[0] + originalVertexCount);
                            *pIdx++ = static_cast<unsigned short>(t.vertIndex[2] + originalVertexCount);
                            numIndices += 3;
                        }
                    }

                }
            }

            // Do light cap
            if (flags & SRF_INCLUDE_LIGHT_CAP) 
            {
                // separate light cap?
                if ((*si)->isLightCapSeparate())
                {
                    // update index count for this shadow renderable
                    indexData->indexCount = numIndices - indexData->indexStart;

                    // get light cap index data for update
                    indexData = (*si)->getLightCapRenderable()->getRenderOperationForUpdate()->indexData;
                    // start indexes after the current total
                    indexData->indexStart = numIndices;
                }

                // Iterate over the triangles which are using this vertex set
                EdgeData::TriangleList::const_iterator ti, tiend;
                EdgeData::TriangleLightFacingList::const_iterator lfi;
                ti = edgeData->triangles.begin() + eg.triStart;
                tiend = ti + eg.triCount;
                lfi = edgeData->triangleLightFacings.begin() + eg.triStart;
                for ( ; ti != tiend; ++ti, ++lfi)
                {
                    const EdgeData::Triangle& t = *ti;
                    assert(t.vertexSet == eg.vertexSet);
                    // Check it's light facing
                    if (*lfi)
                    {
                        assert(t.vertIndex[0] < 65536 && t.vertIndex[1] < 65536 &&
                            t.vertIndex[2] < 65536 && 
                            "16-bit index limit exceeded!");
                        *pIdx++ = static_cast<unsigned short>(t.vertIndex[0]);
                        *pIdx++ = static_cast<unsigned short>(t.vertIndex[1]);
                        *pIdx++ = static_cast<unsigned short>(t.vertIndex[2]);
                        numIndices += 3;
                    }
                }

            }

            // update index count for current index data (either this shadow renderable or its light cap)
            indexData->indexCount = numIndices - indexData->indexStart;

            ++si;
        }

        // In debug mode, check we didn't overrun the index buffer
        assert(numIndices == indexBufferUsedSize + preCountIndexes);
        assert(numIndices <= indexBuffer->getNumIndexes() &&
            "Index buffer overrun while generating shadow volume!! "
            "You must increase the size of the shadow index buffer.");

        indexBufferUsedSize = numIndices;
    }
    // ------------------------------------------------------------------------
    void ShadowCaster::extrudeVertices(
        const HardwareVertexBufferSharedPtr& vertexBuffer, 
        size_t originalVertexCount, const Vector4& light, Real extrudeDist)
    {
        assert (vertexBuffer->getVertexSize() == sizeof(float) * 3
            && "Position buffer should contain only positions!");

        // Extrude the first area of the buffer into the second area
        // Lock the entire buffer for writing, even though we'll only be
        // updating the latter because you can't have 2 locks on the same
        // buffer
        HardwareBufferLockGuard vertexLock(vertexBuffer, HardwareBuffer::HBL_NORMAL);
        float* pSrc = static_cast<float*>(vertexLock.pData);

        // TODO: We should add extra (ununsed) vertices ensure source and
        // destination buffer have same alignment for slight performance gain.
        float* pDest = pSrc + originalVertexCount * 3;

        OptimisedUtil::getImplementation()->extrudeVertices(
            light, extrudeDist,
            pSrc, pDest, originalVertexCount);
    }
    // ------------------------------------------------------------------------
    void ShadowCaster::extrudeBounds(AxisAlignedBox& box, const Vector4& light, Real extrudeDist) const
    {
        Vector3 extrusionDir;

        if (light.w == 0)
        {
            // Parallel projection guarantees min/max relationship remains the same
            extrusionDir.x = -light.x;
            extrusionDir.y = -light.y;
            extrusionDir.z = -light.z;
            extrusionDir.normalise();
            extrusionDir *= extrudeDist;
            box.setExtents(box.getMinimum() + extrusionDir, 
                box.getMaximum() + extrusionDir);
        }
        else
        {
            Vector3 oldMin, oldMax, currentCorner;
            // Getting the original values
            oldMin = box.getMinimum();
            oldMax = box.getMaximum();
            // Starting the box again with a null content
            box.setNull();

            // merging all the extruded corners

            // 0 : min min min
            currentCorner = oldMin;
            extrusionDir.x = currentCorner.x - light.x;
            extrusionDir.y = currentCorner.y - light.y;
            extrusionDir.z = currentCorner.z - light.z;
            box.merge(currentCorner + extrudeDist * extrusionDir.normalisedCopy());

            // 6 : min min max
            // only z has changed
            currentCorner.z = oldMax.z;
            extrusionDir.z = currentCorner.z - light.z;
            box.merge(currentCorner + extrudeDist * extrusionDir.normalisedCopy());

            // 5 : min max max
            currentCorner.y = oldMax.y;
            extrusionDir.y = currentCorner.y - light.y;
            box.merge(currentCorner + extrudeDist * extrusionDir.normalisedCopy());

            // 1 : min max min
            currentCorner.z = oldMin.z;
            extrusionDir.z = currentCorner.z - light.z;
            box.merge(currentCorner + extrudeDist * extrusionDir.normalisedCopy());

            // 2 : max max min
            currentCorner.x = oldMax.x;
            extrusionDir.x = currentCorner.x - light.x;
            box.merge(currentCorner + extrudeDist * extrusionDir.normalisedCopy());

            // 4 : max max max
            currentCorner.z = oldMax.z;
            extrusionDir.z = currentCorner.z - light.z;
            box.merge(currentCorner + extrudeDist * extrusionDir.normalisedCopy());

            // 7 : max min max
            currentCorner.y = oldMin.y;
            extrusionDir.y = currentCorner.y - light.y;
            box.merge(currentCorner + extrudeDist * extrusionDir.normalisedCopy());

            // 3 : max min min
            currentCorner.z = oldMin.z;
            extrusionDir.z = currentCorner.z - light.z;
            box.merge(currentCorner + extrudeDist * extrusionDir.normalisedCopy());

        }

    }
}
