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
#ifndef __CompositorLogic_H__
#define __CompositorLogic_H__

#include "OgrePrerequisites.h"

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/
	/** Interface for compositor logics, which can be automatically binded to compositors,
	*	allowing per-compositor logic (such as attaching a relevant listener) to happen
	*	automatically.
	*	@remarks All methods have empty implementations to not force an implementer into
	*		extending all of them.
    */
    class _OgreExport CompositorLogic
    {
	public:
		/** Called when a compositor instance has been created.
			@remarks
			This happens after its setup was finished, so the chain is also accessible.
			This is an ideal method to automatically attach a compositor listener.
        */
		virtual void compositorInstanceCreated(CompositorInstance* newInstance) {}

		/** Called when a compositor instance has been destroyed
			@remarks
			The chain that contained the compositor is still alive during this call.
        */
		virtual void compositorInstanceDestroyed(CompositorInstance* destroyedInstance) {}

	protected:
		virtual ~CompositorLogic() {}
	};
	/** @} */
	/** @} */
}

#endif
