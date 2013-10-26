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
#ifndef _Bone_H__
#define _Bone_H__

#include "OgreSceneNode.h"
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
    class _OgreExport Bone : public SceneNode
    {
    protected:
		KfTransform const * RESTRICT_ALIAS	mReverseBind;

        /** @copydoc Node::updateFromParentImpl. */
        virtual void updateFromParentImpl(void);

    public:
        /** Constructor, only to be called by the creator SceneManager. */
		Bone( IdType id, SceneManager* creator, NodeMemoryManager *nodeMemoryManager,
				SceneNode *parent, KfTransform const * RESTRICT_ALIAS reverseBind );
        virtual ~Bone();

		/// Static bones are not supported.
		virtual bool setStatic( bool bStatic );

		/// @copydoc Node::_notifyStaticDirty
		virtual void _notifyStaticDirty(void) const;


		/// Bones can't remove children so easily.
        virtual void removeAndDestroyChild( SceneNode *sceneNode );
		/// Bones can't remove children so easily.
        virtual void removeAndDestroyAllChildren(void);

        /** TODO
        */
        /*virtual SceneNode* createChildSceneNode(
				SceneMemoryMgrTypes sceneType = SCENE_DYNAMIC,
				const Vector3& translate = Vector3::ZERO, 
				const Quaternion& rotate = Quaternion::IDENTITY );*/

		/// @See Node::updateAllTransforms
		static void updateAllTransforms( const size_t numNodes, Transform t,
										 KfTransform const * RESTRICT_ALIAS reverseBind,
										 size_t numBinds );
    };
	/** @} */
	/** @} */


}// namespace

#include "OgreHeaderSuffix.h"

#endif
