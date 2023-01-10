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
#ifndef _ShaderParameter_
#define _ShaderParameter_

#include "OgreShaderPrerequisites.h"
#include "OgreVector.h"
#include "OgreMatrix4.h"
#include "OgreGpuProgramParams.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** A class that represents a shader based program parameter.
*/
class _OgreRTSSExport Parameter : public RTShaderSystemAlloc
{
public:
    // Shader parameter semantic.
    enum Semantic
    {
        /// Unknown semantic
        SPS_UNKNOWN = 0,
        /// Position
        SPS_POSITION = 1,
        /// Blending weights
        SPS_BLEND_WEIGHTS = 2,
        /// Blending indices
        SPS_BLEND_INDICES = 3,
        /// Normal, 3 reals per vertex
        SPS_NORMAL = 4,
        /// General floating point color.
        SPS_COLOR = 5,      
        /// Texture coordinates
        SPS_TEXTURE_COORDINATES = 7,
        /// Binormal (Y axis if normal is Z)
        SPS_BINORMAL = 8,
        /// Tangent (X axis if normal is Z)
        SPS_TANGENT = 9,
        /// VFACE
        SPS_FRONT_FACING
    };

    /** Shader parameter content
     * 
     * used to resolve Parameters across different SubRenderState instances
     * Think of it as Semantic extended to the actual parameter content.
     */ 
    enum Content
    {
        /// Unknown content
        SPC_UNKNOWN,

        /// Position in object space
        SPC_POSITION_OBJECT_SPACE,

        /// Position in world space
        SPC_POSITION_WORLD_SPACE,

        /// Position in view space
        SPC_POSITION_VIEW_SPACE,

        /// Position in projective space
        SPC_POSITION_PROJECTIVE_SPACE,

        /// Position in light space index 0-7
        SPC_POSITION_LIGHT_SPACE0,
        SPC_POSITION_LIGHT_SPACE1,
        SPC_POSITION_LIGHT_SPACE2,
        SPC_POSITION_LIGHT_SPACE3,
        SPC_POSITION_LIGHT_SPACE4,
        SPC_POSITION_LIGHT_SPACE5,
        SPC_POSITION_LIGHT_SPACE6,
        SPC_POSITION_LIGHT_SPACE7,

        /// Normal in object space
        SPC_NORMAL_OBJECT_SPACE,

        /// Normal in world space
        SPC_NORMAL_WORLD_SPACE,

        /// Normal in view space
        SPC_NORMAL_VIEW_SPACE,

        /// Normal in tangent space
        SPC_NORMAL_TANGENT_SPACE,

        /// View vector in object space
        SPC_POSTOCAMERA_OBJECT_SPACE,

        /// View vector in world space
        SPC_POSTOCAMERA_WORLD_SPACE,

        /// View vector in view space
        SPC_POSTOCAMERA_VIEW_SPACE,

        /// View vector in tangent space
        SPC_POSTOCAMERA_TANGENT_SPACE,

        /// Light vector in object space index 0-7
        SPC_POSTOLIGHT_OBJECT_SPACE0,
        SPC_POSTOLIGHT_OBJECT_SPACE1,
        SPC_POSTOLIGHT_OBJECT_SPACE2,
        SPC_POSTOLIGHT_OBJECT_SPACE3,
        SPC_POSTOLIGHT_OBJECT_SPACE4,
        SPC_POSTOLIGHT_OBJECT_SPACE5,
        SPC_POSTOLIGHT_OBJECT_SPACE6,
        SPC_POSTOLIGHT_OBJECT_SPACE7,

        /// Light vector in world space index 0-7
        SPC_POSTOLIGHT_WORLD_SPACE0,
        SPC_POSTOLIGHT_WORLD_SPACE1,
        SPC_POSTOLIGHT_WORLD_SPACE2,
        SPC_POSTOLIGHT_WORLD_SPACE3,
        SPC_POSTOLIGHT_WORLD_SPACE4,
        SPC_POSTOLIGHT_WORLD_SPACE5,
        SPC_POSTOLIGHT_WORLD_SPACE6,
        SPC_POSTOLIGHT_WORLD_SPACE7,

