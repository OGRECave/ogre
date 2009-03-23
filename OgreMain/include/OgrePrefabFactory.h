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

#ifndef __PrefabFactory_H__
#define __PrefabFactory_H__

#include "OgrePrerequisites.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Resources
	*  @{
	*/
	/** A factory class that can create various mesh prefabs. 
	@remarks
		This class is used by OgreMeshManager to offload the loading of various prefab types 
		to a central location.
	*/
	class _OgreExport PrefabFactory
	{
	public:
		/** If the given mesh has a known prefab resource name (e.g "Prefab_Plane") 
		    then this prefab will be created as a submesh of the given mesh.

			@param mesh The mesh that the potential prefab will be created in.
			@returns true if a prefab has been created, otherwise false.
		*/
		static bool createPrefab(Mesh* mesh);

	private:
		/// Creates a plane as a submesh of the given mesh
		static void createPlane(Mesh* mesh);

		/// Creates a 100x100x100 cube as a submesh of the given mesh
		static void createCube(Mesh* mesh);

		/// Creates a sphere with a diameter of 100 units as a submesh of the given mesh
		static void createSphere(Mesh* mesh);
	};
	/** @} */
	/** @} */

} // namespace Ogre

#endif
