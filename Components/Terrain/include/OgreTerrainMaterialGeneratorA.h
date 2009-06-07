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
			SM2Profile(const String& name, const String& desc);
			~SM2Profile();
			MaterialPtr generate(const Terrain* terrain);
			void requestOptions(Terrain* terrain);
		
		protected:
			/// Interface definition for helper class to generate shaders
			class ShaderHelper : public TerrainAlloc
			{
			public:
				ShaderHelper() {}
				virtual ~ShaderHelper() {}
				virtual HighLevelGpuProgramPtr generateVertexProgram(const Terrain* terrain, uint8 startLayer);
				virtual HighLevelGpuProgramPtr generateFragmentProgram(const Terrain* terrain, uint8 startLayer);
			protected:
				virtual HighLevelGpuProgramPtr createVertexProgram(const Terrain* terrain, uint8 startLayer) = 0;
				virtual HighLevelGpuProgramPtr createFragmentProgram(const Terrain* terrain, uint8 startLayer) = 0;
				virtual void generateVertexProgramSource(const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream);
				virtual void generateFragmentProgramSource(const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream);
				virtual void generateVpHeader(const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream) = 0;
				virtual void generateFpHeader(const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream) = 0;
				virtual void generateVpLayer(const Terrain* terrain, uint8 layer, StringUtil::StrStreamType& outStream) = 0;
				virtual void generateFpLayer(const Terrain* terrain, uint8 layer, StringUtil::StrStreamType& outStream) = 0;
				virtual void generateVpFooter(const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream) = 0;
				virtual void generateFpFooter(const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream) = 0;
				virtual void defaultVpParams(const Terrain* terrain, uint8 startLayer, const HighLevelGpuProgramPtr& prog);
				virtual void defaultFpParams(const Terrain* terrain, uint8 startLayer, const HighLevelGpuProgramPtr& prog);

			};

			/// Utility class to help with generating shaders for Cg / HLSL.
			class ShaderHelperCg : public ShaderHelper
			{
			protected:
				HighLevelGpuProgramPtr createVertexProgram(const Terrain* terrain, uint8 startLayer);
				HighLevelGpuProgramPtr createFragmentProgram(const Terrain* terrain, uint8 startLayer);
				void generateVpHeader(const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream);
				void generateFpHeader(const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream);
				void generateVpLayer(const Terrain* terrain, uint8 layer, StringUtil::StrStreamType& outStream);
				void generateFpLayer(const Terrain* terrain, uint8 layer, StringUtil::StrStreamType& outStream);
				void generateVpFooter(const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream);
				void generateFpFooter(const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream);
			};

			class ShaderHelperHLSL : public ShaderHelperCg
			{
			protected:
				HighLevelGpuProgramPtr createVertexProgram(const Terrain* terrain, uint8 startLayer);
				HighLevelGpuProgramPtr createFragmentProgram(const Terrain* terrain, uint8 startLayer);
			};

			/// Utility class to help with generating shaders for GLSL.
			class ShaderHelperGLSL : public ShaderHelper
			{
			protected:
				HighLevelGpuProgramPtr createVertexProgram(const Terrain* terrain, uint8 startLayer);
				HighLevelGpuProgramPtr createFragmentProgram(const Terrain* terrain, uint8 startLayer);
				void generateVpHeader(const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream) {}
				void generateFpHeader(const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream) {}
				void generateVpLayer(const Terrain* terrain, uint8 layer, StringUtil::StrStreamType& outStream) {}
				void generateFpLayer(const Terrain* terrain, uint8 layer, StringUtil::StrStreamType& outStream) {}
				void generateVpFooter(const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream) {}
				void generateFpFooter(const Terrain* terrain, uint8 startLayer, StringUtil::StrStreamType& outStream) {}
			};

			ShaderHelper* mShaderGen;

		};




	};



	/** @} */
	/** @} */


}

#endif

