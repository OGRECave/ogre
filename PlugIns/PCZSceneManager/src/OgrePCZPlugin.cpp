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
OgrePCZPlugin.cpp  -  Portal Connected Zone Scene Manager Plugin class
-----------------------------------------------------------------------------
begin                : Mon Feb 19 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 :
-----------------------------------------------------------------------------
*/

#include "OgrePCZPlugin.h"
#include <OgreRoot.h>

namespace Ogre 
{
	const String sPluginName = "Portal Connected Zone Scene Manager";
	//---------------------------------------------------------------------
	PCZPlugin::PCZPlugin()
		:mPCZSMFactory(0)
	{
	}
	//---------------------------------------------------------------------
	const String& PCZPlugin::getName() const
	{
		return sPluginName;
	}
	//---------------------------------------------------------------------
	void PCZPlugin::install()
	{
		// Create objects
		mPCZSMFactory = OGRE_NEW PCZSceneManagerFactory();
		mPCZoneFactoryManager = OGRE_NEW PCZoneFactoryManager();
        mPCZLightFactory = OGRE_NEW PCZLightFactory();
	}
	//---------------------------------------------------------------------
	void PCZPlugin::initialise()
	{
		// Register
		Root::getSingleton().addSceneManagerFactory(mPCZSMFactory);
        Root::getSingleton().addMovableObjectFactory(mPCZLightFactory);

	}
	//---------------------------------------------------------------------
	void PCZPlugin::shutdown()
	{
		// Unregister
		Root::getSingleton().removeSceneManagerFactory(mPCZSMFactory);
        Root::getSingleton().removeMovableObjectFactory(mPCZLightFactory);
	}
	//---------------------------------------------------------------------
	void PCZPlugin::uninstall()
	{
		// destroy 
		OGRE_DELETE mPCZSMFactory;
		mPCZSMFactory = 0;
		OGRE_DELETE mPCZoneFactoryManager;
		mPCZoneFactoryManager = 0;
        OGRE_DELETE mPCZLightFactory;
        mPCZLightFactory = 0;
	}


}
