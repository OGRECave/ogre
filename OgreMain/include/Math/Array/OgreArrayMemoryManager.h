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
#ifndef __ArrayMemoryManager_H__
#define __ArrayMemoryManager_H__

#include "OgrePrerequisites.h"

#include <stddef.h>

namespace Ogre
{
    typedef vector<char*>::type MemoryPoolVec;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Memory
    *  @{
    */

    typedef void (*CleanupRoutines)( char *dstPtr, size_t indexDst, char *srcPtr, size_t indexSrc,
                                     size_t numSlots, size_t numFreeSlots, size_t elementsMemSize );

    /** Abstract memory manager for managing large chunks of contiguous memory, optimized for SoA
        (Structure of Arrays) implementations.
    @remarks
        This class works by requesting slots from preallocated chunks, and then releasing those slots.
        This implementation is most efficient when using LIFO patterns. Since Entities & SceneNodes
        use this manager, this means that you should create the most static ones first, and the most
        dynamic ones last (i.e. the ones that are going to be frequently inserted & removed from scene)
        @par
        WARNING: This class requires its owner to manually call initialize() and destroy() after all
        instances have been destroyed. Otherwise memory leaks will happen!
    @author
        Matias N. Goldberg
    @version
        1.0
    */
    class _OgreExport ArrayMemoryManager
    {
    public:
        //typedef vector<ptrdiff_t>::type PtrdiffVec; //TODO: Modify for Ogre
        typedef std::vector<ptrdiff_t> PtrdiffVec;

        /** When mUsedMemory >= mMaxMemory (that is, we've exhausted all our preallocated memory)
            ArrayMemoryManager will proceed to reallocate all memory. The resulting base pointer
            address may have changed and hence each ArrayVector3, ArrayMatrix4, etc needs to be
            rebased (alter it's mChunkBase pointer).

            It consists in two steps: First build a list of relative differences before deallocation
            and then apply the new base offseting based on that list. The list is needed because
            as per C++ standard, once the memory freed, using the pointers is undefined behavior
            (even freedPtr > someValid pointer is now UB). Most systems using a flat memory model
            in which case the list wouldn't be needed (since 99% of the times, the ptr is just an
            integer). However we can't guarantee in which architectures this code will run on.
        */
        class RebaseListener
        {
        public:
            /** Called when the manager needs to grow it's memory pool to honour more node requests.
                See the class description on why we need to do this (to avoid C++ undefined behavior)
                @remarks
                    Needs to builds a list that will contain the difference in bytes between each
                    ArrayVector3/ArrayMatrix4/etc and the base pointers _in_the_order_ in which the
                    derived class holds those pointers (i.e. in the order the SceneNodes are arranged
                    in memory)
                @param level
                    The hierarchy depth level
                @param basePtrs
                    The base pointers from each pool so we can calculate the differences
                @param utDiffsList
                    The list we'll generate. "outDiffsList" already has enough reserved space
            */
            virtual void buildDiffList( uint16 level, const MemoryPoolVec &basePtrs,
                                        PtrdiffVec &outDiffsList ) = 0;

            /** Called when the manager already grew it's memory pool to honour more node requests.
                @remarks
                    Will use the new base ptr and the list we built in @see buildDiffList() to know
                    what mChunkPtr & mIndex needs to be set for each ArrayVector3/etc we have.
                @param level
                    The hierarchy depth level
                @param newBasePtrs
                    The new base ptr.
                @param diffsList
                    The list built in buildDiffList
            */
            virtual void applyRebase( uint16 level, const MemoryPoolVec &newBasePtrs,
                                      const PtrdiffVec &diffsList ) = 0;

            /** Called when too many nodes were destroyed in a non-LIFO fashion. Without cleaning up,
                the scene manager will waste CPU & bandwidth on processing vectors & matrices that
                are not in use. The more fragmented/unordered those removals were, the worst it is.
                Try to create everything static first, then dynamic content.
                @remarks
                    The manager behaves similarly to a Garbage Collector, as it is triggered after
                    certain amount of nodes have been freed (unless they respected LIFO order)

                    In a way, it's very similar to vector::remove(), as removing an element from
                    the middle means we need to shift everything past that point one place (or more).
                @param level
                    The hierarchy depth level
                @param basePtrs
                    The base ptrs.
                @param startInstance
                    The instance to which past that we need to shift
                @param diffInstances
                    How many places we need to shift backwards.
            */
            virtual void performCleanup( uint16 level, const MemoryPoolVec &basePtrs,
                                         size_t const *elementsMemSizes, size_t startInstance,
                                         size_t diffInstances ) = 0;
        };

    protected:
        ///One per memory type
        MemoryPoolVec               mMemoryPools;
        size_t const                *mElementsMemSizes;
        CleanupRoutines const       *mInitRoutines;
        CleanupRoutines const       *mCleanupRoutines;
        size_t                      mTotalMemoryMultiplier;

        //The following three are measured in instances, not bytes
        size_t              mUsedMemory;
        size_t              mMaxMemory;
        size_t              mMaxHardLimit;
        size_t              mCleanupThreshold;
        typedef std::vector<size_t> SlotsVec; //TODO: Modify for Ogre
        SlotsVec            mAvailableSlots;
        RebaseListener      *mRebaseListener;

        /// The hierarchy depth level. This value is not used by the manager,
        /// just passed to the listeners so they can know to which level it
        /// belongs
        uint16              mLevel;

    public:
        static const size_t MAX_MEMORY_SLOTS;

        /** Constructor. @See intialize. @See destroy.
            @param elementsMemSize
                Array containing the size in bytes of each element type (i.e. NodeElementsMemSize)
            @param initRoutines
                Array containing the cleanup function that will be called when default initializing
                memory. Unlike cleanupRoutines, just leave the function pointer null if all you
                want is just to initialize to 0.
            @param cleanupRoutines
                Array containing the cleanup function that will be called when performing cleanups.
                Many pointers can use the flatCleaner and is the fastest. However Array variables
                (i.e. ArrayVector3) have a layout where flatCleaner won't work correctly because
                the data is interleaved (rather than flat).
            @param numElementsSize
                Number of entries in elementsMemSize
            @param depthLevel
                Value only used to pass to the listener. Identifies to which hierarchy depth
                level this memory manager belongs to.
            @param hintMaxNodes
                Hint on how many SceneNodes we'll be creating.
            @param cleanupThreshold
                The threshold at which a cleanup is triggered after too many nodes have been
                destroyed in a non-LIFO order or without being created again. -1 to disable cleanups.
            @param maxHardLimit
                Maximum amount of SceneNodes. The manager is not allowed to grow and consume more
                memory past that limit. MAX_MEMORY_SLOTS for no limit. This is useful when target
                architecture has much less memory than the dev machine.
                @par
                Note that if hintMaxNodes < maxHardLimit, the manager may be forced to do temporary
                allocations (to do the reallocs) thus during a brief perdiod of time it may consume
                more memory than the established hard limit (up to 2x).
            @param rebaseListener
                The listener to be called when cleaning up or growing the memory pool. If null,
                cleanupThreshold is set to -1 & maxHardLimit will be set to hintMaxNodes
        */
        ArrayMemoryManager( size_t const *elementsMemSize,
                            CleanupRoutines const *initRoutines,
                            CleanupRoutines const *cleanupRoutines,
                            size_t numElementsSize, uint16 depthLevel, size_t hintMaxNodes,
                            size_t cleanupThreshold=100, size_t maxHardLimit=MAX_MEMORY_SLOTS,
                            RebaseListener *rebaseListener=0 );

        /** Initializes mMemoryPools. Once it has been called, destroy() __must__ be called.
            @See destroy
        @remarks
            The destructor won't free the data, if you don't call destroy, memory will leak.
            Calling initialize twice is possible and won't leak, but will free the previous
            memory ptrs without calling the registered RebaseListener. So if there were slots
            in use, their pointers will become dangling. An assert will trigger if this happens.
        */
        void initialize();

        /** Destroys the memory ptrs. @See initialize
        @remarks
            If there were slots in use, make sure they're no longer used, as their ptrs will
            become dangling ptrs. We don't assert because this may be valid behavior (i.e.
            on shutdown)
        */
        void destroy();

        /// Returns mUsedMemory. When ARRAY_PACKED_REALS = 4, and 4 objects have been
        /// created but the 2nd one has been deleted, getNumUsedSlotsIncludingFragmented
        /// will still return 4 until the 4th object is removed or a cleanup is performed
        size_t getNumUsedSlotsIncludingFragmented() const;

        /// Gets available memory in bytes
        size_t getFreeMemory() const;
        /// Gets used memory in bytes (not including waste)
        size_t getUsedMemory() const;
        /// Gets wasted memory (perform a cleanup to fix)
        size_t getWastedMemory() const;
        /// Gets all memory reserved for this manager
        size_t getAllMemory() const;

    protected:
        /** Requests memory for a new slot (could be used for SceneNode, Entities, etc.)
            @remarks
                Try to create everything static first, then dynamic content.
                Values are initialized (position to zero, orientation to identity,
                scale to unit, etc)
            @return
                The requested slot, as an offset to the base memory pointers.
        */
        size_t createNewSlot();

        /** Releases memory acquired through @see createNewSlot
            @remarks
                For optimal results, try to respect LIFO order in the removals
            @param ptrToFirstElement
                Pointer to the first element, what's allocated with mMemoryPools[0]
            @param index
                The index, typically mIndex (range [0; ARRAY_PACKED_REALS) )
        */
        void destroySlot( const char *ptrToFirstElement, uint8 index );

        /** Called when mMemoryPools changes, to give a chance derived class to initialize
            new memory to default values
        @remarks
            Must access slots in range [prevNumSlots; mMaxSlots); as the references
            in [0; prevNumSlots) aren't yet updated.
        @param prevNumSlots
            The previous value of mMaxMemory before changing mMemoryPools
        */
        virtual void initializeEmptySlots( size_t prevNumSlots ) {}
    };




    /** Implementation to create the Transform variables needed by Nodes & SceneNodes
    @author
        Matias N. Goldberg
    @version
        1.0
    */
    class _OgreExport NodeArrayMemoryManager : public ArrayMemoryManager
    {
        /// Dummy node where to point Transform::mParents[i] when they're unused slots.
        Node    *mDummyNode;

    protected:
        /// We overload to set all mParents to point to mDummyNode
        virtual void initializeEmptySlots( size_t prevNumSlots );

    public:
        enum MemoryTypes
        {
            Parent = 0,
            Owner,
            Position,
            Orientation,
            Scale,
            DerivedPosition,
            DerivedOrientation,
            DerivedScale,
            WorldMat,
            InheritOrientation,
            InheritScale,
            NumMemoryTypes
        };

        static const size_t ElementsMemSize[NumMemoryTypes];
        static const CleanupRoutines NodeInitRoutines[NumMemoryTypes];
        static const CleanupRoutines NodeCleanupRoutines[NumMemoryTypes];

        /// @copydoc ArrayMemoryManager::ArrayMemoryManager
        NodeArrayMemoryManager( uint16 depthLevel, size_t hintMaxNodes, Node *dummyNode,
                                size_t cleanupThreshold=100, size_t maxHardLimit=MAX_MEMORY_SLOTS,
                                RebaseListener *rebaseListener=0 );

        /** Requests memory for a new SceneNode (for the Array vectors & matrices)
            May be also be used for a new Entity, etc.
            @remarks
                Try to create everything static first, then dynamic content.
                Values are initialized (position to zero, orientation to identity,
                scale to unit, etc)
            @param outTransform
                Out: The transform with filled memory pointers
        */
        void createNewNode( Transform &outTransform );

        /** Releases memory acquired through @see createNewNode
            @remarks
                For optimal results, try to respect LIFO order in the removals
            @param inOutTransform
                Out: Transform to destroy. Pointers are nullified
        */
        void destroyNode( Transform &inOutTransform );

        /** Retrieves a Transform pointing to the first Node
        @remarks
            @See NodeMemoryManager::getStart
        @param outTransform
            [out] Transform with filled pointers to the first Node in this depth
        @return
            Number of Nodes in this depth level
        */
        size_t getFirstNode( Transform &outTransform );
    };



    /** Implementation to create the ObjectData variables needed by MovableObjects
    @author
        Matias N. Goldberg
    @version
        1.0
    */
    class _OgreExport ObjectDataArrayMemoryManager : public ArrayMemoryManager
    {
        /// Dummy node where to point ObjectData::mParents[i] when they're unused slots.
        Node            *mDummyNode;
        MovableObject   *mDummyObject;

    protected:
        /// We overload to set all mParents to point to mDummyNode
        virtual void initializeEmptySlots( size_t prevNumSlots );

    public:
        enum MemoryTypes
        {
            Parent = 0,
            Owner,
            LocalAabb,
            WorldAabb,
            LocalRadius,
            WorldRadius,
            DistanceToCamera,
            UpperDistance,
            ShadowUpperDistance,
            VisibilityFlags,
            QueryFlags,
            LightMask,
            NumMemoryTypes
        };

        static const size_t ElementsMemSize[NumMemoryTypes];
        static const CleanupRoutines ObjCleanupRoutines[NumMemoryTypes];

        /// @copydoc ArrayMemoryManager::ArrayMemoryManager
        ObjectDataArrayMemoryManager( uint16 depthLevel, size_t hintMaxNodes, Node *dummyNode,
                                        MovableObject *dummyObject, size_t cleanupThreshold=100,
                                        size_t maxHardLimit=MAX_MEMORY_SLOTS,
                                        RebaseListener *rebaseListener=0 );

        /// @copydoc NodeArrayMemoryManager::createNewNode
        void createNewNode( ObjectData &outData );

        /// @copydoc NodeArrayMemoryManager::destroyNode
        void destroyNode( ObjectData &inOutData );

        /// @copydoc NodeArrayMemoryManager::getFirstNode
        size_t getFirstNode( ObjectData &outData );
    };

    extern void cleanerFlat( char *dstPtr, size_t indexDst, char *srcPtr, size_t indexSrc,
                             size_t numSlots, size_t numFreeSlots, size_t elementsMemSize );
    extern void cleanerArrayVector3Zero( char *dstPtr, size_t indexDst, char *srcPtr, size_t indexSrc,
                                         size_t numSlots, size_t numFreeSlots, size_t elementsMemSize );
    extern void cleanerArrayVector3Unit( char *dstPtr, size_t indexDst, char *srcPtr, size_t indexSrc,
                                         size_t numSlots, size_t numFreeSlots, size_t elementsMemSize );
    extern void cleanerArrayQuaternion( char *dstPtr, size_t indexDst, char *srcPtr, size_t indexSrc,
                                        size_t numSlots, size_t numFreeSlots, size_t elementsMemSize );
    extern void cleanerArrayAabb( char *dstPtr, size_t indexDst, char *srcPtr, size_t indexSrc,
                                    size_t numSlots, size_t numFreeSlots, size_t elementsMemSize  );

    /** @} */
    /** @} */
}

#endif
