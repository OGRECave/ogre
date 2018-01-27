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

#include "Math/Array/OgreBoneArrayMemoryManager.h"

#include "Math/Array/OgreBoneTransform.h"

namespace Ogre
{
    const size_t BoneArrayMemoryManager::ElementsMemSize[BoneArrayMemoryManager::NumMemoryTypes] =
    {
        sizeof( Bone** ),               //ArrayMemoryManager::Owner
        3 * sizeof( Ogre::Real ),       //ArrayMemoryManager::Position
        4 * sizeof( Ogre::Real ),       //ArrayMemoryManager::Orientation
        3 * sizeof( Ogre::Real ),       //ArrayMemoryManager::Scale
        sizeof( SimpleMatrixAf4x3** ),  //ArrayMemoryManager::ParentNode
        sizeof( SimpleMatrixAf4x3** ),  //ArrayMemoryManager::ParentMat
        12 * sizeof( Ogre::Real ),      //ArrayMemoryManager::WorldMat
        12 * sizeof( Ogre::Real ),      //ArrayMemoryManager::FinalMat
        sizeof( bool ),                 //ArrayMemoryManager::InheritOrientation
        sizeof( bool )                  //ArrayMemoryManager::InheritScale
    };
    const CleanupRoutines BoneArrayMemoryManager::BoneInitRoutines[NumMemoryTypes] =
    {
        0,                              //ArrayMemoryManager::Owner
        0,                              //ArrayMemoryManager::Position
        cleanerArrayQuaternion,         //ArrayMemoryManager::Orientation
        cleanerArrayVector3Unit,        //ArrayMemoryManager::Scale
        0,                              //ArrayMemoryManager::ParentNode
        0,                              //ArrayMemoryManager::ParentMat
        0,                              //ArrayMemoryManager::WorldMat
        0,                              //ArrayMemoryManager::FinalMat
        0,                              //ArrayMemoryManager::InheritOrientation
        0                               //ArrayMemoryManager::InheritScale
    };
    const CleanupRoutines BoneArrayMemoryManager::BoneCleanupRoutines[NumMemoryTypes] =
    {
        cleanerFlat,                    //ArrayMemoryManager::Owner
        cleanerArrayVector3Zero,        //ArrayMemoryManager::Position
        cleanerArrayQuaternion,         //ArrayMemoryManager::Orientation
        cleanerArrayVector3Unit,        //ArrayMemoryManager::Scale
        cleanerFlat,                    //ArrayMemoryManager::ParentNode
        cleanerFlat,                    //ArrayMemoryManager::ParentMat
        cleanerFlat,                    //ArrayMemoryManager::WorldMat
        cleanerFlat,                    //ArrayMemoryManager::FinalMat
        cleanerFlat,                    //ArrayMemoryManager::InheritOrientation
        cleanerFlat                     //ArrayMemoryManager::InheritScale
    };
    //-----------------------------------------------------------------------------------
    BoneArrayMemoryManager::BoneArrayMemoryManager( uint16 depthLevel, size_t hintMaxNodes,
                                                    size_t cleanupThreshold, size_t maxHardLimit,
                                                    RebaseListener *rebaseListener ) :
            ArrayMemoryManager( ElementsMemSize, BoneInitRoutines, BoneCleanupRoutines,
                                sizeof( ElementsMemSize ) / sizeof( size_t ), depthLevel,
                                hintMaxNodes, cleanupThreshold, maxHardLimit, rebaseListener )
    {
    }
    //-----------------------------------------------------------------------------------
    void BoneArrayMemoryManager::initializeEmptySlots( size_t prevNumSlots )
    {
        ArrayMemoryManager::initializeEmptySlots( prevNumSlots );

        bool *inheritOrientation = reinterpret_cast<bool*>(
                                        mMemoryPools[InheritOrientation] ) + prevNumSlots;
        bool *inheritScale = reinterpret_cast<bool*>( mMemoryPools[InheritScale] ) + prevNumSlots;
        SimpleMatrixAf4x3 const **parentMatPtr = reinterpret_cast<const SimpleMatrixAf4x3**>(
                                                    mMemoryPools[ParentMat] ) + prevNumSlots;
        SimpleMatrixAf4x3 const **parentNodePtr= reinterpret_cast<const SimpleMatrixAf4x3**>(
                                                    mMemoryPools[ParentNode] ) + prevNumSlots;
        for( size_t i=prevNumSlots; i<mMaxMemory; ++i )
        {
            *inheritOrientation++   = true;
            *inheritScale++         = true;
            *parentNodePtr++= &SimpleMatrixAf4x3::IDENTITY;
            *parentMatPtr++ = &SimpleMatrixAf4x3::IDENTITY;
        }
    }
    //-----------------------------------------------------------------------------------
    void BoneArrayMemoryManager::createNewNode( BoneTransform &outTransform )
    {
        const size_t nextSlot = createNewSlot();
        const unsigned char nextSlotIdx = nextSlot % ARRAY_PACKED_REALS;
        const size_t nextSlotBase       = nextSlot - nextSlotIdx;

        //Set memory ptrs
        outTransform.mIndex = nextSlotIdx;
        outTransform.mOwner             = reinterpret_cast<Bone**>( mMemoryPools[Owner] +
                                                nextSlotBase * mElementsMemSizes[Owner] );
        outTransform.mPosition          = reinterpret_cast<ArrayVector3*>( mMemoryPools[Position] +
                                                nextSlotBase * mElementsMemSizes[Position] );
        outTransform.mOrientation       = reinterpret_cast<ArrayQuaternion*>(
                                                mMemoryPools[Orientation] +
                                                nextSlotBase * mElementsMemSizes[Orientation] );
        outTransform.mScale             = reinterpret_cast<ArrayVector3*>( mMemoryPools[Scale] +
                                                nextSlotBase * mElementsMemSizes[Scale] );
        outTransform.mParentNodeTransform=reinterpret_cast<const SimpleMatrixAf4x3**>(
                                                mMemoryPools[ParentNode] +
                                                nextSlotBase * mElementsMemSizes[ParentNode] );
        outTransform.mParentTransform   = reinterpret_cast<const SimpleMatrixAf4x3**>(
                                                mMemoryPools[ParentMat] +
                                                nextSlotBase * mElementsMemSizes[ParentMat] );
        outTransform.mDerivedTransform  = reinterpret_cast<SimpleMatrixAf4x3*>( mMemoryPools[WorldMat] +
                                                nextSlotBase * mElementsMemSizes[WorldMat] );
        outTransform.mFinalTransform    = reinterpret_cast<SimpleMatrixAf4x3*>( mMemoryPools[FinalMat] +
                                                nextSlotBase * mElementsMemSizes[FinalMat] );
        outTransform.mInheritOrientation= reinterpret_cast<bool*>( mMemoryPools[InheritOrientation] +
                                                nextSlotBase * mElementsMemSizes[InheritOrientation] );
        outTransform.mInheritScale      = reinterpret_cast<bool*>( mMemoryPools[InheritScale] +
                                                nextSlotBase * mElementsMemSizes[InheritScale] );

        //Set default values
        outTransform.mOwner[nextSlotIdx]    = 0;
        outTransform.mPosition->setFromVector3( Vector3::ZERO, nextSlotIdx );
        outTransform.mOrientation->setFromQuaternion( Quaternion::IDENTITY, nextSlotIdx );
        outTransform.mScale->setFromVector3( Vector3::UNIT_SCALE, nextSlotIdx );
        outTransform.mParentNodeTransform[nextSlotIdx]  = &SimpleMatrixAf4x3::IDENTITY;
        outTransform.mParentTransform[nextSlotIdx]      = &SimpleMatrixAf4x3::IDENTITY;
        outTransform.mDerivedTransform[nextSlotIdx]     = SimpleMatrixAf4x3::IDENTITY;
        outTransform.mFinalTransform[nextSlotIdx]       = SimpleMatrixAf4x3::IDENTITY;
        outTransform.mInheritOrientation[nextSlotIdx]   = true;
        outTransform.mInheritScale[nextSlotIdx]         = true;
    }
    //-----------------------------------------------------------------------------------
    void BoneArrayMemoryManager::destroyNode( BoneTransform &inOutTransform )
    {
        //Zero out important data that would lead to bugs (Remember SIMD SoA means even if
        //there's one object in scene, 4 objects are still parsed simultaneously)

        inOutTransform.mOwner[inOutTransform.mIndex]            = 0;
        inOutTransform.mParentNodeTransform[inOutTransform.mIndex]=&SimpleMatrixAf4x3::IDENTITY;
        inOutTransform.mParentTransform[inOutTransform.mIndex]  = &SimpleMatrixAf4x3::IDENTITY;
        inOutTransform.mInheritOrientation[inOutTransform.mIndex]= true;
        inOutTransform.mInheritScale[inOutTransform.mIndex]     = true;
        destroySlot( reinterpret_cast<char*>(inOutTransform.mOwner), inOutTransform.mIndex );
        //Zero out all pointers
        inOutTransform = BoneTransform();
    }
    //-----------------------------------------------------------------------------------
    size_t BoneArrayMemoryManager::getFirstNode( BoneTransform &outTransform )
    {
        outTransform.mOwner             = reinterpret_cast<Bone**>( mMemoryPools[Owner] );
        outTransform.mPosition          = reinterpret_cast<ArrayVector3*>( mMemoryPools[Position] );
        outTransform.mOrientation       = reinterpret_cast<ArrayQuaternion*>(
                                                        mMemoryPools[Orientation] );
        outTransform.mScale             = reinterpret_cast<ArrayVector3*>( mMemoryPools[Scale] );
        outTransform.mParentNodeTransform=reinterpret_cast<const SimpleMatrixAf4x3**>(
                                                        mMemoryPools[ParentNode] );
        outTransform.mParentTransform   = reinterpret_cast<const SimpleMatrixAf4x3**>(
                                                        mMemoryPools[ParentMat] );
        outTransform.mDerivedTransform  = reinterpret_cast<SimpleMatrixAf4x3*>( mMemoryPools[WorldMat] );
        outTransform.mFinalTransform    = reinterpret_cast<SimpleMatrixAf4x3*>( mMemoryPools[FinalMat] );
        outTransform.mInheritOrientation= reinterpret_cast<bool*>( mMemoryPools[InheritOrientation] );
        outTransform.mInheritScale      = reinterpret_cast<bool*>( mMemoryPools[InheritScale] );

        return mUsedMemory;
    }
}
