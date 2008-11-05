////////////////////////////////////////////////////////////////////////////////
// mesh.h
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

#ifndef _MESH_H
#define _MESH_H

#include "submesh.h"
#include "skeleton.h"
#include "mayaExportLayer.h"
#include "vertex.h"

namespace OgreMayaExporter
{
	/***** structures to store shared geometry *****/
	typedef struct dagInfotag
	{
		long offset;
		long numVertices;
		MDagPath dagPath;
		BlendShape* pBlendShape;
	} dagInfo;

	typedef struct sharedGeometrytag
	{
		std::vector<vertex> vertices;
		std::vector<dagInfo> dagMap;
	} sharedGeometry;

	typedef stdext::hash_map<int,int> submeshPoseRemapping;

	typedef stdext::hash_map<int,submeshPoseRemapping> poseRemapping; 


	/***** Class Mesh *****/
	class Mesh
	{
	public:
		//constructor
		Mesh(const MString& name = "");
		//destructor
		~Mesh();
		//clear data
		void clear();
		//get pointer to linked skeleton
		Skeleton* getSkeleton();
		//load mesh data from a maya Fn
		MStatus load(const MDagPath& meshDag,ParamList &params);
		//load vertex animations
		MStatus loadAnims(ParamList &params);
		//load blend shape deformers
		MStatus loadBlendShapes(ParamList &params);
		//load blend shape animations
		MStatus loadBlendShapeAnimations(ParamList& params);
		//write to a OGRE binary mesh
		MStatus writeOgreBinary(ParamList &params);

	protected:
		//get uvsets info from the maya mesh
		MStatus getUVSets(const MDagPath& meshDag);
		//get skin cluster linked to the maya mesh
		MStatus getSkinCluster(const MDagPath& meshDag,ParamList& params); 
		//get blend shape deformer linked to the maya mesh
		MStatus getBlendShapeDeformer(const MDagPath& meshDag,ParamList& params);
		//get connected shaders
		MStatus getShaders(const MDagPath& meshDag);
		//get vertex data
		MStatus getVertices(const MDagPath& meshDag,ParamList& params);
		//get vertex bone assignements
		MStatus getVertexBoneWeights(const MDagPath& meshDag,OgreMayaExporter::ParamList &params);
		//get faces data
		MStatus getFaces(const MDagPath& meshDag,ParamList& params);
		//build shared geometry
		MStatus buildSharedGeometry(const MDagPath& meshDag,ParamList& params);
		//create submeshes
		MStatus createSubmeshes(const MDagPath& meshDag,ParamList& params);
		//load a vertex animation clip
		MStatus loadClip(MString& clipName,float start,float stop,float rate,ParamList& params);
		//load a vertex animation track for the whole mesh
		MStatus loadMeshTrack(Animation& a,std::vector<float>& times,ParamList& params);
		//load all submesh animation tracks (one for each submesh)
		MStatus loadSubmeshTracks(Animation& a,std::vector<float>& times,ParamList& params);
		//load a keyframe for the whole mesh
		MStatus loadKeyframe(Track& t,float time,ParamList& params);
		//write shared geometry data to an Ogre compatible mesh
		MStatus createOgreSharedGeometry(Ogre::MeshPtr pMesh,ParamList& params);
		//create an Ogre compatible vertex buffer
		MStatus createOgreVertexBuffer(Ogre::MeshPtr pMesh,Ogre::VertexDeclaration* pDecl,const std::vector<vertex>& vertices);
		//create Ogre poses for pose animation
		MStatus createOgrePoses(Ogre::MeshPtr pMesh,ParamList& params);
		//create vertex animations for an Ogre mesh
		MStatus createOgreVertexAnimations(Ogre::MeshPtr pMesh,ParamList& params);
		//create pose animations for an Ogre mesh
		MStatus createOgrePoseAnimations(Ogre::MeshPtr pMesh,ParamList& params);

		//internal members
		MString m_name;
		long m_numTriangles;
		std::vector<uvset> m_uvsets;
		std::vector<Submesh*> m_submeshes;
		Skeleton* m_pSkeleton;
		sharedGeometry m_sharedGeom;
		std::vector<Animation> m_vertexClips;
		std::vector<Animation> m_BSClips;
		//temporary members (existing only during translation from maya mesh)
		std::vector<vertexInfo> newvertices;
		std::vector<MFloatArray> newweights;
		std::vector<MIntArray> newjointIds;
		MPointArray newpoints;
		MFloatVectorArray newnormals;
		MStringArray newuvsets;
		MFnSkinCluster* pSkinCluster;
		BlendShape* pBlendShape;
		MObjectArray shaders;
		MIntArray shaderPolygonMapping;
		std::vector<faceArray> polygonSets;
		bool opposite;
		poseRemapping m_poseRemapping;
	};

}; // end of namespace

#endif