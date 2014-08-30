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
#ifndef __Lod_Strategy_H__
#define __Lod_Strategy_H__

#include "OgrePrerequisites.h"

#include "OgreMesh.h"
#include "OgreMaterial.h"
#include "Math/Array/OgreArrayConfig.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup LOD
    *  @{
    */
    /** Strategy for determining level of detail.
    @remarks
        Generally, to create a new LOD strategy, all of the following will
        need to be implemented: getValueImpl, getBaseValue, transformBias,
        getIndex, sort, and isSorted.
        In addition, transformUserValue may be overridden.
    */
    class _OgreExport LodStrategy : public LodAlloc
    {
    protected:
        /** Name of this strategy. */
        String mName;

        /** Compute the LOD value for a given movable object relative to a given camera. */
        virtual Real getValueImpl(const MovableObject *movableObject, const Camera *camera) const = 0;

    public:
        /** Constructor accepting name. */
        LodStrategy(const String& name);

        /** Virtual destructor. */
        virtual ~LodStrategy();

        /** Get the value of the first (highest) level of detail. */
        virtual Real getBaseValue() const = 0;

        /** Transform LOD bias so it only needs to be multiplied by the LOD value. */
        virtual Real transformBias(Real factor) const = 0;

        virtual void lodUpdateImpl( const size_t numNodes, ObjectData t,
                                    const Camera *camera, Real bias ) const = 0;

        //Include OgreLodStrategyPrivate.inl in the CPP files that use this function.
        inline static void lodSet( ObjectData &t, Real lodValues[ARRAY_PACKED_REALS] );

        /** Transform user supplied value to internal value.
        @remarks
            By default, performs no transformation.
        @remarks
            Do not throw exceptions for invalid values here, as the LOD strategy
            may be changed such that the values become valid.
        */
        virtual Real transformUserValue(Real userValue) const;

        /** Compute the LOD value for a given movable object relative to a given camera. */
        Real getValue(const MovableObject *movableObject, const Camera *camera) const;

        /** Get the index of the LOD usage which applies to a given value. */
        static ushort getIndex(Real value, const v1::Mesh::MeshLodUsageList& meshLodUsageList);

        /** Get the index of the LOD usage which applies to a given value. */
        static ushort getIndex(Real value, const Material::LodValueArray& materialLodValueArray);

        /** Sort mesh LOD usage list from greatest to least detail */
        static void sort(v1::Mesh::MeshLodUsageList& meshLodUsageList);

        /** Determine if the LOD values are sorted from greatest detail to least detail. */
        static bool isSorted(const v1::Mesh::LodValueArray& values);

        /** Assert that the LOD values are sorted from greatest detail to least detail. */
        static void assertSorted(const v1::Mesh::LodValueArray& values);

        /** Get the name of this strategy. */
        const String& getName() const { return mName; }
    };
    /** @} */
    /** @} */

} // namespace

#include "OgreHeaderSuffix.h"

#endif
