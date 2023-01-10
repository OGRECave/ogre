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

#include "OgreShaderPrecompiledHeaders.h"

namespace Ogre {
namespace RTShader {

static GpuConstantType typeFromContent(Parameter::Content content)
{
    switch (content)
    {
    case Parameter::SPC_BLEND_INDICES:
        return GCT_UINT4;
    case Parameter::SPC_COLOR_DIFFUSE:
    case Parameter::SPC_COLOR_SPECULAR:
    case Parameter::SPC_POSITION_PROJECTIVE_SPACE:
    case Parameter::SPC_POSITION_OBJECT_SPACE:
    case Parameter::SPC_BLEND_WEIGHTS:
    case Parameter::SPC_POSITION_LIGHT_SPACE0:
    case Parameter::SPC_POSITION_LIGHT_SPACE1:
    case Parameter::SPC_POSITION_LIGHT_SPACE2:
    case Parameter::SPC_POSITION_LIGHT_SPACE3:
    case Parameter::SPC_POSITION_LIGHT_SPACE4:
    case Parameter::SPC_POSITION_LIGHT_SPACE5:
    case Parameter::SPC_POSITION_LIGHT_SPACE6:
    case Parameter::SPC_POSITION_LIGHT_SPACE7:
    case Parameter::SPC_TANGENT_OBJECT_SPACE:
        return GCT_FLOAT4;
    case Parameter::SPC_NORMAL_TANGENT_SPACE:
    case Parameter::SPC_NORMAL_OBJECT_SPACE:
    case Parameter::SPC_NORMAL_WORLD_SPACE:
    case Parameter::SPC_NORMAL_VIEW_SPACE:
    case Parameter::SPC_POSTOCAMERA_TANGENT_SPACE:
    case Parameter::SPC_POSTOCAMERA_OBJECT_SPACE:
    case Parameter::SPC_POSTOCAMERA_VIEW_SPACE:
    case Parameter::SPC_POSITION_VIEW_SPACE:
    case Parameter::SPC_POSITION_WORLD_SPACE:
    case Parameter::SPC_LIGHTDIRECTION_OBJECT_SPACE0:
    case Parameter::SPC_LIGHTDIRECTION_OBJECT_SPACE1:
    case Parameter::SPC_LIGHTDIRECTION_OBJECT_SPACE2:
    case Parameter::SPC_LIGHTDIRECTION_OBJECT_SPACE3:
    case Parameter::SPC_LIGHTDIRECTION_OBJECT_SPACE4:
    case Parameter::SPC_LIGHTDIRECTION_OBJECT_SPACE5:
    case Parameter::SPC_LIGHTDIRECTION_OBJECT_SPACE6:
    case Parameter::SPC_LIGHTDIRECTION_OBJECT_SPACE7:
    case Parameter::SPC_POSTOLIGHT_OBJECT_SPACE0:
    case Parameter::SPC_POSTOLIGHT_OBJECT_SPACE1:
    case Parameter::SPC_POSTOLIGHT_OBJECT_SPACE2:
    case Parameter::SPC_POSTOLIGHT_OBJECT_SPACE3:
    case Parameter::SPC_POSTOLIGHT_OBJECT_SPACE4:
    case Parameter::SPC_POSTOLIGHT_OBJECT_SPACE5:
    case Parameter::SPC_POSTOLIGHT_OBJECT_SPACE6:
    case Parameter::SPC_POSTOLIGHT_OBJECT_SPACE7:
    case Parameter::SPC_LIGHTDIRECTION_TANGENT_SPACE0:
    case Parameter::SPC_LIGHTDIRECTION_TANGENT_SPACE1:
    case Parameter::SPC_LIGHTDIRECTION_TANGENT_SPACE2:
    case Parameter::SPC_LIGHTDIRECTION_TANGENT_SPACE3:
    case Parameter::SPC_LIGHTDIRECTION_TANGENT_SPACE4:
    case Parameter::SPC_LIGHTDIRECTION_TANGENT_SPACE5:
    case Parameter::SPC_LIGHTDIRECTION_TANGENT_SPACE6:
    case Parameter::SPC_LIGHTDIRECTION_TANGENT_SPACE7:
    case Parameter::SPC_POSTOLIGHT_TANGENT_SPACE0:
    case Parameter::SPC_POSTOLIGHT_TANGENT_SPACE1:
    case Parameter::SPC_POSTOLIGHT_TANGENT_SPACE2:
    case Parameter::SPC_POSTOLIGHT_TANGENT_SPACE3:
    case Parameter::SPC_POSTOLIGHT_TANGENT_SPACE4:
    case Parameter::SPC_POSTOLIGHT_TANGENT_SPACE5:
    case Parameter::SPC_POSTOLIGHT_TANGENT_SPACE6:
    case Parameter::SPC_POSTOLIGHT_TANGENT_SPACE7:
    case Parameter::SPC_LIGHTDIRECTION_VIEW_SPACE0:
        return GCT_FLOAT3;
    case Parameter::SPC_POINTSPRITE_COORDINATE:
        return GCT_FLOAT2;
    case Parameter::SPC_POINTSPRITE_SIZE:
    case Parameter::SPC_DEPTH_VIEW_SPACE:
        return GCT_FLOAT1;
    case Parameter::SPC_FRONT_FACING:
        return GCT_FLOAT1;
    default:
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "cannot derive type from content");
        break;
    }
}

static Parameter::Semantic semanticFromContent(Parameter::Content content, bool isVSOut = false)
{
    switch (content)
    {
    case Parameter::SPC_COLOR_DIFFUSE:
    case Parameter::SPC_COLOR_SPECULAR:
        return Parameter::SPS_COLOR;
    case Parameter::SPC_POSITION_PROJECTIVE_SPACE:
        return Parameter::SPS_POSITION;
    case Parameter::SPC_BLEND_INDICES:
        return Parameter::SPS_BLEND_INDICES;
    case Parameter::SPC_BLEND_WEIGHTS:
        return Parameter::SPS_BLEND_WEIGHTS;
    case Parameter::SPC_POINTSPRITE_COORDINATE:
        return Parameter::SPS_TEXTURE_COORDINATES;
    case Parameter::SPC_BINORMAL_OBJECT_SPACE:
        return Parameter::SPS_BINORMAL;
    case Parameter::SPC_FRONT_FACING:
        return Parameter::SPS_FRONT_FACING;
    case Parameter::SPC_TANGENT_OBJECT_SPACE:
        if(!isVSOut) return Parameter::SPS_TANGENT;
        OGRE_FALLTHROUGH;
    case Parameter::SPC_POSITION_OBJECT_SPACE:
        if(!isVSOut) return Parameter::SPS_POSITION;
        OGRE_FALLTHROUGH;
    case Parameter::SPC_NORMAL_OBJECT_SPACE:
        if(!isVSOut) return Parameter::SPS_NORMAL;
        OGRE_FALLTHROUGH;
    // the remaining types are VS output types only (or indeed texcoord)
    // for out types we use the TEXCOORD[n] semantics for compatibility
    // with Cg, HLSL SM2.0 where they are the only multivariate semantics
    default:
        return Parameter::SPS_TEXTURE_COORDINATES;
    }
}

/// fixed index for texcoords, next free semantic slot else
static int indexFromContent(Parameter::Content content)
{
    int c = int(content);
    if(c < Parameter::SPC_TEXTURE_COORDINATE0 || c > Parameter::SPC_TEXTURE_COORDINATE7)
        return -1;

    return c - Parameter::SPC_TEXTURE_COORDINATE0;
}

void FunctionStageRef::callFunction(const char* name, const InOut& inout) const
{
    callFunction(name, std::vector<Operand>{inout});
}

void FunctionStageRef::callFunction(const char* name, const std::vector<Operand>& params) const
{
    auto function = new FunctionInvocation(name, mStage);
    function->setOperands(params);
    mParent->addAtomInstance(function);
}

void FunctionStageRef::sampleTexture(const std::vector<Operand>& params) const
{
    auto function = new SampleTextureAtom(mStage);
    function->setOperands(params);
    mParent->addAtomInstance(function);
}

void FunctionStageRef::assign(const std::vector<Operand>& params) const
{
    auto function = new AssignmentAtom(mStage);
    function->setOperands(params);
    mParent->addAtomInstance(function);
}

void FunctionStageRef::binaryOp(char op, const std::vector<Operand>& params) const
{
    auto function = new BinaryOpAtom(op, mStage);
    function->setOperands(params);
    mParent->addAtomInstance(function);
}

void FunctionStageRef::callBuiltin(const char* name, const std::vector<Operand>& params) const
{
    auto function = new BuiltinFunctionAtom(name, mStage);
    function->setOperands(params);
    mParent->addAtomInstance(function);
}

//-----------------------------------------------------------------------------
Function::~Function()
{
    std::map<size_t, FunctionAtomInstanceList>::iterator jt;
    for(jt = mAtomInstances.begin(); jt != mAtomInstances.end(); ++jt)
    {
        for (FunctionAtomInstanceIterator it=jt->second.begin(); it != jt->second.end(); ++it)
            OGRE_DELETE (*it);
    }

    mAtomInstances.clear();

    for (ShaderParameterIterator it = mInputParameters.begin(); it != mInputParameters.end(); ++it)
        (*it).reset();
    mInputParameters.clear();

    for (ShaderParameterIterator it = mOutputParameters.begin(); it != mOutputParameters.end(); ++it)
        (*it).reset();
    mOutputParameters.clear();

    for (ShaderParameterIterator it = mLocalParameters.begin(); it != mLocalParameters.end(); ++it)
        (*it).reset();
    mLocalParameters.clear();

}

static String getParameterName(const char* prefix, Parameter::Semantic semantic, int index)
{
    const char* name = "";
    switch (semantic)
    {
    case Parameter::SPS_POSITION:
        name = "Pos";
        break;
    case Parameter::SPS_BLEND_WEIGHTS:
        name = "BlendWeights";
        break;
    case Parameter::SPS_BLEND_INDICES:
        name = "BlendIndices";
        break;
    case Parameter::SPS_NORMAL:
        name = "Normal";
        break;
    case Parameter::SPS_COLOR:
        name = "Color";
        break;
    case Parameter::SPS_TEXTURE_COORDINATES:
        name = "Texcoord";
        break;
    case Parameter::SPS_BINORMAL:
        name = "BiNormal";
        break;
    case Parameter::SPS_TANGENT:
        name = "Tangent";
        break;
    case Parameter::SPS_FRONT_FACING:
        name = "FrontFacing";
        break;
    case Parameter::SPS_UNKNOWN:
        name = "Param";
        break;
    };
    return StringUtil::format("%s%s_%d", prefix, name, index);
}

//-----------------------------------------------------------------------------
ParameterPtr Function::resolveInputParameter(Parameter::Semantic semantic,
                                        int index,
                                        const Parameter::Content content,
                                        GpuConstantType type)
{
    if(type == GCT_UNKNOWN)
        type = typeFromContent(content);

    ParameterPtr param;

    // Check if desired parameter already defined.
    param = _getParameterByContent(mInputParameters, content, type);
    if (param.get() != NULL)
        return param;

    if(semantic == Parameter::SPS_UNKNOWN)
    {
        semantic = semanticFromContent(content);
        index = indexFromContent(content); // create new parameter for this content
    }

    // Case we have to create new parameter.
    if (index == -1)
    {
        index = 0;

        // Find the next available index of the target semantic.
        ShaderParameterIterator it;

        for (it = mInputParameters.begin(); it != mInputParameters.end(); ++it)
        {
            if ((*it)->getSemantic() == semantic)
            {
                index++;
            }
        }
    }
    else
    {
        // Check if desired parameter already defined.
        param = _getParameterBySemantic(mInputParameters, semantic, index);
        if (param.get() != NULL && param->getContent() == content)
        {
            if (param->getType() == type)
            {
                return param;
            }
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        StringUtil::format("Can not resolve parameter due to type mismatch: semantic: %d, index: %d",
                            semantic, index));
        }
    }

    
    // No parameter found -> create new one.
    OgreAssert(semantic != Parameter::SPS_UNKNOWN, "unknown semantic");
    param =
        std::make_shared<Parameter>(type, getParameterName("i", semantic, index), semantic, index, content);
    addInputParameter(param);

