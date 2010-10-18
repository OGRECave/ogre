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
#ifndef __InstanceManager_H__
#define __InstanceManager_H__

#include "OgrePrerequisites.h"
#include "OgreMesh.h"

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Scene
	*  @{
	*/

	/** This is the main starting point for the new instancing system.
		Each InstanceManager can control one technique and one mesh, but it can manage
		multiple materials at the same time.
		@See SceneManager::createInstanceManager, which creates this InstanceManager. Each one
		must have a unique name. It's wasteless to create two InstanceManagers with the same
		mesh reference, instancing technique and instances per batch count.
		This class takes care of managing batches automatically, so that more are created when
		needed, and reuse existing ones as much as posible; thus the user doesn't have to worry
		of managing all those low level issues.
		@See InstanceBatch & @see InstanceEntity for more information.

        @remarks
			Design discussion webpage: http://www.ogre3d.org/forums/viewtopic.php?f=4&t=59902
        @author
            Matias N. Goldberg ("dark_sylinc")
        @version
            1.0
     */
	class _OgreExport InstanceManager : public FactoryAlloc
	{
	public:
		enum InstancingTechnique
		{
			ShaderBased,			//Any SM 2.0+ @See InstanceBatchShader
			TextureVTF,				//Needs Vertex Texture Fetch & SM 3.0+ @See InstanceBatchVTF
			HardwareInstancing,		//Needs SM 3.0+ and HW instancing support
			InstancingTechniquesCount,
		};
	private:
		typedef vector<InstanceBatch*>::type		InstanceBatchVec;	//vec[batchN] = Batch
		typedef map<String, InstanceBatchVec>::type	InstanceBatchMap;	//map[materialName] = Vec

		const String			m_name;					//Not the name of the mesh
		MeshPtr					m_meshReference;
		InstanceBatchMap		m_instanceBatches;
		size_t					m_idCount;

		InstanceBatchVec		m_dirtyBatches;

		RenderOperation			m_sharedRenderOperation;

		size_t					m_instancesPerBatch;
		InstancingTechnique		m_instancingTechnique;
		uint16					m_instancingFlags;		//@see InstanceManagerFlags

		bool					m_showBoundingBoxes;
		SceneManager			*m_sceneManager;

		/** Finds a batch with at least one free instanced entity we can use.
			If none found, creates one.
		*/
		inline InstanceBatch* getFreeBatch( const String &materialName );

		/** Called when batches are fully exhausted (can't return more instances) so a new batch
			is created.
			For the first time use, it can take big build time.
			It takes care of getting the render operation which will be shared by further batches,
			which decreases their build time, and prevents GPU RAM from skyrocketing.
			@param materialName The material name, to know where to put this batch in the map
			@param firstTime True if this is the first time it is called
			@returns The created InstancedManager for convenience
        */
		InstanceBatch* buildNewBatch( const String &materialName, bool firstTime );

		/** @See defragmentBatches overload, this takes care of an array of batches
			for a specific material */
		void defragmentBatches( bool optimizeCull, vector<InstancedEntity*>::type &entities,
								InstanceBatchVec &fragmentedBatches );

	public:
		InstanceManager( const String &customName, SceneManager *sceneManager,
						 const String &meshName, const String &groupName,
						 InstancingTechnique instancingTechnique, uint16 instancingFlags,
						 size_t instancesPerBatch );
		virtual ~InstanceManager();

		const String& getName() const { return m_name; }

		/** Raises an exception if trying to change it after creating the first InstancedEntity
			@remarks The actual value may be less if the technique doesn't support having so much
			@see getMaxOrBestNumInstancesPerBatches for the usefulness of this function
			@param instancesPerBatch New instances per batch number
		*/
		void setInstancesPerBatch( size_t instancesPerBatch );

		/**	Calculates the maximum (or the best ammount, depending on flags) of instances
			per batch given the suggested size for the technique this manager was created for.
			@remarks
			This is done automatically when creating an instanced entity, but this function in conjunction
			with @see setInstancesPerBatch allows more flexible control over the ammount of instances
			per batch
			@param materialName Name of the material to base on
			@param suggestedSize Suggested ammount of instances per batch
			@param flags @See InstanceManagerFlags
			@returns The max/best ammount of instances per batch given the suggested size and flags
		*/
		size_t getMaxOrBestNumInstancesPerBatch( String materialName, size_t suggestedSize, uint16 flags );

		/** Returns whether the specified RenderTarget is compatible with this DepthBuffer
			That is, this DepthBuffer can be attached to that RenderTarget
            @remarks
                Most APIs impose the following restrictions:
				Width & height must be equal or higher than the render target's
				They must be of the same bit depth.
				They need to have the same FSAA setting
			@param renderTarget The render target to test against
        */
		InstancedEntity* createInstancedEntity( const String &materialName );

		/** This function can be usefull to improve CPU speed after having too many instances
			created, which where now removed, thus freeing many batches with zero used Instanced Entities
			However the batches aren't automatically removed from memory until the InstanceManager is
			destroyed, or this function is called. This function removes those batches which are completely
			unused (only wasting memory).
        */
		void cleanupEmptyBatches(void);

		/** After creating many entities (which turns in many batches) and then removing entities that
			are in the middle of these batches, there might be many batches with many free entities.
			Worst case scenario, there could be left one batch per entity. Imagine there can be
			80 entities per batch, there are 80 batches, making a total of 6400 entities. Then
			6320 of those entities are removed in a very specific way, which leads to having
			80 batches, 80 entities, and GPU vertex shader still needs to process 6400!
			This is called fragmentation. This function reparents the InstancedEntities
			to fewer batches, in this case leaving only one batch with 80 entities

			@remarks
			This function takes time. Make sure to call this only when you're sure there's
			too much of fragmentation and you won't be creating more InstancedEntities soon
			Also in many cases cleanupEmptyBatches() ought to be enough
			Defragmentation is done per material

			@param optimizeCulling When true, entities close toghether will be reorganized
			in the same batch for more efficient CPU culling. This can take more CPU
			time. You want this to be false if you now you're entities are moving very
			randomly which tends them to get separated and spread all over the scene
			(which nullifies any CPU culling)
        */
		void defragmentBatches( bool optimizeCulling );

		/** Toggles display of batches' bounding boxes on and off (not the individual instances).
			Useful for debugging or profiling
		*/
		void showBoundingBoxes( bool bShow );

		bool getShowBoundingBoxes() const { return m_showBoundingBoxes; }

		/** Called by an InstanceBatch when it requests their bounds to be updated for proper culling
			@param dirtyBatch The batch which is dirty, usually same as caller.
		*/
		void _addDirtyBatch( InstanceBatch *dirtyBatch );

		/** Called by SceneManager when we told it we have at least one dirty batch */
		void _updateDirtyBatches(void);
	};
}

#endif
