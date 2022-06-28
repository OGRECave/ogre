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
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS

#define SGX_LIB_NORMALMAP                           "SGXLib_NormalMap"
#define SGX_FUNC_CONSTRUCT_TBNMATRIX                "SGX_ConstructTBNMatrix"
#define SGX_FUNC_FETCHNORMAL                        "SGX_FetchNormal"

namespace Ogre {
namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String NormalMapLighting::Type                      = "NormalMap";

//-----------------------------------------------------------------------
NormalMapLighting::NormalMapLighting()
{
    mNormalMapSamplerIndex          = 0;
    mVSTexCoordSetIndex             = 0;
    mNormalMapSpace                 = NMS_TANGENT;
    mNormalMapSampler = TextureManager::getSingleton().createSampler();
    mNormalMapSampler->setMipmapBias(-1.0);
}

//-----------------------------------------------------------------------
const String& NormalMapLighting::getType() const
{
    return Type;
}
//-----------------------------------------------------------------------
bool NormalMapLighting::createCpuSubPrograms(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* psMain = psProgram->getEntryPointFunction();

    vsProgram->addDependency(FFP_LIB_TRANSFORM);

    psProgram->addDependency(FFP_LIB_TRANSFORM);
    psProgram->addDependency(FFP_LIB_TEXTURING);
    psProgram->addDependency(SGX_LIB_NORMALMAP);

    // Resolve texture coordinates.
    auto vsInTexcoord = vsMain->resolveInputParameter(
        Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + mVSTexCoordSetIndex), GCT_FLOAT2);
    auto vsOutTexcoord = vsMain->resolveOutputParameter(
        Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + mVSTexCoordSetIndex), GCT_FLOAT2);
    auto psInTexcoord = psMain->resolveInputParameter(vsOutTexcoord);

    // Resolve normal.
    auto vsInNormal = vsMain->resolveInputParameter(Parameter::SPC_NORMAL_OBJECT_SPACE);
    auto vsOutNormal = vsMain->resolveOutputParameter(Parameter::SPC_NORMAL_VIEW_SPACE);
    auto viewNormal = psMain->resolveInputParameter(vsOutNormal);
    auto newViewNormal = psMain->resolveLocalParameter(Parameter::SPC_NORMAL_VIEW_SPACE);

    // insert before lighting stage
    auto vstage = vsMain->getStage(FFP_PS_COLOUR_BEGIN + 1);
    auto fstage = psMain->getStage(FFP_PS_COLOUR_BEGIN + 1);

    // Output texture coordinates.
    vstage.assign(vsInTexcoord, vsOutTexcoord);

    // Add the normal fetch function invocation
    auto normalMapSampler = psProgram->resolveParameter(GCT_SAMPLER2D, "gNormalMapSampler", mNormalMapSamplerIndex);
    fstage.callFunction(SGX_FUNC_FETCHNORMAL, normalMapSampler, psInTexcoord, newViewNormal);

    if (mNormalMapSpace & NMS_TANGENT)
    {
        auto vsInTangent = vsMain->resolveInputParameter(Parameter::SPC_TANGENT_OBJECT_SPACE);
        auto vsOutTangent = vsMain->resolveOutputParameter(Parameter::SPC_TANGENT_OBJECT_SPACE);
        auto psInTangent = psMain->resolveInputParameter(vsOutTangent);

        // transform normal & tangent
        auto normalMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_NORMAL_MATRIX);
        vstage.callFunction(FFP_FUNC_TRANSFORM, normalMatrix, vsInNormal, vsOutNormal);
        vstage.callFunction(FFP_FUNC_TRANSFORM, normalMatrix, vsInTangent, vsOutTangent);

        // Construct TBN matrix.
        auto TBNMatrix = psMain->resolveLocalParameter(GCT_MATRIX_3X3, "lMatTBN");
        fstage.callFunction(SGX_FUNC_CONSTRUCT_TBNMATRIX, viewNormal, psInTangent, TBNMatrix);
        // transform normal
        fstage.callFunction(FFP_FUNC_TRANSFORM, TBNMatrix, newViewNormal, newViewNormal);
    }
    else if (mNormalMapSpace & NMS_OBJECT)
    {
        // transform normal in FS
        auto normalMatrix = psProgram->resolveParameter(GpuProgramParameters::ACT_NORMAL_MATRIX);
        fstage.callFunction(FFP_FUNC_TRANSFORM, normalMatrix, newViewNormal, newViewNormal);
    }

    if (mNormalMapSpace == NMS_PARALLAX)
    {
        // assuming: lighting stage computed this
        auto vsOutViewPos = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_VIEW_SPACE);
        auto viewPos = psMain->resolveInputParameter(vsOutViewPos);

        // TODO: user specificed scale and bias
        fstage.callFunction("SGX_Generate_Parallax_Texcoord", {In(normalMapSampler), In(psInTexcoord), In(viewPos),
                                                              In(Vector2(0.04, -0.02)), Out(psInTexcoord)});

        // overwrite texcoord0 unconditionally, only one texcoord set is supported with parallax mapping
        // we are before FFP_PS_TEXTURING, so the new value will be used
        auto texcoord0 = psMain->resolveInputParameter(Parameter::SPC_TEXTURE_COORDINATE0, GCT_FLOAT2);
        fstage.assign(psInTexcoord, texcoord0);
    }

    return true;
}

