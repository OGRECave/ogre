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

#include "OgreBspSceneManagerPlugin.h"
#include "OgreRoot.h"

namespace Ogre 
{
	const String sPluginName = "BSP Scene Manager";
	//---------------------------------------------------------------------
	BspSceneManagerPlugin::BspSceneManagerPlugin()
		:mBspFactory(0), mBspResourceManager(0)
	{

	}
	//---------------------------------------------------------------------
	const String& BspSceneManagerPlugin::getName() const
	{
		return sPluginName;
	}
	//---------------------------------------------------------------------
	void BspSceneManagerPlugin::install()
	{
		// Create new scene manager
		mBspFactory = OGRE_NEW BspSceneManagerFactory();

	}
	//---------------------------------------------------------------------
	void BspSceneManagerPlugin::initialise()
	{
		// Register (factory not dependent on rsys resources)
		Root::getSingleton().addSceneManagerFactory(mBspFactory);
		// Create resource manager (registers itself)
		mBspResourceManager = OGRE_NEW BspResourceManager();
	}
	//---------------------------------------------------------------------
	void BspSceneManagerPlugin::shutdown()
	{
		// Unregister SM factory
		Root::getSingleton().removeSceneManagerFactory(mBspFactory);

		// delete resource manager, will unregister itself
		OGRE_DELETE mBspResourceManager;
		mBspResourceManager = 0;

	}
	//---------------------------------------------------------------------
	void BspSceneManagerPlugin::uninstall()
	{

		OGRE_DELETE mBspFactory;
		mBspFactory = 0;

	}


}
