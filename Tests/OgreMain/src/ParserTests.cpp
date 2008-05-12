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
#include "ParserTests.h"
#include "OgreScriptCompiler.h"

CPPUNIT_TEST_SUITE_REGISTRATION( ParserTests );

using namespace Ogre;

namespace Ogre{
ConcreteNodeListPtr parse(const String &script, const String &source);
ConcreteNodeListPtr parseChunk(const String &script, const String &source);
bool parseNumber(const String &script, Ogre::Real &num);
}

void ParserTests::setUp()
{
}

void ParserTests::tearDown()
{
}


void ParserTests::testSimpleObject()
{
	String str =
		"type name : Parent{\n"
		"}\n";
	ConcreteNodeListPtr nodes = parse(str, "inline");
	CPPUNIT_ASSERT_EQUAL(nodes->empty(), false);

	if(!nodes->empty())
	{
		ConcreteNodePtr node = nodes->front();
		CPPUNIT_ASSERT_EQUAL(node->type, CNT_WORD);
		CPPUNIT_ASSERT_EQUAL(node->token, String("type"));
		CPPUNIT_ASSERT_EQUAL(node->children.empty(), false);

		ConcreteNodeList::iterator i = node->children.begin();
		CPPUNIT_ASSERT_EQUAL(node->type, CNT_WORD);
		CPPUNIT_ASSERT_EQUAL(node->token, String("name"));
	}
}

void ParserTests::testCompositeObject()
{
}

void ParserTests::testImport()
{
}

void ParserTests::testVariableAssign()
{
}

void ParserTests::testFullScript()
{
}

void ParserTests::testChunk()
{
}

void ParserTests::testNumber()
{
}