//-----------------------------------------------------------------------
void NormalMapLighting::copyFrom(const SubRenderState& rhs)
{
    const NormalMapLighting& rhsLighting = static_cast<const NormalMapLighting&>(rhs);

    mNormalMapSpace = rhsLighting.mNormalMapSpace;
    mNormalMapTextureName = rhsLighting.mNormalMapTextureName;
    mNormalMapSampler = rhsLighting.mNormalMapSampler;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    TextureUnitState* normalMapTexture = dstPass->createTextureUnitState();

    normalMapTexture->setTextureName(mNormalMapTextureName);
    normalMapTexture->setSampler(mNormalMapSampler);
    mNormalMapSamplerIndex = dstPass->getNumTextureUnitStates() - 1;

    return true;
}

bool NormalMapLighting::setParameter(const String& name, const String& value)
{
	if(name == "normalmap_space")
	{
        // Normal map defines normals in tangent space.
        if (value == "tangent_space")
        {
            setNormalMapSpace(NMS_TANGENT);
            return true;
        }
        // Normal map defines normals in object space.
        if (value == "object_space")
        {
            setNormalMapSpace(NMS_OBJECT);
            return true;
        }
        if (value == "parallax")
        {
            setNormalMapSpace(NMS_PARALLAX);
            return true;
        }
		return false;
	}

	if(name == "texture")
	{
		mNormalMapTextureName = value;
		return true;
	}

	if(name == "texcoord_index")
	{
		setTexCoordIndex(StringConverter::parseInt(value));
		return true;
	}

    if(name == "sampler")
    {
        auto sampler = TextureManager::getSingleton().getSampler(value);
        if(!sampler)
            return false;
        mNormalMapSampler = sampler;
        return true;
    }

	return false;
}

//-----------------------------------------------------------------------
const String& NormalMapLightingFactory::getType() const
{
    return NormalMapLighting::Type;
}

//-----------------------------------------------------------------------
SubRenderState* NormalMapLightingFactory::createInstance(ScriptCompiler* compiler, 
                                                        PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    if (prop->name == "lighting_stage")
    {
        if(prop->values.size() >= 2)
        {
            String strValue;
            AbstractNodeList::const_iterator it = prop->values.begin();
            
            // Read light model type.
            if(false == SGScriptTranslator::getString(*it, &strValue))
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                return NULL;
            }

            // Case light model type is normal map
            if (strValue == "normal_map")
            {
                ++it;
                if (false == SGScriptTranslator::getString(*it, &strValue))
                {
                    compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    return NULL;
                }

                
                SubRenderState* subRenderState = createOrRetrieveInstance(translator);
                NormalMapLighting* normalMapSubRenderState = static_cast<NormalMapLighting*>(subRenderState);
                
                normalMapSubRenderState->setParameter("texture", strValue);

                
                // Read normal map space type.
                if (prop->values.size() >= 3)
                {                   
                    ++it;
                    if (false == SGScriptTranslator::getString(*it, &strValue))
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return NULL;
                    }

                    if(!normalMapSubRenderState->setParameter("normalmap_space", strValue))
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        return NULL;
                    }
                }

                // Read texture coordinate index.
                if (prop->values.size() >= 4)
                {   
                    unsigned int textureCoordinateIndex = 0;

                    ++it;
                    if (SGScriptTranslator::getUInt(*it, &textureCoordinateIndex))
                    {
                        normalMapSubRenderState->setTexCoordIndex(textureCoordinateIndex);
                    }
                }

                // Read texture filtering format.
                if (prop->values.size() >= 5)
                {
                    ++it;
                    if (false == SGScriptTranslator::getString(*it, &strValue))
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return NULL;
                    }

                    // sampler reference
                    if(!normalMapSubRenderState->setParameter("sampler", strValue))
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        return NULL;
                    }
                }
                                
                return subRenderState;                              
            }
        }
    }

    return NULL;
}

//-----------------------------------------------------------------------
void NormalMapLightingFactory::writeInstance(MaterialSerializer* ser, 
                                             SubRenderState* subRenderState, 
                                             Pass* srcPass, Pass* dstPass)
{
    NormalMapLighting* normalMapSubRenderState = static_cast<NormalMapLighting*>(subRenderState);

    ser->writeAttribute(4, "lighting_stage");
    ser->writeValue("normal_map");
    ser->writeValue(normalMapSubRenderState->getNormalMapTextureName());    
    
    if (normalMapSubRenderState->getNormalMapSpace() == NormalMapLighting::NMS_TANGENT)
    {
        ser->writeValue("tangent_space");
    }
    else if (normalMapSubRenderState->getNormalMapSpace() == NormalMapLighting::NMS_OBJECT)
    {
        ser->writeValue("object_space");
    }

    ser->writeValue(StringConverter::toString(normalMapSubRenderState->getTexCoordIndex()));
}

//-----------------------------------------------------------------------
SubRenderState* NormalMapLightingFactory::createInstanceImpl()
{
    return OGRE_NEW NormalMapLighting;
}

}
}

#endif
