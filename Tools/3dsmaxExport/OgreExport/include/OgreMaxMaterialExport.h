/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 The OGRE Team
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
-----------------------------------------------------------------------------
*/

#if !defined(__OGREMAX_MATERIAL_EXPORTER_H__)
#define __OGREMAX_MATERIAL_EXPORTER_H__

#include "OgreMaxConfig.h"
#include "OgreMaxExport.h"

#include <string>
#include <sstream>
#include <queue>
#include <map>
#include <list>
#include <iosfwd>

//class Mtl;
class IGameMaterial;

namespace OgreMax {

	//typedef std::map< std::string, Mtl* > MaterialMap;
	typedef std::map< std::string, IGameMaterial* > MaterialMap;

	class MaterialExporter  : public OgreMaxExporter
	{

	public:
		MaterialExporter(const Config& config, MaterialMap& map);
		virtual ~MaterialExporter();

		// take in a list of INode* for assembly into Ogre::Mesh form; returns
		// a map of filename --> XML stringstream.
		bool buildMaterial(std::string& output);
		bool buildMaterial(IGameMaterial *mtl, const std::string& matName, std::string &of);

	protected:
		// material file stream functions
		bool streamPass(std::ostream &of, IGameMaterial *mtl);
		bool streamMaterial(std::ostream &of);

		std::string removeSpaces(const std::string &s);

	private:

		MaterialMap& m_materialMap;
	};
}

#endif
