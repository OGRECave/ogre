/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgrePixelCountLodStrategy.h"

#include "OgreViewport.h"

#include <limits>

namespace Ogre {
    //-----------------------------------------------------------------------
    template<> PixelCountLodStrategy* Singleton<PixelCountLodStrategy>::ms_Singleton = 0;
    PixelCountLodStrategy* PixelCountLodStrategy::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    PixelCountLodStrategy& PixelCountLodStrategy::getSingleton(void)
    {
        assert( ms_Singleton );  return ( *ms_Singleton );
    }
    //-----------------------------------------------------------------------
    PixelCountLodStrategy::PixelCountLodStrategy()
        : LodStrategy("PixelCount")
    { }
    //-----------------------------------------------------------------------
    Real PixelCountLodStrategy::getValueImpl(const MovableObject *movableObject, const Ogre::Camera *camera) const
    {
        // Get viewport
        const Viewport *viewport = camera->getViewport();

        // Get viewport area
        Real viewportArea = static_cast<Real>(viewport->getActualWidth() * viewport->getActualHeight());

        // Get area of unprojected circle with object bounding radius
        Real boundingArea = Math::PI * Math::Sqr(movableObject->getBoundingRadius());

        // Base computation on projection type
        switch (camera->getProjectionType())
        {
        case PT_PERSPECTIVE:
            {
                // Get camera distance
                Real distanceSquared = movableObject->getParentNode()->getSquaredViewDepth(camera);

                // Check for 0 distance
                if (distanceSquared <= std::numeric_limits<Real>::epsilon())
                    return getBaseValue();

                // Get projection matrix (this is done to avoid computation of tan(fov / 2))
                const Matrix4& projectionMatrix = camera->getProjectionMatrix();

                // Estimate pixel count
                return (boundingArea * viewportArea * projectionMatrix[0][0] * projectionMatrix[1][1]) / distanceSquared;
            }
        case PT_ORTHOGRAPHIC:
            {
                // Compute orthographic area
                Real orthoArea = camera->getOrthoWindowHeight() * camera->getOrthoWindowWidth();

                // Check for 0 orthographic area
                if (orthoArea <= std::numeric_limits<Real>::epsilon())
                    return getBaseValue();

                // Estimate pixel count
                return (boundingArea * viewportArea) / orthoArea;
            }
        default:
            {
                // This case is not covered for obvious reasons
                throw;
            }
        }
    }
    //---------------------------------------------------------------------
    Real PixelCountLodStrategy::getBaseValue() const
    {
        // Use the maximum possible value as base
        return std::numeric_limits<Real>::max();
    }
    //---------------------------------------------------------------------
    Real PixelCountLodStrategy::transformBias(Real factor) const
    {
        // No transformation required for pixel count strategy
        return factor;
    }
    //---------------------------------------------------------------------
    ushort PixelCountLodStrategy::getIndex(Real value, const Mesh::MeshLodUsageList& meshLodUsageList) const
    {
        // Values are descending
        return getIndexDescending(value, meshLodUsageList);
    }
    //---------------------------------------------------------------------
    ushort PixelCountLodStrategy::getIndex(Real value, const Material::LodValueList& materialLodValueList) const
    {
        // Values are descending
        return getIndexDescending(value, materialLodValueList);
    }
    //---------------------------------------------------------------------
    void PixelCountLodStrategy::sort(Mesh::MeshLodUsageList& meshLodUsageList) const
    {
        // Sort descending
        sortDescending(meshLodUsageList);
    }
    //---------------------------------------------------------------------
    bool PixelCountLodStrategy::isSorted(const Mesh::LodValueList& values) const
    {
        // Check if values are sorted descending
        return isSortedDescending(values);
    }

} // namespace
