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

#define SGX_LIB_TEXTURE_ATLAS "SGXLib_TextureAtlas"

#define SGX_FUNC_ATLAS_SAMPLE_AUTO_ADJUST "SGX_Atlas_Sample_Auto_Adjust"
#define SGX_FUNC_ATLAS_SAMPLE_NORMAL "SGX_Atlas_Sample_Normal"

#define SGX_FUNC_ATLAS_WRAP "SGX_Atlas_Wrap"
#define SGX_FUNC_ATLAS_MIRROR "SGX_Atlas_Mirror"
#define SGX_FUNC_ATLAS_CLAMP "SGX_Atlas_Clamp"
#define SGX_FUNC_ATLAS_BORDER "SGX_Atlas_Border"
#define TAS_MAX_SAFE_ATLASED_TEXTURES 250

namespace Ogre {
template<> RTShader::TextureAtlasSamplerFactory* Singleton<RTShader::TextureAtlasSamplerFactory>::msSingleton = 0;

namespace RTShader {


void operator<<(std::ostream& o, const TextureAtlasSamplerFactory::TextureAtlasAttib& tai)
{
    o << tai.autoBorderAdjust << tai.positionMode << tai.positionOffset;
}

const TextureAtlasTablePtr c_BlankAtlasTable;
const String c_ParamTexel("texel_");
String TextureAtlasSampler::Type = "SGX_TextureAtlasSampler";
String c_RTAtlasKey = "RTAtlas";

//-----------------------------------------------------------------------
TextureAtlasSampler::TextureAtlasSampler() :
    mAtlasTexcoordPos(0),
    mIsTableDataUpdated(false),
    mAutoAdjustPollPosition(true)
{
    mTextureAddressings->u = mTextureAddressings->v = mTextureAddressings->w = TextureUnitState::TAM_UNKNOWN;
    memset(mIsAtlasTextureUnits, 0, sizeof(bool) * TAS_MAX_TEXTURES);
}

//-----------------------------------------------------------------------
const String& TextureAtlasSampler::getType() const
{
    return Type;
}


//-----------------------------------------------------------------------
int TextureAtlasSampler::getExecutionOrder() const
{
    return FFP_TEXTURING + 25;
}

//-----------------------------------------------------------------------
bool TextureAtlasSampler::resolveParameters(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain   = vsProgram->getEntryPointFunction();
    Function* psMain   = psProgram->getEntryPointFunction();    


    //
    // Define vertex shader parameters used to find the position of the textures in the atlas
    //
    Parameter::Content indexContent = (Parameter::Content)((int)Parameter::SPC_TEXTURE_COORDINATE0 + mAtlasTexcoordPos);
    GpuConstantType indexType = GCT_FLOAT4;

    mVSInpTextureTableIndex = vsMain->resolveInputParameter(indexContent, indexType);
        
    
    //
    // Define parameters to carry the information on the location of the texture from the vertex to
    // the pixel shader
    //
    for(ushort i = 0 ; i < TAS_MAX_TEXTURES ; ++ i)
    {
        if (mIsAtlasTextureUnits[i] == true)
        {
            mVSTextureTable[i] = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL, "AtlasData", mAtlasTableDatas[i]->size());
            mVSOutTextureDatas[i] = vsMain->resolveOutputParameter(Parameter::SPC_UNKNOWN, GCT_FLOAT4);
            mPSInpTextureDatas[i] = psMain->resolveInputParameter(mVSOutTextureDatas[i]);
            mPSTextureSizes[i] = psProgram->resolveParameter(GCT_FLOAT2,-1, (uint16)GPV_PER_OBJECT, "AtlasSize");
        }
    }
    return true;
}


//-----------------------------------------------------------------------
bool TextureAtlasSampler::resolveDependencies(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    vsProgram->addDependency(FFP_LIB_COMMON);
    psProgram->addDependency(SGX_LIB_TEXTURE_ATLAS);

    return true;
}

//-----------------------------------------------------------------------
bool TextureAtlasSampler::addFunctionInvocations(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Function* vsMain   = vsProgram->getEntryPointFunction();    
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* psMain   = psProgram->getEntryPointFunction();    
    FunctionAtom* curFuncInvocation = NULL;

    //
    // Calculate the position and size of the texture in the atlas in the vertex shader
    //
    int groupOrder = (FFP_VS_TEXTURING - FFP_VS_LIGHTING) / 2;

    for(ushort i = 0 ; i <  TAS_MAX_TEXTURES; ++i)
    {
        if (mIsAtlasTextureUnits[i] == true)
        {
            Operand::OpMask textureIndexMask = Operand::OPM_X;
            switch (i)
            {
            case 1: textureIndexMask = Operand::OPM_Y; break;
            case 2: textureIndexMask = Operand::OPM_Z; break;
            case 3: textureIndexMask = Operand::OPM_W; break;
            }
            
            curFuncInvocation = OGRE_NEW AssignmentAtom(groupOrder);
            curFuncInvocation->pushOperand(mVSTextureTable[i], Operand::OPS_IN);
            curFuncInvocation->pushOperand(mVSInpTextureTableIndex, Operand::OPS_IN, textureIndexMask, 1);
            curFuncInvocation->pushOperand(mVSOutTextureDatas[i], Operand::OPS_OUT);
            vsMain->addAtomInstance(curFuncInvocation);
        }
    }

    //
    // sample the texture in the fragment shader given the extracted data in the pixel shader
    //


    groupOrder = (FFP_PS_SAMPLING + FFP_PS_TEXTURING) / 2;

    ParameterPtr psAtlasTextureCoord = psMain->resolveLocalParameter(GCT_FLOAT2, "atlasCoord");

    for(ushort j = 0 ; j <  TAS_MAX_TEXTURES; ++j)
    {
        if (mIsAtlasTextureUnits[j] == true)
        {
            //Find the texture coordinates texel and sampler from the original FFPTexturing
            ParameterPtr texcoord = psMain->getInputParameter((Parameter::Content)(Parameter::SPC_TEXTURE_COORDINATE0 + j), GCT_FLOAT2);
            ParameterPtr texel = psMain->getLocalParameter(c_ParamTexel + StringConverter::toString(j));
            UniformParameterPtr sampler = psProgram->getParameterByType(GCT_SAMPLER2D, j);
                
            const char* addressUFuncName = getAdressingFunctionName(mTextureAddressings[j].u);
            const char* addressVFuncName = getAdressingFunctionName(mTextureAddressings[j].v);
            
            //Create a function which will replace the texel with the texture texel
            if (texcoord && texel && sampler && addressUFuncName && addressVFuncName)
            {
                //calculate the U value due to addressing mode
                curFuncInvocation = OGRE_NEW FunctionInvocation(addressUFuncName, groupOrder);
                curFuncInvocation->pushOperand(texcoord, Operand::OPS_IN, Operand::OPM_X);
                curFuncInvocation->pushOperand(psAtlasTextureCoord, Operand::OPS_OUT, Operand::OPM_X);
                psMain->addAtomInstance(curFuncInvocation);

                //calculate the V value due to addressing mode
                curFuncInvocation = OGRE_NEW FunctionInvocation(addressVFuncName, groupOrder);
                curFuncInvocation->pushOperand(texcoord, Operand::OPS_IN, Operand::OPM_Y);
                curFuncInvocation->pushOperand(psAtlasTextureCoord, Operand::OPS_OUT, Operand::OPM_Y);
                psMain->addAtomInstance(curFuncInvocation);

                //sample the texel color
                curFuncInvocation = OGRE_NEW FunctionInvocation(
                    mAutoAdjustPollPosition ? SGX_FUNC_ATLAS_SAMPLE_AUTO_ADJUST : SGX_FUNC_ATLAS_SAMPLE_NORMAL, groupOrder);
                curFuncInvocation->pushOperand(sampler, Operand::OPS_IN);
                curFuncInvocation->pushOperand(texcoord, Operand::OPS_IN, Operand::OPM_XY);
                curFuncInvocation->pushOperand(psAtlasTextureCoord, Operand::OPS_IN);
                curFuncInvocation->pushOperand(mPSInpTextureDatas[j], Operand::OPS_IN);
                curFuncInvocation->pushOperand(mPSTextureSizes[j], Operand::OPS_IN);
                curFuncInvocation->pushOperand(texel, Operand::OPS_OUT);
                psMain->addAtomInstance(curFuncInvocation);

            }
        }
    }
    return true;
}

//-----------------------------------------------------------------------
const char* TextureAtlasSampler::getAdressingFunctionName(TextureAddressingMode mode)
{
    switch (mode)
    {
    case TextureUnitState::TAM_WRAP: return SGX_FUNC_ATLAS_WRAP; 
    case TextureUnitState::TAM_UNKNOWN: return SGX_FUNC_ATLAS_WRAP;
    case TextureUnitState::TAM_MIRROR: return SGX_FUNC_ATLAS_MIRROR;
    case TextureUnitState::TAM_CLAMP: return SGX_FUNC_ATLAS_CLAMP; 
    case TextureUnitState::TAM_BORDER: return SGX_FUNC_ATLAS_BORDER; 
    }
    return NULL;
}


//-----------------------------------------------------------------------
void TextureAtlasSampler::copyFrom(const SubRenderState& rhs)
{
    const TextureAtlasSampler& rhsColour = static_cast<const TextureAtlasSampler&>(rhs);

    mAtlasTexcoordPos = rhsColour.mAtlasTexcoordPos;
    for(ushort j = 0 ; j < TAS_MAX_TEXTURES ; ++j)
    {
        mIsAtlasTextureUnits[j] = rhsColour.mIsAtlasTextureUnits[j];
        mTextureAddressings[j] = rhsColour.mTextureAddressings[j];
        mAtlasTableDatas[j] = rhsColour.mAtlasTableDatas[j];
        mIsAtlasTextureUnits[j] = rhsColour.mIsAtlasTextureUnits[j];
    }
}

//-----------------------------------------------------------------------
void TextureAtlasSampler::updateGpuProgramsParams(Renderable* rend, const Pass* pass,  const AutoParamDataSource* source, const LightList* pLightList)
{
    if (mIsTableDataUpdated == false)
    {
        mIsTableDataUpdated = true;
        for(ushort j = 0 ; j < TAS_MAX_TEXTURES ; ++j)
        {
            if (mIsAtlasTextureUnits[j] == true)
            {
                //
                // Update the information of the size of the atlas textures 
                //
                std::pair< size_t, size_t > texSizeInt = pass->getTextureUnitState(j)->getTextureDimensions();
                Vector2 texSize((Ogre::Real)texSizeInt.first, (Ogre::Real)texSizeInt.second);
                mPSTextureSizes[j]->setGpuParameter(texSize); 

                //
                //Update the information of which texture exist where in the atlas
                //
                GpuProgramParametersSharedPtr vsGpuParams = pass->getVertexProgramParameters();
                std::vector<float> buffer(mAtlasTableDatas[j]->size() * 4);
                for(size_t i = 0 ; i < mAtlasTableDatas[j]->size() ; ++i)
                {
                    buffer[i*4] = (*(mAtlasTableDatas[j]))[i].posU;
                    buffer[i*4 + 1] = (*(mAtlasTableDatas[j]))[i].posV;
                    buffer[i*4 + 2] = (float)Ogre::Math::Log2((*(mAtlasTableDatas[j]))[i].width * texSize.x);
                    buffer[i*4 + 3] = (float)Ogre::Math::Log2((*(mAtlasTableDatas[j]))[i].height * texSize.y);
                }
                vsGpuParams->setNamedConstant(mVSTextureTable[j]->getName(), (const float*)(&(buffer[0])), (mAtlasTableDatas[j])->size()); 
            }
        }
    }
}

//-----------------------------------------------------------------------
bool TextureAtlasSampler::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    mAtlasTexcoordPos = 0; 
    
