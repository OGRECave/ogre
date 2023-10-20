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
#include "OgreShaderImageBasedLighting.h"

#include "OgreShaderProgramWriter.h"
#include "OgreShaderProgramWriterManager.h"
#include "OgreShaderCGProgramWriter.h"
#include "OgreShaderGLSLProgramWriter.h"
#include "OgreShaderGLSLESProgramWriter.h"

// Fixed Function Library: Transform functions
#define FFP_LIB_TRANSFORM                           "FFPLib_Transform"
#define FFP_FUNC_TRANSFORM                          "FFP_Transform"

// Fixed Function Library: Texturing functions
#define FFP_LIB_TEXTURING                           "FFPLib_Texturing"
#define FFP_FUNC_TRANSFORM_TEXCOORD                 "FFP_TransformTexCoord"
#define FFP_FUNC_GENERATE_TEXCOORD_ENV_NORMAL       "FFP_GenerateTexCoord_EnvMap_Normal"
#define FFP_FUNC_GENERATE_TEXCOORD_ENV_SPHERE       "FFP_GenerateTexCoord_EnvMap_Sphere"
#define FFP_FUNC_GENERATE_TEXCOORD_ENV_REFLECT      "FFP_GenerateTexCoord_EnvMap_Reflect"

#define FFP_FUNC_ADDSMOOTH                          "FFP_AddSmooth"
#define FFP_FUNC_DOTPRODUCT                         "FFP_DotProduct"

// Fixed Function Library: Fog functions
#define FFP_LIB_FOG                                 "FFPLib_Fog"

// Fixed Function Library: Alpha Test
#define FFP_LIB_ALPHA_TEST							"FFPLib_AlphaTest"
#define FFP_FUNC_ALPHA_TEST							"FFP_Alpha_Test"

#define SGX_LIB_PERPIXELLIGHTING                    "SGXLib_PerPixelLighting"

namespace Ogre {
namespace RTShader {
class LayeredBlendingFactory : public SubRenderStateFactory
{
public:
    const String& getType() const override;
    SubRenderState* createInstance(ScriptCompiler* compiler, PropertyAbstractNode* prop, TextureUnitState* texState,
                                   SGScriptTranslator* translator) override;
    void writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, const TextureUnitState* srcTextureUnit,
                       const TextureUnitState* dstTextureUnit) override;

private:
    SubRenderState* createInstanceImpl() override;
    LayeredBlending* createOrRetrieveSubRenderState(SGScriptTranslator* translator);
};

class FFPTexturingFactory : public SubRenderStateFactory
{
public:
    const String& getType() const override;
    SubRenderState* createInstance(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass,
                                   SGScriptTranslator* translator) override;
    void writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass, Pass* dstPass) override;
protected:
    SubRenderState* createInstanceImpl() override;
};

uint16 ensureLtcLUTPresent(Pass* dstPass);

} // namespace RTShader
} // namespace Ogre

#endif 
