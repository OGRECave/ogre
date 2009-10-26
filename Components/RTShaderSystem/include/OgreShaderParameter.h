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
#ifndef _ShaderParameter_
#define _ShaderParameter_

#include "OgreShaderPrerequisites.h"
#include "OgreGpuProgram.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** A class that represents a shader based program parameter.
*/
class OGRE_RTSHADERSYSTEM_API Parameter
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
		SPS_TANGENT = 9		
	};

	// Shader parameter content.
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

		/// Tangent vector
		SPC_TANGENT,

		/// Binormal vector
		SPC_BINORMAL,

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

		/// Reserved custom content range to be used by user custom shader extensions.
		SPC_CUSTOM_CONTENT_BEGIN	= 1000,
		SPC_CUSTOM_CONTENT_END		= 2000
	};

// Interface.
public:
	/** Class constructor.
	@param type The type of this parameter.
	@param name The name of this parameter.
	@param semantic The semantic of this parameter.
	@param index The index of this parameter.
	@param content The content of this parameter.
	@param variability How this parameter varies (bitwise combination of GpuProgramVariability).
	*/
	Parameter(GpuConstantType type, const String& name, 
		const Semantic& semantic, int index, 
		const Content& content,
		uint16 variability);

	/** Class constructor.
	@param autoType The auto type of this parameter.
	@param fAutoConstantData The real data for this auto constant parameter.	
	*/
	Parameter(GpuProgramParameters::AutoConstantType autoType, Real fAutoConstantData);

	/** Class constructor.
	@param autoType The auto type of this parameter.
	@param nAutoConstantData The int data for this auto constant parameter.	
	*/
	Parameter(GpuProgramParameters::AutoConstantType autoType, size_t nAutoConstantData);

	/** Class destructor */
	virtual ~Parameter() {};

	/** Get the name of this parameter. */
	const String&			getName							() const { return mName; }

	/** Get the type of this parameter. */
	GpuConstantType			getType							() const { return mType; }

	/** Get the semantic of this parameter. */
	const Semantic&			getSemantic						() const { return mSemantic; }

	/** Get the index of this parameter. */
	int						getIndex						() const { return mIndex; }	

	/** Get auto constant int data of this parameter, in case it is auto constant parameter. */
	Real					getAutoConstantIntData			() const { return mAutoConstantIntData; }	

	/** Get auto constant real data of this parameter, in case it is auto constant parameter. */
	Real					getAutoConstantRealData			() const { return mAutoConstantRealData; }	

	/** Return true if this parameter is a floating point type, false otherwise. */
	bool					isFloat							() const;

	/** Return true if this parameter is a texture sampler type, false otherwise. */
	bool					isSampler						() const;

	/** Return true if this parameter is an auto constant parameter, false otherwise. */
	bool					isAutoConstantParameter				() const { return mIsAutoConstantReal || mIsAutoConstantInt; }

	/** Return true if this parameter an auto constant with int data type, false otherwise. */
	bool					isAutoConstantIntParameter			() const { return mIsAutoConstantInt; }

	/** Return true if this parameter an auto constant with real data type, false otherwise. */
	bool					isAutoConstantRealParameter			() const { return mIsAutoConstantReal; }

	/** Return the auto constant type of this parameter. */
	GpuProgramParameters::AutoConstantType getAutoConstantType	() const { return mAutoConstantType; }

	/** Return the variability of this parameter. */
	uint16					getVariability						() const { return mVariability; }

	/** Return the content of this parameter. */
	Content					getContent							() const { return mContent; }


// Attributes.
protected:
	String									mName;					// Name of this parameter.
	GpuConstantType							mType;					// Type of this parameter.
	Semantic								mSemantic;				// Semantic of this parameter.
	int										mIndex;					// Index of this parameter.
	Content									mContent;				// The content of this parameter.
	bool									mIsAutoConstantReal;	// Is it auto constant real based parameter.
	bool									mIsAutoConstantInt;		// Is it auto constant int based parameter.
	GpuProgramParameters::AutoConstantType	mAutoConstantType;		// The auto constant type of this parameter.
	union
	{
		size_t	mAutoConstantIntData;								// Auto constant int data.
		Real	mAutoConstantRealData;								// Auto constant real data.
	};		
	uint16									mVariability;			// How this parameter varies (bitwise combination of GpuProgramVariability).
};



/** Helper utility class that creates common parameters.
*/
class OGRE_RTSHADERSYSTEM_API ParameterFactory
{

// Interface.
public:

	static Parameter*	createInPosition	(int index);	
	static Parameter*	createOutPosition	(int index);
	
	static Parameter*	createInNormal		(int index);
	static Parameter*	createOutNormal		(int index);
	static Parameter*	createInBiNormal	(int index);
	static Parameter*	createOutBiNormal	(int index);
	static Parameter*	createInTangent		(int index);
	static Parameter*	createOutTangent	(int index);
	static Parameter*	createInColor		(int index);
	static Parameter*	createOutColor		(int index);

	static Parameter*	createInTexcoord	(GpuConstantType type, int index, Parameter::Content content);
	static Parameter*	createOutTexcoord	(GpuConstantType type, int index, Parameter::Content content);
	static Parameter*	createInTexcoord1	(int index, Parameter::Content content);
	static Parameter*	createOutTexcoord1	(int index, Parameter::Content content);
	static Parameter*	createInTexcoord2	(int index, Parameter::Content content);
	static Parameter*	createOutTexcoord2	(int index, Parameter::Content content);
	static Parameter*	createInTexcoord3	(int index, Parameter::Content content);
	static Parameter*	createOutTexcoord3	(int index, Parameter::Content content);
	static Parameter*	createInTexcoord4	(int index, Parameter::Content content);			
	static Parameter*	createOutTexcoord4	(int index, Parameter::Content content);

	static Parameter*	createSampler		(GpuConstantType type, int index);
	static Parameter*	createSampler1D		(int index);
	static Parameter*	createSampler2D		(int index);
	static Parameter*	createSampler3D		(int index);
	static Parameter*	createSamplerCUBE	(int index);	
	
};

typedef std::vector<Parameter*>						ShaderParameterList;
typedef ShaderParameterList::iterator 				ShaderParameterIterator;
typedef ShaderParameterList::const_iterator			ShaderParameterConstIterator;

/** @} */
/** @} */

}
}

#endif