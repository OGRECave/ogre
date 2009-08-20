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
#ifndef _ShaderSubRenderState_
#define _ShaderSubRenderState_

#include "OgrePrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreSceneManager.h"

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32) && !defined(OGRE_STATIC_LIB)
#   ifdef OGRE_CRTSHADERGENERATOR_EXPORTS
#       define OGRE_CRTSHADER_API __declspec(dllexport)
#   else
#       if defined(__MINGW32__)
#           define OGRE_CRTSHADER_API
#       else
#           define OGRE_CRTSHADER_API __declspec(dllimport)
#       endif
#   endif
#elif defined ( OGRE_GCC_VISIBILITY )
#    define OGRE_CRTSHADER_API  __attribute__ ((visibility("default")))
#else
#    define OGRE_CRTSHADER_API
#endif

namespace Ogre {
class Pass;
class Renderable;
class Viewport;

namespace CRTShader {
class ProgramSet;
class RenderState;

/************************************************************************/
/*                                                                      */
/************************************************************************/
class OGRE_CRTSHADER_API SubRenderState
{

// Interface.
public:
	SubRenderState			();
	virtual ~SubRenderState	();

	void					setParent				(RenderState* parent) { mParent = parent; }
	const RenderState*		getParent				() const			  { return mParent; }

	virtual const String&	getType					() const = 0;
	virtual int				getExecutionOrder		() const = 0;
	virtual void			copyFrom				(const SubRenderState& rhs) = 0;
	virtual uint32			getHashCode				();
	virtual bool			createCpuSubPrograms	(ProgramSet* programSet);
	virtual void			updateGpuProgramsParams	(Renderable* rend, Pass* pass,  const AutoParamDataSource* source, 	const LightList* pLightList) { }
	virtual void			preFindVisibleObjects	(SceneManager* source, SceneManager::IlluminationRenderStage irs, Viewport* v) {}

	SubRenderState& operator=	(const SubRenderState& rhs);

// Protected methods
protected:
	virtual bool			resolveParameters		(ProgramSet* programSet);	
	virtual bool			resolveDependencies		(ProgramSet* programSet);
	virtual bool			addFunctionInvocations	(ProgramSet* programSet);


// Attributes.
protected:
	RenderState*		mParent;
};

typedef std::vector<SubRenderState*> 				SubRenderStateList;
typedef SubRenderStateList::iterator 				SubRenderStateIterator;
typedef SubRenderStateList::const_iterator			SubRenderStateConstIterator;


/************************************************************************/
/*                                                                      */
/************************************************************************/
class SubRenderStateFactory
{

public:
	SubRenderStateFactory			() {}
	virtual ~SubRenderStateFactory	();

	virtual const String&	getType				() const = 0;
	
	virtual SubRenderState*	createInstance		();
	virtual void			destroyInstance		(SubRenderState* subRenderState);
	virtual void			destroyAllInstances	();

protected:
	virtual SubRenderState*	createInstanceImpl	() = 0;

protected:
	SubRenderStateList		mSubRenderStateList;
};

}
}

#endif

