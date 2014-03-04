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
#ifndef _ShaderSGXAtlasTexture_
#define _ShaderSGXAtlasTexture_

#include "OgreShaderPrerequisites.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderSubRenderState.h"
#include "OgreTextureUnitState.h"

#define TAS_MAX_TEXTURES 4
namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/
struct _OgreRTSSExport TextureAtlasRecord
{
    TextureAtlasRecord(const String & texOriginalName, const String & texAtlasName, 
                       const float texPosU, const float texPosV, const float texWidth, const float texHeight,
                       const size_t texIndexInAtlas) :
                  posU(texPosU)
                , posV(texPosV)
                , width(texWidth)
                , height(texHeight)
                , originalTextureName(texOriginalName)
                , atlasTextureName(texAtlasName)
                , indexInAtlas(texIndexInAtlas)
    { }

    float posU;
    float posV;
    float width;
    float height;
    String originalTextureName;
    String atlasTextureName;
    size_t indexInAtlas;
};

typedef vector<TextureAtlasRecord>::type TextureAtlasTable;
typedef SharedPtr<TextureAtlasTable> TextureAtlasTablePtr;
typedef map<String, TextureAtlasTablePtr>::type TextureAtlasMap;



/** Implements texture atlas sampling.

This class implements a sub render state which allows sampling of a texture
from a texture atlas. 

Note: This class does not implement the entire texture sub-render state. It
only implement the sampling of textures. This class needs to work in conjunction
with the default FFPTexturing to work.

\par Using the TextureAtlasSampler
There are 2 pieces of information that are need to be provided for this sub
render state to work. 

The first is the texture atlas table. This table consists of several records
containing the position and size of each texture in the in the texture atlas. 
This information needs to be provided per atlas texture. The information is 
entered to the system though the TextureAtlasSamplerFactory using the
functions setTextureAtlasTable and removeTextureAtlasTable.

The second information is the index of the record in the texture atlas table
to which a given texture is associated with. This information is provided
through an extra texture coordinate in the vertex buffer. This texture coordinate
might can be placed either relative or in absolute position. 

For example:
 - given 3 texture sand a relative position of 2, the shader will search assume 
    that the indexes exist in texture coordinate 5 (2+3)
 - given an absolute position of 2, the shader will search for the indexes in 
    texture coordinate 2, regardless of the amount of textures

The position of the indexes can be controlled globally through the 
TextureAtlasSamplerFactory class using the function setTableIndexPosition
The default index position is set to relative + 1.

\par Preparing atlas textures
When preparing the atlas texture to be used in this system you should make sure
that all texture with in the atlas have power-of-2 dimensions. And also that
the inserted textures will be padded with 1 pixel of their own border color.
This will prevent visual artifacts caused when sampling textures at their borders.

\par Border issues
There is an inherit problem in texture atlases. This issue occurs because individual textures within 
the atlas texture are adjacent to one another. when polling the color of a texture near the texture's
edges, especially in lower mipmaps pixel color from other images may be mixed in with the result.
There are 3 ways to handle this issue, each with it's own limitations:
-# Ignore the problem - bad for repetitive images in which the border colour may be quite apparent.
-# Auto adjust the polling position - This the default implementation of the TextureAtlasSampler SRS.
    Auto adjust the polling position in the shader according the mipmap level in use. This means that 
    a different (smaller) section of an image may be polled instead of the original section (especially 
    with in mipmaps). Bad for non repetitive accurate images.
-# Generate a texture atlas where each image will contain around it a wrapped version of itself. 
    This solves all visual problems but is wasteful in gpu memory (up to 3 times the size of the original image)


You can use the NVidia "Texture Atlas Tools" to create the texture. 
*/
class _OgreRTSSExport TextureAtlasSampler : public SubRenderState
{
public:

// Interface.
public:

    /** Class default constructor */
    TextureAtlasSampler();

    /** 
    @see SubRenderState::getType.
    */
    virtual const String& getType() const;

    /** 
    @see SubRenderState::getType.
    */
    virtual int getExecutionOrder() const;

    /** 
    @see SubRenderState::copyFrom.
    */
    virtual void copyFrom(const SubRenderState& rhs);

