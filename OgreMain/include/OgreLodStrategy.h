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
#ifndef __Lod_Strategy_H__
#define __Lod_Strategy_H__

#include "OgrePrerequisites.h"

#include "OgreMesh.h"

#include "OgreMovableObject.h"
#include "OgreCamera.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup LOD
	*  @{
	*/
	/** Strategy for determining level of detail.
    @remarks
        Generally, to create a new lod strategy, all of the following will
        need to be implemented: getValueImpl, getBaseValue, transformBias,
        getIndex, sort, and isSorted.
        In addition, transformUserValue may be overridden.
    */
	class _OgreExport LodStrategy : public LodAlloc
    {
    protected:
        /** Name of this strategy. */
        String mName;

        /** Compute the lod value for a given movable object relative to a given camera. */
        virtual Real getValueImpl(const MovableObject *movableObject, const Camera *camera) const = 0;

    public:
        /** Constructor accepting name. */
        LodStrategy(const String& name);

        /** Virtual destructor. */
        virtual ~LodStrategy();

        /** Get the value of the first (highest) level of detail. */
        virtual Real getBaseValue() const = 0;

        /** Transform lod bias so it only needs to be multiplied by the lod value. */
        virtual Real transformBias(Real factor) const = 0;

        /** Transforum user supplied value to internal value.
        @remarks
            By default, performs no transformation.
        @remarks
            Do not throw exceptions for invalid values here, as the lod strategy
            may be changed such that the values become valid.
        */
        virtual Real transformUserValue(Real userValue) const;

        /** Compute the lod value for a given movable object relative to a given camera. */
        Real getValue(const MovableObject *movableObject, const Camera *camera) const;

        /** Get the index of the lod usage which applies to a given value. */
        virtual ushort getIndex(Real value, const Mesh::MeshLodUsageList& meshLodUsageList) const = 0;

        /** Get the index of the lod usage which applies to a given value. */
        virtual ushort getIndex(Real value, const Material::LodValueList& materialLodValueList) const = 0;

        /** Sort mesh lod usage list from greatest to least detail */
        virtual void sort(Mesh::MeshLodUsageList& meshLodUsageList) const = 0;

        /** Determine if the lod values are sorted from greatest detail to least detail. */
        virtual bool isSorted(const Mesh::LodValueList& values) const = 0;

        /** Assert that the lod values are sorted from greatest detail to least detail. */
        void assertSorted(const Mesh::LodValueList& values) const;

        /** Get the name of this strategy. */
        const String& getName() const { return mName; };

    protected:
        /** Implementation of isSorted suitable for ascending values. */
        static bool isSortedAscending(const Mesh::LodValueList& values);
        /** Implementation of isSorted suitable for descending values. */
        static bool isSortedDescending(const Mesh::LodValueList& values);

        /** Implementation of sort suitable for ascending values. */
        static void sortAscending(Mesh::MeshLodUsageList& meshLodUsageList);
        /** Implementation of sort suitable for descending values. */
        static void sortDescending(Mesh::MeshLodUsageList& meshLodUsageList);

        /** Implementation of getIndex suitable for ascending values. */
        static ushort getIndexAscending(Real value, const Mesh::MeshLodUsageList& meshLodUsageList);
        /** Implementation of getIndex suitable for descending values. */
        static ushort getIndexDescending(Real value, const Mesh::MeshLodUsageList& meshLodUsageList);

        /** Implementation of getIndex suitable for ascending values. */
        static ushort getIndexAscending(Real value, const Material::LodValueList& materialLodValueList);
        /** Implementation of getIndex suitable for descending values. */
        static ushort getIndexDescending(Real value, const Material::LodValueList& materialLodValueList);

    };
	/** @} */
	/** @} */

} // namespace

#endif