    return param;
}

//-----------------------------------------------------------------------------
ParameterPtr Function::resolveOutputParameter(Parameter::Semantic semantic,
                                            int index,
                                            Parameter::Content content,                                         
                                            GpuConstantType type)
{
    if(type == GCT_UNKNOWN)
        type = typeFromContent(content);

    ParameterPtr param;

    // Check if desired parameter already defined.
    param = _getParameterByContent(mOutputParameters, content, type);
    if (param.get() != NULL)
        return param;

    if(semantic == Parameter::SPS_UNKNOWN)
    {
        semantic = semanticFromContent(content, true);
        index = -1; // create new parameter for this content
    }

    // Case we have to create new parameter.
    if (index == -1)
    {
        index = 0;

        // Find the next available index of the target semantic.
        ShaderParameterIterator it;

        for (it = mOutputParameters.begin(); it != mOutputParameters.end(); ++it)
        {
            if ((*it)->getSemantic() == semantic)
            {
                index++;
            }
        }
    }
    else
    {
        // Check if desired parameter already defined.
        param = _getParameterBySemantic(mOutputParameters, semantic, index);
        if (param.get() != NULL && param->getContent() == content)
        {
            if (param->getType() == type)
            {
                return param;
            }
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        StringUtil::format("Can not resolve parameter due to type mismatch: semantic: %d, index: %d",
                            semantic, index));
        }
    }
    

    // No parameter found -> create new one.
    switch (semantic)
    {
    case Parameter::SPS_TEXTURE_COORDINATES:
    case Parameter::SPS_COLOR:
    case Parameter::SPS_POSITION:
        param = std::make_shared<Parameter>(type, getParameterName("o", semantic, index), semantic, index,
                                            content);
        break;

    default:
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                    StringUtil::format("Semantic not supported as output parameter: %d", semantic));
        break;
    }

    addOutputParameter(param);

    return param;
}