    const TextureAtlasSamplerFactory& factory = TextureAtlasSamplerFactory::getSingleton();

    bool hasAtlas = false;
    unsigned short texCount = srcPass->getNumTextureUnitStates();
    for(unsigned short i = 0 ; i < texCount ; ++i)
    {
        TextureUnitState* pState = srcPass->getTextureUnitState(i);
        
        const TextureAtlasTablePtr& table = factory.getTextureAtlasTable(pState->getTextureName()); 
        if (table)
        {
            if (table->size() > TAS_MAX_SAFE_ATLASED_TEXTURES)
            {
                LogManager::getSingleton().logWarning(
                    "Compiling atlas texture has to many internally defined textures. Shader may fail to compile.");
            }
            if (i >= TAS_MAX_TEXTURES)
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
                    "Texture atlas sub-render does not support more than TAS_MAX_TEXTURES (4) atlas textures",
                    "TextureAtlasSampler::preAddToRenderState" );
            }
            if (pState->getTextureType() != TEX_TYPE_2D)
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
                    "Texture atlas sub-render state only supports 2d textures.",
                    "TextureAtlasSampler::preAddToRenderState" );

            }
            
            mAtlasTableDatas[i] = table;
            mTextureAddressings[i] = pState->getTextureAddressingMode();
            mIsAtlasTextureUnits[i] = true;
            hasAtlas = true;
        }
    }
    
    //gather the materials atlas processing attributes 
    //and calculate the position of the indexes 
    TextureAtlasSamplerFactory::TextureAtlasAttib attrib;
    factory.hasMaterialAtlasingAttributes(srcPass->getParent()->getParent(), &attrib);

    mAutoAdjustPollPosition = attrib.autoBorderAdjust;
    mAtlasTexcoordPos = attrib.positionOffset;
    if (attrib.positionMode == TextureAtlasSamplerFactory::ipmRelative)
    {
        mAtlasTexcoordPos += texCount - 1;
    }
    
    return hasAtlas;
}

