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

#include "Math/Array/OgreTransform.h"

#include "OgreNode.h"

namespace Ogre
{
    const size_t NodeArrayMemoryManager::ElementsMemSize[NodeArrayMemoryManager::NumMemoryTypes] =
    {
        sizeof( Node** ),               //ArrayMemoryManager::Parent
        sizeof( Node** ),               //ArrayMemoryManager::Owner
        3 * sizeof( Ogre::Real ),       //ArrayMemoryManager::Position
        4 * sizeof( Ogre::Real ),       //ArrayMemoryManager::Orientation
        3 * sizeof( Ogre::Real ),       //ArrayMemoryManager::Scale
        3 * sizeof( Ogre::Real ),       //ArrayMemoryManager::DerivedPosition
        4 * sizeof( Ogre::Real ),       //ArrayMemoryManager::DerivedOrientation
        3 * sizeof( Ogre::Real ),       //ArrayMemoryManager::DerivedScale
        16 * sizeof( Ogre::Real ),      //ArrayMemoryManager::WorldMat
        sizeof( bool ),                 //ArrayMemoryManager::InheritOrientation
        sizeof( bool )                  //ArrayMemoryManager::InheritScale
    };
    const CleanupRoutines NodeArrayMemoryManager::NodeInitRoutines[NumMemoryTypes] =
    {
        0,                          //ArrayMemoryManager::Parent
        0,                          //ArrayMemoryManager::Owner
        0,                          //ArrayMemoryManager::Position
        cleanerArrayQuaternion,     //ArrayMemoryManager::Orientation
        cleanerArrayVector3Unit,    //ArrayMemoryManager::Scale
        0,                          //ArrayMemoryManager::DerivedPosition
        cleanerArrayQuaternion,     //ArrayMemoryManager::DerivedOrientation
        cleanerArrayVector3Unit,    //ArrayMemoryManager::DerivedScale
        0,                          //ArrayMemoryManager::WorldMat
        0,                          //ArrayMemoryManager::InheritOrientation
        0                           //ArrayMemoryManager::InheritScale
    };
    const CleanupRoutines NodeArrayMemoryManager::NodeCleanupRoutines[NumMemoryTypes] =
    {
        cleanerFlat,                    //ArrayMemoryManager::Parent
        cleanerFlat,                    //ArrayMemoryManager::Owner
        cleanerArrayVector3Zero,        //ArrayMemoryManager::Position
        cleanerArrayQuaternion,         //ArrayMemoryManager::Orientation
        cleanerArrayVector3Unit,        //ArrayMemoryManager::Scale
        cleanerArrayVector3Zero,        //ArrayMemoryManager::DerivedPosition
        cleanerArrayQuaternion,         //ArrayMemoryManager::DerivedOrientation
        cleanerArrayVector3Unit,        //ArrayMemoryManager::DerivedScale
        cleanerFlat,                    //ArrayMemoryManager::WorldMat
        cleanerFlat,                    //ArrayMemoryManager::InheritOrientation
        cleanerFlat                     //ArrayMemoryManager::InheritScale
    };
    //-----------------------------------------------------------------------------------
    NodeArrayMemoryManager::NodeArrayMemoryManager( uint16 depthLevel, size_t hintMaxNodes,
                                                    Node *dummyNode, size_t cleanupThreshold,
                                                    size_t maxHardLimit,
                                                    RebaseListener *rebaseListener ) :
            ArrayMemoryManager( ElementsMemSize, NodeInitRoutines, NodeCleanupRoutines,
                                sizeof( ElementsMemSize ) / sizeof( size_t ), depthLevel,
                                hintMaxNodes, cleanupThreshold, maxHardLimit, rebaseListener ),
            mDummyNode( dummyNode )
    {
    }
    //-----------------------------------------------------------------------------------
    void NodeArrayMemoryManager::initializeEmptySlots( size_t prevNumSlots )
    {
        ArrayMemoryManager::initializeEmptySlots( prevNumSlots );

        Node **nodesPtr = reinterpret_cast<Node**>( mMemoryPools[Parent] ) + prevNumSlots;
        for( size_t i=prevNumSlots; i<mMaxMemory; ++i )
            *nodesPtr++ = mDummyNode;
    }
    //-----------------------------------------------------------------------------------
    void NodeArrayMemoryManager::createNewNode( Transform &outTransform )
    {
        const size_t nextSlot = createNewSlot();
        const unsigned char nextSlotIdx = nextSlot % ARRAY_PACKED_REALS;
        const size_t nextSlotBase       = nextSlot - nextSlotIdx;

        //Set memory ptrs
        outTransform.mIndex = nextSlotIdx;
        outTransform.mParents           = reinterpret_cast<Node**>( mMemoryPools[Parent] +
                                                nextSlotBase * mElementsMemSizes[Parent] );
        outTransform.mOwner             = reinterpret_cast<Node**>( mMemoryPools[Owner] +
                                                nextSlotBase * mElementsMemSizes[Owner] );
        outTransform.mPosition          = reinterpret_cast<ArrayVector3*>( mMemoryPools[Position] +
                                                nextSlotBase * mElementsMemSizes[Position] );
        outTransform.mOrientation       = reinterpret_cast<ArrayQuaternion*>(
                                                mMemoryPools[Orientation] +
                                                nextSlotBase * mElementsMemSizes[Orientation] );
        outTransform.mScale             = reinterpret_cast<ArrayVector3*>( mMemoryPools[Scale] +
                                                nextSlotBase * mElementsMemSizes[Scale] );
        outTransform.mDerivedPosition   = reinterpret_cast<ArrayVector3*>(
                                                mMemoryPools[DerivedPosition] +
                                                nextSlotBase * mElementsMemSizes[DerivedPosition] );
        outTransform.mDerivedOrientation=reinterpret_cast<ArrayQuaternion*>(
                                                mMemoryPools[DerivedOrientation] +
                                                nextSlotBase * mElementsMemSizes[DerivedOrientation] );
        outTransform.mDerivedScale      = reinterpret_cast<ArrayVector3*>( mMemoryPools[DerivedScale] +
                                                nextSlotBase * mElementsMemSizes[DerivedScale] );
        outTransform.mDerivedTransform  = reinterpret_cast<Matrix4*>( mMemoryPools[WorldMat] +
                                                nextSlotBase * mElementsMemSizes[WorldMat] );
        outTransform.mInheritOrientation= reinterpret_cast<bool*>( mMemoryPools[InheritOrientation] +
                                                nextSlotBase * mElementsMemSizes[InheritOrientation] );
        outTransform.mInheritScale      = reinterpret_cast<bool*>( mMemoryPools[InheritScale] +
                                                nextSlotBase * mElementsMemSizes[InheritScale] );

        //Set default values
        outTransform.mParents[nextSlotIdx] = mDummyNode;
        outTransform.mOwner[nextSlotIdx] = 0;
        outTransform.mPosition->setFromVector3( Vector3::ZERO, nextSlotIdx );
        outTransform.mOrientation->setFromQuaternion( Quaternion::IDENTITY, nextSlotIdx );
        outTransform.mScale->setFromVector3( Vector3::UNIT_SCALE, nextSlotIdx );
        outTransform.mDerivedPosition->setFromVector3( Vector3::ZERO, nextSlotIdx );
        outTransform.mDerivedOrientation->setFromQuaternion( Quaternion::IDENTITY, nextSlotIdx );
        outTransform.mDerivedScale->setFromVector3( Vector3::UNIT_SCALE, nextSlotIdx );
        outTransform.mDerivedTransform[nextSlotIdx] = Matrix4::IDENTITY;
        outTransform.mInheritOrientation[nextSlotIdx]   = true;
        outTransform.mInheritScale[nextSlotIdx]         = true;
    }
    //-----------------------------------------------------------------------------------
    void NodeArrayMemoryManager::destroyNode( Transform &inOutTransform )
    {
        //Zero out important data that would lead to bugs (Remember SIMD SoA means even if
        //there's one object in scene, 4 objects are still parsed simultaneously)

        inOutTransform.mParents[inOutTransform.mIndex]  = mDummyNode;
        inOutTransform.mOwner[inOutTransform.mIndex]    = 0;
        destroySlot( reinterpret_cast<char*>(inOutTransform.mParents), inOutTransform.mIndex );
        //Zero out all pointers
        inOutTransform = Transform();
    }
    //-----------------------------------------------------------------------------------
    size_t NodeArrayMemoryManager::getFirstNode( Transform &outTransform )
    {
        outTransform.mParents           = reinterpret_cast<Node**>( mMemoryPools[Parent] );
        outTransform.mOwner             = reinterpret_cast<Node**>( mMemoryPools[Owner] );
        outTransform.mPosition          = reinterpret_cast<ArrayVector3*>( mMemoryPools[Position] );
        outTransform.mOrientation       = reinterpret_cast<ArrayQuaternion*>(
                                                        mMemoryPools[Orientation] );
        outTransform.mScale             = reinterpret_cast<ArrayVector3*>( mMemoryPools[Scale] );
        outTransform.mDerivedPosition   = reinterpret_cast<ArrayVector3*>(
                                                        mMemoryPools[DerivedPosition] );
        outTransform.mDerivedOrientation= reinterpret_cast<ArrayQuaternion*>(
                                                        mMemoryPools[DerivedOrientation] );
        outTransform.mDerivedScale      = reinterpret_cast<ArrayVector3*>( mMemoryPools[DerivedScale] );
        outTransform.mDerivedTransform  = reinterpret_cast<Matrix4*>( mMemoryPools[WorldMat] );
        outTransform.mInheritOrientation= reinterpret_cast<bool*>( mMemoryPools[InheritOrientation] );
        outTransform.mInheritScale      = reinterpret_cast<bool*>( mMemoryPools[InheritScale] );

        return mUsedMemory;
    }
}
