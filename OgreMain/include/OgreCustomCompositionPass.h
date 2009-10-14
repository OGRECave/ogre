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
#ifndef __CustomCompositionPass_H__
#define __CustomCompositionPass_H__

#include "OgrePrerequisites.h"
#include "OgreCompositionPass.h"

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/
	/** Interface for custom composition passes, allowing custom operations (in addition to
	*	the quad, scene and clear operations) in composition passes.
	*	@see CompositorManager::registerCustomCompositionPass
    */
    class _OgreExport CustomCompositionPass
    {
	public:
		/** Create a custom composition operation.
			@param pass The CompositionPass that triggered the request
			@param instance The compositor instance that this operation will be performed in
			@remarks This call only happens once during creation. The RenderSystemOperation will
			get called each render.
			@remarks The created operation must be instanciated using the OGRE_NEW macro.
		*/
		virtual CompositorInstance::RenderSystemOperation* createOperation(
			CompositorInstance* instance, const CompositionPass* pass) = 0;

	protected:
		virtual ~CustomCompositionPass() {}
	};
	/** @} */
	/** @} */
}

#endif
