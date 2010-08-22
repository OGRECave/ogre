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
#ifndef __InstanceBatch_H__
#define __InstanceBatch_H__

#include "OgrePrerequisites.h"
#include "OgreRenderOperation.h"

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Scene
	*  @{
	*/

	/** InstanceBatch forms part of the new Instancing system
		This is an abstract class that must be derived to implement different instancing techniques
		(@see InstanceBatchShader)
		OGRE wasn't truly thought for instancing. OGRE assumes that either:
			a. One MovableObject -> No Renderable
			b. One MovableObject -> One Renderable
			c. One MovableObject -> Many Renderable.
		However, instances work on reverse: Many MovableObject have the same Renderable.
		<b>Instancing is already difficult to cull by a CPU</b>, but the main drawback from this assumption
		is that it makes it even harder to take advantage from OGRE's culling capabilities
		(i.e. @see OctreeSceneManager)
		@par
		To workaround this problem, InstanceBatch updates on almost every frame,
		growing the bounding box to fit all instances that are not being culled individually.
		This helps by avoiding a huge bbox that may cover the whole scene, which decreases shadow
		quality considerably (as it is seen as large shadow receiver)
		Furthermore, if no individual instance is visible, the InstanceBatch switches it's visibility
		(@see MovableObject::setVisible) to avoid sending this Renderable to the GPU. This happens because
		even when no individual instance is visible, their merged bounding box may cause OGRE to think
		the batch is visible (i.e. the camera is looking between object A & B, but A & B aren't visible)
		@par
		<b>As it happens with instancing in general, all instanced entities from the same batch will share
		the same textures and materials</b>
		@par
		Each InstanceBatch preallocates a fixed ammount of m_instancesPerBatch instances once it's been
		built (@see build, @see buildFrom).
		@See createInstancedEntity and @see removeInstancedEntity on how to retrieve those instances
		remove them from scene.
		Note that, on GPU side, removing an instance from scene doesn't save GPU cycles on what
		respects vertex shaders, but saves a little fillrate and pixel shaders; unless all instances
		are removed, which saves GPU.
		For more information, see @InstancedEntity
		For information on how Ogre manages multiple Instance batches, @see InstanceManager

        @remarks
			Design discussion webpage
        @author
            Matias N. Goldberg ("dark_sylinc")
        @version
            1.0
     */
	class _OgreExport InstanceBatch : public Renderable, public MovableObject
	{
	protected:
		typedef vector<InstancedEntity*>::type InstancedEntityVec;

		RenderOperation		m_renderOperation;
		size_t				m_instancesPerBatch;

		MaterialPtr			m_material;

		MeshPtr				 m_meshReference;
		Mesh::IndexMap const *m_indexToBoneMap;

		//Matrix4				m_transforms;
		//InstancedEntities are all allocated at build time and kept as "unused"
		//when they're requested, they're removed from there when requested,
		//and put back again when they're no longer needed
		//Note each InstancedEntity has a unique ID ranging from [0; m_instancesPerBatch)
		InstancedEntityVec	m_instancedEntities;
		InstancedEntityVec	m_unusedEntities;

		//This bbox contains all (visible) instanced entities
		AxisAlignedBox		m_fullBoundingBox;
		Real				m_boundingRadius;
		bool				m_boundsDirty;

		/// Cached distance to last camera for getSquaredViewDepth
		mutable Real mCachedCameraDist;
		/// The camera for which the cached distance is valid
		mutable const Camera *mCachedCamera;

		virtual void setupVertices( const SubMesh* baseSubMesh ) = 0;
		virtual void setupIndices( const SubMesh* baseSubMesh ) = 0;
		virtual void createAllInstancedEntities(void);
		virtual void deleteAllInstancedEntities(void);

		//Returns false on errors that would prevent building this batch from the given submesh
		virtual bool checkSubMeshCompatibility( const SubMesh* baseSubMesh );

		void updateBounds();

	public:
		InstanceBatch( MeshPtr &meshReference, const MaterialPtr &material,
						size_t instancesPerBatch, const Mesh::IndexMap *indexToBoneMap=0 );
		virtual ~InstanceBatch();

		MeshPtr& _getMeshRef() { return m_meshReference; }

		/** Raises an exception if trying to change it after being built
		*/
		void _setInstancesPerBatch( size_t instancesPerBatch );

		const Mesh::IndexMap* _getIndexToBoneMap() const { return m_indexToBoneMap; }

		/** Some techniques have a limit on how many instances can be done.
			Sometimes even depends on the material being used.
			@par
			Note this is a helper function, as such it takes a submesh base to compute
			the parameters, instead of using the object's own. This allows
			querying for a technique without requiering to actually build it.
			@param The base submesh that will be using to build it.
			@returns The max instances limit
		*/
		virtual size_t calculateMaxNumInstances( const SubMesh *baseSubMesh ) const = 0;

		/** Constructs all the data needed to use this batch, as well as the
			InstanceEntities. Placed here because in the constructor virtual
			tables may not have been yet filled.
			@param baseSubMesh A sub mesh which the instances will be based upon from.
			@remarks
				Call this only ONCE. This is done automatically by Ogre::InstanceManager
				Caller is responsable for freeing buffers in this RenderOperation
				Buffers inside the RenderOp may be null if the built failed.
			@returns
				A render operation which is very usefull to pass to other InstanceBatches
				(@see buildFrom) so that they share the same vertex buffers and indices,
				when possible
        */
		virtual RenderOperation build( const SubMesh* baseSubMesh );

		/** Instancing consumes significantly more GPU memory than regular rendering
			methods. However, multiple batches can share most, if not all, of the
			vertex & index buffers to save memory.
			Derived classes are free to overload this method to manipulate what to
			reference from Render Op.
			For example, Hardware based instancing uses it's own vertex buffer for the
			last source binding, but shares the other sources.
			@param renderOperation The RenderOp to reference.
			@remarks
				Caller is responsable for freeing buffers passed as input arguments
				This function replaces the need to call build()
        */
		virtual void buildFrom( const RenderOperation &renderOperation );

		const Ogre::MeshPtr& _getMeshReference(void) const { return m_meshReference; }

		/** Returns true if it can not create more InstancedEntities
			(Num InstancedEntities == m_instancesPerBatch)
		*/
		bool isBatchFull(void) const { return m_unusedEntities.empty(); }

		/** Called by InstancedEntity(s) to tell us we need to update the bounds
			(we touch the SceneNode so the SceneManager aknowledges such change)
        */
		void _boundsDirty(void);

		/** Returns a pointer to a new InstancedEntity ready to use
			Note it's actually preallocated, so no memory allocation happens at
			this point.
			@remarks
				Returns NULL if all instances are being used
        */
		InstancedEntity* createInstancedEntity();

		/** Removes an InstancedEntity from the scene retrieved with
			getNewInstancedEntity, putting back into a queue
			@remarks
				Throws an exception if the instanced entity wasn't created by this batch
				Removed instanced entities save little CPU time, but _not_ GPU
        */
		void removeInstancedEntity( InstancedEntity *instancedEntity );

		//Renderable overloads
		const MaterialPtr& getMaterial(void) const		{ return m_material; }
		void getRenderOperation( RenderOperation& op )	{ op = m_renderOperation; }

		Real getSquaredViewDepth( const Camera* cam ) const;
        const LightList& getLights( void ) const;

		//MovableObject overloads
		const String& getMovableType(void) const;
		void _notifyCurrentCamera( Camera* cam );
		const AxisAlignedBox& getBoundingBox(void) const;
		Real getBoundingRadius(void) const;
		void _updateRenderQueue(RenderQueue* queue);
		void visitRenderables( Renderable::Visitor* visitor, bool debugRenderables = false );
	};
}

#endif