TextureAtlasSamplerFactory::TextureAtlasSamplerFactory()
{

}


TextureAtlasSamplerFactory* TextureAtlasSamplerFactory::getSingletonPtr(void)
{
    return msSingleton;
}
TextureAtlasSamplerFactory& TextureAtlasSamplerFactory::getSingleton(void)
{  
    assert( msSingleton );  return ( *msSingleton );  
}



//-----------------------------------------------------------------------
const String& TextureAtlasSamplerFactory::getType() const
{
    return TextureAtlasSampler::Type;
}

//-----------------------------------------------------------------------
SubRenderState* TextureAtlasSamplerFactory::createInstance(ScriptCompiler* compiler, 
                                                    PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    return NULL;
}

//-----------------------------------------------------------------------
void TextureAtlasSamplerFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, 
                                        Pass* srcPass, Pass* dstPass)
{
}

//-----------------------------------------------------------------------
bool TextureAtlasSamplerFactory::addTexutreAtlasDefinition( const Ogre::String& filename, TextureAtlasTablePtr textureAtlasTable )
{
    std::ifstream inp;
    inp.open(filename.c_str(), std::ios::in | std::ios::binary);
    if(!inp)
    {
        OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND, "'" + filename + "' file not found!", 
            "TextureAtlasSamplerFactory::addTexutreAtlasDefinition" );
    }
    DataStreamPtr stream(OGRE_NEW FileStreamDataStream(filename, &inp, false));
    return addTexutreAtlasDefinition(stream, textureAtlasTable);

}
//-----------------------------------------------------------------------
bool TextureAtlasSamplerFactory::addTexutreAtlasDefinition( DataStreamPtr stream, TextureAtlasTablePtr textureAtlasTable )
{
    stream->seek(0);

    bool isSuccess = false;
    if (stream->isReadable() == true)
    {
        TextureAtlasMap tmpMap;
        
        while (stream->eof() == false)
        {
            String line = stream->getLine(true);
            size_t nonWhiteSpacePos = line.find_first_not_of(" \t\r\n");
            //check this is a line with information
            if ((nonWhiteSpacePos != String::npos) && (line[nonWhiteSpacePos] != '#'))
            {
                //parse the line
                std::vector<String> strings = StringUtil::split(line, ",\t");
                
                if (strings.size() > 8)
                {
                    String textureName = strings[1];

                    TextureAtlasMap::iterator it = tmpMap.find(textureName);
                    if (it == tmpMap.end())
                    {
                        it = tmpMap.emplace(textureName, TextureAtlasTablePtr(new TextureAtlasTable)).first;
                    }
                    
                    // file line format:  <original texture filename>/t/t<atlas filename>, <atlas idx>, <atlas type>, <woffset>, <hoffset>, <depth offset>, <width>, <height>
                    //                              0                           1              2            3             4          5          6               7        8
                    TextureAtlasRecord newRecord(
                        strings[0], // original texture filename
                        strings[1], // atlas filename
                        (float)StringConverter::parseReal(strings[4]), // woffset
                        (float)StringConverter::parseReal(strings[5]), // hoffset
                        (float)StringConverter::parseReal(strings[7]), // width
                        (float)StringConverter::parseReal(strings[8]), // height
                        it->second->size() // texture index in atlas texture
                        );

                    it->second->push_back(newRecord);
                    if (textureAtlasTable)
                    {
                        textureAtlasTable->push_back(newRecord);
                    }

                    isSuccess = true;
                }
            }
        }

        //place the information in the main texture
        size_t maxTextureCount = 0;
        TextureAtlasMap::const_iterator it = tmpMap.begin();
        TextureAtlasMap::const_iterator itEnd = tmpMap.end();
        for(;it != itEnd; ++it)
        {
            setTextureAtlasTable(it->first, it->second);
            maxTextureCount = std::max<size_t>(maxTextureCount, it->second->size());
        }

        if (maxTextureCount > TAS_MAX_SAFE_ATLASED_TEXTURES)
        {
            LogManager::getSingleton().logMessage(LML_CRITICAL, 
                ("Warning : " + stream->getName() +
                " atlas texture has to many internally defined textures. Shader may fail to compile."));
        }
    }
    return isSuccess;
}

