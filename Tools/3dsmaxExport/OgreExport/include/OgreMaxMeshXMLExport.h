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

#if !defined(__OGREMAX_MESHXML_EXPORTER_H__)
#define __OGREMAX_MESHXML_EXPORTER_H__

#include "OgreMaxConfig.h"
#include "OgreMaxExport.h"

#include <string>
#include <queue>
#include <map>
#include "tab.h"

class MaterialMap;
class IGameScene;
class IGameNode;
class IGameObject;
class IGameMaterial;

namespace OgreMax {

	class MeshXMLExporter  : public OgreMaxExporter, public ITreeEnumProc
	{
		typedef struct {
			std::string name;
			int start;
			int end;
		} NamedAnimation;

	public:
		typedef std::map< std::string, std::string > OutputMap;

		MeshXMLExporter(const Config& config, MaterialMap& materialMap);
		virtual ~MeshXMLExporter();

		// generates a list of INode* for assembly into Ogre::Mesh form; returns
		// a map of filename --> XML stringstream.
		bool buildMeshXML(OutputMap& output);

	protected:
		// mesh file stream functions
		bool streamFileHeader(std::ostream &of);
		bool streamFileFooter(std::ostream &of);
		bool streamSubmesh(std::ostream &of, IGameObject *obj, std::string &mtlName);
		bool streamBoneAssignments(std::ostream &of, Modifier *mod, IGameNode *node);

		int getBoneIndex(char *name);
		std::string removeSpaces(const std::string &s);

		int callback(INode *node);

	private:

		bool export(OutputMap& output);

		bool m_createSkeletonLink;
		std::string m_exportFilename;			// filename provided by
		std::string m_filename;			// filename provided by
		std::string m_skeletonFilename;
		std::queue< std::string > m_submeshNames;
		MaterialMap& m_materialMap;
		std::map< std::string, int > m_boneIndexMap;
		int m_currentBoneIndex;

		IGameScene*	m_pGame;
		Tab<INode*> m_nodeTab;
	};
}

#endif