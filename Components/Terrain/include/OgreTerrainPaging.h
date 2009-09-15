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
#include "OgrePageContent.h"
#include "OgrePageContentFactory.h"



namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Terrain
	*  Some details on the terrain component
	*  @{
	*/


	/** Specialisation of page content for a page of terrain.
	@par
		This class forms a bridge between the paging system and a chunk of terrain.

		The data format for this in a file is:<br/>
		<b>TerrainContentData (Identifier 'TERR')</b>\n
		[Version 1]
		<table>
		<tr>
			<td><b>Name</b></td>
			<td><b>Type</b></td>
			<td><b>Description</b></td>
		</tr>
		<tr>
			<td>Terrain orientation</td>
			<td>uint8</td>
			<td>The orientation of the terrain; XZ = 0, XY = 1, YZ = 2</td>
		</tr>
		</table>
	*/
	class _OgreTerrainExport TerrainPageContent : public PageContent
	{
	public:
		TerrainPageContent(PageContentFactory* creator);
		virtual ~TerrainPageContent();

		// Overridden from PageContent
		void save(StreamSerialiser& stream);

	protected:
		// Overridden from PageLoadableUnit
		bool prepareImpl(StreamSerialiser& stream);
		void loadImpl();
		void unloadImpl();
		void unprepareImpl();

		/// The actual terrain
		Terrain* mTerrain;
	};


	/// Factory class for TerrainPageContent
	class _OgreTerrainExport TerrainPageContentFactory : public PageContentFactory
	{
	public:
		static String FACTORY_NAME;

		TerrainPageContentFactory() {}
		~TerrainPageContentFactory() {}

		const String& getName() const { return FACTORY_NAME; }

		PageContent* createInstance() 
		{
			return OGRE_NEW TerrainPageContent(this); 
		}
		void destroyInstance(PageContent* c)
		{
			OGRE_DELETE c;
		}
	};


	/** @} */
	/** @} */
}

#endif
