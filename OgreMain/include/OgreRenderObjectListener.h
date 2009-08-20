/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef __RenderObjectListener_H__
#define __RenderObjectListener_H__

#include "OgrePrerequisites.h"
#include "OgreRenderable.h"
#include "OgrePass.h"

namespace Ogre {

	class Pass;
	class Renderable;

    /** Abstract interface which classes must implement if they wish to receive
        events from the scene manager when single object is about to be rendered. 
    */
	class _OgreExport RenderObjectListener
	{
	public:
		virtual ~RenderObjectListener() {}
		/** Event raised when render single object started.
		@remarks
		This method is called by the SceneManager.
		@param pass The renderable that is going to be rendered.
		@param pass The pass which was set.
		@param source The auto parameter source used within this render call.
		@pLightList The light list in use.
		@suppressRenderStateChanges True if render state changes should be suppressed.
		*/
		virtual void notifyRenderSingleObject(Renderable* rend, const Pass* pass, const AutoParamDataSource* source, 
			const LightList* pLightList, bool suppressRenderStateChanges) = 0;
	};


}

#endif