//-----------------------------------------------------------------------
void TextureAtlasSamplerFactory::setTextureAtlasTable(const String& textureName, const TextureAtlasTablePtr& atlasData, bool autoBorderAdjust)
{
    if (!atlasData || atlasData->empty())
        removeTextureAtlasTable(textureName);
    else mAtlases.emplace(textureName, atlasData);
}

//-----------------------------------------------------------------------
void TextureAtlasSamplerFactory::removeTextureAtlasTable(const String& textureName)
{
    mAtlases.erase(textureName);
}

//-----------------------------------------------------------------------
void TextureAtlasSamplerFactory::removeAllTextureAtlasTables()
{
    mAtlases.clear();
}

//-----------------------------------------------------------------------
const TextureAtlasTablePtr& TextureAtlasSamplerFactory::getTextureAtlasTable(const String& textureName) const
{
    TextureAtlasMap::const_iterator it = mAtlases.find(textureName);
    if (it != mAtlases.end())
    {
        return it->second;
    }
    else return c_BlankAtlasTable;
}

//-----------------------------------------------------------------------
void TextureAtlasSamplerFactory::setDefaultAtlasingAttributes(IndexPositionMode mode, ushort offset, bool autoAdjustBorders)
{
    mDefaultAtlasAttrib = TextureAtlasAttib(mode, offset, autoAdjustBorders);
}

