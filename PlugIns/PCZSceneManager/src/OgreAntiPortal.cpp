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
AntiPortal.cpp  -  

*/

#include "OgreAntiPortal.h"

using namespace Ogre;

AntiPortal::AntiPortal(const String& name, const PORTAL_TYPE type)
	: PortalBase(name, type)
{
}

AntiPortal::~AntiPortal()
{
}

/** @copydoc MovableObject::getMovableType. */
const String& AntiPortal::getMovableType() const
{
	return AntiPortalFactory::FACTORY_TYPE_NAME;
}

//-----------------------------------------------------------------------
String AntiPortalFactory::FACTORY_TYPE_NAME = "AntiPortal";
unsigned long AntiPortalFactory::FACTORY_TYPE_FLAG = 0xFFFFFFFF;
//-----------------------------------------------------------------------

MovableObject* AntiPortalFactory::createInstanceImpl(const String& name, const NameValuePairList* params)
{
	return OGRE_NEW AntiPortal(name, getPortalType(params));
}

void AntiPortalFactory::destroyInstance(MovableObject* obj)
{
	OGRE_DELETE obj;
}
