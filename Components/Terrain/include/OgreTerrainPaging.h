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

#ifndef __Ogre_TerrainPaging_H__
#define __Ogre_TerrainPaging_H__

#include "OgreTerrainPrerequisites.h"
#include "OgrePagedWorldSection.h"
#include "OgrePageManager.h"



namespace Ogre
{
	class PagedWorld;
	class TerrainGroup;
	class TerrainPagedWorldSection;

	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Terrain
	*  Some details on the terrain component
	*  @{
	*/

	/** This class is the 'core' class for paging terrain, that will integrate
		with the larger paging system and provide the appropriate utility
		classes required. 
	@remarks
		You should construct this class after PageManager and before any PagedWorlds
		that might use it are created / loaded. Once constructed, it will make
		the "Terrain" PagedWorldSection type available, which uses a grid strategy for
		paging and uses TerrainGroup for the content. Other content can 
		be embedded in the pages too but the terrain is done like this in order
		to maintain connections between pages and other global data.
	@par
		Because PagedWorld and all attached classes have to be loadable from a stream, 
		most of the functionality is provided behind generalised interfaces. However,
		for constructing a paged terrain in code you can use utility methods on 
		this class. This procedurally created data can then be saved in a generic 
		form which will reconstruct on loading. Alternatively you can use the 
		generic methods and simply cast based on your prior knowledge of the types
		(or checking the type names exposed). 

	*/
	class _OgreTerrainExport TerrainPaging : public TerrainAlloc
	{
	public:
		/** Constructor.
		@param pageMgr The PageManager which this class should attach to.
		*/
		TerrainPaging(PageManager* pageMgr);
		~TerrainPaging();

		/** Create a TerrainPagedWorldSection.
		@remarks
			This is the simplest way to create a world section which is configured
			to contain terrain (among other objects if you want). You can also do this
			by calling PagedWorld::createSection with the type "Terrain" but there
			are more steps to configuring it that way (note: this is how loading 
			works though so it remains generic).
		@param world The PagedWorld that is to contain this section
		@param terrainGroup A TerrainGroup which must have been pre-configured before
			this call, to at least set the origin, the world size, and the file name
			convention.
		@param loadRadius The radius from the camera at which terrain pages will be loaded
		@param holdRadius The radius from the camera at which terrain pages will be held in memory
			but not loaded if they're not already. This must be larger than loadRadius and is used
			to minimise thrashing if the camera goes backwards and forwards at the loading border.
		@param sectionName An optional name to give the section (if none is
			provided, one will be generated)
		@param minX,minY,maxX,maxY The min/max page indexes that the world will try to load pages for, 
			as measured from the origin page at (0,0) by a signed index. The default is -10 to 10
			in each direction or 20x20 pages.
		@returns The world section which is already attached to and owned by the world you passed in. 
			There is no 'destroy' method because you destroy via the PagedWorld, this is just a
			helper function. 
		*/
		TerrainPagedWorldSection* createWorldSection(PagedWorld* world, TerrainGroup* terrainGroup, 
			Real loadRadius, Real holdRadius, const String& sectionName = StringUtil::BLANK, 
			int32 minX = -10, int32 minY = -10, int32 maxX = 10, int32 maxY = 10);

	protected:
		PageManager* mManager;

		class _OgreTerrainExport SectionFactory : public PagedWorldSectionFactory
		{
		public:
			static const String FACTORY_NAME;
			const String& getName() const;
			PagedWorldSection* createInstance(const String& name, PagedWorld* parent, SceneManager* sm);
			void destroyInstance(PagedWorldSection*);

		};

		SectionFactory mSectionFactory;

	};





	/** @} */
	/** @} */
}

#endif
