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
#include "OgreTerrain.h"
#include "OgreStreamSerialiser.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	const uint32 Terrain::TERRAIN_CHUNK_ID = StreamSerialiser::makeIdentifier("TERR");
	const uint16 Terrain::TERRAIN_CHUNK_VERSION = 1;
	//---------------------------------------------------------------------
	Terrain::Terrain()
	{

	}
	//---------------------------------------------------------------------
	Terrain::~Terrain()
	{
	}
	//---------------------------------------------------------------------
	void Terrain::save(StreamSerialiser& stream)
	{
		stream.writeChunkBegin(TERRAIN_CHUNK_ID, TERRAIN_CHUNK_VERSION);


		// TODO


		stream.writeChunkEnd(TERRAIN_CHUNK_ID);
	}
	//---------------------------------------------------------------------
	bool Terrain::prepare(StreamSerialiser& stream)
	{
		if (!stream.readChunkBegin(TERRAIN_CHUNK_ID, TERRAIN_CHUNK_VERSION))
			return false;

		// TODO

		stream.readChunkEnd(TERRAIN_CHUNK_ID);

		return true;
	}
	//---------------------------------------------------------------------
	bool Terrain::prepare(const ImportData& importData)
	{
		// TODO

		return true;

	}
	//---------------------------------------------------------------------
	void Terrain::load()
	{
		// TODO
	}
	//---------------------------------------------------------------------
	void Terrain::unload()
	{
		// TODO
	}
	//---------------------------------------------------------------------
	void Terrain::unprepare()
	{
		// TODO
	}
	//---------------------------------------------------------------------

}

