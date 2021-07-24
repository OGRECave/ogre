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

    //-----------------------------------------------------------------------
    // Define some ConstParameterTypes
    //-----------------------------------------------------------------------

    /** ConstParameterVec2 represents a Vector2 constant.
    */
    class ConstParameterVec2 : public ConstParameter<Vector2>
    {
    public:
        ConstParameterVec2( Vector2 val, 
            GpuConstantType type, 
            const Semantic& semantic,  
            const Content& content) 
            : ConstParameter<Vector2>(val, type, semantic, content)
        {
        }

        ~ConstParameterVec2() {}

        /** 
        @see Parameter::toString.
        */
        virtual String toString () const
        {
            StringStream str;
            str << "vec2(" << std::showpoint << mValue.x << "," << mValue.y << ")";
            return str.str();
        }
    };

    /** ConstParameterVec3 represents a Vector3 constant.
    */
    class ConstParameterVec3 : public ConstParameter<Vector3>
    {
    public:
        ConstParameterVec3( Vector3 val, 
            GpuConstantType type, 
            const Semantic& semantic,  
            const Content& content) 
            : ConstParameter<Vector3>(val, type, semantic, content)
        {
        }
        ~ConstParameterVec3() {}

        /** 
        @see Parameter::toString.
        */
        virtual String toString () const
        {
            StringStream str;
            str << "vec3(" << std::showpoint << mValue.x << "," << mValue.y << "," << mValue.z << ")";
            return str.str();
        }
    };

    /** ConstParameterVec4 represents a Vector2 Vector4.
    */
    class ConstParameterVec4 : public ConstParameter<Vector4>
    {
    public:
        ConstParameterVec4( Vector4 val, 
            GpuConstantType type, 
            const Semantic& semantic,  
            const Content& content) 
            : ConstParameter<Vector4>(val, type, semantic, content)
        {
        }
        ~ConstParameterVec4() {}

        /** 
        @see Parameter::toString.
        */
        virtual String toString () const
        {
            StringStream str;
            str << "vec4(" << std::showpoint << mValue.x << "," << mValue.y << "," << mValue.z << "," << mValue.w << ")";
            return str.str();
        }
    };

    /** ConstParameterFloat represents a float constant.
    */
    class ConstParameterFloat : public ConstParameter<float>
    {
    public:
        ConstParameterFloat(float val, 
            GpuConstantType type, 
            const Semantic& semantic,  
            const Content& content) 
            : ConstParameter<float>(val, type, semantic, content)
        {
        }

        ~ConstParameterFloat() {}

        /** 
        @see Parameter::toString.
        */
        virtual String toString () const
        {
            return StringConverter::toString(mValue, 6, 0, ' ', std::ios::showpoint);
        }
    };
    /** ConstParameterInt represents an int constant.
    */
    class ConstParameterInt : public ConstParameter<int>
    {
    public:
        ConstParameterInt(int val, 
            GpuConstantType type, 
            const Semantic& semantic,  
            const Content& content) 
            : ConstParameter<int>(val, type, semantic, content)
        {
        }

        ~ConstParameterInt() {}

        /** 
        @see Parameter::toString.
        */
        virtual String toString () const
        {
            return Ogre::StringConverter::toString(mValue);
        }
    };

//-----------------------------------------------------------------------
Parameter::Parameter() : mName(""), mType(GCT_UNKNOWN), mSemantic(SPS_UNKNOWN), mIndex(0), mContent(SPC_UNKNOWN), mSize(0), mUsed(false)
{
}

//-----------------------------------------------------------------------
Parameter::Parameter(GpuConstantType type, const String& name, 
            const Semantic& semantic, int index, 
            const Content& content, size_t size) :
    mName(name), mType(type), mSemantic(semantic), mIndex(index), mContent(content), mSize(size), mUsed(false)
{
}

//-----------------------------------------------------------------------
UniformParameter::UniformParameter(GpuConstantType type, const String& name, 
                 const Semantic& semantic, int index, 
                 const Content& content,
                 uint16 variability, size_t size) : Parameter(type, name, semantic, index, content, size)
{
    mIsAutoConstantReal     = false;    
    mIsAutoConstantInt      = false;
    mAutoConstantIntData    = 0;
    mVariability            = variability;
    mParamsPtr              = NULL;
    mPhysicalIndex          = -1;
    mAutoConstantType       = GpuProgramParameters::ACT_UNKNOWN;
}

