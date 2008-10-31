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
OgreOctreeZonePlugin.cpp  -  Octree Zone Plugin class for PCZSceneManager

-----------------------------------------------------------------------------
begin                : Mon Apr 16 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 :
-----------------------------------------------------------------------------
*/

#include <OgreRoot.h>
#include "OgreOctreeZonePlugin.h"
#include "OgrePCZSceneManager.h"

namespace Ogre 
{
	const String sPluginName = "Octree Zone Factory";
	//---------------------------------------------------------------------
	OctreeZonePlugin::OctreeZonePlugin()
		:mOctreeZoneFactory(0),
		 mTerrainZoneFactory(0),
		 mTerrainZonePSListenerManager(0)
	{

	}
	//---------------------------------------------------------------------
	const String& OctreeZonePlugin::getName() const
	{
		return sPluginName;
	}
	//---------------------------------------------------------------------
	void OctreeZonePlugin::install()
	{
		// Create objects
		mOctreeZoneFactory = OGRE_NEW OctreeZoneFactory();
		mTerrainZoneFactory = OGRE_NEW TerrainZoneFactory();
		mTerrainZonePSListenerManager = OGRE_NEW TerrainZonePageSourceListenerManager();

	}
	//---------------------------------------------------------------------
	void OctreeZonePlugin::initialise()
	{
		// Register
		PCZoneFactoryManager & pczfm = PCZoneFactoryManager::getSingleton();
		pczfm.registerPCZoneFactory(mOctreeZoneFactory);
		pczfm.registerPCZoneFactory(mTerrainZoneFactory);
	}
	//---------------------------------------------------------------------
	void OctreeZonePlugin::shutdown()
	{
		// Unregister
		PCZoneFactoryManager & pczfm = PCZoneFactoryManager::getSingleton();
		pczfm.unregisterPCZoneFactory(mOctreeZoneFactory);
		pczfm.unregisterPCZoneFactory(mTerrainZoneFactory);
	}
	//---------------------------------------------------------------------
	void OctreeZonePlugin::uninstall()
	{
		// destroy 
		OGRE_DELETE mTerrainZonePSListenerManager;
		mTerrainZonePSListenerManager = 0;
		OGRE_DELETE mTerrainZoneFactory;
		mTerrainZoneFactory = 0;
		OGRE_DELETE mOctreeZoneFactory;
		mOctreeZoneFactory = 0;
	}


}
