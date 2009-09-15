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
#include "OgreRoot.h"
#include "OgreHlslFixedFuncEmuShaderGenerator.h"

namespace Ogre 
{
	//---------------------------------------------------------------------
	HlslFixedFuncEmuShaderGenerator::HlslFixedFuncEmuShaderGenerator()
	{
		mLanguageName	= "hlsl";
		mName			= "hlsl";
		mVpTarget		= "vs_3_0";
		mFpTarget		= "ps_3_0";
	}
	//---------------------------------------------------------------------
	HlslFixedFuncEmuShaderGenerator::~HlslFixedFuncEmuShaderGenerator()
	{

	}
	//---------------------------------------------------------------------
	const String HlslFixedFuncEmuShaderGenerator::getShaderSource( const String & vertexProgramName,
		const String & fragmentProgramName, const VertexBufferDeclaration & vertexBufferDeclaration, 
		FixedFuncState & fixedFuncState )
	{
		bool bHasColor = vertexBufferDeclaration.hasColor();
		uint8 texcoordCount = vertexBufferDeclaration.getTexcoordCount();
		bool hasNormal = false;

		String shaderSource = "";

		shaderSource = shaderSource + "struct VS_INPUT { ";

		map<uint8, VertexElementType>::type texCordVecType;

		uint8 semanticCount[100];
		ZeroMemory(semanticCount, sizeof(uint8) * 100);
		const VertexBufferElementList & vertexBufferElementList = vertexBufferDeclaration.getVertexBufferElementList();
		for (unsigned short i = 0 ; i < vertexBufferElementList.size() ; i++)
		{
			VertexElementSemantic semantic = vertexBufferElementList[i].getVertexElementSemantic();
			VertexElementType type = vertexBufferElementList[i].getVertexElementType();

			String thisElementSemanticCount = StringConverter::toString(semanticCount[semantic]);
			semanticCount[semantic]++;
			String parameterType = "";
			String parameterName = "";
			String parameterShaderTypeName = "";

			switch (type)
			{
			case VET_FLOAT1:
				parameterType = "float";
				break;
			case VET_FLOAT2:
				parameterType = "float2";
				break;
			case VET_FLOAT3:
				parameterType = "float3";
				break;
			case VET_FLOAT4:
				parameterType = "float4";
				break;
			case VET_COLOUR:
			case VET_COLOUR_ARGB:
			case VET_COLOUR_ABGR:
				parameterType = "unsigned int";		
				break;
			case VET_SHORT1:
				parameterType = "short";		
				break;
			case VET_SHORT2:
				parameterType = "short2";		
				break;
			case VET_SHORT3:
				parameterType = "short3";		
				break;
			case VET_SHORT4:
				parameterType = "short4";		
				break;
			case VET_UBYTE4:
				parameterType = "float4";
				break;

			}
			switch (semantic)
			{
			case VES_POSITION:
				parameterName = "Position";
				parameterShaderTypeName = "POSITION";
				//parameterType = "float4"; // position must be float4 (and not float3 like in the buffer)
				break;
			case VES_BLEND_WEIGHTS:
				parameterName = "BlendWeight";
				parameterShaderTypeName = "BLENDWEIGHT";
				break;
			case VES_BLEND_INDICES:
				parameterName = "BlendIndices";
				parameterShaderTypeName = "BLENDINDICES";
				break;
			case VES_NORMAL:
				parameterName = "Normal";
				parameterShaderTypeName = "NORMAL";
				hasNormal = true;
				break;
			case VES_DIFFUSE:
				parameterName = "DiffuseColor";
				parameterShaderTypeName = "COLOR";
				break;
			case VES_SPECULAR:
				parameterName = "SpecularColor";
				parameterShaderTypeName = "COLOR";
				thisElementSemanticCount = StringConverter::toString(semanticCount[VES_DIFFUSE]); // Diffuse is the "COLOR" count...
				semanticCount[VES_DIFFUSE]++;
				break;
			case VES_TEXTURE_COORDINATES:
				parameterName = "Texcoord";
				texCordVecType[semanticCount[semantic]-1] = type;
				parameterShaderTypeName = "TEXCOORD";
				break;
			case VES_BINORMAL:
				parameterName = "Binormal";
				parameterShaderTypeName = "BINORMAL";
				break;
			case VES_TANGENT:
				parameterName = "Tangent";
				parameterShaderTypeName = "TANGENT";
				break;
			}



			shaderSource = shaderSource + parameterType + " " + parameterName + thisElementSemanticCount + " : " + parameterShaderTypeName + thisElementSemanticCount + ";\r\n";
		}

		shaderSource = shaderSource + " };";

		shaderSource = shaderSource + "float4x4  World;\r\n";
		shaderSource = shaderSource + "float4x4  View;\r\n";
		shaderSource = shaderSource + "float4x4  Projection;\r\n";
		shaderSource = shaderSource + "float4x4  ViewIT;\r\n";
		shaderSource = shaderSource + "float4x4  WorldViewIT;\r\n";	
	
		for(size_t i = 0 ; i < fixedFuncState.getTextureLayerStateList().size() ; i++)
		{
			String layerCounter = StringConverter::toString(i);
			shaderSource = shaderSource + "float4x4  TextureMatrix" + layerCounter + ";\r\n";
		}

		switch (fixedFuncState.getGeneralFixedFuncState().getFogMode())
		{
		case FOG_NONE:
			break;
		case FOG_EXP:
		case FOG_EXP2:
			shaderSource = shaderSource + "float FogDensity;\r\n";
			break;
		case FOG_LINEAR:
			shaderSource = shaderSource + "float FogStart;\r\n";
			shaderSource = shaderSource + "float FogEnd;\r\n";
			break;
		}

		shaderSource = shaderSource + "float4 BaseLightAmbient;\r\n";
		if (fixedFuncState.getGeneralFixedFuncState().getLightingEnabled() && fixedFuncState.getGeneralFixedFuncState().getTotalNumberOfLights() > 0)
		{

			// Point Light
			for(uint8 i = 0 ; i < fixedFuncState.getGeneralFixedFuncState().getLightTypeCount(Light::LT_POINT) ; i++)
			{
				String prefix = "PointLight" + StringConverter::toString(i) + "_";
				shaderSource = shaderSource + "float3 " + prefix + "Position;\r\n";
				shaderSource = shaderSource + "float4 " + prefix + "Ambient;\r\n";
				shaderSource = shaderSource + "float4 " + prefix + "Diffuse;\r\n";
				shaderSource = shaderSource + "float4 " + prefix + "Specular;\r\n";				
				shaderSource = shaderSource + "float  " + prefix + "Range;\r\n";				
				shaderSource = shaderSource + "float3 " + prefix + "Attenuation;\r\n";				
			}

			// Directional Light
			for(uint8 i = 0 ; i < fixedFuncState.getGeneralFixedFuncState().getLightTypeCount(Light::LT_DIRECTIONAL) ; i++)
			{
				String prefix = "DirectionalLight" + StringConverter::toString(i) + "_";
				shaderSource = shaderSource + "float3 " + prefix + "Direction;\r\n";
				shaderSource = shaderSource + "float4 " + prefix + "Ambient;\r\n";
				shaderSource = shaderSource + "float4 " + prefix + "Diffuse;\r\n";
				shaderSource = shaderSource + "float4 " + prefix + "Specular;\r\n";				
			}

			// Spot Light
			for(uint8 i = 0 ; i < fixedFuncState.getGeneralFixedFuncState().getLightTypeCount(Light::LT_SPOTLIGHT) ; i++)
			{
				String prefix = "SpotLight" + StringConverter::toString(i) + "_";
				shaderSource = shaderSource + "float3 " + prefix + "Direction;\r\n";
				shaderSource = shaderSource + "float3 " + prefix + "Position;\r\n";
				shaderSource = shaderSource + "float4 " + prefix + "Ambient;\r\n";
				shaderSource = shaderSource + "float4 " + prefix + "Diffuse;\r\n";
				shaderSource = shaderSource + "float4 " + prefix + "Specular;\r\n";				
				shaderSource = shaderSource + "float3 " + prefix + "Attenuation;\r\n";				
				shaderSource = shaderSource + "float3 " + prefix + "Spot;\r\n";					
			}

		}
		


		shaderSource = shaderSource + "struct VS_OUTPUT\r\n";
		shaderSource = shaderSource + "{\r\n";
		shaderSource = shaderSource + "float4 Pos : SV_POSITION;\r\n";
		for(size_t i = 0 ; i < fixedFuncState.getTextureLayerStateList().size() ; i++)
		{
			String layerCounter = StringConverter::toString(i);

			const TextureLayerState & curTextureLayerState = fixedFuncState.getTextureLayerStateList()[i];
			switch(texCordVecType[i])
			{
			case VET_FLOAT1:
				shaderSource = shaderSource + "float1 Texcoord" + layerCounter + " : TEXCOORD" + layerCounter + ";\r\n";
				break;
			case VET_FLOAT2:
				if(curTextureLayerState.getTextureType()==TEX_TYPE_CUBE_MAP )
				{
					shaderSource = shaderSource + "float3 Texcoord" + layerCounter + " : TEXCOORD" + layerCounter + ";\r\n";
				}
				else
				{
					shaderSource = shaderSource + "float2 Texcoord" + layerCounter + " : TEXCOORD" + layerCounter + ";\r\n";
				}
				break;
			case VET_FLOAT3:
				shaderSource = shaderSource + "float3 Texcoord" + layerCounter + " : TEXCOORD" + layerCounter + ";\r\n";
				break;
			}

		}
		
		shaderSource = shaderSource + "float4 Color : COLOR0;\r\n";
		shaderSource = shaderSource + "float4 ColorSpec : COLOR1;\r\n";

		if (fixedFuncState.getGeneralFixedFuncState().getFogMode() != FOG_NONE)
		{
			shaderSource = shaderSource + "float fogDist : FOGDISTANCE;\r\n"; 
		}

		shaderSource = shaderSource + "};\r\n";

		shaderSource = shaderSource + "VS_OUTPUT " + vertexProgramName + "( VS_INPUT input )\r\n";
		shaderSource = shaderSource + "{\r\n";
		shaderSource = shaderSource + "VS_OUTPUT output = (VS_OUTPUT)0;\r\n";	
		shaderSource = shaderSource + "float4 worldPos = mul(  float4( input.Position0 , 1 ), World);\r\n";
		shaderSource = shaderSource + "float4 cameraPos = mul(  worldPos, View );\r\n";
		shaderSource = shaderSource + "output.Pos = mul( cameraPos, Projection );\r\n";	

		if (hasNormal)
		{
			shaderSource = shaderSource + "float3 Normal = input.Normal0;\r\n";	
		}
		else
		{
			shaderSource = shaderSource + "float3 Normal = float3(0.0, 0.0, 0.0);\r\n";	
		}
		



		for(size_t i = 0 ; i < fixedFuncState.getTextureLayerStateList().size() ; i++)
		{
			const TextureLayerState & curTextureLayerState = fixedFuncState.getTextureLayerStateList()[i];
			String layerCounter = StringConverter::toString(i);
			String coordIdx = StringConverter::toString(curTextureLayerState.getCoordIndex());

			shaderSource = shaderSource + "{\r\n";
			switch(curTextureLayerState.getTexCoordCalcMethod())
			{
			case TEXCALC_NONE:
				if (curTextureLayerState.getCoordIndex() < texcoordCount)
				{

					switch(texCordVecType[i])
					{
					case VET_FLOAT1:
						shaderSource = shaderSource + "output.Texcoord" + layerCounter + " = input.Texcoord" + coordIdx + ";\r\n";		
						break;
					case VET_FLOAT2:
						shaderSource = shaderSource + "float4 texCordWithMatrix = float4(input.Texcoord" + coordIdx + ".x, input.Texcoord" + coordIdx + ".y, 0, 1);\r\n";
						shaderSource = shaderSource + "texCordWithMatrix = mul(texCordWithMatrix, TextureMatrix" + layerCounter + " );\r\n";
						shaderSource = shaderSource + "output.Texcoord" + layerCounter + " = texCordWithMatrix.xy;\r\n";		
						break;
					case VET_FLOAT3:
						shaderSource = shaderSource + "output.Texcoord" + layerCounter + " = input.Texcoord" + coordIdx + ";\r\n";		
						break;
					}

				}
				else
				{

					switch(texCordVecType[i])
					{
					case VET_FLOAT1:
						shaderSource = shaderSource + "output.Texcoord" + layerCounter + " = 0.0;\r\n"; // so no error
						break;
					case VET_FLOAT2:
						shaderSource = shaderSource + "output.Texcoord" + layerCounter + " = float2(0.0, 0.0);\r\n"; // so no error
						break;
					case VET_FLOAT3:
						shaderSource = shaderSource + "output.Texcoord" + layerCounter + " = float3(0.0, 0.0, 0.0);\r\n"; // so no error
						break;
					}
				}
				break;
			case TEXCALC_ENVIRONMENT_MAP: 
				//shaderSource = shaderSource + "float3 ecPosition3 = cameraPos.xyz/cameraPos.w;\r\n";
				shaderSource = shaderSource + "float3 u = normalize(cameraPos.xyz);\r\n";
				shaderSource = shaderSource + "float3 r = reflect(u, Normal);\r\n";
				shaderSource = shaderSource + "float  m = 2.0 * sqrt(r.x * r.x + r.y * r.y + (r.z + 1.0) * (r.z + 1.0));\r\n";
				shaderSource = shaderSource + "output.Texcoord" + layerCounter + " = float2 (r.x / m + 0.5, r.y / m + 0.5);\r\n";
				break;
			case TEXCALC_ENVIRONMENT_MAP_PLANAR:
				break;
			case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
				assert(curTextureLayerState.getTextureType() == TEX_TYPE_CUBE_MAP);
				shaderSource = shaderSource + "{\r\n";	
				shaderSource = shaderSource + "	float4 worldNorm = mul(float4(Normal, 0), World);\r\n";	
//				shaderSource = shaderSource + "	float4 viewNorm = mul(worldNorm, View);\r\n";	
//				shaderSource = shaderSource + "	viewNorm = normalize(viewNorm);\r\n";	
//				shaderSource = shaderSource + "output.Texcoord" + layerCounter + " = reflect(viewNorm.xyz, float3(0.0,0.0,-1.0));\r\n";	
				shaderSource = shaderSource + "	float3 viewNorm = worldPos.xyz - cameraPos.xyz;\r\n";	
				shaderSource = shaderSource + "output.Texcoord" + layerCounter + " = reflect(viewNorm.xyz, worldNorm);\r\n";	
				shaderSource = shaderSource + "}\r\n";	
				break;
			case TEXCALC_ENVIRONMENT_MAP_NORMAL:
				break;
			case TEXCALC_PROJECTIVE_TEXTURE:

				switch(texCordVecType[i])
				{
				case VET_FLOAT1:
					shaderSource = shaderSource + "{\r\n";	
					shaderSource = shaderSource + "	float4 cameraPosNorm = normalize(cameraPos);\r\n";	
					shaderSource = shaderSource + "output.Texcoord" + layerCounter + ".x = 0.5 + cameraPosNorm.x;\r\n";	
					shaderSource = shaderSource + "}\r\n";					break;
				case VET_FLOAT2:
				case VET_FLOAT3:
					shaderSource = shaderSource + "{\r\n";	
					shaderSource = shaderSource + "	float4 cameraPosNorm = normalize(cameraPos);\r\n";	
					shaderSource = shaderSource + "output.Texcoord" + layerCounter + ".x = 0.5 + cameraPosNorm.x;\r\n";	
					shaderSource = shaderSource + "output.Texcoord" + layerCounter + ".y = 0.5 - cameraPosNorm.y;\r\n";	
					shaderSource = shaderSource + "}\r\n";					break;
					break;
				}
				break;
			}
			shaderSource = shaderSource + "}\r\n";

		}

		shaderSource = shaderSource + "output.ColorSpec = float4(0.0, 0.0, 0.0, 0.0);\r\n";


		if (fixedFuncState.getGeneralFixedFuncState().getLightingEnabled() && fixedFuncState.getGeneralFixedFuncState().getTotalNumberOfLights() > 0)
		{
		//	shaderSource = shaderSource + "output.Color = BaseLightAmbient;\r\n";
			if (bHasColor)
			{
				shaderSource = shaderSource + "output.Color.x = ((input.DiffuseColor0 >> 24) & 0xFF) / 255.0f;\r\n";
				shaderSource = shaderSource + "output.Color.y = ((input.DiffuseColor0 >> 16) & 0xFF) / 255.0f;\r\n"; 
				shaderSource = shaderSource + "output.Color.z = ((input.DiffuseColor0 >> 8) & 0xFF) / 255.0f;\r\n";
				shaderSource = shaderSource + "output.Color.w = (input.DiffuseColor0 & 0xFF) / 255.0f;\r\n";
			}


			shaderSource = shaderSource + "float3 N = mul(normalize(Normal), (float3x3)WorldViewIT);\r\n";
			shaderSource = shaderSource + "float3 V = -normalize(cameraPos);\r\n";

			shaderSource = shaderSource + "#define fMaterialPower 16.f\r\n";

			// Point Light
			for(uint8 i = 0 ; i < fixedFuncState.getGeneralFixedFuncState().getLightTypeCount(Light::LT_POINT) ; i++)
			{
				String prefix = "PointLight" + StringConverter::toString(i) + "_";
				shaderSource = shaderSource + "{\r\n";
				shaderSource = shaderSource + "  float3 PosDiff = " + prefix + "Position - worldPos.xyz;\r\n";
				shaderSource = shaderSource + "  float3 L = mul(normalize(PosDiff), (float3x3)ViewIT);\r\n";
				shaderSource = shaderSource + "  float NdotL = dot(N, L);\r\n";
				shaderSource = shaderSource + "  float4 Color = " + prefix + "Ambient;\r\n";
				shaderSource = shaderSource + "  float4 ColorSpec = 0;\r\n";
				shaderSource = shaderSource + "  float fAtten = 1.f;\r\n";
				shaderSource = shaderSource + "  if(NdotL >= 0.f)\r\n";
				shaderSource = shaderSource + "  {\r\n";
				shaderSource = shaderSource + "    //compute diffuse color\r\n";
				shaderSource = shaderSource + "    Color += NdotL * " + prefix + "Diffuse;\r\n";
				shaderSource = shaderSource + "    //add specular component\r\n";
				shaderSource = shaderSource + "    float3 H = normalize(L + V);   //half vector\r\n";
				shaderSource = shaderSource + "    ColorSpec = pow(max(0, dot(H, N)), fMaterialPower) * " + prefix + "Specular;\r\n";
				shaderSource = shaderSource + "    float LD = length(PosDiff);\r\n";
				shaderSource = shaderSource + "    if(LD > " + prefix + "Range)\r\n";
				shaderSource = shaderSource + "    {\r\n";
				shaderSource = shaderSource + "      fAtten = 0.f;\r\n";
				shaderSource = shaderSource + "    }\r\n";
				shaderSource = shaderSource + "    else\r\n";
				shaderSource = shaderSource + "    {\r\n";
				shaderSource = shaderSource + "      fAtten *= 1.f/(" + prefix + "Attenuation.x + " + prefix + "Attenuation.y*LD + " + prefix + "Attenuation.z*LD*LD);\r\n";
				shaderSource = shaderSource + "    }\r\n";
				shaderSource = shaderSource + "    Color *= fAtten;\r\n";
				shaderSource = shaderSource + "    ColorSpec *= fAtten;\r\n";
				shaderSource = shaderSource + "    output.Color += Color;\r\n";
				shaderSource = shaderSource + "    output.ColorSpec += ColorSpec;\r\n";
				shaderSource = shaderSource + "  }\r\n";
				shaderSource = shaderSource + "}\r\n";

			}

			// Directional Light
			for(uint8 i = 0 ; i < fixedFuncState.getGeneralFixedFuncState().getLightTypeCount(Light::LT_DIRECTIONAL) ; i++)
			{
				String prefix = "DirectionalLight" + StringConverter::toString(i) + "_";
				shaderSource = shaderSource + "{\r\n";
				shaderSource = shaderSource + "  float3 L = mul(-normalize(" + prefix + "Direction), (float3x3)ViewIT);\r\n";
				shaderSource = shaderSource + "  float NdotL = dot(N, L);\r\n";
				shaderSource = shaderSource + "  float4 Color = " + prefix + "Ambient;\r\n";
				shaderSource = shaderSource + "  float4 ColorSpec = 0;\r\n";
				shaderSource = shaderSource + "  if(NdotL > 0.f)\r\n";
				shaderSource = shaderSource + "  {\r\n";
				shaderSource = shaderSource + "    //compute diffuse color\r\n";
				shaderSource = shaderSource + "    Color += NdotL * " + prefix + "Diffuse;\r\n";
				shaderSource = shaderSource + "    //add specular component\r\n";
				shaderSource = shaderSource + "    float3 H = normalize(L + V);   //half vector\r\n";
				shaderSource = shaderSource + "    ColorSpec = pow(max(0, dot(H, N)), fMaterialPower) * " + prefix + "Specular;\r\n";
				shaderSource = shaderSource + "    output.Color += Color;\r\n";
				shaderSource = shaderSource + "    output.ColorSpec += ColorSpec;\r\n";
				shaderSource = shaderSource + "  }\r\n";
				shaderSource = shaderSource + "}\r\n";
			}

			// Spot Light
			for(uint8 i = 0 ; i < fixedFuncState.getGeneralFixedFuncState().getLightTypeCount(Light::LT_SPOTLIGHT) ; i++)
			{
				String prefix = "SpotLight" + StringConverter::toString(i) + "_";
				shaderSource = shaderSource + "{\r\n";
				shaderSource = shaderSource + "  float3 PosDiff = " + prefix + "Position-(float3)mul(input.Position0, World);\r\n";
				shaderSource = shaderSource + "   float3 L = mul(normalize(PosDiff), (float3x3)ViewIT);\r\n";
				shaderSource = shaderSource + "   float NdotL = dot(N, L);\r\n";
				shaderSource = shaderSource + "   float4 Color = " + prefix + "Ambient;\r\n";
				shaderSource = shaderSource + "   float4 ColorSpec = 0;\r\n";
				shaderSource = shaderSource + "   float fAttenSpot = 1.f;\r\n";
				shaderSource = shaderSource + "   if(NdotL >= 0.f)\r\n";
				shaderSource = shaderSource + "   {\r\n";
				shaderSource = shaderSource + "      //compute diffuse color\r\n";
				shaderSource = shaderSource + "      output.Color += NdotL * " + prefix + "Diffuse;\r\n";
				shaderSource = shaderSource + "      //add specular component\r\n";
				shaderSource = shaderSource + "       float3 H = normalize(L + V);   //half vector\r\n";
				shaderSource = shaderSource + "       output.ColorSpec = pow(max(0, dot(H, N)), fMaterialPower) * " + prefix + "Specular;\r\n";
				shaderSource = shaderSource + "      float LD = length(PosDiff);\r\n";
				//shaderSource = shaderSource + "      if(LD > lights[i].fRange)\r\n";
				//shaderSource = shaderSource + "      {\r\n";
				//shaderSource = shaderSource + "         fAttenSpot = 0.f;\r\n";
				//shaderSource = shaderSource + "      }\r\n";
				//shaderSource = shaderSource + "      else\r\n";
				shaderSource = shaderSource + "      {\r\n";
				shaderSource = shaderSource + "         fAttenSpot *= 1.f/(" + prefix + "Attenuation.x + " + prefix + "Attenuation.y*LD + " + prefix + "Attenuation.z*LD*LD);\r\n";
				shaderSource = shaderSource + "      }\r\n";
				shaderSource = shaderSource + "      //spot cone computation\r\n";
				shaderSource = shaderSource + "      float3 L2 = mul(-normalize(" + prefix + "Direction), (float3x3)ViewIT);\r\n";
				shaderSource = shaderSource + "      float rho = dot(L, L2);\r\n";
				shaderSource = shaderSource + "      fAttenSpot *= pow(saturate((rho - " + prefix + "Spot.y)/(" + prefix + "Spot.x - " + prefix + "Spot.y)), " + prefix + "Spot.z);\r\n";
				shaderSource = shaderSource + "		Color *= fAttenSpot;\r\n";
				shaderSource = shaderSource + "		ColorSpec *= fAttenSpot;\r\n";
				shaderSource = shaderSource + "    output.Color += Color;\r\n";
				shaderSource = shaderSource + "    output.ColorSpec += ColorSpec;\r\n";
				shaderSource = shaderSource + "   }\r\n";
				shaderSource = shaderSource + "}\r\n";
			}
		}
		else
		{
			if (bHasColor)
			{
				shaderSource = shaderSource + "output.Color.w = ((input.DiffuseColor0 >> 24) & 0xFF) / 255.0f;\r\n";
				shaderSource = shaderSource + "output.Color.x = ((input.DiffuseColor0 >> 16) & 0xFF) / 255.0f;\r\n"; 
				shaderSource = shaderSource + "output.Color.y = ((input.DiffuseColor0 >> 8) & 0xFF) / 255.0f;\r\n";
				shaderSource = shaderSource + "output.Color.z = (input.DiffuseColor0 & 0xFF) / 255.0f;\r\n";
			}
			else
			{
			//	shaderSource = shaderSource + "output.Color = BaseLightAmbient;\r\n";//float4(1.0, 1.0, 1.0, 1.0);\r\n";
				shaderSource = shaderSource + "output.Color = float4(1.0, 1.0, 1.0, 1.0);\r\n";
			}
		}

		switch (fixedFuncState.getGeneralFixedFuncState().getFogMode())
		{
		case FOG_NONE:
			break;
		case FOG_EXP:
		case FOG_EXP2:
		case FOG_LINEAR:
			shaderSource = shaderSource + "output.fogDist = length(cameraPos.xyz);\r\n";
			break;
		}

		switch (fixedFuncState.getGeneralFixedFuncState().getFogMode())
		{
		case FOG_NONE:
			break;
		case FOG_EXP:
			shaderSource = shaderSource + "#define E 2.71828\r\n";
			shaderSource = shaderSource + "output.fogDist = 1.0 / pow( E, output.fogDist*FogDensity );\r\n";
			shaderSource = shaderSource + "output.fogDist = clamp( output.fogDist, 0, 1 );\r\n";
			break;
		case FOG_EXP2:
			shaderSource = shaderSource + "#define E 2.71828\r\n";
			shaderSource = shaderSource + "output.fogDist = 1.0 / pow( E, output.fogDist*output.fogDist*FogDensity*FogDensity );\r\n";
			shaderSource = shaderSource + "output.fogDist = clamp( output.fogDist, 0, 1 );\r\n";
			break;
		case FOG_LINEAR:
			shaderSource = shaderSource + "output.fogDist = (FogEnd - output.fogDist)/(FogEnd - FogStart);\r\n";
			shaderSource = shaderSource + "output.fogDist = clamp( output.fogDist, 0, 1 );\r\n";
			break;
		}

		shaderSource = shaderSource + "return output;}\r\n";

		/////////////////////////////////////
		// here starts the fragment shader
		/////////////////////////////////////

		for(size_t i = 0 ; i < fixedFuncState.getTextureLayerStateList().size() ; i++)
		{
			String layerCounter = StringConverter::toString(i);
			const TextureLayerState & curTextureLayerState = fixedFuncState.getTextureLayerStateList()[i];
//			shaderSource = shaderSource + "sampler Texture" + layerCounter + " : register(s" + layerCounter + ");\r\n";

			switch(curTextureLayerState.getTextureType())
			{
			case TEX_TYPE_1D:
				shaderSource = shaderSource + "sampler1D Texture" + layerCounter + " : register(s" + layerCounter + ");\r\n";
				break;
			case TEX_TYPE_2D:
				shaderSource = shaderSource + "sampler2D Texture" + layerCounter + " : register(s" + layerCounter + ");\r\n";
				break;
			case TEX_TYPE_CUBE_MAP:
				shaderSource = shaderSource + "samplerCUBE Texture" + layerCounter + " : register(s" + layerCounter + ");\r\n";
				break;
			case TEX_TYPE_3D:
				shaderSource = shaderSource + "sampler3D Texture" + layerCounter + " : register(s" + layerCounter + ");\r\n";
				break;

			}
			
		}
		
		shaderSource = shaderSource + "float4  FogColor;\r\n";


		//define color blending
		for(size_t i = 0 ; i < fixedFuncState.getTextureLayerStateList().size() ; i++)
		{
			const TextureLayerState & curTextureLayerState = fixedFuncState.getTextureLayerStateList()[i];
			LayerBlendModeEx blend = curTextureLayerState.getLayerBlendModeEx();
			String layerCounter = StringConverter::toString(i);
			if(blend.source1==LBS_MANUAL)
			{
				//colourArg1
				shaderSource = shaderSource + "float4 Texture" + layerCounter + "_colourArg1;\r\n";
			}
			if(blend.source2==LBS_MANUAL)
			{
				//colourArg2
				shaderSource = shaderSource + "float4 Texture" + layerCounter + "_colourArg2;\r\n";
			}

		}
		//end
		shaderSource = shaderSource + "float4 " + fragmentProgramName + "( VS_OUTPUT input ) : SV_Target\r\n";
		shaderSource = shaderSource + "{\r\n";

		shaderSource = shaderSource + "float4 finalColor = input.Color + input.ColorSpec;\r\n";


		for(size_t i = 0 ; i < fixedFuncState.getTextureLayerStateList().size() ; i++)
		{
			const TextureLayerState & curTextureLayerState = fixedFuncState.getTextureLayerStateList()[i];
			String layerCounter = StringConverter::toString(i);
			shaderSource = shaderSource + "{\r\n";
			switch(curTextureLayerState.getTextureType())
			{
			case TEX_TYPE_1D:
				{
					switch(texCordVecType[i])
					{
					case VET_FLOAT1:
						shaderSource = shaderSource + "float4 texColor = tex1D(Texture" + layerCounter + ", input.Texcoord" + layerCounter + ");\r\n";
						break;
					case VET_FLOAT2:
						shaderSource = shaderSource + "float4 texColor = tex1D(Texture" + layerCounter + ", input.Texcoord" + layerCounter + ".x);\r\n";
						break;
					}
				}
				
				break;
			case TEX_TYPE_2D:
				{
					switch(texCordVecType[i])
					{
					case VET_FLOAT1:
						shaderSource = shaderSource + "float4 texColor = tex2D(Texture" + layerCounter + ", float2(input.Texcoord" + layerCounter + ", 0.0));\r\n";
						break;
					case VET_FLOAT2:
						shaderSource = shaderSource + "float4 texColor = tex2D(Texture" + layerCounter + ", input.Texcoord" + layerCounter + ");\r\n";
						break;
					}
				}

				break;
			case TEX_TYPE_CUBE_MAP:
/*				switch(texCordVecType[i])
				{
				case VET_FLOAT1:
					shaderSource = shaderSource + "float4 texColor = texCUBE(Texture" + layerCounter + ", float3(input.Texcoord" + layerCounter + ", 0.0, 0.0));\r\n";
					break;
				case VET_FLOAT2:
					shaderSource = shaderSource + "float4 texColor = texCUBE(Texture" + layerCounter + ", float3(input.Texcoord" + layerCounter + ".x, input.Texcoord" + layerCounter + ".y, 0.0));\r\n";
					break;
				case VET_FLOAT3:
					shaderSource = shaderSource + "float4 texColor = texCUBE(Texture" + layerCounter + ", input.Texcoord" + layerCounter + ");\r\n";
				}
				*/
				shaderSource = shaderSource + "float4 texColor = texCUBE(Texture" + layerCounter + ", input.Texcoord" + layerCounter + ");\r\n";

				break;
			case TEX_TYPE_3D:
				switch(texCordVecType[i])
				{
				case VET_FLOAT1:
					shaderSource = shaderSource + "float4 texColor = tex3D(Texture" + layerCounter + ", float3(input.Texcoord" + layerCounter + ", 0.0, 0.0));\r\n";
					break;
				case VET_FLOAT2:
					shaderSource = shaderSource + "float4 texColor = tex3D(Texture" + layerCounter + ", float3(input.Texcoord" + layerCounter + ".x, input.Texcoord" + layerCounter + ".y, 0.0));\r\n";
					break;
				case VET_FLOAT3:
					shaderSource = shaderSource + "float4 texColor = tex3D(Texture" + layerCounter + ", input.Texcoord" + layerCounter + ");\r\n";
				}
				break;
			}
			LayerBlendModeEx blend = curTextureLayerState.getLayerBlendModeEx();
			switch(blend.source1)
			{
			case LBS_CURRENT:
				shaderSource = shaderSource + "float4 source1 = finalColor;\r\n";
				break;
			case LBS_TEXTURE:
				shaderSource = shaderSource + "float4 source1 = texColor;\r\n";
				break;
			case LBS_DIFFUSE:
				shaderSource = shaderSource + "float4 source1 = input.Color;\r\n";
				break;
			case LBS_SPECULAR:
				shaderSource = shaderSource + "float4 source1 = input.ColorSpec;\r\n";
				break;

			case LBS_MANUAL:
				shaderSource = shaderSource + "Texture" + layerCounter + "_colourArg1=float4("
					+StringConverter::toString(blend.colourArg1.r)+","
					+StringConverter::toString(blend.colourArg1.g)+","
					+StringConverter::toString(blend.colourArg1.b)+","
					+StringConverter::toString(blend.colourArg1.a)+");\r\n";
				shaderSource = shaderSource + "float4 source1 = Texture" + layerCounter + "_colourArg1;\r\n";
				break;
			}
			switch(blend.source2)
			{
			case LBS_CURRENT:
				shaderSource = shaderSource + "float4 source2 = finalColor;\r\n";
				break;
			case LBS_TEXTURE:
				shaderSource = shaderSource + "float4 source2 = texColor;\r\n";
				break;
			case LBS_DIFFUSE:
				shaderSource = shaderSource + "float4 source2 = input.Color;\r\n";
				break;
			case LBS_SPECULAR:
				shaderSource = shaderSource + "float4 source2 = input.ColorSpec;\r\n";
				break;
			case LBS_MANUAL:
				shaderSource = shaderSource + "Texture" + layerCounter + "_colourArg2=float4("
					+StringConverter::toString(blend.colourArg2.r)+","
					+StringConverter::toString(blend.colourArg2.g)+","
					+StringConverter::toString(blend.colourArg2.b)+","
					+StringConverter::toString(blend.colourArg2.a)+");\r\n";
				shaderSource = shaderSource + "float4 source2 = Texture" + layerCounter + "_colourArg2;\r\n";
				break;
			}


			switch(blend.operation)
			{
			case LBX_SOURCE1:
				shaderSource = shaderSource + "finalColor = source1;\r\n";
				break;
			case LBX_SOURCE2:
				shaderSource = shaderSource + "finalColor = source2;\r\n";
				break;
			case LBX_MODULATE:
				shaderSource = shaderSource + "finalColor = source1 * source2;\r\n";
				break;
			case LBX_MODULATE_X2:
				shaderSource = shaderSource + "finalColor = source1 * source2 * 2.0;\r\n";
				break;
			case LBX_MODULATE_X4:
				shaderSource = shaderSource + "finalColor = source1 * source2 * 4.0;\r\n";
				break;
			case LBX_ADD:
				shaderSource = shaderSource + "finalColor = source1 + source2;\r\n";
				break;
			case LBX_ADD_SIGNED:
				shaderSource = shaderSource + "finalColor = source1 + source2 - 0.5;\r\n";
				break;
			case LBX_ADD_SMOOTH:
				shaderSource = shaderSource + "finalColor = source1 + source2 - (source1 * source2);\r\n";
				break;
			case LBX_SUBTRACT:
				shaderSource = shaderSource + "finalColor = source1 - source2;\r\n";
				break;
			case LBX_BLEND_DIFFUSE_ALPHA:
				shaderSource = shaderSource + "finalColor = source1 * input.Color.w + source2 * (1.0 - input.Color.w);\r\n";
				break;
			case LBX_BLEND_TEXTURE_ALPHA:
				shaderSource = shaderSource + "finalColor = source1 * texColor.w + source2 * (1.0 - texColor.w);\r\n";
				break;
			case LBX_BLEND_CURRENT_ALPHA:
				shaderSource = shaderSource + "finalColor = source1 * finalColor.w + source2 * (1.0 - finalColor.w);\r\n";
				break;
			case LBX_BLEND_MANUAL:
				shaderSource = shaderSource + "finalColor = source1 * " + 
					Ogre::StringConverter::toString(blend.factor) + 
					" + source2 * (1.0 - " + 
					Ogre::StringConverter::toString(blend.factor) + ");\r\n";
				break;
			case LBX_DOTPRODUCT:
				shaderSource = shaderSource + "finalColor = product(source1,source2);\r\n";
				break;
			case LBX_BLEND_DIFFUSE_COLOUR:
				shaderSource = shaderSource + "finalColor = source1 * input.Color + source2 * (float4(1.0,1.0,1.0,1.0) - input.Color);\r\n";
				break;
			}
			shaderSource = shaderSource + "}\r\n";
		}

		if (fixedFuncState.getGeneralFixedFuncState().getFogMode() != FOG_NONE)
		{

			shaderSource = shaderSource + "finalColor.xyz = input.fogDist * finalColor.xyz + (1.0 - input.fogDist)*FogColor.xyz;\r\n";

		}

//		shaderSource = shaderSource + "finalColor.a=1.0;\r\n";
		shaderSource = shaderSource + "return finalColor;\r\n";
		shaderSource = shaderSource + "}\r\n";
		return shaderSource;
	}
	//---------------------------------------------------------------------
	FixedFuncPrograms * HlslFixedFuncEmuShaderGenerator::createFixedFuncPrograms()
	{
		return new HlslFixedFuncPrograms();
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	Hlsl4FixedFuncEmuShaderGenerator::Hlsl4FixedFuncEmuShaderGenerator()
	{
		mLanguageName	= "hlsl";
		mName			= "hlsl4";
		mVpTarget		= "vs_4_0";
		mFpTarget		= "ps_4_0";
	}
	//---------------------------------------------------------------------
	Hlsl4FixedFuncEmuShaderGenerator::~Hlsl4FixedFuncEmuShaderGenerator()
	{

	}
	//---------------------------------------------------------------------
	const String Hlsl4FixedFuncEmuShaderGenerator::getShaderSource( const String & vertexProgramName, const String & fragmentProgramName, const VertexBufferDeclaration & vertexBufferDeclaration, FixedFuncState & fixedFuncState )
	{
		return HlslFixedFuncEmuShaderGenerator::getShaderSource(vertexProgramName, fragmentProgramName, vertexBufferDeclaration, fixedFuncState);
	}
	//---------------------------------------------------------------------
	void HlslFixedFuncEmuShaderGenerator::HlslFixedFuncPrograms::setFixedFuncProgramsParameters( const FixedFuncProgramsParameters & params )
	{
		_setProgramMatrix4Parameter(GPT_VERTEX_PROGRAM, "World", params.getWorldMat());
		_setProgramMatrix4Parameter(GPT_VERTEX_PROGRAM, "View", params.getViewMat());
		_setProgramMatrix4Parameter(GPT_VERTEX_PROGRAM, "Projection", params.getProjectionMat());

		_setProgramMatrix4Parameter(GPT_VERTEX_PROGRAM, "ViewIT", params.getViewMat().inverse().transpose());


		Matrix4 WorldViewIT = params.getViewMat() * params.getWorldMat();
		WorldViewIT = WorldViewIT.inverse().transpose();
		_setProgramMatrix4Parameter(GPT_VERTEX_PROGRAM, "WorldViewIT", WorldViewIT);


		_setProgramColorParameter(GPT_VERTEX_PROGRAM, "BaseLightAmbient", params.getLightAmbient());
		if (params.getLightingEnabled() && params.getLights().size() > 0)
		{
		
			uint pointLightCount = 0;
			uint directionalLightCount = 0;
			uint spotLightCount = 0;
			for(size_t i = 0 ; i < params.getLights().size() ; i++)
			{
				Light * curLight = params.getLights()[i];
				String prefix;

				switch (curLight->getType())
				{
				case Light::LT_POINT:
					prefix = "PointLight" + StringConverter::toString(pointLightCount) + "_";
					pointLightCount++;
					break;
				case Light::LT_DIRECTIONAL:
					prefix = "DirectionalLight" + StringConverter::toString(directionalLightCount) + "_";
					directionalLightCount++;
					break;
				case Light::LT_SPOTLIGHT:
					prefix = "SpotLight" + StringConverter::toString(spotLightCount) + "_";
					spotLightCount++;
					break;
				} 

				_setProgramColorParameter(GPT_VERTEX_PROGRAM, prefix + "Ambient", ColourValue::Black);
				_setProgramColorParameter(GPT_VERTEX_PROGRAM, prefix + "Diffuse", curLight->getDiffuseColour());
				_setProgramColorParameter(GPT_VERTEX_PROGRAM, prefix + "Specular", curLight->getSpecularColour());		

				switch (curLight->getType())
				{
				case Light::LT_POINT:
					{
						_setProgramVector3Parameter(GPT_VERTEX_PROGRAM, prefix + "Position", curLight->getPosition());
						_setProgramFloatParameter(GPT_VERTEX_PROGRAM, prefix + "Range", curLight->getAttenuationRange());

						Vector3 attenuation;
						attenuation[0] = curLight->getAttenuationConstant();
						attenuation[1] = curLight->getAttenuationLinear();
						attenuation[2] = curLight->getAttenuationQuadric();
						_setProgramVector3Parameter(GPT_VERTEX_PROGRAM, prefix + "Attenuation", attenuation);
					}
					break;
				case Light::LT_DIRECTIONAL:
					_setProgramVector3Parameter(GPT_VERTEX_PROGRAM, prefix + "Direction", curLight->getDirection());

					break;
				case Light::LT_SPOTLIGHT:
					{

						_setProgramVector3Parameter(GPT_VERTEX_PROGRAM, prefix + "Direction", curLight->getDirection());
						_setProgramVector3Parameter(GPT_VERTEX_PROGRAM, prefix + "Position", curLight->getPosition());

						Vector3 attenuation;
						attenuation[0] = curLight->getAttenuationConstant();
						attenuation[1] = curLight->getAttenuationLinear();
						attenuation[2] = curLight->getAttenuationQuadric();
						_setProgramVector3Parameter(GPT_VERTEX_PROGRAM, prefix + "Attenuation", attenuation);

						Vector3 spot;
						spot[0] = curLight->getSpotlightInnerAngle().valueRadians() ;
						spot[1] = curLight->getSpotlightOuterAngle().valueRadians();
						spot[2] = curLight->getSpotlightFalloff();
						_setProgramVector3Parameter(GPT_VERTEX_PROGRAM, prefix + "Spot", spot);					
					}
					break;
				} // end of - switch (curLight->getType())
			} // end of - for(size_t i = 0 ; i < params.getLights().size() ; i++) 
		} // end of -  if (params.getLightingEnabled())





		switch (params.getFogMode())
		{
		case FOG_NONE:
			break;
		case FOG_EXP:
		case FOG_EXP2:
			_setProgramFloatParameter(GPT_VERTEX_PROGRAM, "FogDensity", params.getFogDensitiy());
			break;
		case FOG_LINEAR:
			_setProgramFloatParameter(GPT_VERTEX_PROGRAM, "FogStart", params.getFogStart());
			_setProgramFloatParameter(GPT_VERTEX_PROGRAM, "FogEnd", params.getFogEnd());
			break;
		}

		if (params.getFogMode() != FOG_NONE)
		{
			_setProgramColorParameter(GPT_FRAGMENT_PROGRAM, "FogColor", params.getFogColour());
		}


		for(size_t i = 0 ; i < params.getTextureMatrices().size() && i < mFixedFuncState.getTextureLayerStateList().size(); i++)
		{
			if (params.isTextureStageEnabled(i))
			{
				if (params.isTextureStageEnabled(i))
				{
					_setProgramMatrix4Parameter(GPT_VERTEX_PROGRAM, "TextureMatrix" + StringConverter::toString(i), params.getTextureMatrices()[i]);
				}
			}
		}



	}
}