        /// Light vector in view space index 0-7
        SPC_POSTOLIGHT_VIEW_SPACE0,
        SPC_POSTOLIGHT_VIEW_SPACE1,
        SPC_POSTOLIGHT_VIEW_SPACE2,
        SPC_POSTOLIGHT_VIEW_SPACE3,
        SPC_POSTOLIGHT_VIEW_SPACE4,
        SPC_POSTOLIGHT_VIEW_SPACE5,
        SPC_POSTOLIGHT_VIEW_SPACE6,
        SPC_POSTOLIGHT_VIEW_SPACE7,

        /// Light vector in tangent space index 0-7
        SPC_POSTOLIGHT_TANGENT_SPACE0,
        SPC_POSTOLIGHT_TANGENT_SPACE1,
        SPC_POSTOLIGHT_TANGENT_SPACE2,
        SPC_POSTOLIGHT_TANGENT_SPACE3,
        SPC_POSTOLIGHT_TANGENT_SPACE4,
        SPC_POSTOLIGHT_TANGENT_SPACE5,
        SPC_POSTOLIGHT_TANGENT_SPACE6,
        SPC_POSTOLIGHT_TANGENT_SPACE7,

        /// Light direction in object space index 0-7
        SPC_LIGHTDIRECTION_OBJECT_SPACE0,
        SPC_LIGHTDIRECTION_OBJECT_SPACE1,
        SPC_LIGHTDIRECTION_OBJECT_SPACE2,
        SPC_LIGHTDIRECTION_OBJECT_SPACE3,
        SPC_LIGHTDIRECTION_OBJECT_SPACE4,
        SPC_LIGHTDIRECTION_OBJECT_SPACE5,
        SPC_LIGHTDIRECTION_OBJECT_SPACE6,
        SPC_LIGHTDIRECTION_OBJECT_SPACE7,

        /// Light direction in world space index 0-7
        SPC_LIGHTDIRECTION_WORLD_SPACE0,
        SPC_LIGHTDIRECTION_WORLD_SPACE1,
        SPC_LIGHTDIRECTION_WORLD_SPACE2,
        SPC_LIGHTDIRECTION_WORLD_SPACE3,
        SPC_LIGHTDIRECTION_WORLD_SPACE4,
        SPC_LIGHTDIRECTION_WORLD_SPACE5,
        SPC_LIGHTDIRECTION_WORLD_SPACE6,
        SPC_LIGHTDIRECTION_WORLD_SPACE7,

        /// Light direction in view space index 0-7
        SPC_LIGHTDIRECTION_VIEW_SPACE0,
        SPC_LIGHTDIRECTION_VIEW_SPACE1,
        SPC_LIGHTDIRECTION_VIEW_SPACE2,
        SPC_LIGHTDIRECTION_VIEW_SPACE3,
        SPC_LIGHTDIRECTION_VIEW_SPACE4,
        SPC_LIGHTDIRECTION_VIEW_SPACE5,
        SPC_LIGHTDIRECTION_VIEW_SPACE6,
        SPC_LIGHTDIRECTION_VIEW_SPACE7,

        /// Light direction in tangent space index 0-7
        SPC_LIGHTDIRECTION_TANGENT_SPACE0,
        SPC_LIGHTDIRECTION_TANGENT_SPACE1,
        SPC_LIGHTDIRECTION_TANGENT_SPACE2,
        SPC_LIGHTDIRECTION_TANGENT_SPACE3,
        SPC_LIGHTDIRECTION_TANGENT_SPACE4,
        SPC_LIGHTDIRECTION_TANGENT_SPACE5,
        SPC_LIGHTDIRECTION_TANGENT_SPACE6,
        SPC_LIGHTDIRECTION_TANGENT_SPACE7,

        /// Light position in object space index 0-7
        SPC_LIGHTPOSITION_OBJECT_SPACE0,
        SPC_LIGHTPOSITION_OBJECT_SPACE1,
        SPC_LIGHTPOSITION_OBJECT_SPACE2,
        SPC_LIGHTPOSITION_OBJECT_SPACE3,
        SPC_LIGHTPOSITION_OBJECT_SPACE4,
        SPC_LIGHTPOSITION_OBJECT_SPACE5,
        SPC_LIGHTPOSITION_OBJECT_SPACE6,
        SPC_LIGHTPOSITION_OBJECT_SPACE7,

