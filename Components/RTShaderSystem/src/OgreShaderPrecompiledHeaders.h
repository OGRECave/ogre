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

#ifndef __OgreShaderPrecompiledHeaders__
#define __OgreShaderPrecompiledHeaders__

#include <algorithm> // for std::sort

#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgreSceneManager.h"
#include "OgreViewport.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreSceneManager.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreTextureManager.h"

#include "OgreShaderFunction.h"
#include "OgreShaderFunctionAtom.h"
#include "OgreShaderGenerator.h"
#include "OgreShaderProgram.h"
#include "OgreShaderProgramSet.h"
#include "OgreShaderProgramManager.h"
#include "OgreShaderProgramProcessor.h"
#include "OgreShaderRenderState.h"
#include "OgreShaderMaterialSerializerListener.h"
#include "OgreShaderProgramWriterManager.h"

#include "OgreShaderFFPRenderState.h"
#include "OgreShaderFFPRenderStateBuilder.h"
#include "OgreShaderFFPTransform.h"
#include "OgreShaderFFPLighting.h"
#include "OgreShaderFFPColour.h"
#include "OgreShaderFFPTexturing.h"
#include "OgreShaderFFPFog.h"
#include "OgreShaderFFPAlphaTest.h"

#include "OgreShaderExPerPixelLighting.h"
#include "OgreShaderExNormalMapLighting.h"
#include "OgreShaderExIntegratedPSSM3.h"
#include "OgreShaderExLayeredBlending.h"
#include "OgreShaderExHardwareSkinningTechnique.h"
#include "OgreShaderExHardwareSkinning.h"
#include "OgreShaderExLinearSkinning.h"
#include "OgreShaderExDualQuaternionSkinning.h"
#include "OgreShaderExTriplanarTexturing.h"
#include "OgreShaderExGBuffer.h"
#include "OgreShaderExWBOIT.h"
#include "OgreShaderCookTorranceLighting.h"

#include "OgreShaderHLSLProgramProcessor.h"
#include "OgreShaderGLSLProgramProcessor.h"

#include "OgreShaderProgramWriter.h"
#include "OgreShaderProgramWriterManager.h"
#include "OgreShaderCGProgramWriter.h"
#include "OgreShaderHLSLProgramWriter.h"
#include "OgreShaderGLSLProgramWriter.h"
#include "OgreShaderGLSLESProgramWriter.h"

// Fixed Function Library: Common functions
#define FFP_FUNC_LERP                               "FFP_Lerp"
#define FFP_FUNC_DOTPRODUCT                         "FFP_DotProduct"
#define FFP_FUNC_NORMALIZE                          "FFP_Normalize"

// Fixed Function Library: Transform functions
#define FFP_LIB_TRANSFORM                           "FFPLib_Transform"
#define FFP_FUNC_TRANSFORM                          "FFP_Transform"

// Fixed Function Library: Texturing functions
#define FFP_FUNC_TRANSFORM_TEXCOORD                 "FFP_TransformTexCoord"
#define FFP_FUNC_GENERATE_TEXCOORD_ENV_NORMAL       "FFP_GenerateTexCoord_EnvMap_Normal"
#define FFP_FUNC_GENERATE_TEXCOORD_ENV_SPHERE       "FFP_GenerateTexCoord_EnvMap_Sphere"
#define FFP_FUNC_GENERATE_TEXCOORD_ENV_REFLECT      "FFP_GenerateTexCoord_EnvMap_Reflect"
#define FFP_FUNC_GENERATE_TEXCOORD_PROJECTION       "FFP_GenerateTexCoord_Projection"

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

#define SGX_LIB_PERPIXELLIGHTING                    "SGXLib_PerPixelLighting"
#define SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSE          "SGX_Light_Directional_Diffuse"
#define SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSESPECULAR  "SGX_Light_Directional_DiffuseSpecular"
#define SGX_FUNC_LIGHT_POINT_DIFFUSE                "SGX_Light_Point_Diffuse"
#define SGX_FUNC_LIGHT_POINT_DIFFUSESPECULAR        "SGX_Light_Point_DiffuseSpecular"
#define SGX_FUNC_LIGHT_SPOT_DIFFUSE                 "SGX_Light_Spot_Diffuse"
#define SGX_FUNC_LIGHT_SPOT_DIFFUSESPECULAR         "SGX_Light_Spot_DiffuseSpecular"

#endif 
