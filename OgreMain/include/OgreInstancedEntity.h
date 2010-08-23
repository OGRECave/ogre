/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#ifndef __InstancedEntity_H__
#define __InstancedEntity_H__

#include "OgreMovableObject.h"

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Scene
	*  @{
	*/

	/** @See InstanceBatch to understand how instancing works.

		Instanced entities work in a very similar way as how an Entity works, as this interface
		tries to mimic it as most as possible to make the transition between Entity and InstancedEntity
		as straightforward and simple as possible.
		There are a couple inherent limitations, for example setRenderQueueGroup only works on
		the InstanceBatch level, not the individual instance. This limits Z sorting for alpha blending
		quite significantly
		An InstancedEntity won't appear in scene until a SceneNode is attached to them. Once the
		InstancedEntity is no longer needed, call InstanceBatch::removeInstancedEntity to put them
		back into a queue so the batch can return it back again when a new instance is requested.
		@par
		Internally, instanced entities that are not visible (i.e. by setting setVisible(false) or
		when they don't have a SceneNode attached to them) a Zero world matrix is sent to the vertex shader
		which in turn causes a zero area triangle.
		This obviously means no Vertex shader benefit, but saves a bit of pixel shader processing power.
		Also this means this object won't be considered when sizing the InstanceBatch's bounding box.
		@par
		Each instance has an instance ID unique within the batch, which matches the ID sent to the vertex
		shader by some techniques (like InstanceBatchShader).
		@par
		Differences between Entity and InstancedEntity:
		* Entities don't show up in screen until they're attached to a SceneNode that is part of the scene
		graph (i.e. ultimate parent is the root scene node). InstancedEntities don't show up until they're
		attached to _any_ SceneNode
		* Setting RenderQueueGroup and other Renderable stuff works at InstanceBatch level, not
		InstancedEntity. This is obviously a limitation from instancing in general, not this particular
		implementation

        @remarks
			Design discussion webpage
        @author
            Matias N. Goldberg ("dark_sylinc")
        @version
            1.0
     */
	class _OgreExport InstancedEntity : public Ogre::MovableObject
	{
		friend InstanceBatch;
		friend InstanceBatchShader;
		friend InstanceBatchVTF;

		const uint32		m_instanceID;
		InstanceBatch		*m_batchOwner;

		AnimationStateSet	*mAnimationState;
		SkeletonInstance	*m_skeletonInstance;
		Matrix4				*mBoneMatrices;			//Local space
		Matrix4				*mBoneWorldMatrices;	//World space
		Matrix4				mLastParentXform;
		unsigned long		mFrameAnimationLastUpdated;

		//Returns number of matrices writen to xform, assumes xform has enough space
		size_t getTransforms( Matrix4 *xform );

		//Returns true if this InstancedObject is visible to the current camera
		bool findVisible( Camera *camera );

	public:
		InstancedEntity( InstanceBatch *batchOwner, uint32 instanceID );
		virtual ~InstancedEntity();

		InstanceBatch* _getOwner() const { return m_batchOwner; }

		const String& getMovableType(void) const;

		const AxisAlignedBox& getBoundingBox(void) const;
		Real getBoundingRadius(void) const;

		//This is used by our batch owner to get the closest entity's depth, returns infinity
		//when not attached to a scene node
		Real getSquaredViewDepth( const Camera* cam ) const;

		//Do nothing, InstanceBatch takes care of this.
		void _updateRenderQueue( RenderQueue* queue )	{}
		void visitRenderables( Renderable::Visitor* visitor, bool debugRenderables = false ) {}

		/** @see Entity::hasSkeleton */
		bool hasSkeleton(void) const { return m_skeletonInstance != 0; }
		/** @see Entity::getSkeleton */
		SkeletonInstance* getSkeleton(void) const { return m_skeletonInstance; }

		/** @see Entity::getAnimationState */
		AnimationState* getAnimationState(const String& name) const;
		/** @see Entity::getAllAnimationStates */
		AnimationStateSet* getAllAnimationStates(void) const;

		/** Called by InstanceBatch in <i>his</i> _updateRenderQueue to tell us we need
			to calculate our bone matrices.
			@remarks Assumes it has a skeleton (m_skeletonInstance != 0)
		*/
		void _updateAnimation(void);
	};
}

#endif
