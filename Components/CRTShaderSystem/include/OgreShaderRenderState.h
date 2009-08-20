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
#ifndef _ShaderRenderState_
#define _ShaderRenderState_

#include "OgrePrerequisites.h"
#include "OgreShaderSubRenderState.h"
#include "OgreSharedPtr.h"

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
namespace CRTShader {


/************************************************************************/
/*                                                                      */
/************************************************************************/
class OGRE_CRTSHADER_API RenderState
{

// Interface.
public:
	RenderState();
	~RenderState();

	void		reset						();
	void		addSubRenderState			(SubRenderState* subRenderState);
	void		append						(const RenderState& other);
	
	void		copyFrom					(const RenderState& rhs);		
	RenderState& operator=					(const RenderState& rhs);


	uint32			getHashCode				();

	bool			createCpuPrograms		(ProgramSet* programSet);
	void			updateGpuProgramsParams	(Renderable* rend, Pass* pass, const AutoParamDataSource* source, const LightList* pLightList);
	void			preFindVisibleObjects	(SceneManager* source, SceneManager::IlluminationRenderStage irs, Viewport* v);

	const SubRenderStateList&	getSubStateList() const { return mSubRenderStateList; }

	void			setMaxLightCount		(const int maxLightCount[3]);
	void			getMaxLightCount		(int maxLightCount[3]) const;
	
// Protected methods
protected:
	void			sortSubRenderStates		();
	static int		sSubRenderStateCompare	(const void * p0, const void *p1);


// Attributes.
protected:
	SubRenderStateList	mSubRenderStateList;
	bool				mSubRenderStateSortValid;
	uint32				mHashCode;
	bool				mHashCodeValid;
	int					mMaxLightCount[3];
		
};

typedef SharedPtr<RenderState> RenderStatePtr;

}
}

#endif