static GpuConstantType getGCType(const GpuProgramParameters::AutoConstantDefinition* def)
{
    assert(def->elementType == GpuProgramParameters::ET_REAL);

    switch (def->elementCount)
    {
    default:
    case 1:
        return GCT_FLOAT1;
    case 2:
        return GCT_FLOAT2;
    case 3:
        return GCT_FLOAT3;
    case 4:
        return GCT_FLOAT4;
    case 8:
        return GCT_MATRIX_2X4;
    case 9:
        return GCT_MATRIX_3X3;
    case 12:
        return GCT_MATRIX_3X4;
    case 16:
        return GCT_MATRIX_4X4;
    }
}

//-----------------------------------------------------------------------
UniformParameter::UniformParameter(GpuProgramParameters::AutoConstantType autoType, float fAutoConstantData, size_t size)
{
    auto parameterDef = GpuProgramParameters::getAutoConstantDefinition(autoType);
    assert(parameterDef);

    mName               = parameterDef->name;
    if (fAutoConstantData != 0.0)
    {
        mName += StringConverter::toString(fAutoConstantData);
        //replace possible illegal point character in name
        std::replace(mName.begin(), mName.end(), '.', '_'); 
    }
    mType               = getGCType(parameterDef);
    mSemantic           = SPS_UNKNOWN;
    mIndex              = -1;
    mContent            = SPC_UNKNOWN;
    mIsAutoConstantReal = true; 
    mIsAutoConstantInt  = false;
    mAutoConstantType   = autoType;
    mAutoConstantRealData = fAutoConstantData;
    mVariability        = (uint16)GPV_GLOBAL;
    mParamsPtr           = NULL;
    mPhysicalIndex       = -1;
    mSize                = size;
}

//-----------------------------------------------------------------------
UniformParameter::UniformParameter(GpuProgramParameters::AutoConstantType autoType, float fAutoConstantData, size_t size, GpuConstantType type)
{
    auto parameterDef = GpuProgramParameters::getAutoConstantDefinition(autoType);
    assert(parameterDef);

    mName               = parameterDef->name;
    if (fAutoConstantData != 0.0)
    {
        mName += StringConverter::toString(fAutoConstantData);
        //replace possible illegal point character in name
        std::replace(mName.begin(), mName.end(), '.', '_'); 
    }
    mType               = type;
    mSemantic           = SPS_UNKNOWN;
    mIndex              = -1;
    mContent            = SPC_UNKNOWN;
    mIsAutoConstantReal = true; 
    mIsAutoConstantInt  = false;
    mAutoConstantType   = autoType;
    mAutoConstantRealData = fAutoConstantData;
    mVariability        = (uint16)GPV_GLOBAL;
    mParamsPtr           = NULL;
    mPhysicalIndex       = -1;
    mSize                = size;
}

//-----------------------------------------------------------------------
UniformParameter::UniformParameter(GpuProgramParameters::AutoConstantType autoType, uint32 nAutoConstantData, size_t size)
{
    auto parameterDef = GpuProgramParameters::getAutoConstantDefinition(autoType);
    assert(parameterDef);

    mName               = parameterDef->name;
    if (nAutoConstantData != 0)
        mName += StringConverter::toString(nAutoConstantData);
    mType               = getGCType(parameterDef);
    mSemantic           = SPS_UNKNOWN;
    mIndex              = -1;
    mContent            = SPC_UNKNOWN;
    mIsAutoConstantReal = false;    
    mIsAutoConstantInt  = true;
    mAutoConstantType   = autoType;
    mAutoConstantIntData = nAutoConstantData;
    mVariability        = (uint16)GPV_GLOBAL;
    mParamsPtr           = NULL;
    mPhysicalIndex       = -1;
    mSize                = size;
}

