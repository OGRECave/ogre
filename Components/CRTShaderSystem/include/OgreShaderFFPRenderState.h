/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
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
#ifndef _ShaderFFPState_
#define _ShaderFFPState_

#include "OgrePrerequisites.h"

namespace Ogre {
namespace CRTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
enum FFPVertexShaderStage
{
	FFP_VS_PRE_PROCESS					= 0,	
	FFP_VS_TRANSFORM					= 100,
	FFP_VS_COLOUR						= 200,
	FFP_VS_LIGHTING						= 300,
	FFP_VS_TEXTURE_COORD				= 400,		
	FFP_VS_FOG							= 500,	
	FFP_VS_POST_PROCESS					= 2000,
};

enum FFPFragmentShaderStage
{
	FFP_PS_PRE_PROCESS					= 0,	
	FFP_PS_COLOUR_BEGIN					= 100,
	FFP_PS_TEXTURE_STAGE0				= 200,
	FFP_PS_TEXTURE_STAGE1				= 300,
	FFP_PS_TEXTURE_STAGE2				= 400,
	FFP_PS_TEXTURE_STAGE3				= 500,
	FFP_PS_TEXTURE_STAGE4				= 600,
	FFP_PS_TEXTURE_STAGE5				= 700,
	FFP_PS_TEXTURE_STAGE6				= 800,
	FFP_PS_TEXTURE_STAGE7				= 900,
	FFP_PS_COLOUR_END					= 1000,
	FFP_PS_FOG							= 1100,
	FFP_PS_POST_PROCESS					= 2000,
};

enum FFPShaderStage
{
	FFP_PRE_PROCESS						= 0,	
	FFP_TRANSFORM						= 100,	
	FFP_COLOUR							= 200,	
	FFP_LIGHTING						= 300,
	FFP_TEXTURE_STAGE0					= 400,
	FFP_TEXTURE_STAGE1					= 500,
	FFP_TEXTURE_STAGE2					= 600,
	FFP_TEXTURE_STAGE3					= 700,
	FFP_TEXTURE_STAGE4					= 800,
	FFP_TEXTURE_STAGE5					= 900,
	FFP_TEXTURE_STAGE6					= 1000,
	FFP_TEXTURE_STAGE7					= 1100,
	FFP_FOG								= 1200,
	FFP_POST_PROCESS					= 2000,
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
#define FFP_LIB_COMMON								"FFPLib_Common"
#define FFP_FUNC_ASSIGN								"FFP_Assign"
#define FFP_FUNC_CONSTRUCT							"FFP_Construct"
#define FFP_FUNC_MODULATE							"FFP_Modulate"
#define FFP_FUNC_ADD								"FFP_Add"
#define FFP_FUNC_SUBTRACT							"FFP_Subtract"
#define FFP_FUNC_LERP								"FFP_Lerp"
#define FFP_FUNC_DOTPRODUCT							"FFP_DotProduct"

#define FFP_LIB_TRANSFORM							"FFPLib_Transform"
#define FFP_FUNC_TRANSFORM							"FFP_Transform"

#define FFP_LIB_LIGHTING							"FFPLib_Lighting"
#define FFP_FUNC_LIGHT_DIRECTIONAL_DIFFUSE			"FFP_Light_Directional_Diffuse"
#define FFP_FUNC_LIGHT_DIRECTIONAL_DIFFUSESPECULAR	"FFP_Light_Directional_DiffuseSpecular"
#define FFP_FUNC_LIGHT_POINT_DIFFUSE				"FFP_Light_Point_Diffuse"
#define FFP_FUNC_LIGHT_POINT_DIFFUSESPECULAR		"FFP_Light_Point_DiffuseSpecular"
#define FFP_FUNC_LIGHT_SPOT_DIFFUSE					"FFP_Light_Spot_Diffuse"
#define FFP_FUNC_LIGHT_SPOT_DIFFUSESPECULAR			"FFP_Light_Spot_DiffuseSpecular"

#define FFP_LIB_TEXTURESTAGE						"FFPLib_TextureStage"
#define FFP_FUNC_TRANSFORM_TEXCOORD					"FFP_TransformTexCoord"
#define FFP_FUNC_GENERATE_TEXCOORD_ENV_NORMAL		"FFP_GenerateTexCoord_EnvMap_Normal"
#define FFP_FUNC_GENERATE_TEXCOORD_ENV_SPHERE		"FFP_GenerateTexCoord_EnvMap_Sphere"
#define FFP_FUNC_GENERATE_TEXCOORD_ENV_REFLECT		"FFP_GenerateTexCoord_EnvMap_Reflect"
#define FFP_FUNC_GENERATE_TEXCOORD_PROJECTION		"FFP_GenerateTexCoord_Projection"
#define FFP_FUNC_SAMPLE_TEXTURE						"FFP_SampleTexture"
#define FFP_FUNC_SAMPLE_TEXTURE_PROJ				"FFP_SampleTextureProj"
#define FFP_FUNC_MODULATEX2							"FFP_ModulateX2"
#define FFP_FUNC_MODULATEX4							"FFP_ModulateX4"
#define FFP_FUNC_ADDSIGNED							"FFP_AddSigned"
#define FFP_FUNC_ADDMOOTH							"FFP_AddSmooth"




#define FFP_LIB_FOG									"FFPLib_Fog"
#define FFP_FUNC_VERTEXFOG_LINEAR					"FFP_VertexFog_Linear"
#define FFP_FUNC_VERTEXFOG_EXP						"FFP_VertexFog_Exp"
#define FFP_FUNC_VERTEXFOG_EXP2						"FFP_VertexFog_Exp2"
#define FFP_FUNC_PIXELFOG_DEPTH						"FFP_PixelFog_Depth"
#define FFP_FUNC_PIXELFOG_LINEAR					"FFP_PixelFog_Linear"
#define FFP_FUNC_PIXELFOG_EXP						"FFP_PixelFog_Exp"
#define FFP_FUNC_PIXELFOG_EXP2						"FFP_PixelFog_Exp2"




/************************************************************************/
/*                                                                      */
/************************************************************************/
class FFPRenderState
{

// Interface.
public:
	FFPRenderState	();
	~FFPRenderState	();

	
protected:


};

	
}
}

#endif

