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
Portal.cpp  -  
-----------------------------------------------------------------------------
begin                : Tue Feb 20 2007
author               : Eric Cha
email                : ericc@xenopi.com
current TODO's       : none known
-----------------------------------------------------------------------------
*/

#include "OgrePortal.h"
#include "OgrePCZSceneNode.h"
#include "OgreSphere.h"
#include "OgreSegment.h"
#include "OgreRay.h"
#include "OgrePCZone.h"   // need access to real zone class 

using namespace Ogre;

Portal::Portal(const String& name, const PORTAL_TYPE type)
	: PortalBase(name, type),
	mTargetZone(0),
	mTargetPortal(0)
{
}

Portal::~Portal()
{
}

// Set the 1st Zone the Portal connects to
void Portal::setTargetZone(PCZone* zone)
{
	mTargetZone = zone;
}

// Set the Portal the Portal connects to
void Portal::setTargetPortal(Portal* portal)
{
	mTargetPortal = portal;
}

/** @copydoc MovableObject::getMovableType. */
const String& Portal::getMovableType() const
{
	return PortalFactory::FACTORY_TYPE_NAME;
}

//-----------------------------------------------------------------------
String PortalFactory::FACTORY_TYPE_NAME = "Portal";
unsigned long PortalFactory::FACTORY_TYPE_FLAG = 0xFFFFFFFF;
//-----------------------------------------------------------------------

MovableObject* PortalFactory::createInstanceImpl(const String& name, const NameValuePairList* params)
{
	return OGRE_NEW Portal(name, getPortalType(params));
}

void PortalFactory::destroyInstance(MovableObject* obj)
{
	OGRE_DELETE obj;
}
