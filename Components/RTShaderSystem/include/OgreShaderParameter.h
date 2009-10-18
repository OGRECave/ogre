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
		/// Unknonw semantic
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

// Interface.
public:
	/** Class constructor.
	@param type The type of this parameter.
	@param name The name of this parameter.
	@param index The index of this parameter.
	@param variability How this parameter varies (bitwise combination of GpuProgramVariability).
	*/
	Parameter(GpuConstantType type, const String& name, 
		const Semantic& semantic, int index, uint16 variability);

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

// Attributes.
protected:
	String									mName;					// Name of this parameter.
	GpuConstantType							mType;					// Type of this parameter.
	Semantic								mSemantic;				// Semantic of this parameter.
	int										mIndex;					// Index of this parameter.
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

	static Parameter*	createInTexcoord	(GpuConstantType type, int index);
	static Parameter*	createOutTexcoord	(GpuConstantType type, int index);
	static Parameter*	createInTexcoord1	(int index);
	static Parameter*	createOutTexcoord1	(int index);
	static Parameter*	createInTexcoord2	(int index);
	static Parameter*	createOutTexcoord2	(int index);
	static Parameter*	createInTexcoord3	(int index);
	static Parameter*	createOutTexcoord3	(int index);
	static Parameter*	createInTexcoord4	(int index);			
	static Parameter*	createOutTexcoord4	(int index);

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