//-----------------------------------------------------------------------------
ParameterPtr Function::resolveLocalParameter(GpuConstantType type, const String& name)
{
    ParameterPtr param;

    param = _getParameterByName(mLocalParameters, name);
    if (param.get() != NULL)
    {
        if (param->getType() == type)
        {
            return param;
        }
        else 
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Can not resolve local parameter due to type mismatch");
        }       
    }
        
    param = std::make_shared<Parameter>(type, name, Parameter::SPS_UNKNOWN, 0, Parameter::SPC_UNKNOWN);
    addParameter(mLocalParameters, param);
            
    return param;
}

ParameterPtr Function::resolveLocalStructParameter(const String& type, const String& name)
{
    ParameterPtr param;

    param = _getParameterByName(mLocalParameters, name);
    if (param)
    {
        OgreAssert(param->getStructType() == type, "A parameter with the same name but different type already exists");
        return param;
    }

    param = std::make_shared<Parameter>(GCT_UNKNOWN, name, Parameter::SPS_UNKNOWN, 0, Parameter::SPC_UNKNOWN);
    param->setStructType(type);
    addParameter(mLocalParameters, param);

    return param;
}

//-----------------------------------------------------------------------------
ParameterPtr Function::resolveLocalParameter(const Parameter::Content content, GpuConstantType type)
{
    ParameterPtr param;

    if(type == GCT_UNKNOWN) type = typeFromContent(content);

    param = _getParameterByContent(mLocalParameters, content, type);
    if (param.get() != NULL)    
        return param;

    param = std::make_shared<Parameter>(
        type, getParameterName("l", semanticFromContent(content), mLocalParameters.size()),
        Parameter::SPS_UNKNOWN, 0, content);
    addParameter(mLocalParameters, param);

    return param;
}

