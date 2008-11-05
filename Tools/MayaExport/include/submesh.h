////////////////////////////////////////////////////////////////////////////////
// submesh.h
// Author       : Francesco Giordana
// Sponsored by : Anygma N.V. (http://www.nazooka.com)
// Start Date   : January 13, 2005
// Copyright    : (C) 2006 by Francesco Giordana
// Email        : fra.giordana@tiscali.it
////////////////////////////////////////////////////////////////////////////////

/*********************************************************************************
*                                                                                *
*   This program is free software; you can redistribute it and/or modify         *
*   it under the terms of the GNU Lesser General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or            *
*   (at your option) any later version.                                          *
*                                                                                *
**********************************************************************************/

#ifndef _SUBMESH_H
#define _SUBMESH_H

#include "mayaExportLayer.h"
#include "paramList.h"
#include "materialSet.h"
#include "animation.h"
#include "vertex.h"
#include "blendshape.h"

namespace OgreMayaExporter
{
	/***** Class Submesh *****/
	class Submesh
	{
	public:
		//constructor
		Submesh(const MString& name = "");
		//destructor
		~Submesh();
		//clear data
		void clear();
		//load data
		MStatus loadMaterial(MObject& shader,MStringArray& uvsets,ParamList& params);
		MStatus load(const MDagPath& dag,std::vector<face>& faces, std::vector<vertexInfo>& vertInfo, MPointArray& points,
			MFloatVectorArray& normals, MStringArray& texcoordsets,ParamList& params,bool opposite = false); 
		//load a keyframe for the whole mesh
		MStatus loadKeyframe(Track& t,float time,ParamList& params);
		//get number of triangles composing the submesh
		long numTriangles();
		//get number of vertices
		long numVertices();
		//get submesh name
		MString& name();
		//write submesh data to an Ogre compatible mesh
		MStatus createOgreSubmesh(Ogre::MeshPtr pMesh,const ParamList& params);
		//create an Ogre compatible vertex buffer
		MStatus createOgreVertexBuffer(Ogre::SubMesh* pSubmesh,Ogre::VertexDeclaration* pDecl,const std::vector<vertex>& vertices);

	public:
		//public members
		MString m_name;
		Material* m_pMaterial;
		long m_numTriangles;
		long m_numVertices;
		std::vector<long> m_indices;
		std::vector<vertex> m_vertices;
		std::vector<face> m_faces;
		std::vector<uvset> m_uvsets;
		bool m_use32bitIndexes;
		MDagPath m_dagPath;
		BlendShape* m_pBlendShape;
		MBoundingBox m_boundingBox;
	};

}; // end of namespace

#endif