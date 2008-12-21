/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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
        Real viewportArea = viewport->getActualWidth() * viewport->getActualHeight();

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
