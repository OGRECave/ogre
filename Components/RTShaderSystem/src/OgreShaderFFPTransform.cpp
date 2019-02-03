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

//-----------------------------------------------------------------------
const String& FFPTransform::getType() const
{
    return Type;
}


//-----------------------------------------------------------------------
int FFPTransform::getExecutionOrder() const
{
    return FFP_TRANSFORM;
}

bool FFPTransform::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    mSetPointSize = srcPass->getPointSize() != 1.0f || srcPass->isPointAttenuationEnabled();
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

    auto stage = vsEntry->getStage(FFP_VS_TRANSFORM);
    stage.callFunction(FFP_FUNC_TRANSFORM, wvpMatrix, positionIn, positionOut);

    if(!mSetPointSize || ShaderGenerator::getSingleton().getTargetLanguage() == "hlsl") // not supported with DX11
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
}

//-----------------------------------------------------------------------
const String& FFPTransformFactory::getType() const
{
    return FFPTransform::Type;
}

//-----------------------------------------------------------------------
SubRenderState* FFPTransformFactory::createInstance(ScriptCompiler* compiler, 
                                                   PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    if (prop->name == "transform_stage")
    {
        if(prop->values.size() == 1)
        {
            String modelType;

            if(false == SGScriptTranslator::getString(prop->values.front(), &modelType))
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                return NULL;
            }

            if (modelType == "ffp")
            {
                return createOrRetrieveInstance(translator);
            }
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