//-----------------------------------------------------------------------
UniformParameter::UniformParameter(GpuProgramParameters::AutoConstantType autoType, uint32 nAutoConstantData, size_t size, GpuConstantType type)
{
    auto parameterDef = GpuProgramParameters::getAutoConstantDefinition(autoType);
    assert(parameterDef);

    mName               = parameterDef->name;
    if (nAutoConstantData != 0)
        mName += StringConverter::toString(nAutoConstantData);
    mType               = type;
    mSemantic           = SPS_UNKNOWN;
    mIndex              = -1;
    mContent            = SPC_UNKNOWN;
    mIsAutoConstantReal = false;    
    mIsAutoConstantInt  = true;
    mAutoConstantType   = autoType;
    mAutoConstantIntData = nAutoConstantData;
    mVariability        = (uint16)GPV_GLOBAL;
    mParamsPtr           = NULL;
    mPhysicalIndex       = -1;
    mSize                = size;
}

//-----------------------------------------------------------------------
void UniformParameter::bind(GpuProgramParametersSharedPtr paramsPtr)
{
    // do not throw on failure: some RS optimize unused uniforms away. Also unit tests run without any RS
    const GpuConstantDefinition* def = paramsPtr->_findNamedConstantDefinition(mBindName.empty() ? mName : mBindName, false);

    if (def != NULL)
    {
        mParamsPtr = paramsPtr.get();
        mPhysicalIndex = def->physicalIndex;
        mElementSize = def->elementSize;
        mVariability = def->variability;
    }
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInPosition(int index, Parameter::Content content)
{
    return std::make_shared<Parameter>(GCT_FLOAT4, "iPos_" + StringConverter::toString(index),
                                       Parameter::SPS_POSITION, index,
                                       content);
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createOutPosition(int index)
{
    return ParameterPtr(OGRE_NEW Parameter(GCT_FLOAT4, "oPos_" + StringConverter::toString(index), 
        Parameter::SPS_POSITION, index, 
        Parameter::SPC_POSITION_PROJECTIVE_SPACE));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInNormal(int index)
{
    return ParameterPtr(OGRE_NEW Parameter(GCT_FLOAT3, "iNormal_" + StringConverter::toString(index), 
        Parameter::SPS_NORMAL, index, 
        Parameter::SPC_NORMAL_OBJECT_SPACE));
}


//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInWeights(int index)
{
    return ParameterPtr(OGRE_NEW Parameter(GCT_FLOAT4, "iBlendWeights_" + StringConverter::toString(index), 
        Parameter::SPS_BLEND_WEIGHTS, index, 
        Parameter::SPC_BLEND_WEIGHTS));
}


//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInIndices(int index)
{
	return ParameterPtr(OGRE_NEW Parameter(
		GCT_UINT4
	, "iBlendIndices_" + StringConverter::toString(index), 
        Parameter::SPS_BLEND_INDICES, index, 
        Parameter::SPC_BLEND_INDICES));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInBiNormal(int index)
{
    return ParameterPtr(OGRE_NEW Parameter(GCT_FLOAT3, "iBiNormal_" + StringConverter::toString(index), 
        Parameter::SPS_BINORMAL, index, 
        Parameter::SPC_BINORMAL_OBJECT_SPACE));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInTangent(int index)
{
    return ParameterPtr(OGRE_NEW Parameter(GCT_FLOAT3, "iTangent_" + StringConverter::toString(index), 
        Parameter::SPS_TANGENT, index, 
        Parameter::SPC_TANGENT_OBJECT_SPACE));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createOutNormal(int index)
{
    return ParameterPtr(OGRE_NEW Parameter(GCT_FLOAT3, "oNormal_" + StringConverter::toString(index), 
        Parameter::SPS_NORMAL, index, 
        Parameter::SPC_NORMAL_OBJECT_SPACE));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createOutBiNormal(int index)
{
    return ParameterPtr(OGRE_NEW Parameter(GCT_FLOAT3, "oBiNormal_" + StringConverter::toString(index), 
        Parameter::SPS_BINORMAL, index, 
        Parameter::SPC_BINORMAL_OBJECT_SPACE));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createOutTangent(int index)
{
    return ParameterPtr(OGRE_NEW Parameter(GCT_FLOAT3, "oTangent_" + StringConverter::toString(index), 
        Parameter::SPS_TANGENT, index, 
        Parameter::SPC_TANGENT_OBJECT_SPACE));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInColor(int index)
{
    return ParameterPtr(OGRE_NEW Parameter(GCT_FLOAT4, "iColor_" + StringConverter::toString(index), 
        Parameter::SPS_COLOR, index, 
        index == 0 ? Parameter::SPC_COLOR_DIFFUSE : Parameter::SPC_COLOR_SPECULAR));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createOutColor(int index)
{
    return ParameterPtr(OGRE_NEW Parameter(GCT_FLOAT4, "oColor_" + StringConverter::toString(index), 
        Parameter::SPS_COLOR, index, 
        index == 0 ? Parameter::SPC_COLOR_DIFFUSE : Parameter::SPC_COLOR_SPECULAR));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInTexcoord(GpuConstantType type, int index, Parameter::Content content)
{
    switch (type)
    {
    case GCT_FLOAT1:
    case GCT_FLOAT2:
    case GCT_FLOAT3:
    case GCT_FLOAT4:
    case GCT_MATRIX_2X2:
    case GCT_MATRIX_2X3:
    case GCT_MATRIX_2X4:
    case GCT_MATRIX_3X2:
    case GCT_MATRIX_3X3:
    case GCT_MATRIX_3X4:
    case GCT_MATRIX_4X2:
    case GCT_MATRIX_4X3:
    case GCT_MATRIX_4X4:
    case GCT_INT1:
    case GCT_INT2:
    case GCT_INT3:
    case GCT_INT4:
    case GCT_UINT1:
    case GCT_UINT2:
    case GCT_UINT3:
    case GCT_UINT4:
        return std::make_shared<Parameter>(type, StringUtil::format("iTexcoord_%d", index),
                                           Parameter::SPS_TEXTURE_COORDINATES, index, content);
    default:
    case GCT_SAMPLER1D:
    case GCT_SAMPLER2D:
    case GCT_SAMPLER2DARRAY:
    case GCT_SAMPLER3D:
    case GCT_SAMPLERCUBE:
    case GCT_SAMPLER1DSHADOW:
    case GCT_SAMPLER2DSHADOW:
    case GCT_UNKNOWN:
        break;
    }

    return ParameterPtr();
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createOutTexcoord(GpuConstantType type, int index, Parameter::Content content)
{
    switch (type)
    {
    case GCT_FLOAT1:
    case GCT_FLOAT2:
    case GCT_FLOAT3:
    case GCT_FLOAT4:
        return std::make_shared<Parameter>(type, StringUtil::format("oTexcoord_%d", index),
                                           Parameter::SPS_TEXTURE_COORDINATES, index, content);
    default:
    case GCT_SAMPLER1D:
    case GCT_SAMPLER2D:
    case GCT_SAMPLER2DARRAY:
    case GCT_SAMPLER3D:
    case GCT_SAMPLERCUBE:
    case GCT_SAMPLER1DSHADOW:
    case GCT_SAMPLER2DSHADOW:
    case GCT_MATRIX_2X2:
    case GCT_MATRIX_2X3:
    case GCT_MATRIX_2X4:
    case GCT_MATRIX_3X2:
    case GCT_MATRIX_3X3:
    case GCT_MATRIX_3X4:
    case GCT_MATRIX_4X2:
    case GCT_MATRIX_4X3:
    case GCT_MATRIX_4X4:
    case GCT_INT1:
    case GCT_INT2:
    case GCT_INT3:
    case GCT_INT4:
    case GCT_UINT1:
    case GCT_UINT2:
    case GCT_UINT3:
    case GCT_UINT4:
    case GCT_UNKNOWN:
        break;
    }

    return ParameterPtr();
}

//-----------------------------------------------------------------------
UniformParameterPtr ParameterFactory::createSampler(GpuConstantType type, int index)
{
    switch (type)
    {
    case GCT_SAMPLER1D:
        return createSampler1D(index);

    case GCT_SAMPLER2D:
        return createSampler2D(index);

    case GCT_SAMPLER2DARRAY:
        return createSampler2DArray(index);

    case GCT_SAMPLER3D:
        return createSampler3D(index);

    case GCT_SAMPLERCUBE:
        return createSamplerCUBE(index);

    default:
    case GCT_SAMPLER1DSHADOW:
    case GCT_SAMPLER2DSHADOW:
    case GCT_MATRIX_2X2:
    case GCT_MATRIX_2X3:
    case GCT_MATRIX_2X4:
    case GCT_MATRIX_3X2:
    case GCT_MATRIX_3X3:
    case GCT_MATRIX_3X4:
    case GCT_MATRIX_4X2:
    case GCT_MATRIX_4X3:
    case GCT_MATRIX_4X4:
    case GCT_INT1:
    case GCT_INT2:
    case GCT_INT3:
    case GCT_INT4:
    case GCT_UINT1:
    case GCT_UINT2:
    case GCT_UINT3:
    case GCT_UINT4:
    case GCT_UNKNOWN:
        break;
    }

    return UniformParameterPtr();
    
}

//-----------------------------------------------------------------------
UniformParameterPtr ParameterFactory::createSampler1D(int index)
{
    return UniformParameterPtr(OGRE_NEW UniformParameter(GCT_SAMPLER1D, "gSampler1D_" + StringConverter::toString(index), 
        Parameter::SPS_UNKNOWN, index, 
        Parameter::SPC_UNKNOWN,
        (uint16)GPV_GLOBAL, 1));
}

//-----------------------------------------------------------------------
UniformParameterPtr ParameterFactory::createSampler2D(int index)
{
    return UniformParameterPtr(OGRE_NEW UniformParameter(GCT_SAMPLER2D, "gSampler2D_" + StringConverter::toString(index), 
        Parameter::SPS_UNKNOWN, index, 
        Parameter::SPC_UNKNOWN,
        (uint16)GPV_GLOBAL, 1));
}

//-----------------------------------------------------------------------
UniformParameterPtr ParameterFactory::createSampler2DArray(int index)
{
    return UniformParameterPtr(OGRE_NEW UniformParameter(GCT_SAMPLER2DARRAY, "gSampler2DArray_" + StringConverter::toString(index), 
                                                         Parameter::SPS_UNKNOWN, index, 
                                                         Parameter::SPC_UNKNOWN,
                                                         (uint16)GPV_GLOBAL, 1));
}

//-----------------------------------------------------------------------
UniformParameterPtr ParameterFactory::createSampler3D(int index)
{
    return UniformParameterPtr(OGRE_NEW UniformParameter(GCT_SAMPLER3D, "gSampler3D_" + StringConverter::toString(index), 
        Parameter::SPS_UNKNOWN, index, 
        Parameter::SPC_UNKNOWN,
        (uint16)GPV_GLOBAL, 1));
}

//-----------------------------------------------------------------------
UniformParameterPtr ParameterFactory::createSamplerCUBE(int index)
{
    return UniformParameterPtr(OGRE_NEW UniformParameter(GCT_SAMPLERCUBE, "gSamplerCUBE_" + StringConverter::toString(index), 
        Parameter::SPS_UNKNOWN, index, 
        Parameter::SPC_UNKNOWN,
        (uint16)GPV_GLOBAL, 1));
}
//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createConstParam(const Vector2& val)
{
    return ParameterPtr(OGRE_NEW ConstParameterVec2(val, GCT_FLOAT2, Parameter::SPS_UNKNOWN,
                                                    Parameter::SPC_UNKNOWN));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createConstParam(const Vector3& val)
{
    return ParameterPtr(OGRE_NEW ConstParameterVec3(val, GCT_FLOAT3, Parameter::SPS_UNKNOWN,
                                                    Parameter::SPC_UNKNOWN));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createConstParam(const Vector4& val)
{
    return ParameterPtr(OGRE_NEW ConstParameterVec4(val, GCT_FLOAT4, Parameter::SPS_UNKNOWN,
                                                    Parameter::SPC_UNKNOWN));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createConstParam(float val)
{
    return ParameterPtr(OGRE_NEW ConstParameterFloat(val, GCT_FLOAT1, Parameter::SPS_UNKNOWN,
                                                     Parameter::SPC_UNKNOWN));
}

//-----------------------------------------------------------------------
UniformParameterPtr ParameterFactory::createUniform(GpuConstantType type, 
                                             int index, uint16 variability,
                                             const String& suggestedName,
                                             size_t size)
{
    UniformParameterPtr param;
    
    param = UniformParameterPtr(OGRE_NEW UniformParameter(type, suggestedName + StringConverter::toString(index), 
        Parameter::SPS_UNKNOWN, index, 
        Parameter::SPC_UNKNOWN, variability, size));
        
    return param;
}

}
}
