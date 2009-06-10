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
#include "TerrainTests.h"
#include "OgreTerrain.h"
#include "OgreConfigFile.h"
#include "OgreResourceGroupManager.h"


CPPUNIT_TEST_SUITE_REGISTRATION( TerrainTests );

void TerrainTests::setUp()
{
	mRoot = OGRE_NEW Root();

	// Load resource paths from config file
	ConfigFile cf;
	cf.load("resources.cfg");

	// Go through all sections & settings in the file
	ConfigFile::SectionIterator seci = cf.getSectionIterator();

	String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		ConfigFile::SettingsMultiMap *settings = seci.getNext();
		ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			ResourceGroupManager::getSingleton().addResourceLocation(
				archName, typeName, secName);

		}
	}

	mSceneMgr = mRoot->createSceneManager(ST_GENERIC);

}

void TerrainTests::tearDown()
{
	OGRE_DELETE mRoot;
}


void TerrainTests::testCreate()
{
	Terrain* t = OGRE_NEW Terrain(mSceneMgr);
	Image img;
	img.load("terrain.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	Terrain::ImportData imp;
	imp.inputImage = &img;
	imp.terrainSize = 513;
	imp.worldSize = 1000;
	imp.minBatchSize = 33;
	imp.maxBatchSize = 65;
	t->prepare(imp);
	// don't load, this requires GPU access
	//t->load();
	

	



	OGRE_DELETE t;
}

