/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef _ShaderParameter_
#define _ShaderParameter_

#include "OgrePrerequisites.h"
#include "OgreGpuProgram.h"

namespace Ogre {
namespace CRTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
class Parameter
{
public:
	// Shader parameter semantic.
	enum Semantic
	{
		/// Unknonw semantic
		SPS_UNKNOWN = 0,
		/// Position, 3 reals per vertex
		SPS_POSITION = 1,
		/// Blending weights
		SPS_BLEND_WEIGHTS = 2,
		/// Blending indices
		SPS_BLEND_INDICES = 3,
		/// Normal, 3 reals per vertex
		SPS_NORMAL = 4,
		/// General color.
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
	Parameter(GpuConstantType type, const String& name, 
		const Semantic& semantic, int index);

	Parameter(GpuProgramParameters::AutoConstantType autoType, Real fAutoConstantData);
	Parameter(GpuProgramParameters::AutoConstantType autoType, size_t nAutoConstantData);

	virtual ~Parameter() {};

	const String&			getName							() const { return mName; }

	GpuConstantType			getType							() const { return mType; }
	const Semantic&			getSemantic						() const { return mSemantic; }
	int						getIndex						() const { return mIndex; }	
	Real					getAutoConstantIntData			() const { return mAutoConstantIntData; }	
	Real					getAutoConstantRealData			() const { return mAutoConstantRealData; }		
	bool					isFloat							() const;
	bool					isSampler						() const;

	bool					isAutoConstantParameter				() const { return mIsAutoConstantReal || mIsAutoConstantInt; }
	bool					isAutoConstantIntParameter			() const { return mIsAutoConstantInt; }
	bool					isAutoConstantRealParameter			() const { return mIsAutoConstantReal; }
	GpuProgramParameters::AutoConstantType getAutoConstantType	() const { return mAutoConstantType; }

// Attributes.
protected:
	String									mName;
	GpuConstantType							mType;
	Semantic								mSemantic;
	int										mIndex;
	bool									mIsAutoConstantReal;
	bool									mIsAutoConstantInt;
	GpuProgramParameters::AutoConstantType	mAutoConstantType;
	union
	{
		size_t	mAutoConstantIntData;
		Real	mAutoConstantRealData;
	};		
};



/************************************************************************/
/* Vertex / Fragment shader standard parameters.                                                        
/************************************************************************/
class ParameterFactory
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
}
}

#endif