    /** 
    @see SubRenderState::updateGpuProgramsParams.
    */
    virtual void updateGpuProgramsParams(Renderable* rend, Pass* pass,  const AutoParamDataSource* source,  const LightList* pLightList);

    /** 
    @see SubRenderState::preAddToRenderState.
    */
    virtual bool preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass);

    static String Type;

// Protected methods
protected:  
    virtual bool resolveParameters(ProgramSet* programSet); 
    virtual bool resolveDependencies(ProgramSet* programSet);
    virtual bool addFunctionInvocations(ProgramSet* programSet);

    /** 
    Given an address mode returns the function name which calculates the UV values for that addressing mode
    */
    const char* getAdressingFunctionName(TextureUnitState::TextureAddressingMode mode);


// Attributes.
protected:
    /// The index of the information on the texture in the table
    ParameterPtr mVSInpTextureTableIndex;
    
    /// The addressing mode for each texture
    TextureUnitState::UVWAddressingMode mTextureAddressings[TAS_MAX_TEXTURES];
    /// The position and size of the texture in the atlas
    ParameterPtr mVSOutTextureDatas[TAS_MAX_TEXTURES];
    /// The position and size of the texture in the atlas
    ParameterPtr mPSInpTextureDatas[TAS_MAX_TEXTURES];
    /// A parameter carrying the sizes of the atlas textures
    UniformParameterPtr mPSTextureSizes[TAS_MAX_TEXTURES];
    /// The table containing information on the textures in the atlas
    UniformParameterPtr mVSTextureTable[TAS_MAX_TEXTURES];

    /// The position of the texture coordinates containing the index information
    ushort mAtlasTexcoordPos; 
    /// The texture atlas table data
    TextureAtlasTablePtr mAtlasTableDatas[TAS_MAX_TEXTURES];
    /// For each texture unit in the pass tells if it uses atlas texture
    bool mIsAtlasTextureUnits[TAS_MAX_TEXTURES];
    /// Tells if the data in mAtlasTableData has been uploaded to the corresponding mVSTextureTable parameter
    bool mIsTableDataUpdated;
    /// Tells whether border issue handling uses auto adjust polling position.
    bool mAutoAdjustPollPosition;
};



/** 
A factory that enables creation of TextureAtlasSampler instances.
@remarks Sub class of SubRenderStateFactory
*/
class _OgreRTSSExport TextureAtlasSamplerFactory : public SubRenderStateFactory, public Singleton<TextureAtlasSamplerFactory>
{
public:
    enum IndexPositionMode
    {
        ipmRelative,
        ipmAbsolute
    };

    struct TextureAtlasAttib
    {
        TextureAtlasAttib(IndexPositionMode _posMode = ipmRelative, ushort _posOffset = 1,
            bool _autoBorderAdjust = true) : positionMode(_posMode), positionOffset(_posOffset),
             autoBorderAdjust(_autoBorderAdjust) {}

        IndexPositionMode positionMode;
        ushort positionOffset;
        bool autoBorderAdjust;
    };

public:

    //TextureAtlasSamplerFactory c_tor
    TextureAtlasSamplerFactory();

    //Singleton implementation
    static TextureAtlasSamplerFactory* getSingletonPtr(void);
    static TextureAtlasSamplerFactory& getSingleton(void);
    


    /** 
    @see SubRenderStateFactory::getType.
    */
    virtual const String& getType() const;

    /** 
    @see SubRenderStateFactory::createInstance.
    */
    virtual SubRenderState* createInstance(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator);

