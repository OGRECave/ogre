/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#include "Animation/OgreBone.h"
#include "OgreNode.h"
#include "OgreLogManager.h"

#include "Math/Array/OgreBoneMemoryManager.h"
#include "Math/Array/OgreKfTransform.h"
#include "Math/Array/OgreBooleanMask.h"

#ifndef NDEBUG
	#define CACHED_TRANSFORM_OUT_OF_DATE() this->setCachedTransformOutOfDate()
#else
	#define CACHED_TRANSFORM_OUT_OF_DATE() ((void)0)
#endif

namespace Ogre {
	//-----------------------------------------------------------------------
	Bone::Bone() :
		IdObject( 0 ),
		mReverseBind( 0 ),
		mTransform( BoneTransform() ),
#ifndef NDEBUG
		mCachedTransformOutOfDate( true ),
		mDebugParentNode( 0 ),
		mInitialized( false ),
#endif
		mDepthLevel( 0 ),
		mParent( 0 ),
		mName( "@Dummy Bone" ),
		mBoneMemoryManager( 0 ),
		mGlobalIndex( -1 ),
		mParentIndex( -1 )
	{
		
	}
	//-----------------------------------------------------------------------
	Bone::~Bone()
	{
		assert( !mInitialized && "Must call _deinitialize() before destructor!" );
	}
	//-----------------------------------------------------------------------
	void Bone::_initialize( IdType id, BoneMemoryManager *boneMemoryManager,
							Bone *parent, ArrayMatrixAf4x3 const * RESTRICT_ALIAS reverseBind )
	{
		assert( !mInitialized );

#ifndef NDEBUG
		mInitialized = true;
#endif

		this->_setId( id );
		mReverseBind		= reverseBind;
		mParent				= parent;
		mBoneMemoryManager	= boneMemoryManager;

		if( mParent )
			mDepthLevel = mParent->mDepthLevel + 1;

		//Will initialize mTransform
		mBoneMemoryManager->nodeCreated( mTransform, mDepthLevel );
		mTransform.mOwner[mTransform.mIndex] = this;
		if( mParent )
		{
			const BoneTransform parentTransform = mParent->mTransform;
			mTransform.mParentTransform[mTransform.mIndex] =
								&parentTransform.mDerivedTransform[parentTransform.mIndex];

			mParent->mChildren.push_back( this );
			this->mParentIndex = mParent->mChildren.size() - 1;
		}
	}
	//-----------------------------------------------------------------------
	void Bone::_deinitialize(void)
	{
		removeAllChildren();
		if( mParent )
			mParent->removeChild( this );

		if( mBoneMemoryManager )
			mBoneMemoryManager->nodeDestroyed( mTransform, mDepthLevel );

		mReverseBind		= 0;
		mBoneMemoryManager	= 0;

#ifndef NDEBUG
		mInitialized = false;
#endif
	}
	//-----------------------------------------------------------------------
	void Bone::unsetParent(void)
	{
		if( mParent )
		{
			mTransform.mParentTransform[mTransform.mIndex] = &SimpleMatrixAf4x3::IDENTITY;
			mParent = 0;//Needs to be set now, if nodeDetached triggers a cleanup,
						//_memoryRebased will be called on us

			//BoneMemoryManager will set mTransform.mParentTransform to a dummy
			//transform (as well as transfering the memory)
			mBoneMemoryManager->nodeDettached( mTransform, mDepthLevel );

			if( mDepthLevel != 0 )
			{
				mDepthLevel = 0;

				//Propagate the change to our children
				BoneVec::const_iterator itor = mChildren.begin();
				BoneVec::const_iterator end  = mChildren.end();

				while( itor != end )
				{
					(*itor)->parentDepthLevelChanged();
					++itor;
				}
			}
		}
	}
	//-----------------------------------------------------------------------
	void Bone::parentDepthLevelChanged(void)
	{
		mBoneMemoryManager->nodeMoved( mTransform, mDepthLevel, mParent->mDepthLevel + 1 );
		mDepthLevel = mParent->mDepthLevel + 1;

		//Keep propagating changes to our children
		BoneVec::const_iterator itor = mChildren.begin();
		BoneVec::const_iterator end  = mChildren.end();

		while( itor != end )
		{
			(*itor)->parentDepthLevelChanged();
			++itor;
		}
	}
	//-----------------------------------------------------------------------
	void Bone::setCachedTransformOutOfDate(void) const
	{
#ifndef NDEBUG
		mCachedTransformOutOfDate = true;

		BoneVec::const_iterator itor = mChildren.begin();
		BoneVec::const_iterator end  = mChildren.end();

		while( itor != end )
		{
			(*itor)->setCachedTransformOutOfDate();
			++itor;
		}
#else
		OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR,
					"Do not call setCachedTransformOutOfDate in Release builds.\n"
					"Use CACHED_TRANSFORM_OUT_OF_DATE macro instead!",
					"Bone::setCachedTransformOutOfDate" );
#endif
	}
	//-----------------------------------------------------------------------
	void Bone::resetParentTransformPtr(void)
	{
		if( mParent )
		{
			const BoneTransform parentTransform = mParent->mTransform;
			mTransform.mParentTransform[mTransform.mIndex] =
									&parentTransform.mDerivedTransform[parentTransform.mIndex];
		}

		_memoryRebased();
	}
	//-----------------------------------------------------------------------
	void Bone::_memoryRebased(void)
	{
		BoneVec::iterator itor = mChildren.begin();
		BoneVec::iterator end  = mChildren.end();
		while( itor != end )
		{
			(*itor)->resetParentTransformPtr();
			++itor;
		}
	}
	//-----------------------------------------------------------------------
	void Bone::_setNodeParent( Node *nodeParent )
	{
		if( mParent )
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Only Root bones call this function.",
						 "Bone::_addNodeParent" );
		}

