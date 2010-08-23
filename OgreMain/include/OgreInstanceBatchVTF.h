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
#ifndef __InstanceBatchVTF_H__
#define __InstanceBatchVTF_H__

#include "OgreInstanceBatch.h"
#include "OgreTexture.h"

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Scene
	*  @{
	*/

	/** Instancing implementation using vertex texture through Vertex Texture Fetch (VTF)
		This implementation has the following advantages:
		  * Supports huge ammount of instances per batch
		  * Supports skinning of huge ammounts of instances per batch
		  * Doesn't need shader constants registers.
		  * Best suited for skinned entities

		But beware the disadvantages:
		  * VTF isn't (controversely) supported on old ATI X1800 hardware
		  * VTF is only fast on modern GPUs (ATI Radeon HD, GeForce 8 series onwards)
		  * On GeForce 6/7 series VTF is too slow
		  * Only one bone weight per vertex is supported

		Each InstanceBatchVTF has it's own texture, which occupies memory in VRAM.
		VRAM usage can be computed by doing 12 bytes * numInstances * numBones
		@par
		The material requires at least a texture unit stage named "InstancingVTF"

        @remarks
			Design discussion webpage: http://www.ogre3d.org/forums/viewtopic.php?f=4&t=59902
        @author
            Matias N. Goldberg ("dark_sylinc")
        @version
            1.0
     */
	class _OgreExport InstanceBatchVTF : public InstanceBatch
	{
		typedef vector<Matrix4>::type Matrix4Vec;

		size_t					m_numWorldMatrices;	//Num bones * num instances
		TexturePtr				m_matrixTexture;	//The VTF
		Matrix4Vec				m_xform;			//Used for temp buffer in updateVertexTexture

		void setupVertices( const SubMesh* baseSubMesh );
		void setupIndices( const SubMesh* baseSubMesh );

		/** Setups the material to use a vertex texture */
		void setupMaterialToUseVTF( TextureType textureType );

		/** Creates the vertex texture */
		void createVertexTexture( const SubMesh* baseSubMesh );

		/** Creates 2 TEXCOORD semantics that will be used to sample the vertex texture */
		void createVertexSemantics( const SubMesh* baseSubMesh, VertexData *thisVertexData,
									VertexData *baseVertexData );

		/** Keeps filling the VTF with world matrix data */
		void updateVertexTexture(void);

	public:
		InstanceBatchVTF( MeshPtr &meshReference, const MaterialPtr &material, size_t instancesPerBatch,
							 const Mesh::IndexMap *indexToBoneMap, const String &batchName );
		virtual ~InstanceBatchVTF();

		size_t calculateMaxNumInstances( const SubMesh *baseSubMesh ) const;

		void buildFrom( const SubMesh *baseSubMesh, const RenderOperation &renderOperation );

		//Renderable overloads
		void getWorldTransforms( Matrix4* xform ) const;
		unsigned short getNumWorldTransforms(void) const;

		/** Overloaded to be able to updated the vertex texture */
		void _updateRenderQueue(RenderQueue* queue);
	};
}

#endif
