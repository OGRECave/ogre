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
#ifndef __Transform_H__
#define __Transform_H__

#include "OgreArrayMatrix4.h"
#include "OgreArrayMemoryManager.h"

namespace Ogre
{
    /** Represents the transform of a single object, arranged in SoA (Structure of Arrays) */
    struct Transform
    {
        /// Which of the packed values is ours. Value in range [0; 4) for SSE2
        unsigned char   mIndex;

        /// Holds the pointers to each parent. Ours is mParents[mIndex]
        Node            **mParents;

        /// The Node that owns this Transform. Ours is mOwner[mIndex]
        Node            **mOwner;

        /// Stores the position/translation of a node relative to its parent.
        ArrayVector3    * RESTRICT_ALIAS    mPosition;

        /// Stores the orientation of a node relative to it's parent.
        ArrayQuaternion * RESTRICT_ALIAS    mOrientation;

        /// Stores the scaling factor applied to a node
        ArrayVector3    * RESTRICT_ALIAS    mScale;

        /// Caches the combined position from all parent nodes.
        ArrayVector3    * RESTRICT_ALIAS    mDerivedPosition;

        /// Caches the combined orientation from all parent nodes.
        ArrayQuaternion * RESTRICT_ALIAS    mDerivedOrientation;

        /// Caches the combined scale from all parent nodes.
        ArrayVector3    * RESTRICT_ALIAS    mDerivedScale;

        /// Caches the full transform into a 4x4 matrix. Note it's not Array form! (It's AoS)
        Matrix4         * RESTRICT_ALIAS    mDerivedTransform;

        /// Stores whether this node inherits orientation from it's parent.
        /// Ours is mInheritOrientation[mIndex]
        bool    * RESTRICT_ALIAS mInheritOrientation;

        /// Stores whether this node inherits scale from it's parent.
        /// Ours is mInheritScale[mIndex]
        bool    * RESTRICT_ALIAS mInheritScale;

        Transform() :
            mIndex( 0 ),
            mParents( 0 ),
            mOwner( 0 ),
            mPosition( 0 ),
            mOrientation( 0 ),
            mScale( 0 ),
            mDerivedPosition( 0 ),
            mDerivedOrientation( 0 ),
            mDerivedScale( 0 ),
            mDerivedTransform( 0 ),
            mInheritOrientation( 0 ),
            mInheritScale( 0 )
        {
        }

        /** Copies all the scalar data from the parameter into this
        @remarks
            A normal "=" operator, or an assignment constructor Transform( Transform & )
            wouldn't work. This is because ArrayVector3 & co. would try to copy all the
            packed values, while we just want the scalar ones.

            Furthermore, it would be confusing, because it would be not clear whether
            those two options should memcpy memory, or rebase the pointers, hence
            explicit functions are much preferred. @See rebasePtrs

            Note that we do NOT copy the mIndex member.
        */
        void copy( const Transform &inCopy )
        {
            mParents[mIndex]    = inCopy.mParents[inCopy.mIndex];
            mOwner[mIndex]      = inCopy.mOwner[inCopy.mIndex];

            Vector3 tmp;
            Quaternion qTmp;

            //Position
            inCopy.mPosition->getAsVector3( tmp, inCopy.mIndex );
            mPosition->setFromVector3( tmp, mIndex );

            //Orientation
            inCopy.mOrientation->getAsQuaternion( qTmp, inCopy.mIndex );
            mOrientation->setFromQuaternion( qTmp, mIndex );

            //Scale
            inCopy.mScale->getAsVector3( tmp, inCopy.mIndex );
            mScale->setFromVector3( tmp, mIndex );

            //Derived position
            inCopy.mDerivedPosition->getAsVector3( tmp, inCopy.mIndex );
            mDerivedPosition->setFromVector3( tmp, mIndex );

            //Derived orientation
            inCopy.mDerivedOrientation->getAsQuaternion( qTmp, inCopy.mIndex );
            mDerivedOrientation->setFromQuaternion( qTmp, mIndex );

            //Derived scale
            inCopy.mDerivedScale->getAsVector3( tmp, inCopy.mIndex );
            mDerivedScale->setFromVector3( tmp, mIndex );

            mDerivedTransform[mIndex] = inCopy.mDerivedTransform[inCopy.mIndex];

            mInheritOrientation[mIndex] = inCopy.mInheritOrientation[inCopy.mIndex];
            mInheritScale[mIndex]       = inCopy.mInheritScale[inCopy.mIndex];
        }

