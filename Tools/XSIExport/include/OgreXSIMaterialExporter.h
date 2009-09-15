/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under 
the terms of the GNU Lesser General Public License as published by the Free Software 
Foundation; either version 2 of the License, or (at your option) any later 
version.

This program is distributed in the hope that it will be useful, but WITHOUT 
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with 
this program; if not, write to the Free Software Foundation, Inc., 59 Temple 
Place - Suite 330, Boston, MA 02111-1307, USA, or go to 
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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


