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
#ifndef _ShaderFFPColur_
#define _ShaderFFPColur_

#include "OgrePrerequisites.h"
#include "OgreShaderSubRenderState.h"
#include "OgreShaderParameter.h"

namespace Ogre {
namespace CRTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
class FFPColour : public SubRenderState
{
public:
	enum StageFlags
	{
		SF_VS_INPUT_DIFFUSE		= 1 << 1,
		SF_VS_INPUT_SPECULAR	= 1 << 2,
		SF_VS_OUTPUT_DIFFUSE	= 1 << 3,
		SF_VS_OUTPUT_SPECULAR	= 1 << 4,
		SF_PS_INPUT_DIFFUSE		= 1 << 5,
		SF_PS_INPUT_SPECULAR	= 1 << 6,
		SF_PS_OUTPUT_DIFFUSE	= 1 << 7,
		SF_PS_OUTPUT_SPECULAR	= 1 << 8,
	};

// Interface.
public:
	FFPColour();
	virtual ~FFPColour();

	virtual const String&	getType					() const;
	virtual int				getExecutionOrder		() const;
	virtual void			copyFrom				(const SubRenderState& rhs);
	virtual uint32			getHashCode				();

	void				setResolveStageFlags		(unsigned int flags) { mResolveStageFlags = flags; }
	unsigned int		getResolveStageFlags		() const			 { return mResolveStageFlags; }
	void				addResolveStageMask 		(unsigned int mask)  { mResolveStageFlags |= mask; }
	void				removeResolveStageMask 		(unsigned int mask)  { mResolveStageFlags &= ~mask; }

	static String Type;

// Protected methods
protected:	
	virtual bool			resolveParameters		(ProgramSet* programSet);	
	virtual bool			resolveDependencies		(ProgramSet* programSet);
	virtual bool			addFunctionInvocations	(ProgramSet* programSet);

// Attributes.
protected:
	Parameter*		mVSInputDiffuse;
	Parameter*		mVSInputSpecular;
	Parameter*		mVSOutputDiffuse;
	Parameter*		mVSOutputSpecular;
	Parameter*		mPSInputDiffuse;
	Parameter*		mPSInputSpecular;
	Parameter*		mPSOutputDiffuse;
	Parameter*		mPSOutputSpecular;	
	unsigned int	mResolveStageFlags;
};


/************************************************************************/
/*                                                                      */
/************************************************************************/
class FFPColourFactory : public SubRenderStateFactory
{
public:
	virtual const String&	getType				() const;
	
protected:
	virtual SubRenderState*	createInstanceImpl	();


};

}
}

#endif

