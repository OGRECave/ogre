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

#ifndef __Ogre_TerrainMaterialGeneratorA_H__
#define __Ogre_TerrainMaterialGeneratorA_H__

#include "OgreTerrainPrerequisites.h"
#include "OgreTerrainMaterialGenerator.h"

namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Terrain
	*  Some details on the terrain component
	*  @{
	*/


	/** A TerrainMaterialGenerator which can cope with normal mapped, specular mapped
		terrain. 
	*/
	class _OgreTerrainExport TerrainMaterialGeneratorA : public TerrainMaterialGenerator
	{
	public:
		TerrainMaterialGeneratorA();
		~TerrainMaterialGeneratorA();

		/** Shader model 2 profile target. 
		*/
		class _OgreTerrainExport SM2Profile : public TerrainMaterialGenerator::Profile
		{
		public:
			SM2Profile(TerrainMaterialGenerator* parent, const String& name, const String& desc);
			~SM2Profile();
			MaterialPtr generate(const Terrain* terrain);
			void requestOptions(Terrain* terrain);

			/** Whether to support normal mapping per layer in the shader (default true). 
			*/
			bool isLayerNormalMappingEnabled() const  { return mLayerNormalMappingEnabled; }
			/** Whether to support normal mapping per layer in the shader (default true). 
			*/
			void setLayerNormalMappingEnabled(bool enabled);
			/** Whether to support parallax mapping per layer in the shader (default true). 
			*/
			bool isLayerParallaxMappingEnabled() const  { return mLayerParallaxMappingEnabled; }
			/** Whether to support parallax mapping per layer in the shader (default true). 
			*/
			void setLayerParallaxMappingEnabled(bool enabled);
			/** Whether to support specular mapping per layer in the shader (default true). 
			*/
			bool isLayerSpecularMappingEnabled() const  { return mLayerSpecularMappingEnabled; }
			/** Whether to support specular mapping per layer in the shader (default true). 
			*/
			void setLayerSpecularMappingEnabled(bool enabled);
			/** Whether to support a global colour map over the terrain in the shader,
				if it's present (default true). 
			*/
			bool isGlobalColourMapEnabled() const  { return mGlobalColourMapEnabled; }
			/** Whether to support a global colour map over the terrain in the shader,
			if it's present (default true). 
			*/
			void setGlobalColourMapEnabled(bool enabled);
		
		protected:
			/// Interface definition for helper class to generate shaders
			class ShaderHelper : public TerrainAlloc
			{
			public:
				ShaderHelper() {}
				virtual ~ShaderHelper() {}
				virtual HighLevelGpuProgramPtr generateVertexProgram(const SM2Profile* prof, const Terrain* terrain);
				virtual HighLevelGpuProgramPtr generateFragmentProgram(const SM2Profile* prof, const Terrain* terrain);
			protected:
				virtual HighLevelGpuProgramPtr createVertexProgram(const Terrain* terrain) = 0;
				virtual HighLevelGpuProgramPtr createFragmentProgram(const Terrain* terrain) = 0;
				virtual void generateVertexProgramSource(const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream);
				virtual void generateFragmentProgramSource(const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream);
				virtual void generateVpHeader(const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream) = 0;
				virtual void generateFpHeader(const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream) = 0;
				virtual void generateVpLayer(const SM2Profile* prof, const Terrain* terrain, uint layer, StringUtil::StrStreamType& outStream) = 0;
				virtual void generateFpLayer(const SM2Profile* prof, const Terrain* terrain, uint layer, StringUtil::StrStreamType& outStream) = 0;
				virtual void generateVpFooter(const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream) = 0;
				virtual void generateFpFooter(const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream) = 0;
				virtual void defaultVpParams(const SM2Profile* prof, const Terrain* terrain, const HighLevelGpuProgramPtr& prog);
				virtual void defaultFpParams(const SM2Profile* prof, const Terrain* terrain, const HighLevelGpuProgramPtr& prog);
				virtual String getChannel(uint idx);

			};

			/// Utility class to help with generating shaders for Cg / HLSL.
			class ShaderHelperCg : public ShaderHelper
			{
			protected:
				HighLevelGpuProgramPtr createVertexProgram(const Terrain* terrain);
				HighLevelGpuProgramPtr createFragmentProgram(const Terrain* terrain);
				void generateVpHeader(const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream);
				void generateFpHeader(const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream);
				void generateVpLayer(const SM2Profile* prof, const Terrain* terrain, uint layer, StringUtil::StrStreamType& outStream);
				void generateFpLayer(const SM2Profile* prof, const Terrain* terrain, uint layer, StringUtil::StrStreamType& outStream);
				void generateVpFooter(const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream);
				void generateFpFooter(const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream);
			};

			class ShaderHelperHLSL : public ShaderHelperCg
			{
			protected:
				HighLevelGpuProgramPtr createVertexProgram(const Terrain* terrain);
				HighLevelGpuProgramPtr createFragmentProgram(const Terrain* terrain);
			};

			/// Utility class to help with generating shaders for GLSL.
			class ShaderHelperGLSL : public ShaderHelper
			{
			protected:
				HighLevelGpuProgramPtr createVertexProgram(const Terrain* terrain);
				HighLevelGpuProgramPtr createFragmentProgram(const Terrain* terrain);
				void generateVpHeader(const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream) {}
				void generateFpHeader(const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream) {}
				void generateVpLayer(const SM2Profile* prof, const Terrain* terrain, uint layer, StringUtil::StrStreamType& outStream) {}
				void generateFpLayer(const SM2Profile* prof, const Terrain* terrain, uint layer, StringUtil::StrStreamType& outStream) {}
				void generateVpFooter(const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream) {}
				void generateFpFooter(const SM2Profile* prof, const Terrain* terrain, StringUtil::StrStreamType& outStream) {}
			};

			ShaderHelper* mShaderGen;
			bool mLayerNormalMappingEnabled;
			bool mLayerParallaxMappingEnabled;
			bool mLayerSpecularMappingEnabled;
			bool mGlobalColourMapEnabled;

		};




	};



	/** @} */
	/** @} */


}

#endif