//-----------------------------------------------------------------------------
void Function::addInputParameter(ParameterPtr parameter)
{

    // Check that parameter with the same semantic and index in input parameters list.
    if (_getParameterBySemantic(mInputParameters, parameter->getSemantic(), parameter->getIndex()).get() != NULL)
    {
        OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
            "Parameter <" + parameter->getName() + "> has equal semantic parameter");
    }

    addParameter(mInputParameters, parameter);
}

//-----------------------------------------------------------------------------
void Function::addOutputParameter(ParameterPtr parameter)
{
    // Check that parameter with the same semantic and index in output parameters list.
    if (_getParameterBySemantic(mOutputParameters, parameter->getSemantic(), parameter->getIndex()).get() != NULL)
    {
        OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
            "Parameter <" + parameter->getName() + "> has equal semantic parameter");
    }

    addParameter(mOutputParameters, parameter);
}

//-----------------------------------------------------------------------------
void Function::deleteInputParameter(ParameterPtr parameter)
{
    deleteParameter(mInputParameters, parameter);
}

//-----------------------------------------------------------------------------
void Function::deleteOutputParameter(ParameterPtr parameter)
{
    deleteParameter(mOutputParameters, parameter);
}

//-----------------------------------------------------------------------------
void Function::deleteAllInputParameters()
{
    mInputParameters.clear();
}

