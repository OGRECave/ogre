/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef _ShaderFFPState_
#define _ShaderFFPState_

#include "OgreShaderPrerequisites.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

// Fixed Function vertex shader stages.
enum FFPVertexShaderStage
{
    FFP_VS_PRE_PROCESS                  = 0,    
    FFP_VS_TRANSFORM                    = 100,
    FFP_VS_COLOUR                       = 200,
    FFP_VS_LIGHTING                     = 300,
    FFP_VS_TEXTURING                    = 400,      
    FFP_VS_FOG                          = 500,  
    FFP_VS_POST_PROCESS                 = 2000
};

// Fixed Function fragment shader stages.
enum FFPFragmentShaderStage
{
    FFP_PS_PRE_PROCESS                  = 0,    
    FFP_PS_COLOUR_BEGIN                 = 100,
    FFP_PS_SAMPLING                     = 150,
    FFP_PS_TEXTURING                    = 200,  
    FFP_PS_COLOUR_END                   = 300,
    FFP_PS_FOG                          = 400,
    FFP_PS_POST_PROCESS                 = 500,
	FFP_PS_ALPHA_TEST					= 1000
};

// Fixed Function generic stages.
enum FFPShaderStage
{
    FFP_PRE_PROCESS                     = 0,    
    FFP_TRANSFORM                       = 100,  
    FFP_COLOUR                          = 200,  
    FFP_LIGHTING                        = 300,
    FFP_TEXTURING                       = 400,
    FFP_FOG                             = 500,
    FFP_POST_PROCESS                    = 600,
	FFP_ALPHA_TEST						= 1000
};

// Fixed Function Library: Common functions
#define FFP_LIB_COMMON                              "FFPLib_Common"
#define FFP_FUNC_MODULATE                           "FFP_Modulate"
#define FFP_FUNC_ADD                                "FFP_Add"
#define FFP_FUNC_SUBTRACT                           "FFP_Subtract"
#define FFP_FUNC_LERP                               "FFP_Lerp"
#define FFP_FUNC_DOTPRODUCT                         "FFP_DotProduct"
#define FFP_FUNC_NORMALIZE                          "FFP_Normalize"

// Fixed Function Library: Transform functions
#define FFP_LIB_TRANSFORM                           "FFPLib_Transform"
#define FFP_FUNC_TRANSFORM                          "FFP_Transform"

// Fixed Function Library: Lighting functions
#define FFP_LIB_LIGHTING                            "FFPLib_Lighting"
#define FFP_FUNC_LIGHT_DIRECTIONAL_DIFFUSE          "FFP_Light_Directional_Diffuse"
#define FFP_FUNC_LIGHT_DIRECTIONAL_DIFFUSESPECULAR  "FFP_Light_Directional_DiffuseSpecular"
#define FFP_FUNC_LIGHT_POINT_DIFFUSE                "FFP_Light_Point_Diffuse"
#define FFP_FUNC_LIGHT_POINT_DIFFUSESPECULAR        "FFP_Light_Point_DiffuseSpecular"
#define FFP_FUNC_LIGHT_SPOT_DIFFUSE                 "FFP_Light_Spot_Diffuse"
#define FFP_FUNC_LIGHT_SPOT_DIFFUSESPECULAR         "FFP_Light_Spot_DiffuseSpecular"

// Fixed Function Library: Texturing functions
#define FFP_LIB_TEXTURING                           "FFPLib_Texturing"
#define FFP_FUNC_TRANSFORM_TEXCOORD                 "FFP_TransformTexCoord"
#define FFP_FUNC_GENERATE_TEXCOORD_ENV_NORMAL       "FFP_GenerateTexCoord_EnvMap_Normal"
#define FFP_FUNC_GENERATE_TEXCOORD_ENV_SPHERE       "FFP_GenerateTexCoord_EnvMap_Sphere"
#define FFP_FUNC_GENERATE_TEXCOORD_ENV_REFLECT      "FFP_GenerateTexCoord_EnvMap_Reflect"
#define FFP_FUNC_GENERATE_TEXCOORD_PROJECTION       "FFP_GenerateTexCoord_Projection"
#define FFP_FUNC_SAMPLE_TEXTURE                     "FFP_SampleTexture"

#define FFP_FUNC_SAMPLE_TEXTURE_PROJ                "FFP_SampleTextureProj"
#define FFP_FUNC_MODULATEX2                         "FFP_ModulateX2"
#define FFP_FUNC_MODULATEX4                         "FFP_ModulateX4"
#define FFP_FUNC_ADDSIGNED                          "FFP_AddSigned"
#define FFP_FUNC_ADDSMOOTH                          "FFP_AddSmooth"

// Fixed Function Library: Fog functions
#define FFP_LIB_FOG                                 "FFPLib_Fog"
#define FFP_FUNC_VERTEXFOG_LINEAR                   "FFP_VertexFog_Linear"
#define FFP_FUNC_VERTEXFOG_EXP                      "FFP_VertexFog_Exp"
#define FFP_FUNC_VERTEXFOG_EXP2                     "FFP_VertexFog_Exp2"
#define FFP_FUNC_PIXELFOG_DEPTH                     "FFP_PixelFog_Depth"
#define FFP_FUNC_PIXELFOG_LINEAR                    "FFP_PixelFog_Linear"
#define FFP_FUNC_PIXELFOG_EXP                       "FFP_PixelFog_Exp"
#define FFP_FUNC_PIXELFOG_EXP2                      "FFP_PixelFog_Exp2"

// Fixed Function Library: Alpha Test
#define FFP_LIB_ALPHA_TEST							"FFPLib_AlphaTest"
#define FFP_FUNC_ALPHA_TEST							"FFP_Alpha_Test"

/** @} */
/** @} */

}
}

#endif

