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
#include "OgreLodStrategy.h"

#include "OgreMesh.h"
#include "OgreMaterial.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    LodStrategy::LodStrategy(const String& name)
        : mName(name)
    { }
    //-----------------------------------------------------------------------
    LodStrategy::~LodStrategy()
    { }
    //-----------------------------------------------------------------------
    Real LodStrategy::transformUserValue(Real userValue) const
    {
        // No transformation by default
        return userValue;
    }
    //-----------------------------------------------------------------------
    Real LodStrategy::getValue(const MovableObject *movableObject, const Camera *camera) const
    {
        // Just return implementation with lod camera
        return getValueImpl(movableObject, camera->getLodCamera());
    }
    //-----------------------------------------------------------------------
    void LodStrategy::assertSorted(const Mesh::LodValueList &values) const
    {
        assert(isSorted(values) && "The lod values must be sorted");
    }
    //---------------------------------------------------------------------
    bool LodStrategy::isSortedAscending(const Mesh::LodValueList& values)
    {
        Mesh::LodValueList::const_iterator it = values.begin();
        Real prev = (*it);
        for (++it; it != values.end(); ++it)
        {
            Real cur = (*it);
            if (cur < prev)
                return false;
            prev = cur;
        }

        return true;
    }
    //---------------------------------------------------------------------
    bool LodStrategy::isSortedDescending(const Mesh::LodValueList& values)
    {
        Mesh::LodValueList::const_iterator it = values.begin();
        Real prev = (*it);
        for (++it; it != values.end(); ++it)
        {
            Real cur = (*it);
            if (cur > prev)
                return false;
            prev = cur;
        }

        return true;
    }
    //---------------------------------------------------------------------
    struct LodUsageSortLess :
    public std::binary_function<const MeshLodUsage&, const MeshLodUsage&, bool>
    {
        bool operator() (const MeshLodUsage& mesh1, const MeshLodUsage& mesh2)
        {
            // sort ascending
            return mesh1.value < mesh2.value;
        }
    };
    void LodStrategy::sortAscending(Mesh::MeshLodUsageList& meshLodUsageList)
    {
        // Perform standard sort
        std::sort(meshLodUsageList.begin(), meshLodUsageList.end(), LodUsageSortLess());
    }
    //---------------------------------------------------------------------
    struct LodUsageSortGreater :
    public std::binary_function<const MeshLodUsage&, const MeshLodUsage&, bool>
    {
        bool operator() (const MeshLodUsage& mesh1, const MeshLodUsage& mesh2)
        {
            // sort decending
            return mesh1.value > mesh2.value;
        }
    };
    void LodStrategy::sortDescending(Mesh::MeshLodUsageList& meshLodUsageList)
    {
        // Perform standard sort
        std::sort(meshLodUsageList.begin(), meshLodUsageList.end(), LodUsageSortGreater());
    }
    //---------------------------------------------------------------------
    ushort LodStrategy::getIndexAscending(Real value, const Mesh::MeshLodUsageList& meshLodUsageList)
    {
        Mesh::MeshLodUsageList::const_iterator i, iend;
        iend = meshLodUsageList.end();
        ushort index = 0;
        for (i = meshLodUsageList.begin(); i != iend; ++i, ++index)
        {
            if (i->value > value)
            {
                return index - 1;
            }
        }

        // If we fall all the way through, use the highest value
        return static_cast<ushort>(meshLodUsageList.size() - 1);
    }
    //---------------------------------------------------------------------
    ushort LodStrategy::getIndexDescending(Real value, const Mesh::MeshLodUsageList& meshLodUsageList)
    {
        Mesh::MeshLodUsageList::const_iterator i, iend;
        iend = meshLodUsageList.end();
        ushort index = 0;
        for (i = meshLodUsageList.begin(); i != iend; ++i, ++index)
        {
            if (i->value < value)
            {
                return index - 1;
            }
        }

        // If we fall all the way through, use the highest value
        return static_cast<ushort>(meshLodUsageList.size() - 1);
    }
    //---------------------------------------------------------------------
    ushort LodStrategy::getIndexAscending(Real value, const Material::LodValueList& materialLodValueList)
    {
        Material::LodValueList::const_iterator i, iend;
        iend = materialLodValueList.end();
        unsigned short index = 0;
        for (i = materialLodValueList.begin(); i != iend; ++i, ++index)
        {
            if (*i > value)
            {
                return index - 1;
            }
        }

        // If we fall all the way through, use the highest value
        return static_cast<ushort>(materialLodValueList.size() - 1);
    }
    //---------------------------------------------------------------------
    ushort LodStrategy::getIndexDescending(Real value, const Material::LodValueList& materialLodValueList)
    {
        Material::LodValueList::const_iterator i, iend;
        iend = materialLodValueList.end();
        unsigned short index = 0;
        for (i = materialLodValueList.begin(); i != iend; ++i, ++index)
        {
            if (*i < value)
            {
                return index - 1;
            }
        }

        // If we fall all the way through, use the highest value
        return static_cast<ushort>(materialLodValueList.size() - 1);
    }

} // namespace
