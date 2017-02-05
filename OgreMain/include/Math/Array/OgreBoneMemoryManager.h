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
#ifndef _OgreBoneMemoryManager_H_
#define _OgreBoneMemoryManager_H_

#include "OgreBoneTransform.h"
#include "OgreBoneArrayMemoryManager.h"

#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Memory
    *  @{
    */

    struct BySkeletonDef;

    /** Wrap-around class that contains multiple ArrayMemoryManager, one per hierarchy depth
    @remarks
        This is the main memory manager that actually manages Bone, and have to be called
        when a new Bone was created, when a Bone gets detached from it's parent,
        when it's attached again, etc.
        @par
        Note that some SceneManager implementations (i.e. Octree like) may want to have more
        than one BoneMemoryManager, for example one per octant.
    */
    class _OgreExport BoneMemoryManager : ArrayMemoryManager::RebaseListener
    {
        typedef vector<BoneArrayMemoryManager>::type ArrayMemoryManagerVec;
        /// ArrayMemoryManagers grouped by hierarchy depth
        ArrayMemoryManagerVec       mMemoryManagers;

        BySkeletonDef               *mBoneRebaseListener;

        /** Makes mMemoryManagers big enough to be able to fulfill mMemoryManagers[newDepth]
        @param newDepth
            Hierarchy level depth we wish to grow to.
        */
        void growToDepth( size_t newDepth );

    public:
        BoneMemoryManager();
        virtual ~BoneMemoryManager();

        /** Since skeleton's hierarchy structure is known beforehand, we use this information for setting
            a better optimal cleanup value, to avoid excessive stalls when destroying skeleton instances.
        */
        void _growToDepth( const vector<size_t>::type &bonesPerDepth );

        /** Requests memory for the given transform for the first, initializing values.
        @param outTransform
            Transform with filled pointers
        @param depth
            Hierarchy level depth. 0 if not connected.
        */
        void nodeCreated( BoneTransform &outTransform, size_t depth );

        /** Requests memory for the given transform to be attached, transferring
            existing values inside to the new memory block
        @remarks
            Do NOT call this function twice in a row without having called
            nodeDettached in the middle
        @param outTransform
            Transform with filled pointers
        @param depth
            Hierarchy level depth the node belongs to. If 0, nothing happens.
        */
        void nodeAttached( BoneTransform &outTransform, size_t depth );

        /** Releases current memory and requests memory from the root level
        @remarks
            Do NOT call this function twice in a row without having called
            nodeAttached in the middle
        @par
            outTransform.mParents[outTransform.mIndex] is reset to a dummy parent node
        @param outTransform
            Transform with filled pointers
        @param depth
            Current hierarchy level depth it belongs to. If 0, nothing happens.
        */
        void nodeDettached( BoneTransform &outTransform, size_t depth );

        /** Releases current memory
        @param outTransform
            Transform with nullified pointers
        @param depth
            Current hierarchy level depth it belongs to.
        */
        void nodeDestroyed( BoneTransform &outTransform, size_t depth );

        /** Requests memory for the given Node to be moved to a different depth level,
            transferring existing values inside to the new memory slot
        @param inOutTransform
            Transform with filled pointers
        @param oldDepth
            Current hierarchy level depth it belongs to.
        @param newDepth
            Hierarchy level depth it wants to belongs to.
        */
        void nodeMoved( BoneTransform &inOutTransform, size_t oldDepth, size_t newDepth );

        /** Releases memory belonging to us, not before copying it into another manager.
        @remarks
            This function is useful when implementing multiple Memory Managers in Scene Managers
            or when switching nodes from Static to/from Dynamic.
        @param inOutTransform
            Valid Transform that belongs to us. Output will belong to the other memory mgr.
        @param depth
            Current hierarchy level depth it belongs to.
        @param dstBoneMemoryManager
            BoneMemoryManager that will now own the transform.
        */
        void migrateTo( BoneTransform &inOutTransform, size_t depth,
                        BoneMemoryManager *dstBoneMemoryManager );

        /** Retrieves the number of depth levels that have been created.
        @remarks
            The return value is equal or below mMemoryManagers.size(), you should cache
            the result instead of calling this function too often.
        */
        size_t getNumDepths() const;

        /** Retrieves a Transform pointing to the first Node in the given depth
        @param outTransform
            [out] Transform with filled pointers to the first Node in this depth
        @param depth
            Current hierarchy level depth it belongs to.
        @return
            Number of Nodes in this depth level
        */
        size_t getFirstNode( BoneTransform &outTransform, size_t depth );

        void setBoneRebaseListener( BySkeletonDef *l )          { mBoneRebaseListener = l; }

        //Derived from ArrayMemoryManager::RebaseListener
        virtual void buildDiffList( uint16 level, const MemoryPoolVec &basePtrs,
                                    ArrayMemoryManager::PtrdiffVec &outDiffsList );
        virtual void applyRebase( uint16 level, const MemoryPoolVec &newBasePtrs,
                                  const ArrayMemoryManager::PtrdiffVec &diffsList );
        virtual void performCleanup( uint16 level, const MemoryPoolVec &basePtrs,
                                     size_t const *elementsMemSizes,
                                     size_t startInstance, size_t diffInstances );
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