        /// Light position in world space index 0-7
        SPC_LIGHTPOSITION_WORLD_SPACE0,
        SPC_LIGHTPOSITION_WORLD_SPACE1,
        SPC_LIGHTPOSITION_WORLD_SPACE2,
        SPC_LIGHTPOSITION_WORLD_SPACE3,
        SPC_LIGHTPOSITION_WORLD_SPACE4,
        SPC_LIGHTPOSITION_WORLD_SPACE5,
        SPC_LIGHTPOSITION_WORLD_SPACE6,
        SPC_LIGHTPOSITION_WORLD_SPACE7,

        /// Light position in view space index 0-7
        SPC_LIGHTPOSITIONVIEW_SPACE0,
        SPC_LIGHTPOSITIONVIEW_SPACE1,
        SPC_LIGHTPOSITIONVIEW_SPACE2,
        SPC_LIGHTPOSITIONVIEW_SPACE3,
        SPC_LIGHTPOSITIONVIEW_SPACE4,
        SPC_LIGHTPOSITIONVIEW_SPACE5,
        SPC_LIGHTPOSITIONVIEW_SPACE6,
        SPC_LIGHTPOSITIONVIEW_SPACE7,

        /// Light position in tangent space index 0-7
        SPC_LIGHTPOSITION_TANGENT_SPACE,

        /// Blending weights
        SPC_BLEND_WEIGHTS,

        /// Blending indices
        SPC_BLEND_INDICES,
        
        /// Tangent in object space
        SPC_TANGENT_OBJECT_SPACE,

        /// Tangent in world space
        SPC_TANGENT_WORLD_SPACE,

        /// Tangent in view space
        SPC_TANGENT_VIEW_SPACE,

        /// Tangent in tangent space
        SPC_TANGENT_TANGENT_SPACE,

        /// Binormal in object space
        SPC_BINORMAL_OBJECT_SPACE,

        /// Binormal in world space
        SPC_BINORMAL_WORLD_SPACE,

        /// Binormal in view space
        SPC_BINORMAL_VIEW_SPACE,

        /// Binormal in tangent space
        SPC_BINORMAL_TANGENT_SPACE,

        /// Diffuse color
        SPC_COLOR_DIFFUSE,

        /// Specular color
        SPC_COLOR_SPECULAR,

        /// Depth in object space
        SPC_DEPTH_OBJECT_SPACE,

        /// Depth in world space
        SPC_DEPTH_WORLD_SPACE,

        /// Depth in view space
        SPC_DEPTH_VIEW_SPACE,

        /// Depth in projective space
        SPC_DEPTH_PROJECTIVE_SPACE,

        /// Texture coordinate set index 0-7
        SPC_TEXTURE_COORDINATE0,        
        SPC_TEXTURE_COORDINATE1,        
        SPC_TEXTURE_COORDINATE2,        
        SPC_TEXTURE_COORDINATE3,    
        SPC_TEXTURE_COORDINATE4,
        SPC_TEXTURE_COORDINATE5,
        SPC_TEXTURE_COORDINATE6,
        SPC_TEXTURE_COORDINATE7,
		
        /// point sprite coordinates
        SPC_POINTSPRITE_COORDINATE,

        /// point sprite size
        SPC_POINTSPRITE_SIZE,

        /// gl_FrontFacing
        SPC_FRONT_FACING,

        /// Reserved custom content range to be used by user custom shader extensions.
        SPC_CUSTOM_CONTENT_BEGIN    = 1000,
        SPC_CUSTOM_CONTENT_END      = 2000
    };

// Interface.
public:
    /** */
    Parameter();

    /** Class constructor.
    @param type The type of this parameter.
    @param name The name of this parameter.
    @param semantic The semantic of this parameter.
    @param index The index of this parameter.
    @param content The content of this parameter.
    @param size
    */
    Parameter(GpuConstantType type, const String& name, 
        const Semantic& semantic, int index, 
        const Content& content, size_t size = 0);

    /** Class destructor */
    virtual ~Parameter() {};

