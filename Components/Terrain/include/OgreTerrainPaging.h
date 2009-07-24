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
