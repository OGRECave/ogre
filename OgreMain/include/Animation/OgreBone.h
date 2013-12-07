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
#ifndef _Bone_H_
#define _Bone_H_

#include "OgrePrerequisites.h"

#include "OgreId.h"
#include "Math/Array/OgreBoneTransform.h"

#include "OgreHeaderPrefix.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Animation
	*  @{
	*/

	/** Class representing a Bone in the join hierarchy of a skeleton.
        @remarks
            Unlike 1.9; a Bone is practically a SceneNode in all purposes. It can even have
			objects directly attached to them. The only reason we need to overload is because
			the mDerivedTransform matrix is constructed differently, since it is not in world
			space, but rather in "offset space" (world space minus the reverse transform of the
			original bind pose).
			mDerivedPosition, mDeriverdOrientation and mDerivedScale are still in world space
			though, it's only the 4x4 matrix we send to the GPU that is in a different space.
    */
	class _OgreExport Bone : public NodeAlloc, public IdObject
    {
		typedef vector<Bone*>::type BoneVec;
    protected:
		ArrayMatrixAf4x3 const * RESTRICT_ALIAS	mReverseBind;
		BoneTransform							mTransform;

#ifndef NDEBUG
		mutable bool mCachedTransformOutOfDate;
#endif

		/// Depth level in the hierarchy tree (0: Root node, 1: Child of root, etc)
		uint16	mDepthLevel;
		/// Pointer to parent node
		Bone	*mParent;
		/// Collection of pointers to direct children
		BoneVec	mChildren;

		String	mName;

		/// The memory manager used to allocate the Transform.
		BoneMemoryManager *mBoneMemoryManager;

		/// @copydoc Node::_updateFromParent
		void _updateFromParent(void);

		/// @copydoc Node::updateFromParentImpl.
		void updateFromParentImpl(void);

		void setCachedTransformOutOfDate(void) const;

    public:
		/** Index in the vector holding this node reference (could be our parent node, or a global array
			tracking all created nodes to avoid memory leaks). Used for O(1) removals.
		@remarks
			It is the parent (or our creator) the one that sets this value, not ourselves. Do NOT modify
			it manually.
		*/
		size_t mGlobalIndex;
		/// @copydoc mGlobalIndex
		size_t mParentIndex;

        /** Constructor, only to be called by the creator SceneManager. */
		Bone( IdType id, BoneMemoryManager *boneMemoryManager,
				Bone *parent, ArrayMatrixAf4x3 const * RESTRICT_ALIAS reverseBind );

		Bone( const BoneTransform transformPtrs );

        virtual ~Bone();

		/// Returns how deep in the hierarchy we are (eg. 0 -> root node, 1 -> child of root)
		uint16 getDepthLevel() const								{ return mDepthLevel; }

		/// Returns a direct access to the Transform state
		BoneTransform& _getTransform()								{ return mTransform; }

		void _setReverseBindPtr( const ArrayMatrixAf4x3 *ptr )		{ mReverseBind = ptr; }

		/// Sets a custom name for this node. Doesn't have to be unique
		void setName( const String &name )							{ mName = name; }

		/// Returns the name of the node.
		const String& getName(void) const							{ return mName; }

		/// Gets this Bones's parent (NULL if this is the root).
		Bone* getParent(void) const									{ return mParent; }

		/** Sets a given orientation in local space (ie. relative to its parent)
		@remarks
			Don't call this function too often, as we need to convert to SoA
		*/
		inline void setOrientation( Quaternion q );

		/** Returns a quaternion representing the nodes orientation.
		@remarks
			Don't call this function too often, as we need to convert from SoA
		*/
		inline Quaternion getOrientation() const;

		/** Sets the position of the node relative to its parent.
		@remarks
			Don't call this function too often, as we need to convert to SoA
		*/
		inline void setPosition( const Vector3& pos );

		/** Gets the position of the node relative to its parent.
		@remarks
			Don't call this function too often, as we need to convert from SoA
		*/
		inline Vector3 getPosition(void) const;

		/** Sets the scale of the node relative to its parent.
		@remarks
			Don't call this function too often, as we need to convert to SoA
		*/
		inline void setScale( const Vector3& pos );

		/** Gets the scale of the node relative to its parent.
		@remarks
			Don't call this function too often, as we need to convert from SoA
		*/
		inline Vector3 getScale(void) const;

		/** Tells the Bone whether it should inherit orientation from it's parent node.
		@remarks
			@See Node::setInheritOrientation remarks.
			Note that Nodes and bones inherit scale and orientation differently, because
			Bones support non-uniform scaling, whereas Nodes don't.
		@par
			They may behave differently, because we assume inherited scale is never negative
			(due to this information being lost when embedded into a matrix. This mimics
			the behavior of major 3D modeling tools. i.e. scaling by x = -1 & y = -1 is
			the same as rotating 180° around Z axis)
			Default is true.
		@param inherit If true, this node's orientation will be affected by its parent's orientation.
			If false, it will not be affected.
		*/
		void setInheritOrientation(bool inherit);

		/** Returns true if this node is affected by orientation applied to the parent node.
		@remarks
			@See setInheritOrientation for more info.
		*/
		bool getInheritOrientation(void) const;

		/** Tells the node whether it should inherit scaling factors from it's parent node.
		@remarks
			@See setInheritOrientation.
		@param inherit If true, this node's scale will be affected by its parent's scale. If false,
			it will not be affected.
		*/
		void setInheritScale(bool inherit);

		/** Returns true if this node is affected by scaling factors applied to the parent node.
		@remarks
			@See setInheritOrientation for more info.
		*/
		bool getInheritScale(void) const;

		/** Gets the full transformation matrix for this node.
		@remarks
			This method returns the full transformation matrix
			for this node, including the effect of any parent Bone
			transformations. Assumes the caches are already updated
		*/
		FORCEINLINE const SimpleMatrixAf4x3& _getFullTransform(void) const
		{
			assert( !mCachedTransformOutOfDate );
			return mTransform.mDerivedTransform[mTransform.mIndex];
		}

		/** @See _getDerivedScaleUpdated remarks. @See _getFullTransform */
		const SimpleMatrixAf4x3& _getFullTransformUpdated(void);

		/** Drops the specified child from this node.
		@remarks
			Does not delete the node, just detaches it from
			this parent, potentially to be reattached elsewhere.
		@par
			Asserts if child is not one of our children.
		*/
		void removeChild( Bone* child );

		/** Removes all child Nodes attached to this node. Does not delete the nodes, just detaches them from
			this parent, potentially to be reattached elsewhere.
		*/
		void removeAllChildren(void);

		/** Notifies the given node is a child of us, we just add it to mChildren
			and set node->mParentIndex. <b>Internal Use.</b>
		@param node
			The bone/node that is a child of us.
		*/
		void _notifyOfChild( Bone *node );

        /** TODO
        */
        /*virtual SceneNode* createChildSceneNode(
				SceneMemoryMgrTypes sceneType = SCENE_DYNAMIC,
				const Vector3& translate = Vector3::ZERO, 
				const Quaternion& rotate = Quaternion::IDENTITY );*/

		/// @See Node::updateAllTransforms
		static void updateAllTransforms( const size_t numNodes, BoneTransform t,
										 ArrayMatrixAf4x3 const * RESTRICT_ALIAS reverseBind,
										 size_t numBinds );
    };
	/** @} */
	/** @} */


}// namespace

#include "OgreBone.inl"

#include "OgreHeaderSuffix.h"

#endif
