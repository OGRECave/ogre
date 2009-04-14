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
#include "PageCoreTests.h"
#include "OgrePaging.h"

CPPUNIT_TEST_SUITE_REGISTRATION( PageCoreTests );

void PageCoreTests::setUp()
{
	mRoot = OGRE_NEW Root();
	mPageManager = OGRE_NEW PageManager();

	mRoot->addResourceLocation("./", "FileSystem");

	mSceneMgr = mRoot->createSceneManager(ST_GENERIC);

}

void PageCoreTests::tearDown()
{
	OGRE_DELETE mPageManager;
	OGRE_DELETE mRoot;
}


void PageCoreTests::testSimpleCreateSaveLoadWorld()
{
	String worldName = "MyWorld";
	String filename = "myworld.world";
	String sectionName1 = "Section1";
	String sectionName2 = "Section2";
	PagedWorld* world = mPageManager->createWorld(worldName);
	PagedWorldSection* section = world->createSection("Grid2D", mSceneMgr, sectionName1);
	section = world->createSection("Grid2D", mSceneMgr, sectionName2);

	// Create a page
	Page* p = section->loadOrCreatePage(Vector3::ZERO);

	SimplePageContentCollection* coll = static_cast<SimplePageContentCollection*>(
		p->createContentCollection("Simple"));

	TerrainPageContent* terrain = static_cast<TerrainPageContent*>(
		coll->createContent("Terrain"));

	// manually set page to loaded since we've populated it
	p->setLoaded();
	
	world->save(filename);

	mPageManager->destroyWorld(world);
	world = 0;

	world = mPageManager->loadWorld(filename);

	CPPUNIT_ASSERT_EQUAL(worldName, world->getName());
	CPPUNIT_ASSERT_EQUAL((size_t)2, world->getSectionCount());

	section = world->getSection(sectionName1);
	CPPUNIT_ASSERT(section != 0);
	section = world->getSection(sectionName2);
	CPPUNIT_ASSERT(section != 0);



}