#ifndef NDEBUG
		mDebugParentNode = nodeParent;
#endif
		if( nodeParent )
		{
			//This "Hack" just works. Don't ask. And it's fast!
			//(we're responsible for ensuring the memory layout matches)
			Transform parentTransf = nodeParent->_getTransform();
			mTransform.mParentTransform[mTransform.mIndex] = reinterpret_cast<SimpleMatrixAf4x3*>(
												&parentTransf.mDerivedTransform[parentTransf.mIndex] );
		}
		else
		{
			mTransform.mParentTransform[mTransform.mIndex] = &SimpleMatrixAf4x3::IDENTITY;
		}
	}
	//-----------------------------------------------------------------------
	void Bone::setInheritOrientation(bool inherit)
	{
		mTransform.mInheritOrientation[mTransform.mIndex] = inherit;
		CACHED_TRANSFORM_OUT_OF_DATE();
	}
	//-----------------------------------------------------------------------
	bool Bone::getInheritOrientation(void) const
	{
		return mTransform.mInheritOrientation[mTransform.mIndex];
	}
	//-----------------------------------------------------------------------
	void Bone::setInheritScale(bool inherit)
	{
		mTransform.mInheritScale[mTransform.mIndex] = inherit;
		CACHED_TRANSFORM_OUT_OF_DATE();
	}
	//-----------------------------------------------------------------------
	bool Bone::getInheritScale(void) const
	{
		return mTransform.mInheritScale[mTransform.mIndex];
	}
	//-----------------------------------------------------------------------
	const SimpleMatrixAf4x3& Bone::_getFullTransformUpdated(void)
	{
		_updateFromParent();
		return mTransform.mDerivedTransform[mTransform.mIndex];
	}
	//-----------------------------------------------------------------------
	void Bone::_updateFromParent(void)
	{
		if( mParent )
			mParent->_updateFromParent();

		updateFromParentImpl();
	}
	//-----------------------------------------------------------------------
	void Bone::updateFromParentImpl(void)
	{
		//Retrieve from parents. Unfortunately we need to do SoA -> AoS -> SoA conversion
		ArrayMatrixAf4x3 parentMat;
		parentMat.loadFromAoS( mTransform.mParentTransform );

		//ArrayMatrixAf4x3::retain is quite lengthy in instruction count, and the
		//general case is to inherit both attributes. This branch is justified.
		if( BooleanMask4::allBitsSet( mTransform.mInheritOrientation, mTransform.mInheritScale ) )
		{
			ArrayMaskR inheritOrientation	= BooleanMask4::getMask( mTransform.mInheritOrientation );
			ArrayMaskR inheritScale			= BooleanMask4::getMask( mTransform.mInheritScale );
			parentMat.retain( inheritOrientation, inheritScale );
		}

		//BIG TODO: When in local space, reverse bind can be applied before making the transforms.
		ArrayMatrixAf4x3 mat;
		mat.makeTransform( *mTransform.mPosition, *mTransform.mScale, *mTransform.mOrientation );
		mat = ((*mReverseBind) * mat) * parentMat;

		/*
			Calculating the bone matrices
			-----------------------------
			Now that we have the derived scaling factors, orientations & positions matrices,
			we have to compute the Matrix4x3 to apply to the vertices of a mesh.
			Because any modification of a vertex has to be relative to the bone, we must
			first reverse transform by the Bone's original derived position/orientation/scale,
			then transform by the new derived position/orientation/scale.
		*/
		mat.storeToAoS( mTransform.mDerivedTransform );

#ifndef NDEBUG
		for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
		{
			if( mTransform.mOwner[j] )
				mTransform.mOwner[j]->mCachedTransformOutOfDate = false;
		}
#endif
	}
	//-----------------------------------------------------------------------
	void Bone::updateAllTransforms( const size_t numNodes, BoneTransform t,
									ArrayMatrixAf4x3 const * RESTRICT_ALIAS _reverseBind,
									size_t numBinds )
	{
		size_t currentBind = 0;
		numBinds = (numBinds + ARRAY_PACKED_REALS - 1) / ARRAY_PACKED_REALS;

		ArrayMatrixAf4x3 derivedTransform;
		for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
		{
			//Retrieve from parents. Unfortunately we need to do SoA -> AoS -> SoA conversion
			ArrayMatrixAf4x3 parentMat;

			parentMat.loadFromAoS( t.mParentTransform );

			//ArrayMatrixAf4x3::retain is quite lengthy in instruction count, and the
			//general case is to inherit both attributes. This branch is justified.
			if( !BooleanMask4::allBitsSet( t.mInheritOrientation, t.mInheritScale ) )
			{
				ArrayMaskR inheritOrientation	= BooleanMask4::getMask( t.mInheritOrientation );
				ArrayMaskR inheritScale			= BooleanMask4::getMask( t.mInheritScale );
				parentMat.retain( inheritOrientation, inheritScale );
			}

			const ArrayMatrixAf4x3 * RESTRICT_ALIAS reverseBind = _reverseBind + currentBind;

			derivedTransform.makeTransform( *t.mPosition, *t.mScale, *t.mOrientation );
			derivedTransform = ((*reverseBind) * derivedTransform) * parentMat;

			/*
				Calculating the bone matrices
				-----------------------------
				Now that we have the derived scaling factors, orientations & positions matrices,
				we have to compute the Matrix4x3 to apply to the vertices of a mesh.
				Because any modification of a vertex has to be relative to the bone, we must
				first reverse transform by the Bone's original derived position/orientation/scale,
				then transform by the new derived position/orientation/scale.
			*/
			derivedTransform.storeToAoS( t.mDerivedTransform );

#ifndef NDEBUG
			for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
			{
				if( t.mOwner[j] )
					t.mOwner[j]->mCachedTransformOutOfDate = false;
			}
#endif

			t.advancePack();
			currentBind = ( currentBind + 1 ) % numBinds;
		}
	}
	//-----------------------------------------------------------------------
	void Bone::removeChild( Bone* child )
	{
		assert( child->getParent() == this && "Node says it's not our child" );
		assert( child->mParentIndex < mChildren.size() && "mParentIndex was out of date!!!" );

		if( child->mParentIndex < mChildren.size() )
		{
			BoneVec::iterator itor = mChildren.begin() + child->mParentIndex;

			assert( child == *itor && "mParentIndex was out of date!!!" );

			if( child == *itor )
			{
				itor = efficientVectorRemove( mChildren, itor );
				child->unsetParent();
				child->mParentIndex = -1;

				//The node that was at the end got swapped and has now a different index
				if( itor != mChildren.end() )
					(*itor)->mParentIndex = itor - mChildren.begin();
			}
		}
	}
	//-----------------------------------------------------------------------
	void Bone::removeAllChildren(void)
	{
		BoneVec::iterator itor = mChildren.begin();
		BoneVec::iterator end  = mChildren.end();
		while( itor != end )
		{
			(*itor)->unsetParent();
			(*itor)->mParentIndex = -1;
			++itor;
		}
		mChildren.clear();
	}
}

#undef CACHED_TRANSFORM_OUT_OF_DATE
