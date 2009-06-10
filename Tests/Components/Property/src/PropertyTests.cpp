/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#include "PropertyTests.h"

CPPUNIT_TEST_SUITE_REGISTRATION( PropertyTests );

void PropertyTests::setUp()
{

}

void PropertyTests::tearDown()
{
}

class Foo
{
protected:
	String mName;
public:
	void setName(const String& name) { mName = name; }
	const String& getName() const { return mName; }
};

void PropertyTests::testStringProp()
{
	PropertyDefMap propertyDefs;
	Foo foo;
	PropertySet props;

	PropertyDefMap::iterator defi = propertyDefs.insert(PropertyDefMap::value_type("name", 
			PropertyDef("name", 
			"The name of the object.", PROP_STRING))).first;

	props.addProperty(
		OGRE_NEW Property<String>(&(defi->second),
		boost::bind(&Foo::getName, &foo), 
		boost::bind(&Foo::setName, &foo, _1)));

	Ogre::String strName, strTest;
	strTest = "A simple name";
	props.setValue("name", strTest);
	props.getValue("name", strName);

	CPPUNIT_ASSERT_EQUAL(strTest, strName);

}

