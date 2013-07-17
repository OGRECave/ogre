
#ifndef NDEBUG
	//Needed by the dynamic_cast assert
	#include "OgreSceneNode.h"
#endif

namespace Ogre
{
	inline void MovableObject::setVisibilityFlags( uint32 flags )
	{
		mObjectData.mVisibilityFlags[mObjectData.mIndex] = flags;
	}
	//-----------------------------------------------------------------------------------
	inline void MovableObject::addVisibilityFlags( uint32 flags )
	{
		mObjectData.mVisibilityFlags[mObjectData.mIndex] |= flags;
	}
	//-----------------------------------------------------------------------------------
	inline void MovableObject::removeVisibilityFlags( uint32 flags )
	{
		mObjectData.mVisibilityFlags[mObjectData.mIndex] &= ~flags;
	}
	//-----------------------------------------------------------------------------------
	inline uint32 MovableObject::getVisibilityFlags(void) const
	{
		return mObjectData.mVisibilityFlags[mObjectData.mIndex];
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
		mObjectData.mVisible[mObjectData.mIndex] = visible;
	}
	//-----------------------------------------------------------------------------------
	inline bool MovableObject::getVisible(void) const
	{
		return mObjectData.mVisible[mObjectData.mIndex];
	}
	//-----------------------------------------------------------------------------------
	inline SceneNode* MovableObject::getParentSceneNode(void) const
	{
		assert( dynamic_cast<SceneNode*>( mParentNode ) );
		return static_cast<SceneNode*>( mParentNode );
	}
}
