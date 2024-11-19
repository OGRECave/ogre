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
#include "OgreShaderPrecompiledHeaders.h"
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS

namespace Ogre {
namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String FFPTransform::Type = "FFP_Transform";
const String SRS_TRANSFORM = "FFP_Transform";

//-----------------------------------------------------------------------
const String& FFPTransform::getType() const
{
    return SRS_TRANSFORM;
}


//-----------------------------------------------------------------------
int FFPTransform::getExecutionOrder() const
{
    return FFP_TRANSFORM;
}

bool FFPTransform::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    mSetPointSize = srcPass->getPointSize() != 1.0f || srcPass->isPointAttenuationEnabled();
    mDoLightCalculations = srcPass->getLightingEnabled();
    return true;
}

//-----------------------------------------------------------------------
bool FFPTransform::createCpuSubPrograms(ProgramSet* programSet)
{
    //! [param_resolve]
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Function* vsEntry = vsProgram->getEntryPointFunction();
    
    // Resolve World View Projection Matrix.
    UniformParameterPtr wvpMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);

    // Resolve input position parameter.
    ParameterPtr positionIn = vsEntry->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
    
    // Resolve output position parameter.
    ParameterPtr positionOut = vsEntry->resolveOutputParameter(Parameter::SPC_POSITION_PROJECTIVE_SPACE);
    //! [param_resolve]

    // Add dependency.
    vsProgram->addDependency(FFP_LIB_TRANSFORM);

    bool isHLSL = ShaderGenerator::getSingleton().getTargetLanguage() == "hlsl";

    // This requires GLES3.0
    if (ShaderGenerator::getSingleton().getTargetLanguage() == "glsles" &&
        !GpuProgramManager::getSingleton().isSyntaxSupported("glsl300es"))
        mInstancingTexCoordIndex = 0;

    auto stage = vsEntry->getStage(FFP_VS_TRANSFORM);
    if(mInstancingTexCoordIndex)
    {
        vsProgram->setInstancingIncluded(true);
        // this at prevents software skinning, which is not supported with instancing
        vsProgram->setSkeletalAnimationIncluded(true);
        if (isHLSL)
        {
            // set hlsl shader to use row-major matrices instead of column-major.
            // it enables the use of auto-bound 3x4 matrices in generated hlsl shader.
            vsProgram->setUseColumnMajorMatrices(false);
        }

        auto wMatrix = vsEntry->resolveInputParameter(Parameter::SPC_TEXTURE_COORDINATE0 + mInstancingTexCoordIndex,
                                                      GCT_MATRIX_3X4);
        stage.callFunction(FFP_FUNC_TRANSFORM, wMatrix, positionIn, Out(positionIn).xyz());

        if(mDoLightCalculations)
        {
            auto vsInNormal = vsEntry->resolveInputParameter(Parameter::SPC_NORMAL_OBJECT_SPACE);
            stage.callFunction(FFP_FUNC_TRANSFORM, wMatrix, vsInNormal, vsInNormal);
        }
        // we can end here because the world matrix will be identity with instanced rendering
        // so the code below will work as indended
    }
    stage.callBuiltin("mul", wvpMatrix, positionIn, positionOut);

    if(!mSetPointSize || isHLSL) // not supported with DX11
        return true;

    UniformParameterPtr pointParams = vsProgram->resolveParameter(GpuProgramParameters::ACT_POINT_PARAMS);
    ParameterPtr pointSize = vsEntry->resolveOutputParameter(Parameter::SPC_POINTSPRITE_SIZE);

    // using eye space depth only instead of the eye real distance
    // its faster to obtain, so lets call it close enough..
    stage.callFunction("FFP_DerivePointSize", pointParams, In(positionOut).w(), pointSize);

    return true;
}


//-----------------------------------------------------------------------
void FFPTransform::copyFrom(const SubRenderState& rhs)
{
    const FFPTransform& rhsTransform = static_cast<const FFPTransform&>(rhs);
    mSetPointSize = rhsTransform.mSetPointSize;
    mInstancingTexCoordIndex = rhsTransform.mInstancingTexCoordIndex;
}

bool FFPTransform::setParameter(const String& name, const String& value)
{
    if (name == "instanced")
    {
        return StringConverter::parse(value, mInstancingTexCoordIndex);
    }
    return false;
}

//-----------------------------------------------------------------------
const String& FFPTransformFactory::getType() const
{
    return SRS_TRANSFORM;
}

//-----------------------------------------------------------------------
SubRenderState* FFPTransformFactory::createInstance(ScriptCompiler* compiler, 
                                                   PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    if (prop->name == "transform_stage")
    {
        if(prop->values.size() > 0)
        {
            auto it = prop->values.begin();
            if((*it)->getString() != "instanced")
                return NULL;

            int texCoordSlot = 1;
            if(++it != prop->values.end() && !SGScriptTranslator::getInt(*++it, &texCoordSlot))
                return NULL;

            auto ret = createOrRetrieveInstance(translator);
            ret->setParameter("instanced", std::to_string(texCoordSlot));

            return ret;
        }
    }

    return NULL;
}

//-----------------------------------------------------------------------
void FFPTransformFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, 
                                       Pass* srcPass, Pass* dstPass)
{
    ser->writeAttribute(4, "transform_stage");
    ser->writeValue("ffp");
}

//-----------------------------------------------------------------------
SubRenderState* FFPTransformFactory::createInstanceImpl()
{
    return OGRE_NEW FFPTransform;
}


}
}

#endif