    /** Get the name of this parameter. */
    const String& getName() const { return mName; }

    /// internal function for aliasing to GLSL builtins e.g. gl_Position
    void _rename(const String& newName, bool onlyLocal = false)
    {
        if(onlyLocal)
            mBindName = mName;
        mName = newName;
    }

    /** Get the type of this parameter. */
    GpuConstantType getType() const { return mType; }

    /** Get the semantic of this parameter. */
    const Semantic& getSemantic() const { return mSemantic; }

    /** Get the index of this parameter. */
    int getIndex() const { return mIndex; } 

    /** Return the content of this parameter. */
    Content getContent() const { return mContent; }

    /** Returns true if this instance is a ConstParameter otherwise false. */
    virtual bool isConstParameter() const { return false; }

    /** Returns the string representation of this parameter. */
    virtual String toString() const { return mName; }
    
    /** Returns Whether this parameter is an array. */
    bool isArray() const { return mSize > 0; }

    /** Returns the number of elements in the parameter (for arrays). */
    size_t getSize() const { return mSize; }
    
    /** Sets the number of elements in the parameter (for arrays). */
    void setSize(size_t size) { mSize = size; }

    /// track whether this was used
    void setUsed(bool used) { mUsed = used; }
    bool isUsed() { return mUsed; }

    /// Is highp needed when using GLSL ES
    bool isHighP() const { return mIsHighP; }
    void setHighP(bool highP) { mIsHighP = highP; }

    const String& getStructType() const { return mStructType; }
    void setStructType(const String& structType) { mStructType = structType; }

// Attributes.
protected:
    // Name of this parameter.
    String mName;

    // only used for local renaming
    String mBindName;

    // Type of this parameter.
    GpuConstantType mType;

    // Type, if this is a struct
    String mStructType;

    // Semantic of this parameter.
    Semantic mSemantic;
    // Index of this parameter.
    int mIndex;
    // The content of this parameter.
    Content mContent;
    // Number of elements in the parameter (for arrays)
    size_t mSize;
    
    bool mUsed;
    bool mIsHighP;
};

typedef ShaderParameterList::iterator           ShaderParameterIterator;
typedef ShaderParameterList::const_iterator     ShaderParameterConstIterator;

/** Uniform parameter class. Allow fast access to GPU parameter updates.
*/
class _OgreRTSSExport UniformParameter : public Parameter
{
public:

    /** Class constructor.
    @param type The type of this parameter.
    @param name The name of this parameter.
    @param semantic The semantic of this parameter.
    @param index The index of this parameter.
    @param content The content of this parameter.
    @param variability How this parameter varies (bitwise combination of GpuProgramVariability).
    @param size number of elements in the parameter.    
    */
    UniformParameter(GpuConstantType type, const String& name, 
        const Semantic& semantic, int index, 
        const Content& content,
        uint16 variability, size_t size);

    /** Class constructor.
    @param autoType The auto type of this parameter.
    @param fAutoConstantData The real data for this auto constant parameter.    
    @param size number of elements in the parameter.    
    */
    UniformParameter(GpuProgramParameters::AutoConstantType autoType, float fAutoConstantData, size_t size);
    
    /** Class constructor.
    @param autoType The auto type of this parameter.
    @param fAutoConstantData The real data for this auto constant parameter.    
    @param size number of elements in the parameter.
    @param type The desired data type of this auto constant parameter.
    */
    UniformParameter(GpuProgramParameters::AutoConstantType autoType, float fAutoConstantData, size_t size, GpuConstantType type);

    /** Class constructor.
    @param autoType The auto type of this parameter.
    @param nAutoConstantData The int data for this auto constant parameter. 
    @param size number of elements in the parameter.    
    */
    UniformParameter(GpuProgramParameters::AutoConstantType autoType, uint32 nAutoConstantData, size_t size);
    
    /** Class constructor.
    @param autoType The auto type of this parameter.
    @param nAutoConstantData The int data for this auto constant parameter. 
    @param size number of elements in the parameter.
    @param type The desired data type of this auto constant parameter.
    */
    UniformParameter(GpuProgramParameters::AutoConstantType autoType, uint32 nAutoConstantData, size_t size, GpuConstantType type);

    
    /** Get auto constant int data of this parameter, in case it is auto constant parameter. */
    uint32 getAutoConstantIntData() const { return isArray() ? getSize() : mAutoConstantIntData; }

