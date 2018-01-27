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
#ifndef _BoneArrayMemoryManager_H_
#define _BoneArrayMemoryManager_H_

#include "OgreArrayMemoryManager.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Memory
    *  @{
    */

    /** Implementation to create the Transform variables needed by Bones
    @author
        Matias N. Goldberg
    @version
        1.0
    */
    class _OgreExport BoneArrayMemoryManager : public ArrayMemoryManager
    {
    protected:
        /// We overload to set all mParentTransform to point to a dummy matrix
        virtual void initializeEmptySlots( size_t prevNumSlots );

    public:
        enum MemoryTypes
        {
            Owner = 0,
            Position,
            Orientation,
            Scale,
            ParentMat,
            ParentNode,
            WorldMat,
            FinalMat,
            InheritOrientation,
            InheritScale,
            NumMemoryTypes
        };

        static const size_t ElementsMemSize[NumMemoryTypes];
        static const CleanupRoutines BoneInitRoutines[NumMemoryTypes];
        static const CleanupRoutines BoneCleanupRoutines[NumMemoryTypes];

        /// @copydoc ArrayMemoryManager::ArrayMemoryManager
        BoneArrayMemoryManager(uint16 depthLevel, size_t hintMaxNodes,
                                size_t cleanupThreshold=100,
                                size_t maxHardLimit=MAX_MEMORY_SLOTS,
                                RebaseListener *rebaseListener=0 );

        /** Requests memory for a new Bone (for the Array vectors & matrices)
            May be also be used for a new Entity, etc.
            @param outTransform
                Out: The transform with filled memory pointers
        */
        void createNewNode( BoneTransform &outTransform );

        /** Releases memory acquired through @see createNewNode
            @remarks
                For optimal results, try to respect LIFO order in the removals
            @param inOutTransform
                Out: Transform to destroy. Pointers are nullified
        */
        void destroyNode( BoneTransform &inOutTransform );

        /** Retrieves a BoneTransform pointing to the first Bone
        @remarks
            @See BoneMemoryManager::getStart
        @param outTransform
            [out] Transform with filled pointers to the first Node in this depth
        @return
            Number of Nodes in this depth level
        */
        size_t getFirstNode( BoneTransform &outTransform );
    };

    /** @} */
    /** @} */
}

#endif