        /** Rebases all the pointers from our SoA structs so that they point to a new location
            calculated from a base pointer, and a difference offset. The index (i.e.
            mPosition._getIndex()) is assumed to stay the same
        @remarks
            @See RebaseListener This function is intended to be used when the pool memory is growing
            hence the memory base address may change, so we need to reallocate and update all pointers
            accordingly
        */
        void rebasePtrs( const MemoryPoolVec &newBasePtrs, const ptrdiff_t diff )
        {
            mParents    = reinterpret_cast<Node**>( newBasePtrs[NodeArrayMemoryManager::Parent] + diff );
            mOwner      = reinterpret_cast<Node**>( newBasePtrs[NodeArrayMemoryManager::Owner] + diff );

            mPosition           = reinterpret_cast<ArrayVector3*>(
                                    newBasePtrs[NodeArrayMemoryManager::Position] + diff );
            mOrientation        = reinterpret_cast<ArrayQuaternion*>(
                                    newBasePtrs[NodeArrayMemoryManager::Orientation] + diff );
            mScale              = reinterpret_cast<ArrayVector3*>(
                                    newBasePtrs[NodeArrayMemoryManager::Scale] + diff );

            mDerivedPosition    = reinterpret_cast<ArrayVector3*>(
                                    newBasePtrs[NodeArrayMemoryManager::DerivedPosition] + diff );
            mDerivedOrientation = reinterpret_cast<ArrayQuaternion*>(
                                    newBasePtrs[NodeArrayMemoryManager::DerivedOrientation] + diff );
            mDerivedScale       = reinterpret_cast<ArrayVector3*>(
                                    newBasePtrs[NodeArrayMemoryManager::DerivedScale] + diff );

            mDerivedTransform   = reinterpret_cast<Matrix4*>(
                                    newBasePtrs[NodeArrayMemoryManager::WorldMat] + diff );

            mInheritOrientation = reinterpret_cast<bool*>(
                                    newBasePtrs[NodeArrayMemoryManager::InheritOrientation] + diff );
            mInheritScale       = reinterpret_cast<bool*>(
                                    newBasePtrs[NodeArrayMemoryManager::InheritScale] + diff );
        }

        /** Advances all pointers to the next pack, i.e. if we're processing 4 elements at a time, move to
            the next 4 elements.
        */
        void advancePack()
        {
            mParents            += ARRAY_PACKED_REALS;
            mOwner              += ARRAY_PACKED_REALS;
            ++mPosition;
            ++mOrientation;
            ++mScale;
            ++mDerivedPosition;
            ++mDerivedOrientation;
            ++mDerivedScale;
            mDerivedTransform   += ARRAY_PACKED_REALS;
            mInheritOrientation += ARRAY_PACKED_REALS;
            mInheritScale       += ARRAY_PACKED_REALS;
        }

        void advancePack( size_t numAdvance )
        {
            mParents            += ARRAY_PACKED_REALS * numAdvance;
            mOwner              += ARRAY_PACKED_REALS * numAdvance;
            mPosition           += numAdvance;
            mOrientation        += numAdvance;
            mScale              += numAdvance;
            mDerivedPosition    += numAdvance;
            mDerivedOrientation += numAdvance;
            mDerivedScale       += numAdvance;
            mDerivedTransform   += ARRAY_PACKED_REALS * numAdvance;
            mInheritOrientation += ARRAY_PACKED_REALS * numAdvance;
            mInheritScale       += ARRAY_PACKED_REALS * numAdvance;
        }
    };
}

#endif
