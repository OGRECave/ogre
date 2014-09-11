/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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

#include "OgreExport.h"
#include "OgreMaxMeshXMLExport.h"
#include "OgreMaxVertex.h"

#include "max.h"
#include "plugapi.h"
#include "stdmat.h"
#include "impexp.h"
#include "CS/BipedApi.h"
#include "CS/KeyTrack.h"
#include "CS/phyexp.h"
#include "iparamb2.h"
#include "iskin.h"

#include "IGame/IGame.h"

namespace OgreMax
{
	MeshXMLExporter::MeshXMLExporter(const Config& config, MaterialMap& map) : m_materialMap(map), OgreMaxExporter(config)
	{
		m_createSkeletonLink = false;
		m_pGame = 0;

		m_currentBoneIndex = 0;				// used to map bone names to bone indices for vertex assignment and later skeleton export
	}

	MeshXMLExporter::~MeshXMLExporter()
	{
	}

	bool MeshXMLExporter::buildMeshXML(OutputMap& output)
	{
		return export(output);
	}


	// "callback" is called in response to the EnumTree() call made below. That call visits every node in the 
	// scene and calls this procedure for each one. 
	int MeshXMLExporter::callback(INode *node) {

		// SKELOBJ_CLASS_ID = 0x9125 = 37157
		// BIPED_CLASS_ID = 0x9155 = 37205
		// BIPSLAVE_CONTROL_CLASS_ID = 0x9154 = 37204
		// BIPBODY_CONTROL_CLASS_ID = 0x9156 = 37206
		// FOOTPRINT_CLASS_ID = 0x3011 = 12305
		// DUMMY_CLASS_ID = 0x876234 = 8872500

		TimeValue start = m_i->GetAnimRange().Start();
		Object *obj = node->EvalWorldState(start).obj;
		Class_ID cid = obj->ClassID();

		// nodes that have Biped controllers are bones -- ignore the ones that are dummies
		if (cid == Class_ID(DUMMY_CLASS_ID, 0))
			return TREE_CONTINUE;

		Control *c = node->GetTMController();
		if ((c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
			(c->ClassID() == BIPBODY_CONTROL_CLASS_ID) ||
			(c->ClassID() == FOOTPRINT_CLASS_ID)) {

				if (node->GetParentNode() != NULL) {
					// stick this in the bone-index map for later use
					m_boneIndexMap.insert(std::map< std::string, int >::value_type(std::string(node->GetName()), m_currentBoneIndex++));
				}

				return TREE_CONTINUE;
		}
		// if the node cannot be converted to a TriObject (mesh), ignore it
		if (!obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
			return TREE_CONTINUE;

		// create a list of nodes to process
		if (m_config.getExportSelected()) {
			if (node->Selected()) {
				m_nodeTab.Append(1, &node);
			}
		}
		else {
			m_nodeTab.Append(1, &node);
		}

		return TREE_CONTINUE;
	}

	////////////////////////////////////////////////////////////////////////////////
	// Regardless of whether the user chooses to export submesh-per-file or to put 
	// all submeshes in the same file, we will deal with submaterials by creating
	// "duplicate" vertices based on normal and texcoord differences. In other words,
	// "unique" vertices will be based on smoothing-group normals (if used, face normals if 
	// not) and face texcoords, not the inherent vertex normals and texcoords.
	// 
	// The basic concept is the same as used in the MaxScript exporter (as well as 
	// the exporters for other 3D tools): iterate the faces, extracting normal and
	// texcoord info along with the position and color data; vertices are "unique-ified"
	// as they are added to the vertex list.
	////////////////////////////////////////////////////////////////////////////////

	bool MeshXMLExporter::export(OutputMap& output) {

		try {

			// write XML to a strstream
			std::stringstream of;

			while (!m_submeshNames.empty())
				m_submeshNames.pop();

			if (m_pGame) {
				m_pGame->ReleaseIGame();
				m_pGame = 0;
			}

			m_ei->theScene->EnumTree(this);

			m_pGame = GetIGameInterface();
			IGameConversionManager* cm = GetConversionManager();
			cm->SetCoordSystem(IGameConversionManager::IGAME_OGL);
			m_pGame->InitialiseIGame(m_nodeTab, false);
			m_pGame->SetStaticFrame(0);
			int nodeCount = m_pGame->GetTopLevelNodeCount();

			if (nodeCount == 0) {
				MessageBox(GetActiveWindow(), "No nodes available to export, aborting...", "Nothing To Export", MB_ICONINFORMATION);
				m_pGame->ReleaseIGame();
				return false;
			}

			// if we are writing everything to one file, use the name provided when the user first started the export;
			// otherwise, create filenames on the basis of the node (submesh) names
			std::string fileName;
			IGameNode* node = m_pGame->GetTopLevelNode(0);
			if (!m_config.getExportMultipleFiles())
				fileName = m_config.getExportFilename();
			else {
				fileName = m_config.getExportPath() + "\\";
				fileName += node->GetName();
				fileName += ".mesh.xml";
			}

			// write start of XML data
			streamFileHeader(of);

			int nodeIdx = 0;
			std::map<std::string, std::string> materialScripts;

			while (nodeIdx < nodeCount) {

				std::string mtlName;
				IGameNode* node = m_pGame->GetTopLevelNode(nodeIdx);
				IGameObject* obj = node->GetIGameObject();

				// InitializeData() is important -- it performs all of the WSM/time eval for us; no data without it
				obj->InitializeData();
				
				IGameMaterial* mtl = node->GetNodeMaterial();
				if (mtl == NULL)
					mtlName = m_config.getDefaultMaterialName();
				else
					mtlName = mtl->GetMaterialName();

				// clean out any spaces the user left in their material name
				std::string::size_type pos;
				while ((pos = mtlName.find_first_of(' ')) != std::string::npos)
					mtlName.replace(pos, 1, _T("_"));

				if (materialScripts.find(mtlName) == materialScripts.end()) {

					// export new material script
					MaterialExporter mexp(m_config, m_materialMap);
					std::string script;

					mexp.buildMaterial(mtl, mtlName, script);
					materialScripts[mtlName] = script;
				}

				//if (streamSubmesh(of, node, mtlName))
				if (streamSubmesh(of, obj, mtlName))
					m_submeshNames.push(std::string(node->GetName()));

				node->ReleaseIGameObject();
				nodeIdx++;

				if (m_config.getExportMultipleFiles() || nodeIdx == nodeCount) {

					// write end of this file's XML
					streamFileFooter(of);

					// insert new filename --> stream pair into output map
					output[fileName] = std::string(of.str());
					of.str("");

					if (nodeIdx != nodeCount) {
						fileName = m_config.getExportPath() + "\\";
						node = m_pGame->GetTopLevelNode(nodeIdx);
						fileName += node->GetName();
						fileName += ".mesh.xml";

						// start over again with new data
						streamFileHeader(of);
					}
				}
			}

			m_pGame->ReleaseIGame();

			// export materials if we are writing those
			if (m_config.getExportMaterial()) {

				std::ofstream materialFile;
				materialFile.open((m_config.getExportPath() + "\\" + m_config.getMaterialFilename()).c_str(), std::ios::out);

				if (materialFile.is_open()) {
					for (std::map<std::string, std::string>::iterator it = materialScripts.begin();
						it != materialScripts.end(); ++it)
					{
						materialFile << it->second;
					}

					materialFile.close();
				}
			}

			return true;
		}
		catch (...) {
			MessageBox(GetActiveWindow(), "An unexpected error has occurred while trying to export, aborting", "Error", MB_ICONEXCLAMATION);

			if (m_pGame)
				m_pGame->ReleaseIGame();

			return false;
		}
	}

	bool MeshXMLExporter::streamFileHeader(std::ostream &of) {

		// write the XML header tags
		of << "<?xml version=\"1.0\"?>" << std::endl;
		of << "<mesh>" << std::endl;

		// *************** Export Submeshes ***************
		of << "\t<submeshes>" << std::endl;

		of.precision(6);
		of << std::fixed;

		return true;
	}

	bool MeshXMLExporter::streamFileFooter(std::ostream &of) {

		of << "\t</submeshes>" << std::endl;
		// *************** End Submeshes Export ***********

		// if there is a skeleton involved, link that filename here
		if (m_createSkeletonLink) {
			std::string skeletonFilename(m_skeletonFilename);
			skeletonFilename = skeletonFilename.substr(0, skeletonFilename.find(".xml"));

			of << "\t<skeletonlink name=\"" << skeletonFilename << "\" />" << std::endl;
		}


		// *************** Export Submesh Names ***************
		of << "\t<submeshnames>" << std::endl;

		int idx = 0;
		while (!m_submeshNames.empty()) {
			of << "\t\t<submeshname name=\"" << m_submeshNames.front() << "\" index=\"" << idx << "\" />" << std::endl;
			idx++;
			m_submeshNames.pop();
		}

		of << "\t</submeshnames>" << std::endl;
		// *************** End Submesh Names Export ***********

		of << "</mesh>" << std::endl;

		return true;
	}

	//bool MeshXMLExporter::streamSubmesh(std::ostream &of, IGameNode *node, std::string &mtlName) {
	bool MeshXMLExporter::streamSubmesh(std::ostream &of, IGameObject *obj, std::string &mtlName) {
		
		//IGameObject* obj = node->GetIGameObject();
		if (obj->GetIGameType() != IGameMesh::IGAME_MESH)
			return false;

		// InitializeData() is important -- it performs all of the WSM/time eval for us; no face data without it
//		obj->InitializeData();
		IGameMesh* mesh = (IGameMesh*) obj;
		int vertCount = mesh->GetNumberOfVerts();
		int faceCount = mesh->GetNumberOfFaces();

		Tab<int> matIds = mesh->GetActiveMatIDs();
		Tab<DWORD> smGrpIds = mesh->GetActiveSmgrps();
		Tab<int> texMaps = mesh->GetActiveMapChannelNum();

		of << "\t\t<submesh ";
		
		if (mtlName.length() > 0)
			of << "material=\"" << mtlName << "\" ";

		of << "usesharedvertices=\"false\" use32bitindexes=\"";
		of << (vertCount > 65535);
		of << "\">" << std::endl;

		// *************** Export Face List ***************
		of << "\t\t\t<faces count=\"" << faceCount << "\">" << std::endl;

		//std::vector<UVVert

		// iterate the face list, putting vertices in the list for this submesh
		VertexList vertexList;

		for (int i=0; i<faceCount; i++) {

			of << "\t\t\t\t<face";
			FaceEx* face = mesh->GetFace(i);

			// do this for each vertex on the face
			for (int vi=0; vi<3; vi++) {
				Point3 p = mesh->GetVertex(face->vert[vi]);

				Vertex v(p.x, p.y, p.z);

				if (m_config.getExportVertexColours()) {
					Point3 c = mesh->GetColorVertex(face->vert[vi]);
					float a = mesh->GetAlphaVertex(face->vert[vi]);

					v.setColour(c.x, c.y, c.z, a);
				}

				Point3 n = mesh->GetNormal(face, vi);
				v.setNormal(n.x, n.y, n.z);

				// get each set of texcoords for this vertex
				for (int ch=0; ch < texMaps.Count(); ch++) {

					Point3 tv;
					DWORD indices[3];

					if (mesh->GetMapFaceIndex(texMaps[ch], i, indices))
						tv = mesh->GetMapVertex(texMaps[ch], indices[vi]);
					else
						tv = mesh->GetMapVertex(texMaps[ch], face->vert[vi]);

					v.addTexCoord(texMaps[ch], tv.x, tv.y, tv.z);
				}

				int actualVertexIndex = vertexList.add(v);

				of << " v" << vi + 1 << "=\"" << actualVertexIndex << "\"";
			}
 
			of << " />" << std::endl;
		}

		of << "\t\t\t</faces>" << std::endl;
		// *************** End Export Face List ***************


		// *************** Export Geometry ***************
		of << "\t\t\t<geometry vertexcount=\"" << vertexList.size() << "\">" << std::endl;

		// *************** Export Vertex Buffer ***************
		bool exportNormals = true;

		of << std::boolalpha;
		of << "\t\t\t\t<vertexbuffer positions=\"true\" normals=\"" << exportNormals << "\" colours_diffuse=\"" << m_config.getExportVertexColours() << "\" texture_coords=\"" << texMaps.Count() << "\"";
		
		for (i=0; i<texMaps.Count(); i++)
			of << " texture_coords_dimensions_" << i << "=\"2\"";
		
		of << ">" << std::endl;

		int numVerts = vertexList.size();
		for (i=0; i < numVerts; i++) {

			const Vertex& v = vertexList.front();

			of << "\t\t\t\t\t<vertex>" << std::endl;
			of << std::showpoint;

			const Ogre::Vector3& p = v.getPosition();
			float x = p.x;
			float y = p.y;
			float z = p.z;

			of << "\t\t\t\t\t\t<position x=\"" << x << "\" y=\"" << y << "\" z=\"" << z << "\" />" << std::endl;

			if (m_config.getExportVertexColours()) {

				float r = v.getColour().r;
				float g = v.getColour().g;
				float b = v.getColour().b;

				of << "\t\t\t\t\t\t<colour_diffuse value=\"\t" << r << "\t" << g << "\t" << b << "\" />" << std::endl;
			}

			if (exportNormals) {

				const Ogre::Vector3& n = v.getNormal();
				float x = n.x;
				float y = n.y;
				float z = n.z;

				of << "\t\t\t\t\t\t<normal x=\"" << x << "\" y=\"" << y << "\" z=\"" << z << "\" />" << std::endl;
			}

			// output the tex coords for each map used
			for (int ti=0; ti<texMaps.Count(); ti++) {
				int texMap = texMaps[ti];

				const Ogre::Vector3& uvw = v.getUVW(texMap); 

				switch (m_config.getTexCoord2D()) {
					case OgreMax::UV:
						of << "\t\t\t\t\t\t<texcoord u=\"" << uvw.x << "\" v=\"" << (1.0 - uvw.y) << "\" />" << std::endl; 
						break;
					case OgreMax::VW:
						of << "\t\t\t\t\t\t<texcoord v=\"" << uvw.y << "\" w=\"" << (1.0 - uvw.z) << "\" />" << std::endl; 
						break;
					case OgreMax::WU:
						of << "\t\t\t\t\t\t<texcoord w=\"" << uvw.z << "\" u=\"" << (1.0 - uvw.x) << "\" />" << std::endl; 
						break;
				}
			}
			
			of << std::noshowpoint;
			of << "\t\t\t\t\t</vertex>" << std::endl;
			vertexList.pop();
		}

		of << "\t\t\t\t</vertexbuffer>" << std::endl;
		// *************** End Export Vertex Buffer ***************

		of << "\t\t\t</geometry>" << std::endl;
		// *************** End Export Geometry ***********

		of << "\t\t</submesh>" << std::endl;

		// this skin extraction code based on an article found here:
		// http://www.cfxweb.net/modules.php?name=News&file=article&sid=1029
/*		Object *oRef = node->GetObjectRef();

		if (oRef->SuperClassID() == GEN_DERIVOB_CLASS_ID) {
			IDerivedObject *dObj = (IDerivedObject *)oRef;
			Modifier *oMod = dObj->GetModifier(0);

			if (oMod->ClassID() == SKIN_CLASSID) {

				// flag the export of a skeleton link element
				m_createSkeletonLink = true;

				// stream the boneassignments element
				streamBoneAssignments(of, oMod, node);
			}
		}

		of << "\t\t</submesh>" << std::endl;

		if (obj != tri)
			delete tri;
*/
//		node->ReleaseIGameObject();
		return true;
	}

	//bool MeshXMLExporter::streamBoneAssignments(std::ostream &of, Modifier *oMod, INode *node) {
	bool MeshXMLExporter::streamBoneAssignments(std::ostream &of, Modifier *oMod, IGameNode *node) {

		// wrangle a pointer to the skinning data
/*		ISkin *skin = (ISkin *) oMod->GetInterface(I_SKIN);
		ISkinContextData *skinData = skin->GetContextInterface(node);

		// loop through all the vertices, writing out skinning data as we go
		int skinnedVertexCount = skinData->GetNumPoints();

		if (skinnedVertexCount > 0) {

			of << "\t\t\t<boneassignments>" << std::endl;

			int i;
			for(i=0; i<skinnedVertexCount; i++) {

				// grab the bone indices for this vertex
				int vertexBoneInfluences = skinData->GetNumAssignedBones(i);

				if (vertexBoneInfluences > 0) {

					int j;
					for (j=0; j < vertexBoneInfluences; j++) {

						// get weight per bone influence -- Ogre will ignore bones above
						// 4 and sum the weights to make it work, so leverage that feature
						int boneIdx = getBoneIndex(skin->GetBoneName(skinData->GetAssignedBone(i, j)));
						float vertexWeight = skinData->GetBoneWeight(i, j);

						of << "\t\t\t\t<vertexboneassignment vertexindex=\"" << i << "\" boneindex=\"" << boneIdx << "\" weight=\"" << vertexWeight << "\" />" << std::endl;
					}
				}
			}
		
			of << "\t\t\t</boneassignments>" << std::endl;
		}
*/
		return true;
	}

	int MeshXMLExporter::getBoneIndex(char *boneName) {

		std::map< std::string, int >::const_iterator it = m_boneIndexMap.find(std::string(boneName));
		if (it == m_boneIndexMap.end()) {
			m_boneIndexMap.insert(std::map< std::string, int >::value_type(std::string(boneName), m_currentBoneIndex));
			return m_currentBoneIndex++;
		}
		else
			return it->second;
	}

#if 0
	bool MeshXMLExporter::export(OutputMap& output) {

		try {

			// all options have been set when actions were taken, so we can just start exporting stuff here

			// write XML to a strstream
			std::stringstream of;
			bool rtn = true;

			// clear the node list and submesh name list
			m_nodeList.clear();
			while (!m_submeshNames.empty())
				m_submeshNames.pop();

			// populate the node list
			m_ei->theScene->EnumTree(this);

			// check to see if there's anything to export
			if (m_nodeList.size() == 0) {
				MessageBox(GetActiveWindow(), "No nodes available to export, aborting...", "Nothing To Export", MB_ICONINFORMATION);
				return false;
			}

			// if we are writing everything to one file, use the name provided when the user first started the export;
			// otherwise, create filenames on the basis of the node (submesh) names
			std::string fileName;
			if (!m_config.getExportMultipleFiles())
				fileName = m_config.getExportFilename();
			else {
				fileName = m_config.getExportPath() + "\\";
				INode *n = *(m_nodeList.begin());
				fileName += n->GetName();
				fileName += ".mesh.xml";
			}

			// write start of XML data
			streamFileHeader(of);

			std::list<INode *>::iterator it = m_nodeList.begin();

			// user request -- if no material is assigned to this node, then use an export-specified 
			// default material name
			Mtl *nodeMtl = (*it)->GetMtl();
			if (nodeMtl == NULL) {
				m_materialMap[m_config.getDefaultMaterialName()] = 0; // we can do this as often as we like -- it will just overwrite zero with zero
			}

			while (it != m_nodeList.end()) {

				// we already filtered out nodes that had NULL materials, and those that could
				// not be converted to TriObjects, so now we know everything we have is good

				INode *node = *it;
				std::string mtlName;
				
				Mtl *mtl = node->GetMtl();
				if (mtl == NULL)
					mtlName = m_config.getDefaultMaterialName();
				else
					mtlName = mtl->GetName();

				// map a material name to its Mtl pointer so that we can retrieve these later
				std::string::size_type pos;

				// clean out any spaces the user left in their material name
				while ((pos = mtlName.find_first_of(' ')) != std::string::npos)
					mtlName.replace(pos, 1, _T("_"));

				m_materialMap[mtlName] = mtl;

				if (streamSubmesh(of, node, mtlName))
					m_submeshNames.push(std::string((*it)->GetName()));

				it++;

				// if we are doing one mesh per file, then close this one and open a new one
				if (m_config.getExportMultipleFiles() || it == m_nodeList.end()) {

					// write end of this file's XML
					streamFileFooter(of);

					// insert new filename --> stream pair into output map
					output[fileName] = std::string(of.str());
					of.str("");

					if (it != m_nodeList.end()) {
						fileName = m_config.getExportPath() + "\\";
						INode *n = *it;
						fileName += n->GetName();
						fileName += ".mesh.xml";

						// start over again with new data
						streamFileHeader(of);
					}
				}
			}

			return rtn;
		}
		catch (...) {
			MessageBox(GetActiveWindow(), "An unexpected error has occurred while trying to export, aborting", "Error", MB_ICONEXCLAMATION);
			return false;
		}
	}

	bool MeshXMLExporter::streamFileHeader(std::ostream &of) {

		// write the XML header tags
		of << "<?xml version=\"1.0\"?>" << std::endl;
		of << "<mesh>" << std::endl;

		// *************** Export Submeshes ***************
		of << "\t<submeshes>" << std::endl;

		of.precision(6);
		of << std::fixed;

		return true;
	}

	bool MeshXMLExporter::streamFileFooter(std::ostream &of) {

		of << "\t</submeshes>" << std::endl;
		// *************** End Submeshes Export ***********

		// if there is a skeleton involved, link that filename here
		if (m_createSkeletonLink) {
			std::string skeletonFilename(m_skeletonFilename);
			skeletonFilename = skeletonFilename.substr(0, skeletonFilename.find(".xml"));

			of << "\t<skeletonlink name=\"" << skeletonFilename << "\" />" << std::endl;
		}


		// *************** Export Submesh Names ***************
		of << "\t<submeshnames>" << std::endl;

		int idx = 0;
		while (!m_submeshNames.empty()) {
			of << "\t\t<submeshname name=\"" << m_submeshNames.front() << "\" index=\"" << idx << "\" />" << std::endl;
			idx++;
			m_submeshNames.pop();
		}

		of << "\t</submeshnames>" << std::endl;
		// *************** End Submesh Names Export ***********

		of << "</mesh>" << std::endl;

		return true;
	}

	bool MeshXMLExporter::streamSubmesh(std::ostream &of, INode *node, std::string &mtlName) {
		
		Object *obj = node->EvalWorldState(m_i->GetTime()).obj;
		TriObject *tri = (TriObject *) obj->ConvertToType(m_i->GetTime(), Class_ID(TRIOBJ_CLASS_ID, 0));

		int i;
		Mesh mesh = tri->GetMesh();
		Mtl* mtl = node->GetMtl();
		Matrix3 ptm = node->GetObjTMAfterWSM(m_i->GetTime());

		int vertCount = mesh.getNumVerts();
		int faceCount = mesh.getNumFaces();

		of << "\t\t<submesh ";
		
		if (mtlName.length() > 0)
			of << "material=\"" << mtlName << "\" ";

		of << "usesharedvertices=\"false\" use32bitindexes=\"";
		of << (vertCount > 65535);
		of << "\">" << std::endl;

		// *************** Export Face List ***************
		of << "\t\t\t<faces count=\"" << faceCount << "\">" << std::endl;

		// store UV coords from the faces, not from the vertices themselves
		std::vector<UVVert> texCoords;
		texCoords.reserve(vertCount);

		for (i=0; i<faceCount; i++) {
			int v1 = mesh.faces[i].v[0];
			int v2 = mesh.faces[i].v[1];
			int v3 = mesh.faces[i].v[2];

			UVVert uv;
			uv.x = mesh.tvFace[i].t[0];
			uv.y = mesh.tvFace[i].t[1];
			uv.z = mesh.tvFace[i].t[2];

			if (m_config.getInvertNormals()) {
				int tmp = v2;
				v2 = v3;
				v3 = tmp;
			}

			of << "\t\t\t\t<face v1=\"" << v1 << "\" v2=\"" << v2 << "\" v3=\"" << v3 << "\" />" << std::endl;
		}

		of << "\t\t\t</faces>" << std::endl;
		// *************** End Export Face List ***************


		// *************** Export Geometry ***************
		of << "\t\t\t<geometry vertexcount=\"" << vertCount << "\">" << std::endl;

		// *************** Export Vertex Buffer ***************
		if (m_config.getRebuildNormals()) {
			mesh.buildNormals();
		}

		bool exportNormals = (mesh.normalsBuilt > 0);

		of << std::boolalpha;

		// NB: we don't export tex coords unless we are exporting a material as well
		// NB: apparently Max cannot just tell us how many texmaps a material uses...so we have to add them up

		// we need a list of enabled map indices for later
		std::list<unsigned int> texMaps;
		if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
			StdMat* sMtl = (StdMat*) mtl;
			if (m_config.getExportMaterial()) {
				for (int t=0; t<sMtl->NumSubTexmaps(); t++)
		
					if (sMtl->MapEnabled(t)) {
						BitmapTex* TMap = (BitmapTex*)sMtl->GetSubTexmap(ID_DI);
						if (!TMap)
						return false;

						//Check if we have a Bitmap Texture material (BitmapTex)
						if (TMap->ClassID() != Class_ID(BMTEX_CLASS_ID,0))
						return false; //Not interesting

						UVGen * pUVGen = TMap->GetTheUVGen() ;
						if(!pUVGen)
						return false;

						const int iAxis = pUVGen->GetAxis();
						texMaps.push_back(t);
				}
			}
		}

		of << "\t\t\t\t<vertexbuffer positions=\"true\" normals=\"" << exportNormals << "\" colours_diffuse=\"" << m_config.getExportVertexColours() << "\" texture_coords=\"" << texMaps.size() << "\"";
		
		for (i=0; i<texMaps.size(); i++)
			of << " texture_coords_dimensions_" << i << "=\"2\"";
		
		of << ">" << std::endl;

		for (i=0; i<vertCount; i++) {
			Point3 v = mesh.getVert(i);

			// transform v into parent's coord system
			v = v * ptm;

			Point3 vc;
			vc.x = 0.0f;
			vc.y = 0.0f;
			vc.z = 0.0f;

			if (mesh.vertCol != 0) {
				vc = mesh.vertCol[i];
			}

			of << "\t\t\t\t\t<vertex>" << std::endl;
			of << std::showpoint;

			float x = v.x;// * m_scale;
			float y = v.y;// * m_scale;
			float z = v.z;// * m_scale;

			if (m_config.getInvertYZ()) {
				float tmp = y;
				y = z;
				z = -tmp;
			}

			of << "\t\t\t\t\t\t<position x=\"" << x << "\" y=\"" << y << "\" z=\"" << z << "\" />" << std::endl;

			if (m_config.getExportVertexColours())
				of << "\t\t\t\t\t\t<colour_diffuse value=\"\t" << vc.x << "\t" << vc.y << "\t" << vc.z << "\" />" << std::endl;
			
			if (exportNormals) {
	//			Point3 n = mesh.getNormal(i);
				RVertex *rv = mesh.getRVertPtr(i);
				Point3 n = rv->rn.getNormal();
					n = n.Normalize();

				float x = n.x;
				float y = n.y;
				float z = n.z;

				if (m_config.getInvertYZ()) {
					float tmp = y;
					y = z;
					z = -tmp;
				}
				
				if (m_config.getInvertNormals())
					of << "\t\t\t\t\t\t<normal x=\"" << -x << "\" y=\"" << -y << "\" z=\"" << -z << "\" />" << std::endl;
				else
					of << "\t\t\t\t\t\t<normal x=\"" << x << "\" y=\"" << y << "\" z=\"" << z << "\" />" << std::endl;
			}

			// output the tex coords for each map used
			std::list<unsigned int>::iterator texMap = texMaps.begin();
			while (texMap != texMaps.end()) {
				UVVert uv;
				
				if (*texMap <= 1)
					uv = mesh.tVerts[i];
				else
					uv = mesh.mapVerts(*texMap)[i];

				switch (m_config.getTexCoord2D()) {
					case OgreMax::UV:
						of << "\t\t\t\t\t\t<texcoord u=\"" << uv.x << "\" v=\"" << (1.0 - uv.y) << "\" />" << std::endl; 
						break;
					case OgreMax::VW:
						of << "\t\t\t\t\t\t<texcoord v=\"" << uv.y << "\" w=\"" << (1.0 - uv.z) << "\" />" << std::endl; 
						break;
					case OgreMax::WU:
						of << "\t\t\t\t\t\t<texcoord w=\"" << uv.z << "\" u=\"" << (1.0 - uv.x) << "\" />" << std::endl; 
						break;
				}

				texMap++;
			}
			
			of << std::noshowpoint;
			of << "\t\t\t\t\t</vertex>" << std::endl;
		}

		of << "\t\t\t\t</vertexbuffer>" << std::endl;
		// *************** End Export Vertex Buffer ***************

		of << "\t\t\t</geometry>" << std::endl;
		// *************** End Export Geometry ***********

		// this skin extraction code based on an article found here:
		// http://www.cfxweb.net/modules.php?name=News&file=article&sid=1029
		Object *oRef = node->GetObjectRef();

		if (oRef->SuperClassID() == GEN_DERIVOB_CLASS_ID) {
			IDerivedObject *dObj = (IDerivedObject *)oRef;
			Modifier *oMod = dObj->GetModifier(0);

			if (oMod->ClassID() == SKIN_CLASSID) {

				// flag the export of a skeleton link element
				m_createSkeletonLink = true;

				// stream the boneassignments element
				streamBoneAssignments(of, oMod, node);
			}
		}

		of << "\t\t</submesh>" << std::endl;

		if (obj != tri)
			delete tri;

		return true;
	}

	bool MeshXMLExporter::streamBoneAssignments(std::ostream &of, Modifier *oMod, INode *node) {
		

		// wrangle a pointer to the skinning data
		ISkin *skin = (ISkin *) oMod->GetInterface(I_SKIN);
		ISkinContextData *skinData = skin->GetContextInterface(node);

		// loop through all the vertices, writing out skinning data as we go
		int skinnedVertexCount = skinData->GetNumPoints();

		if (skinnedVertexCount > 0) {

			of << "\t\t\t<boneassignments>" << std::endl;

			int i;
			for(i=0; i<skinnedVertexCount; i++) {

				// grab the bone indices for this vertex
				int vertexBoneInfluences = skinData->GetNumAssignedBones(i);

				if (vertexBoneInfluences > 0) {

					int j;
					for (j=0; j < vertexBoneInfluences; j++) {

						// get weight per bone influence -- Ogre will ignore bones above
						// 4 and sum the weights to make it work, so leverage that feature
						int boneIdx = getBoneIndex(skin->GetBoneName(skinData->GetAssignedBone(i, j)));
						float vertexWeight = skinData->GetBoneWeight(i, j);

						of << "\t\t\t\t<vertexboneassignment vertexindex=\"" << i << "\" boneindex=\"" << boneIdx << "\" weight=\"" << vertexWeight << "\" />" << std::endl;
					}
				}
			}
		
			of << "\t\t\t</boneassignments>" << std::endl;
		}

		return true;
	}

	int MeshXMLExporter::getBoneIndex(char *boneName) {

		std::map< std::string, int >::const_iterator it = m_boneIndexMap.find(std::string(boneName));
		if (it == m_boneIndexMap.end()) {
			m_boneIndexMap.insert(std::map< std::string, int >::value_type(std::string(boneName), m_currentBoneIndex));
			return m_currentBoneIndex++;
		}
		else
			return it->second;
	}

	// pulled directly from the Sparks site: 
	// http://sparks.discreet.com/Knowledgebase/sdkdocs_v8/prog/cs/cs_physique_export.html
	// Also available in the SDK docs. Used to find out if this node has a physique modifier or not.
	// If it does, it returns a pointer to the modifier, and if not, returns NULL. This can be used to 
	// determine whether a node is bone or mesh -- mesh nodes will have Physique modifiers, bone nodes
	// will not.
	Modifier* MeshXMLExporter::FindPhysiqueModifier (INode* nodePtr)
	{
		// Get object from node. Abort if no object.
		Object* ObjectPtr = nodePtr->GetObjectRef();

		if (!ObjectPtr) return NULL;

		// Is derived object ?
		while (ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID && ObjectPtr)
		{
			// Yes -> Cast.
			IDerivedObject *DerivedObjectPtr = (IDerivedObject *)(ObjectPtr);
							
			// Iterate over all entries of the modifier stack.
			int ModStackIndex = 0;
			while (ModStackIndex < DerivedObjectPtr->NumModifiers())
			{
				// Get current modifier.
				Modifier* ModifierPtr = DerivedObjectPtr->GetModifier(ModStackIndex);

				// Is this Physique ?
				if (ModifierPtr->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B))
				{
					// Yes -> Exit.
					return ModifierPtr;
				}

				// Next modifier stack entry.
				ModStackIndex++;
			}
			ObjectPtr = DerivedObjectPtr->GetObjRef();
		}

		// Not found.
		return NULL;
	}
#endif

}

