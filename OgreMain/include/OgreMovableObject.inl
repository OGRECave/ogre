
#ifndef NDEBUG
	//Needed by the dynamic_cast assert
	#include "OgreSceneNode.h"
#endif

namespace Ogre
{
	inline void MovableObject::setVisibilityFlags( uint32 flags )
	{
		mObjectData.mVisibilityFlags[mObjectData.mIndex] = (flags & RESERVED_VISIBILITY_FLAGS) |
						(mObjectData.mVisibilityFlags[mObjectData.mIndex] & ~RESERVED_VISIBILITY_FLAGS);
	}
	//-----------------------------------------------------------------------------------
	inline void MovableObject::addVisibilityFlags( uint32 flags )
	{
		mObjectData.mVisibilityFlags[mObjectData.mIndex] |= flags & RESERVED_VISIBILITY_FLAGS;
	}
	//-----------------------------------------------------------------------------------
	inline void MovableObject::removeVisibilityFlags( uint32 flags )
	{
		mObjectData.mVisibilityFlags[mObjectData.mIndex] &= ~(flags & RESERVED_VISIBILITY_FLAGS);
	}
	//-----------------------------------------------------------------------------------
	inline uint32 MovableObject::getVisibilityFlags(void) const
	{
		return mObjectData.mVisibilityFlags[mObjectData.mIndex] & RESERVED_VISIBILITY_FLAGS;
	}
	//-----------------------------------------------------------------------------------
	inline void MovableObject::setQueryFlags( uint32 flags )
	{
		mObjectData.mQueryFlags[mObjectData.mIndex] = flags;
	}
	//-----------------------------------------------------------------------------------
	inline void MovableObject::addQueryFlags( uint32 flags )
	{
		mObjectData.mQueryFlags[mObjectData.mIndex] |= flags;
	}
	//-----------------------------------------------------------------------------------
	inline void MovableObject::removeQueryFlags( uint32 flags )
	{
		mObjectData.mQueryFlags[mObjectData.mIndex] &= ~flags;
	}
	//-----------------------------------------------------------------------------------
	inline uint32 MovableObject::getQueryFlags(void) const
	{
		return mObjectData.mQueryFlags[mObjectData.mIndex];
	}
	//-----------------------------------------------------------------------------------
	inline uint32 MovableObject::getLightMask() const
	{
		return mObjectData.mLightMask[mObjectData.mIndex];
	}
	//-----------------------------------------------------------------------------------
	inline void MovableObject::setLightMask( uint32 lightMask )
	{
		mObjectData.mLightMask[mObjectData.mIndex] = lightMask;
	}
	//-----------------------------------------------------------------------------------
	inline void MovableObject::setRenderingDistance( Real dist )
	{
		assert( dist > 0.0f );
		if( dist > 0.0f )
		{
			mUpperDistance = dist; 
			mObjectData.mSquaredUpperDistance[mObjectData.mIndex] = mUpperDistance * mUpperDistance;
		}
	}
	//-----------------------------------------------------------------------------------
	inline void MovableObject::setVisible( bool visible )
	{
		assert( (!visible || mParentNode) && "Setting to visible an object without "
				"attachment is not supported!" );

		if( visible )
			mObjectData.mVisibilityFlags[mObjectData.mIndex] |= LAYER_VISIBILITY;
		else
			mObjectData.mVisibilityFlags[mObjectData.mIndex] &= ~LAYER_VISIBILITY;
	}
	//-----------------------------------------------------------------------------------
	inline bool MovableObject::getVisible(void) const
	{
		return (mObjectData.mVisibilityFlags[mObjectData.mIndex] & LAYER_VISIBILITY) != 0;
	}
	//-----------------------------------------------------------------------------------
	inline void MovableObject::setCastShadows( bool enabled )
	{
		if( enabled )
			mObjectData.mVisibilityFlags[mObjectData.mIndex] |= LAYER_SHADOW_CASTER;
		else
			mObjectData.mVisibilityFlags[mObjectData.mIndex] &= ~LAYER_SHADOW_CASTER;
	}
	//-----------------------------------------------------------------------------------
	inline bool MovableObject::getCastShadows(void) const
	{
		return (mObjectData.mVisibilityFlags[mObjectData.mIndex] & LAYER_SHADOW_CASTER) != 0;
	}
	//-----------------------------------------------------------------------------------
	inline SceneNode* MovableObject::getParentSceneNode(void) const
	{
		assert( !mParentNode || dynamic_cast<SceneNode*>( mParentNode ) );
		return static_cast<SceneNode*>( mParentNode );
	}
}
