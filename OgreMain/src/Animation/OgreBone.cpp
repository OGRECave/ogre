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
#include "OgreLogManager.h"

#include "Math/Array/OgreKfTransform.h"
#include "Math/Array/OgreBooleanMask.h"

namespace Ogre {
	Bone::Bone( IdType id, SceneManager* creator, NodeMemoryManager *nodeMemoryManager,
				SceneNode *parent, KfTransform const * RESTRICT_ALIAS reverseBind ) : 
		SceneNode( id, creator, nodeMemoryManager, parent ),
		mReverseBind( reverseBind )
	{
	}
	//-----------------------------------------------------------------------
	Bone::Bone( const Transform &transformPtrs ) : SceneNode( transformPtrs )
	{
	}
	//-----------------------------------------------------------------------
	Bone::~Bone()
	{
	}
	//-----------------------------------------------------------------------
	void Bone::updateFromParentImpl(void)
	{
		//Retrieve from parents. Unfortunately we need to do SoA -> AoS -> SoA conversion
		ArrayVector3 parentPos, parentScale;
		ArrayQuaternion parentRot;

		for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
		{
			Vector3 pos, scale;
			Quaternion qRot;
			const Transform &parentTransform = mTransform.mParents[j]->mTransform;
			parentTransform.mDerivedPosition->getAsVector3( pos, parentTransform.mIndex );
			parentTransform.mDerivedOrientation->getAsQuaternion( qRot, parentTransform.mIndex );
			parentTransform.mDerivedScale->getAsVector3( scale, parentTransform.mIndex );

			parentPos.setFromVector3( pos, j );
			parentRot.setFromQuaternion( qRot, j );
			parentScale.setFromVector3( scale, j );
		}

		parentRot.Cmov4( BooleanMask4::getMask( mTransform.mInheritOrientation ),
						 ArrayQuaternion::IDENTITY );
		parentScale.Cmov4( BooleanMask4::getMask( mTransform.mInheritScale ),
							ArrayVector3::UNIT_SCALE );

		// Scale own position by parent scale, NB just combine
        // as equivalent axes, no shearing
        *mTransform.mDerivedScale = parentScale * (*mTransform.mScale);

		// Combine orientation with that of parent
		*mTransform.mDerivedOrientation = parentRot * (*mTransform.mOrientation);

		// Change position vector based on parent's orientation & scale
		*mTransform.mDerivedPosition = parentRot * (parentScale * (*mTransform.mPosition));

		// Add altered position vector to parents
		*mTransform.mDerivedPosition += parentPos;

		/*
				Calculating the bone matrices
				-----------------------------
				Now that we have the derived scaling factors, orientations & positions in the
				OldBone nodes, we have to compute the Matrix4 to apply to the vertices of a mesh.
				Because any modification of a vertex has to be relative to the bone, we must
				first reverse transform by the OldBone's original derived position/orientation/scale,
				then transform by the new derived position/orientation/scale.
				Also note we combine scale as equivalent axes, no shearing.
			*/

		ArrayVector3 locScale = *mTransform.mDerivedScale * mReverseBind->mScale;

		// Combine orientation with binding pose inverse orientation
		ArrayQuaternion locRotate = *mTransform.mDerivedOrientation * mReverseBind->mOrientation;

		// Combine position with binding pose inverse position,
		// Note that translation is relative to scale & rotation,
		// so first reverse transform original derived position to
		// binding pose bone space, and then transform to current
		// derived bone space.
		ArrayVector3 locPos = *mTransform.mDerivedPosition +
								locRotate * (locScale * mReverseBind->mPosition);

		ArrayMatrix4 derivedTransform;
		derivedTransform.makeTransform( locPos, locScale, locRotate );
		derivedTransform.storeToAoS( mTransform.mDerivedTransform );
#ifndef NDEBUG
		for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
		{
			if( mTransform.mOwner[j] )
				mTransform.mOwner[j]->mCachedTransformOutOfDate = false;
		}
#endif
	}
	//-----------------------------------------------------------------------
	void Bone::updateAllTransforms( const size_t numNodes, Transform t,
									KfTransform const * RESTRICT_ALIAS _reverseBind,
									size_t numBinds )
	{
		size_t currentBind = 0;
		numBinds = (numBinds + ARRAY_PACKED_REALS - 1) / ARRAY_PACKED_REALS;

		ArrayMatrix4 derivedTransform;
		for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
		{
			//Retrieve from parents. Unfortunately we need to do SoA -> AoS -> SoA conversion
			ArrayVector3 parentPos, parentScale;
			ArrayQuaternion parentRot;

			for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
			{
				Vector3 pos, scale;
				Quaternion qRot;
				const Transform &parentTransform = t.mParents[j]->mTransform;
				parentTransform.mDerivedPosition->getAsVector3( pos, parentTransform.mIndex );
				parentTransform.mDerivedOrientation->getAsQuaternion( qRot, parentTransform.mIndex );
				parentTransform.mDerivedScale->getAsVector3( scale, parentTransform.mIndex );

				parentPos.setFromVector3( pos, j );
				parentRot.setFromQuaternion( qRot, j );
				parentScale.setFromVector3( scale, j );
			}

			parentRot.Cmov4( BooleanMask4::getMask( t.mInheritOrientation ),
							 ArrayQuaternion::IDENTITY );
			parentScale.Cmov4( BooleanMask4::getMask( t.mInheritScale ),
								ArrayVector3::UNIT_SCALE );

			// Scale own position by parent scale, NB just combine
            // as equivalent axes, no shearing
            *t.mDerivedScale = parentScale * (*t.mScale);

			// Combine orientation with that of parent
			*t.mDerivedOrientation = parentRot * (*t.mOrientation);

			// Change position vector based on parent's orientation & scale
			*t.mDerivedPosition = parentRot * (parentScale * (*t.mPosition));

			// Add altered position vector to parents
			*t.mDerivedPosition += parentPos;

			/*
				Calculating the bone matrices
				-----------------------------
				Now that we have the derived scaling factors, orientations & positions in the
				OldBone nodes, we have to compute the Matrix4 to apply to the vertices of a mesh.
				Because any modification of a vertex has to be relative to the bone, we must
				first reverse transform by the OldBone's original derived position/orientation/scale,
				then transform by the new derived position/orientation/scale.
				Also note we combine scale as equivalent axes, no shearing.
			*/

			const KfTransform * RESTRICT_ALIAS reverseBind = _reverseBind + currentBind;

			// Combine scale with binding pose inverse scale,
			// NB just combine as equivalent axes, no shearing
			ArrayVector3 locScale = *t.mDerivedScale * reverseBind->mScale;

			// Combine orientation with binding pose inverse orientation
			ArrayQuaternion locRotate = *t.mDerivedOrientation * reverseBind->mOrientation;

			// Combine position with binding pose inverse position,
			// Note that translation is relative to scale & rotation,
			// so first reverse transform original derived position to
			// binding pose bone space, and then transform to current
			// derived bone space.
			ArrayVector3 locPos = *t.mDerivedPosition + locRotate * (locScale * reverseBind->mPosition);

			derivedTransform.makeTransform( locPos, locScale, locRotate );
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
	bool Bone::setStatic( bool bStatic )
	{
		static bool warned = false;
		if( !warned )
		{
			LogManager::getSingleton().logMessage( "WARNING: Bones do not support being static" );
			warned = true;
		}

		return false;
	}
	//-----------------------------------------------------------------------
	void Bone::_notifyStaticDirty(void) const
	{
		assert( "Bone::_notifyStaticDirty shouldn't be called (no static for bones)" );
	}
	//-----------------------------------------------------------------------
	void Bone::removeAndDestroyChild( SceneNode *sceneNode )
	{
		//TODO
		OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "Bones can't remove their children.",
					"Bone::removeAndDestroyChild" );
	}
	//-----------------------------------------------------------------------
	void Bone::removeAndDestroyAllChildren(void)
	{
		//TODO
		OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "Bones can't remove their children.",
					"Bone::removeAndDestroyChild" );
	}
	//-----------------------------------------------------------------------
	void Bone::_notifyOfChild( Bone *node )
	{
		mChildren.push_back( node );
		node->mParentIndex = mChildren.size() - 1;
	}
}

