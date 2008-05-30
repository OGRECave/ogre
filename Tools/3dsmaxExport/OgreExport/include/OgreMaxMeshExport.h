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

#include "OgrePrerequisites.h"
#include "OgreMesh.h"

#include "OgreMaxConfig.h"
#include "OgreMaxExport.h"

#include "max.h"
#include "INode.h"
#include <list>

typedef std::list<INode*> NodeList;

namespace OgreMax {

	class MeshExporter : public OgreMaxExporter
	{
	public:

		MeshExporter(const Config& config);
		virtual ~MeshExporter();

		// take in a list of INode* for assembly into Ogre::Mesh form; returns
		// a MeshPtr pointing to the newly minted Ogre Mesh object.
		Ogre::MeshPtr buildMesh(NodeList nodeList);

	private:

		// Pointer to Mesh object for export
		Ogre::MeshPtr	m_pMesh;
	};
}