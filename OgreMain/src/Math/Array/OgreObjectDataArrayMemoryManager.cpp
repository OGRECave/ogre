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

#include "Math/Array/OgreArrayMemoryManager.h"

#include "Math/Array/OgreObjectData.h"

#include "OgreMovableObject.h"

namespace Ogre
{
    const size_t ObjectDataArrayMemoryManager::ElementsMemSize
                            [ObjectDataArrayMemoryManager::NumMemoryTypes] =
    {
        sizeof( Node** ),               //ArrayMemoryManager::Parent
        sizeof( MovableObject** ),      //ArrayMemoryManager::Owner
        6 * sizeof( Ogre::Real ),       //ArrayMemoryManager::LocalAabb
        6 * sizeof( Ogre::Real ),       //ArrayMemoryManager::WorldAabb
        1 * sizeof( Ogre::Real ),       //ArrayMemoryManager::LocalRadius
        1 * sizeof( Ogre::Real ),       //ArrayMemoryManager::WorldRadius
        1 * sizeof( Ogre::RealAsUint ), //ArrayMemoryManager::DistanceToCamera
        1 * sizeof( Ogre::Real ),       //ArrayMemoryManager::SquaredUpperDistance
        1 * sizeof( Ogre::Real ),       //ArrayMemoryManager::ShadowUpperDistance
        1 * sizeof( Ogre::uint32 ),     //ArrayMemoryManager::VisibilityFlags
        1 * sizeof( Ogre::uint32 ),     //ArrayMemoryManager::QueryFlags
        1 * sizeof( Ogre::uint32 ),     //ArrayMemoryManager::LightMask
    };
    const CleanupRoutines ObjectDataArrayMemoryManager::ObjCleanupRoutines[NumMemoryTypes] =
    {
        cleanerFlat,                    //ArrayMemoryManager::Parent
        cleanerFlat,                    //ArrayMemoryManager::Owner
        cleanerArrayAabb,               //ArrayMemoryManager::LocalAabb
        cleanerArrayAabb,               //ArrayMemoryManager::WorldAabb
        cleanerFlat,                    //ArrayMemoryManager::LocalRadius
        cleanerFlat,                    //ArrayMemoryManager::WorldRadius
        cleanerFlat,                    //ArrayMemoryManager::DistanceToCamera
        cleanerFlat,                    //ArrayMemoryManager::SquaredUpperDistance
        cleanerFlat,                    //ArrayMemoryManager::ShadowUpperDistance
        cleanerFlat,                    //ArrayMemoryManager::VisibilityFlags
        cleanerFlat,                    //ArrayMemoryManager::QueryFlags
        cleanerFlat,                    //ArrayMemoryManager::LightMask
    };
    //-----------------------------------------------------------------------------------
    ObjectDataArrayMemoryManager::ObjectDataArrayMemoryManager( uint16 depthLevel, size_t hintMaxNodes,
                                                    Node *dummyNode, MovableObject *dummyObject,
                                                    size_t cleanupThreshold, size_t maxHardLimit,
                                                    RebaseListener *rebaseListener ) :
            ArrayMemoryManager( ElementsMemSize, 0, ObjCleanupRoutines,
                                sizeof( ElementsMemSize ) / sizeof( size_t ), depthLevel,
                                hintMaxNodes, cleanupThreshold, maxHardLimit, rebaseListener ),
            mDummyNode( dummyNode ),
            mDummyObject( dummyObject )
    {
    }
    //-----------------------------------------------------------------------------------
    void ObjectDataArrayMemoryManager::initializeEmptySlots( size_t prevNumSlots )
    {
        ArrayMemoryManager::initializeEmptySlots( prevNumSlots );

        Node **nodesPtr = reinterpret_cast<Node**>( mMemoryPools[Parent] ) + prevNumSlots;
        MovableObject **ownersPtr = reinterpret_cast<MovableObject**>(mMemoryPools[Owner])+prevNumSlots;
        for( size_t i=prevNumSlots; i<mMaxMemory; ++i )
        {
            *nodesPtr++ = mDummyNode;
            *ownersPtr++ = mDummyObject;
        }
    }
    //-----------------------------------------------------------------------------------
    void ObjectDataArrayMemoryManager::createNewNode( ObjectData &outData )
    {
        const size_t nextSlot = createNewSlot();
        const unsigned char nextSlotIdx = nextSlot % ARRAY_PACKED_REALS;
        const size_t nextSlotBase       = nextSlot - nextSlotIdx;

        //Set memory ptrs
        outData.mIndex = nextSlotIdx;
        outData.mParents            = reinterpret_cast<Node**>( mMemoryPools[Parent] +
                                                nextSlotBase * mElementsMemSizes[Parent] );
        outData.mOwner              = reinterpret_cast<MovableObject**>( mMemoryPools[Owner] +
                                                nextSlotBase * mElementsMemSizes[Owner] );
        outData.mLocalAabb          = reinterpret_cast<ArrayAabb*>( mMemoryPools[LocalAabb] +
                                                nextSlotBase * mElementsMemSizes[LocalAabb] );
        outData.mWorldAabb          = reinterpret_cast<ArrayAabb*>( mMemoryPools[WorldAabb] +
                                                nextSlotBase * mElementsMemSizes[WorldAabb] );
        outData.mLocalRadius        = reinterpret_cast<Real*>( mMemoryPools[LocalRadius] +
                                                nextSlotBase * mElementsMemSizes[LocalRadius] );
        outData.mWorldRadius        = reinterpret_cast<Real*>( mMemoryPools[WorldRadius] +
                                                nextSlotBase * mElementsMemSizes[WorldRadius] );
        outData.mDistanceToCamera   = reinterpret_cast<RealAsUint*>( mMemoryPools[DistanceToCamera] +
                                                nextSlotBase * mElementsMemSizes[DistanceToCamera] );
        outData.mUpperDistance[0]   = reinterpret_cast<Real*>( mMemoryPools[UpperDistance] +
                                                nextSlotBase * mElementsMemSizes[UpperDistance] );
        outData.mUpperDistance[1]  = reinterpret_cast<Real*>(mMemoryPools[ShadowUpperDistance] +
                                                nextSlotBase * mElementsMemSizes[ShadowUpperDistance]);
        outData.mVisibilityFlags    = reinterpret_cast<uint32*>( mMemoryPools[VisibilityFlags] +
                                                nextSlotBase * mElementsMemSizes[VisibilityFlags] );
        outData.mQueryFlags         = reinterpret_cast<uint32*>( mMemoryPools[QueryFlags] +
                                                nextSlotBase * mElementsMemSizes[QueryFlags] );
        outData.mLightMask          = reinterpret_cast<uint32*>( mMemoryPools[LightMask] +
                                                nextSlotBase * mElementsMemSizes[LightMask] );

        //Set default values
        outData.mParents[nextSlotIdx]   = mDummyNode;
        outData.mOwner[nextSlotIdx]     = 0; //Caller has to fill it. Otherwise a crash is a good warning
        outData.mLocalAabb->setFromAabb( Aabb::BOX_INFINITE, nextSlotIdx );
        outData.mWorldAabb->setFromAabb( Aabb::BOX_INFINITE, nextSlotIdx );
        outData.mLocalRadius[nextSlotIdx]           = std::numeric_limits<Real>::infinity();
        outData.mWorldRadius[nextSlotIdx]           = std::numeric_limits<Real>::infinity();
        outData.mDistanceToCamera[nextSlotIdx]      = 0;
        outData.mUpperDistance[0][nextSlotIdx]      = std::numeric_limits<Real>::max();
        outData.mUpperDistance[1][nextSlotIdx]      = std::numeric_limits<Real>::max();
        outData.mVisibilityFlags[nextSlotIdx]       = MovableObject::getDefaultVisibilityFlags();
        outData.mQueryFlags[nextSlotIdx]            = MovableObject::getDefaultQueryFlags();
        outData.mLightMask[nextSlotIdx]             = 0xFFFFFFFF;
    }
    //-----------------------------------------------------------------------------------
    void ObjectDataArrayMemoryManager::destroyNode( ObjectData &inOutData )
    {
        //Zero out important data that would lead to bugs (Remember SIMD SoA means even if
        //there's one object in scene, 4 objects are still parsed simultaneously)
        inOutData.mParents[inOutData.mIndex]            = mDummyNode;
        inOutData.mOwner[inOutData.mIndex]              = mDummyObject;
        inOutData.mVisibilityFlags[inOutData.mIndex]    = 0;
        inOutData.mQueryFlags[inOutData.mIndex]         = 0;
        inOutData.mLightMask[inOutData.mIndex]          = 0;
        destroySlot( reinterpret_cast<char*>(inOutData.mParents), inOutData.mIndex );
        //Zero out all pointers
        inOutData = ObjectData();
    }
    //-----------------------------------------------------------------------------------
    size_t ObjectDataArrayMemoryManager::getFirstNode( ObjectData &outData )
    {
        // Quick hack to fill all pointer variables (I'm lazy to type!)
        memcpy( &outData.mParents, &mMemoryPools[0], sizeof(void*) * mMemoryPools.size() );
        return mUsedMemory;
    }
}
