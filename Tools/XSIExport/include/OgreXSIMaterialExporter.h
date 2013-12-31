/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#ifndef __XSIMATERIALEXPORTER_H__
#define __XSIMATERIALEXPORTER_H__

#include "OgreXSIHelper.h"
#include "OgreBlendMode.h"
#include "OgreMaterialSerializer.h"

namespace Ogre {

	class XsiMaterialExporter
	{
	public:
		XsiMaterialExporter();
		virtual ~XsiMaterialExporter();

		/** Export a set of XSI materials to a material script.
		@param materials List of materials to export
		@param filename Name of the script file to create
		@param copyTextures Whether to copy any textures used into the same
			folder as the material script.
		*/
		void exportMaterials(MaterialMap& materials, TextureProjectionMap& texProjMap, 
			const String& filename, bool copyTextures);
	protected:	
		MaterialSerializer mMatSerializer;
		
		typedef std::multimap<long,TextureUnitState*> TextureUnitTargetMap;
		/// Map of target id -> texture unit to match up tex transforms
		TextureUnitTargetMap mTextureUnitTargetMap;
		/// Pass queue, used to invert ordering
		PassQueue mPassQueue;
		/// Texture projection map
		TextureProjectionMap mTextureProjectionMap;

		
		// Export a single material
		void exportMaterial(MaterialEntry* matEntry, bool copyTextures, 
			const String& texturePath);
		/// Fill in all the details of a pass
		void populatePass(Pass* pass, XSI::Shader& xsishader);
		/// Fill in the texture units - note this won't pick up transforms yet
		void populatePassTextures(Pass* pass, PassEntry* passEntry, 
			bool copyTextures, const String& texturePath);
		/// Find any texture transforms and hook them up via 'target'
		void populatePassTextureTransforms(Pass* pass, XSI::Shader& xsishader);
		/// Populate basic rejection parameters for the pass
		void populatePassDepthCull(Pass* pass, XSI::Shader& xsishader);
		/// Populate lighting parameters for the pass
		void populatePassLighting(Pass* pass, XSI::Shader& xsishader);
		/// Populate scene blending parameters for the pass
		void populatePassSceneBlend(Pass* pass, XSI::Shader& xsishader);
		void populatePassCgPrograms(Pass* pass, XSI::Shader& xsishader);
		void populatePassHLSLPrograms(Pass* pass, XSI::Shader& xsishader);
		void populatePassD3DAssemblerPrograms(Pass* pass, XSI::Shader& xsishader);
		void populateOGLFiltering(TextureUnitState* tex, XSI::Shader& xsishader);
		void populateDXFiltering(TextureUnitState* tex, XSI::Shader& xsishader);


		// Utility method to get texture coord set from tspace_id name
		unsigned short getTextureCoordIndex(const String& tspace);

		/// Add a 2D texture from a shader
		TextureUnitState* add2DTexture(Pass* pass, XSI::Shader& shader, 
			bool copyTextures, const String& targetFolder);
		/// Add a cubic texture from a shader
		TextureUnitState* addCubicTexture(Pass* pass, XSI::Shader& shader, 
			bool copyTextures, const String& targetFolder);


		void clearPassQueue(void);

		SceneBlendFactor convertSceneBlend(short xsiVal);
		TextureUnitState::TextureAddressingMode convertAddressingMode(short xsiVal);
		void convertTexGenOGL(TextureUnitState* tex, long xsiVal, XSI::Shader& shader);
		void convertTexGenDX(TextureUnitState* tex, long xsiVal, XSI::Shader& shader);
	};
}

#endif