    /** Get auto constant real data of this parameter, in case it is auto constant parameter. */
    float getAutoConstantRealData() const { return mAutoConstantRealData; }

    /** Return true if this parameter is a floating point type, false otherwise. */
    bool isFloat() const { return GpuConstantDefinition::isFloat(mType); }

    /** Return true if this parameter is a texture sampler type, false otherwise. */
    bool isSampler() const { return GpuConstantDefinition::isSampler(mType); }

    /** Return true if this parameter is an auto constant parameter, false otherwise. */
    bool isAutoConstantParameter() const { return mIsAutoConstantReal || mIsAutoConstantInt; }

    /** Return true if this parameter an auto constant with int data type, false otherwise. */
    bool isAutoConstantIntParameter() const { return mIsAutoConstantInt; }

    /** Return true if this parameter an auto constant with real data type, false otherwise. */
    bool isAutoConstantRealParameter() const { return mIsAutoConstantReal; }

    /** Return the auto constant type of this parameter. */
    GpuProgramParameters::AutoConstantType getAutoConstantType  () const { return mAutoConstantType; }

    /** Return the variability of this parameter. */
    uint16 getVariability() const { return mVariability; }

    /** Bind this parameter to the corresponding GPU parameter. */
    void bind(GpuProgramParametersSharedPtr paramsPtr);

public:

    /** Update the GPU parameter with the given value. */   
    void setGpuParameter(int val)
    { 
        if (mParamsPtr != NULL)
        {
            mParamsPtr->_writeRawConstant(mPhysicalIndex, val);
        }
    }

    /** Update the GPU parameter with the given value. */   
    void setGpuParameter(Real val)  
    { 
        if (mParamsPtr != NULL)
        {
            mParamsPtr->_writeRawConstant(mPhysicalIndex, val);
        }
    }

    /** Update the GPU parameter with the given value. */   
    void setGpuParameter(const ColourValue& val)  
    { 
        if (mParamsPtr != NULL)
        {
            mParamsPtr->_writeRawConstant(mPhysicalIndex, val);
        }
    }

    /** Update the GPU parameter with the given value. */   
    void setGpuParameter(const Vector2& val)  
    { 
        if (mParamsPtr != NULL)
        {
            mParamsPtr->_writeRawConstants(mPhysicalIndex, val.ptr(), 2);
        }
    }
    
    /** Update the GPU parameter with the given value. */   
    void setGpuParameter(const Vector3& val)  
    { 
        if (mParamsPtr != NULL)
        {
            mParamsPtr->_writeRawConstant(mPhysicalIndex, val);
        }
    }

    /** Update the GPU parameter with the given value. */   
    void setGpuParameter(const Vector4& val)  
    { 
        if (mParamsPtr != NULL)
        {
            mParamsPtr->_writeRawConstant(mPhysicalIndex, val);     
        }
    }

    void setGpuParameter(const Matrix3& val)
    {
        if (mParamsPtr == NULL) return;

        if(mElementSize == 9) // check if tight packing is supported
        {
            mParamsPtr->_writeRawConstant(mPhysicalIndex, val, 9);
        }
        else
        {
            mParamsPtr->_writeRawConstant(mPhysicalIndex, Matrix4(val), mElementSize);
        }
    }

    /** Update the GPU parameter with the given value. */   
    void setGpuParameter(const Matrix4& val)  
    { 
        if (mParamsPtr != NULL)
        {
            mParamsPtr->_writeRawConstant(mPhysicalIndex, val, 16);
        }
    }

    /** Update the GPU parameter with the given value. */   
    void setGpuParameter(const float *val, size_t count, size_t multiple = 4)  
    { 
        if (mParamsPtr != NULL)
        {
            mParamsPtr->_writeRawConstants(mPhysicalIndex, val, count * multiple);
        }
    }

    /** Update the GPU parameter with the given value. */   
    void setGpuParameter(const double *val, size_t count, size_t multiple = 4)  
    { 
        if (mParamsPtr != NULL)
        {
            mParamsPtr->_writeRawConstants(mPhysicalIndex, val, count * multiple);
        }
    }