    /** 
    @see SubRenderStateFactory::writeInstance.
    */
    virtual void writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass, Pass* dstPass);
    
    /**
        Adds a texture atlas definition from a stream. 

        This function loads a texture atlas definition file from a stream. The accepted format for
        this file is the NVidia Texture Atlas Tools ".tai" file format. This file as
    
        The ".tai" format consist of lines, where each line corresponds to a specific texture
        in the texture atlas. Each line has the following format:
        # <original texture filename>/t/t<atlas filename>, <atlas idx>, <atlas type>, <woffset>, <hoffset>, <depth offset>, <width>, <height>
    
        @param filename The full path to the file containing a ".tai" format data.
        @param textureAtlasTable A table into which the data in the stream will be filled. This
            parameter will be filled only if it is not null. The system factory keeps a copy of this
            information in any case.
    */
    bool addTexutreAtlasDefinition( const Ogre::String& filename, TextureAtlasTablePtr textureAtlasTable = TextureAtlasTablePtr());
    /**
        Adds a texture atlas definition from a stream. 

        This function loads a texture atlas definition file from a stream. The accepted format for
        this file is the NVidia Texture Atlas Tools ".tai" file format. This file as
    
        The ".tai" format consist of lines, where each line corresponds to a specific texture
        in the texture atlas. Each line has the following format:
        # <original texture filename>/t/t<atlas filename>, <atlas idx>, <atlas type>, <woffset>, <hoffset>, <depth offset>, <width>, <height>
    
        @param stream A stream to a file containing ".tai" format data
        @param textureAtlasTable A table into which the data in the stream will be filled. This
            parameter will be filled only if it is not null. The system factory keeps a copy of this
            information in any case.
    */
    bool addTexutreAtlasDefinition( DataStreamPtr stream, TextureAtlasTablePtr textureAtlasTable = TextureAtlasTablePtr());

    /**
        Set the texture atlas information for a given texture
        @param textureName Name of an atlas texture
        @param atlasData A list of records containing the position and size of each 
            texture in the atlas
        @param autoBorderAdjust Sets whether to automatically adjust the image polling area for border 
            issues.See the Border issues paragraph under the class documentation for more information.
    */
    void setTextureAtlasTable(const String& textureName, const TextureAtlasTablePtr& atlasData, bool autoBorderAdjust = true);
    
    /** 
        Removes the texture atlas information from a given texture
        @param textureName Name of an atlas texture
    */
    void removeTextureAtlasTable(const String& textureName);

    /** 
        Removes all texture atlas table information
    */
    void removeAllTextureAtlasTables();

    /** 
        Retrieve the texture atlas table information for a given texture
        @param textureName Name of an atlas texture
    */
    const TextureAtlasTablePtr& getTextureAtlasTable(const String& textureName) const;

    /**
        Set the default attributes concerning atlas texture processing
        @param mode The index positioning mode. Tells relative to where the the texture coordinates
            containing the atlas image to use are positioned.
        @param offset The index positioning offset. Tells the offset relative to the index positioning 
            mode.
        @param autoAdjustBorders Tells whether to automatically adjust the polled area in the texture
            relative to the used mipmap level.
        @see TextureAtlasSampler
    */
    void setDefaultAtlasingAttributes(IndexPositionMode mode, ushort offset, bool autoAdjustBorders);

    /**
        Returns the default attributes of texture atlas processing
        @see setDefaultAtlasingAttributes
    */
    const TextureAtlasAttib& getDefaultAtlasingAttributes() const;

    /**
        Set the default attributes concerning atlas texture processing for a specific material
        @param material The material to which to add the information
        @param mode The index positioning mode. Tells relative to where the the texture coordinates
            containing the atlas image to use are positioned.
        @param offset The index positioning offset. Tells the offset relative to the index positioning 
            mode.
        @param autoAdjustBorders Tells whether to automatically adjust the polled area in the texture
            relative to the used mipmap level.
        @see TextureAtlasSampler
    */
    void setMaterialAtlasingAttributes(Ogre::Material* material, 
        IndexPositionMode mode, ushort offset, bool autoAdjustBorders);


    /**
        Tells whether a specific material has atlas attributes associated with it. And returns the
        attributes to be used.
        @see setMaterialAtlasingAttributes
    */
    bool hasMaterialAtlasingAttributes(Ogre::Material* material, TextureAtlasAttib* attrib = NULL) const;
    
protected:

    

    /** 
    @see SubRenderStateFactory::createInstanceImpl.
    */
    virtual SubRenderState* createInstanceImpl();

private:

    //Holds a mapping of texture names and the atlas table information associated with them
    TextureAtlasMap mAtlases;

    TextureAtlasAttib mDefaultAtlasAttrib;
};

_OgreRTSSExport void operator<<(std::ostream& o, const TextureAtlasSamplerFactory::TextureAtlasAttib& tai);

/** @} */
/** @} */

}
}

#endif
#endif
