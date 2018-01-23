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
#include "OgreLodStrategy.h"

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
        // Just return implementation with LOD camera
        return getValueImpl(movableObject, camera->getLodCamera());
    }
    //-----------------------------------------------------------------------
    void LodStrategy::assertSorted(const Mesh::LodValueList &values) const
    {
        assert(isSorted(values) && "The LOD values must be sorted");
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
            // Sort ascending
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
            // Sort descending
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
                return index ? index - 1 : 0;
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
                return index ? index - 1 : 0;
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
                return index ? index - 1 : 0;
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
                return index ? index - 1 : 0;
            }
        }

        // If we fall all the way through, use the highest value
        return static_cast<ushort>(materialLodValueList.size() - 1);
    }

} // namespace
