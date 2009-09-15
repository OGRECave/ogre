/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
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
