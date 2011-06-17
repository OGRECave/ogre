/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#include "OgreShaderParameter.h"

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
	virtual void updateGpuProgramsParams(Renderable* rend, Pass* pass,  const AutoParamDataSource* source, 	const LightList* pLightList);

	/** 
	@see SubRenderState::preAddToRenderState.
	*/
	virtual bool preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass);
	
	static String Type;

// Protected methods
protected:	
	virtual bool			resolveParameters		(ProgramSet* programSet);	
	virtual bool			resolveDependencies		(ProgramSet* programSet);
	virtual bool			addFunctionInvocations	(ProgramSet* programSet);

	/** 
	Given an address mode returns the function name which calculates the UV values for that addressing mode
	*/
	const char* getAdressingFunctionName(TextureUnitState::TextureAddressingMode mode);


// Attributes.
protected:
	ParameterPtr mVSInpTextureTableIndex; // The index of the information on the texture in the table
	
	TextureUnitState::UVWAddressingMode mTextureAddressings[TAS_MAX_TEXTURES]; // The addressing mode for each texture
	ParameterPtr mVSOutTextureDatas[TAS_MAX_TEXTURES]; // The position and size of the texture in the atlas 
	ParameterPtr mPSInpTextureDatas[TAS_MAX_TEXTURES]; // The position and size of the texture in the atlas
	UniformParameterPtr mPSTextureSizes[TAS_MAX_TEXTURES]; //A parameter carrying the sizes of the atlas textures
	UniformParameterPtr mVSTextureTable[TAS_MAX_TEXTURES]; // The table containing information on the textures in the atlas

	//The position of the texture coordinates containing the index information 
	ushort mAtlasTexcoordPos; 
	//The texture atlas table data
	TextureAtlasTablePtr mAtlasTableDatas[TAS_MAX_TEXTURES];
	//For each texture unit in the pass tells if it uses atlas texture
	bool mIsAtlasTextureUnits[TAS_MAX_TEXTURES];
	//Tells if the data in mAtlasTableData has been uploaded to the corresponding mVSTextureTable parameter
	bool mIsTableDataUpdated;
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
public:

	//TextureAtlasSamplerFactory c_tor
	TextureAtlasSamplerFactory();

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
	
		@param stream A stream to a file containing ".tai" format data
		@param textureAtlasTable A table into which the data in the stream will be filled. This
			parameter will be filled only if it is not null. The system factory keeps a copy of this
			information in any case.
	*/
	bool addTexutreAtlasDefinition( DataStreamPtr stream, TextureAtlasTablePtr textureAtlasTable = TextureAtlasTablePtr());

	/**
		Set the texture atlas information for a given texture
		@param textureName Name of an atlas texture
		@param atlasData a list of records containing the position and size of each 
			texture in the atlas
	*/
	void setTextureAtlasTable(const String& textureName, const TextureAtlasTablePtr& atlasData);
	
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
		Retrieve the texture atlas information for a given texture
		@param textureName Name of an atlas texture
	*/
	const TextureAtlasTablePtr& getTextureAtlasTable(const String& textureName) const;

	/**
		Set the position of the atlas table indexes within the texcoords of the vertex data
		@see TextureAtlasSampler
	*/
	void setTableIndexPosition(IndexPositionMode mode, ushort offset);

	/**
		Get the positioning mode of the atlas table indexes within the texcoords of the vertex data
		@see TextureAtlasSampler
		*/
	IndexPositionMode getTableIndexPositionMode() const;

	/**
		Get the offset of the atlas table indexes within the texcoords of the vertex data
		@see TextureAtlasSampler
		*/
	ushort getTableIndexPositionOffset() const;

	
protected:

	/** 
	@see SubRenderStateFactory::createInstanceImpl.
	*/
	virtual SubRenderState*	createInstanceImpl();

private:

	//Holds a mapping of texture names and the atlas table information associated with them
	TextureAtlasMap mAtlases;

	//The positioning mode of the atlas table indexes within the texcoords of the vertex data
	IndexPositionMode mIndexPositionMode;
	
	//The offset of the atlas table indexes within the texcoords of the vertex data
	ushort mIndexPositionOffset;
};

/** @} */
/** @} */

}
}

#endif
#endif