//-----------------------------------------------------------------------
const TextureAtlasSamplerFactory::TextureAtlasAttib& TextureAtlasSamplerFactory::getDefaultAtlasingAttributes() const
{
    return mDefaultAtlasAttrib;
}

//-----------------------------------------------------------------------
void TextureAtlasSamplerFactory::setMaterialAtlasingAttributes(Ogre::Material* material, 
        IndexPositionMode mode, ushort offset, bool autoAdjustBorders)
{
    if ((material) && (material->getNumTechniques()))
    {
        material->getTechnique(0)->getUserObjectBindings().setUserAny(c_RTAtlasKey, 
            TextureAtlasAttib(mode, offset, autoAdjustBorders));
    }
}


//-----------------------------------------------------------------------
bool TextureAtlasSamplerFactory::hasMaterialAtlasingAttributes(Ogre::Material* material, 
                                                                 TextureAtlasAttib* attrib) const
{
    bool isMaterialSpecific = false;
    if ((material) && (material->getNumTechniques()))
    {
        const Ogre::Any& anyAttrib = 
            //find if the "IsTerrain" flag exists in the first technique
            material->getTechnique(0)->getUserObjectBindings().getUserAny(c_RTAtlasKey);
        isMaterialSpecific = anyAttrib.has_value();
        if ((isMaterialSpecific) && (attrib))
        {
            *attrib = Ogre::any_cast<TextureAtlasAttib>(anyAttrib);
        }
    }
    if ((!isMaterialSpecific) && (attrib))
    {
        *attrib = mDefaultAtlasAttrib;
    }
    return isMaterialSpecific;  
}

//-----------------------------------------------------------------------
SubRenderState* TextureAtlasSamplerFactory::createInstanceImpl()
{
    return OGRE_NEW TextureAtlasSampler;
}


}
}

#endif