    /** Update the GPU parameter with the given value. */   
    void setGpuParameter(const int *val, size_t count, size_t multiple = 4)  
    { 
        if (mParamsPtr != NULL)
        {
            mParamsPtr->_writeRawConstants(mPhysicalIndex, val, count * multiple);
        }
    }

    /// light index or array size
    void updateExtraInfo(uint32 data)
    {
        if (!mParamsPtr)
            return;

        mParamsPtr->_setRawAutoConstant(mPhysicalIndex, mAutoConstantType, data, mVariability,
                                        mElementSize);
    }

private:
    // Is it auto constant real based parameter.
    bool mIsAutoConstantReal;
    // Is it auto constant int based parameter.
    bool mIsAutoConstantInt;
    GpuProgramParameters::AutoConstantType  mAutoConstantType;      // The auto constant type of this parameter.
    union
    {
        // Auto constant int data.
        uint32 mAutoConstantIntData;
        // Auto constant real data.
        float mAutoConstantRealData;
    };      
    // How this parameter varies (bitwise combination of GpuProgramVariability).
    uint16 mVariability;
    // The actual GPU parameters pointer.
    GpuProgramParameters* mParamsPtr;
    // The physical index of this parameter in the GPU program.
    size_t mPhysicalIndex;
    // The size of this parameter in the GPU program
    uint8 mElementSize;
};

typedef std::vector<UniformParameterPtr>       UniformParameterList;
typedef UniformParameterList::iterator          UniformParameterIterator;
typedef UniformParameterList::const_iterator    UniformParameterConstIterator;

/** Helper template which is the base for our ConstParameters
*/
template <class valueType>
class ConstParameter : public Parameter
{
public:

    ConstParameter( valueType val, 
        GpuConstantType type, 
        const Semantic& semantic,  
        const Content& content) 
        : Parameter(type, "Constant", semantic, 0, content)
    {
        mValue = val;
    }

    virtual             ~ConstParameter     () {}

    /** Returns the native value of this parameter. (for example a Vector3) */
    const valueType& getValue() const { return mValue; }

    /** 
    @see Parameter::isConstParameter.
    */
    bool isConstParameter() const override { return true; }

    /** 
    @see Parameter::toString.
    */
    String toString() const override = 0;

protected:
    valueType mValue;
};

/** Helper utility class that creates common parameters.
*/
class _OgreRTSSExport ParameterFactory
{

    // Interface.
public:

    static ParameterPtr createInPosition(int index, Parameter::Content content = Parameter::SPC_POSITION_OBJECT_SPACE);
    static ParameterPtr createOutPosition(int index);

    static ParameterPtr createInNormal(int index);
    static ParameterPtr createInWeights(int index);
    static ParameterPtr createInIndices(int index);
    static ParameterPtr createOutNormal(int index);
    static ParameterPtr createInBiNormal(int index);
    static ParameterPtr createOutBiNormal(int index);
    static ParameterPtr createInTangent(int index);
    static ParameterPtr createOutTangent(int index);
    static ParameterPtr createInColor(int index);
    static ParameterPtr createOutColor(int index);

    static ParameterPtr createInTexcoord(GpuConstantType type, int index, Parameter::Content content);
    static ParameterPtr createOutTexcoord(GpuConstantType type, int index, Parameter::Content content);

    static ParameterPtr createConstParam(const Vector2& val);
    static ParameterPtr createConstParam(const Vector3& val);
    static ParameterPtr createConstParam(const Vector4& val);
    static ParameterPtr createConstParam(float val);

    static UniformParameterPtr createSampler(GpuConstantType type, int index);
    static UniformParameterPtr createSampler1D(int index);
    static UniformParameterPtr createSampler2D(int index);
    static UniformParameterPtr createSampler2DArray(int index);
    static UniformParameterPtr createSampler3D(int index);
    static UniformParameterPtr createSamplerCUBE(int index);    

    static UniformParameterPtr createUniform(GpuConstantType type, int index, uint16 variability, const String& suggestedName, size_t size);
};



/** @} */
/** @} */

}
}

#endif
