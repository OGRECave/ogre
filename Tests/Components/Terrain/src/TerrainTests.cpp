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

