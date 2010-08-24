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
		@See SceneManager::createInstanceManager, which creates this InstanceManager. Each
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
			ShaderBased,			//Any SM 2.0+
			HardwareInstancing,		//Needs SM 3.0+ and HW instancing support
			TextureVTF,				//Needs Vertex Texture Fetch & SM 3.0+
									//only fast on ATI HD 2000+ Radeon and GeForce 8+
		};
	private:
		typedef vector<InstanceBatch*>::type		InstanceBatchVec;	//vec[batchN] = Batch
		typedef map<String, InstanceBatchVec>::type	InstanceBatchMap;	//map[materialName] = Vec

		const String			m_name;					//Not the name of the mesh
		MeshPtr					m_meshReference;
		InstanceBatchMap		m_instanceBatches;
		size_t					m_idCount;

		RenderOperation			m_sharedRenderOperation;

		size_t					m_instancesPerBatch;
		InstancingTechnique		m_instancingTechnique;

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

	public:
		InstanceManager( const String &customName, SceneManager *sceneManager,
						 const String &meshName, const String &groupName,
						 InstancingTechnique instancingTechnique, size_t instancesPerBatch );
		virtual ~InstanceManager();

		const String& getName() const { return m_name; }

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
	};
}

#endif