//-----------------------------------------------------------------------------
void Function::deleteAllOutputParameters()
{
    mOutputParameters.clear();
}
//-----------------------------------------------------------------------------
void Function::addParameter(ShaderParameterList& parameterList, ParameterPtr parameter)
                                        
{
    // Check that parameter with the same name doest exist in input parameters list.
    if (_getParameterByName(mInputParameters, parameter->getName()).get() != NULL)
    {
        OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
            "Parameter <" + parameter->getName() + "> already declared");
    }

    // Check that parameter with the same name doest exist in output parameters list.
    if (_getParameterByName(mOutputParameters, parameter->getName()).get() != NULL)
    {
        OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
            "Parameter <" + parameter->getName() + "> already declared");
    }


    // Add to given parameters list.
    parameterList.push_back(parameter);
}

//-----------------------------------------------------------------------------
void Function::deleteParameter(ShaderParameterList& parameterList, ParameterPtr parameter)
{
    ShaderParameterIterator it;

    for (it = parameterList.begin(); it != parameterList.end(); ++it)
    {
        if (*it == parameter)
        {
            (*it).reset();
            parameterList.erase(it);
            break;
        }
    }
}

//-----------------------------------------------------------------------------
ParameterPtr Function::_getParameterByName( const ShaderParameterList& parameterList, const String& name )
{
    ShaderParameterConstIterator it;

    for (it = parameterList.begin(); it != parameterList.end(); ++it)
    {
        if ((*it)->getName() == name)
        {
            return *it;
        }
    }

    return ParameterPtr();
}

//-----------------------------------------------------------------------------
ParameterPtr Function::_getParameterBySemantic(const ShaderParameterList& parameterList,
                                                const Parameter::Semantic semantic, 
                                                int index)
{
    ShaderParameterConstIterator it;

    for (it = parameterList.begin(); it != parameterList.end(); ++it)
    {
        if ((*it)->getSemantic() == semantic &&
            (*it)->getIndex() == index)
        {
            return *it;
        }
    }

    return ParameterPtr();
}

//-----------------------------------------------------------------------------
ParameterPtr Function::_getParameterByContent(const ShaderParameterList& parameterList, const Parameter::Content content, GpuConstantType type)
{
    ShaderParameterConstIterator it;

    if(type == GCT_UNKNOWN)
        type = typeFromContent(content);

    // Search only for known content.
    if (content != Parameter::SPC_UNKNOWN)  
    {
        for (it = parameterList.begin(); it != parameterList.end(); ++it)
        {
            if ((*it)->getContent() == content &&
                (*it)->getType() == type)
            {
                return *it;
            }
        }
    }
    
    return ParameterPtr();
}


//-----------------------------------------------------------------------------
void Function::addAtomInstance(FunctionAtom* atomInstance)
{
    mAtomInstances[atomInstance->getGroupExecutionOrder()].push_back(atomInstance);
    mSortedAtomInstances.clear();
}

//-----------------------------------------------------------------------------
bool Function::deleteAtomInstance(FunctionAtom* atomInstance)
{
    FunctionAtomInstanceIterator it;
    size_t g = atomInstance->getGroupExecutionOrder();
    for (it=mAtomInstances[g].begin(); it != mAtomInstances[g].end(); ++it)
    {
        if (*it == atomInstance)
        {
            OGRE_DELETE atomInstance;
            mAtomInstances[g].erase(it);
            mSortedAtomInstances.clear();
            return true;
        }       
    }

    return false;
    
}

//-----------------------------------------------------------------------------
const FunctionAtomInstanceList& Function::getAtomInstances()
{
    if(!mSortedAtomInstances.empty())
        return mSortedAtomInstances;

    // put atom instances into order
    std::map<size_t, FunctionAtomInstanceList>::const_iterator it;
    for(it = mAtomInstances.begin(); it != mAtomInstances.end(); ++it)
    {
        mSortedAtomInstances.insert(mSortedAtomInstances.end(), it->second.begin(),
                                    it->second.end());
    }

    return mSortedAtomInstances;
}

}
}
