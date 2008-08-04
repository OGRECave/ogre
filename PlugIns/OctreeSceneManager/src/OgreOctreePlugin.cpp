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

#include "OgreOctreePlugin.h"
#include "OgreRoot.h"

namespace Ogre 
{
	const String sPluginName = "Octree & Terrain Scene Manager";
	//---------------------------------------------------------------------
	OctreePlugin::OctreePlugin()
		:mOctreeSMFactory(0), mTerrainSMFactory(0), mTerrainPSListenerManager(0)
	{

	}
	//---------------------------------------------------------------------
	const String& OctreePlugin::getName() const
	{
		return sPluginName;
	}
	//---------------------------------------------------------------------
	void OctreePlugin::install()
	{
		// Create objects
		mOctreeSMFactory = OGRE_NEW OctreeSceneManagerFactory();
		mTerrainSMFactory = OGRE_NEW TerrainSceneManagerFactory();
		mTerrainPSListenerManager = OGRE_NEW TerrainPageSourceListenerManager();

	}
	//---------------------------------------------------------------------
	void OctreePlugin::initialise()
	{
		// Register
		Root::getSingleton().addSceneManagerFactory(mOctreeSMFactory);
		Root::getSingleton().addSceneManagerFactory(mTerrainSMFactory);
	}
	//---------------------------------------------------------------------
	void OctreePlugin::shutdown()
	{
		// Unregister
		Root::getSingleton().removeSceneManagerFactory(mTerrainSMFactory);
		Root::getSingleton().removeSceneManagerFactory(mOctreeSMFactory);
	}
	//---------------------------------------------------------------------
	void OctreePlugin::uninstall()
	{
		// destroy 
		OGRE_DELETE mTerrainPSListenerManager;
		mTerrainPSListenerManager = 0;
		OGRE_DELETE mTerrainSMFactory;
		mTerrainSMFactory = 0;
		OGRE_DELETE mOctreeSMFactory;
		mOctreeSMFactory = 0;


	}


}
