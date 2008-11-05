////////////////////////////////////////////////////////////////////////////////
// mesh.cpp
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

#include "mesh.h"
#include <maya/MFnMatrixData.h>

namespace OgreMayaExporter
{
	/***** Class Mesh *****/
	// constructor
	Mesh::Mesh(const MString& name)
	{
		m_name = name;
		m_numTriangles = 0;
		m_pSkeleton = NULL;
		m_sharedGeom.vertices.clear();
		m_sharedGeom.dagMap.clear();
		m_vertexClips.clear();
		m_BSClips.clear();
	}

	// destructor
	Mesh::~Mesh()
	{
		clear();
	}

	// clear data
	void Mesh::clear()
	{
		m_name = "";
		m_numTriangles = 0;
		for (int i=0; i<m_submeshes.size(); i++)
			delete m_submeshes[i];
		for (int i=0; i<m_sharedGeom.dagMap.size(); i++)
		{
			if (m_sharedGeom.dagMap[i].pBlendShape)
				delete m_sharedGeom.dagMap[i].pBlendShape;
		}
		m_sharedGeom.vertices.clear();
		m_sharedGeom.dagMap.clear();
		m_vertexClips.clear();
		m_BSClips.clear();
		m_uvsets.clear();
		m_submeshes.clear();
		if (m_pSkeleton)
			delete m_pSkeleton;
		m_pSkeleton = NULL;
		m_poseRemapping.clear();
	}

	// get pointer to linked skeleton
	Skeleton* Mesh::getSkeleton()
	{
		return m_pSkeleton;
	}

	/*******************************************************************************
	 *                    Load mesh data from a Maya node                          *
	 *******************************************************************************/
	MStatus Mesh::load(const MDagPath& meshDag,ParamList &params)
	{
		MStatus stat;
		// Check that given DagPath corresponds to a mesh node
		if (!meshDag.hasFn(MFn::kMesh))
			return MS::kFailure;

		// Set mesh name
		m_name = "mayaExport";

		// Initialise temporary variables
		newvertices.clear();
		newweights.clear();
		newjointIds.clear();
		newuvsets.clear();
		newpoints.clear();
		newnormals.clear();
		params.currentRootJoints.clear();
		opposite = false;
		shaders.clear();
		shaderPolygonMapping.clear();
		polygonSets.clear();
		pSkinCluster = NULL;
		pBlendShape = NULL;

		// Get mesh uvsets
		stat = getUVSets(meshDag);		
		if (stat != MS::kSuccess)
		{
			std::cout << "Error retrieving uvsets for current mesh\n";
			std::cout.flush();
		}
		// Get linked skin cluster
		stat = getSkinCluster(meshDag,params);
		if (stat != MS::kSuccess)
		{
			std::cout << "Error retrieving skin cluster linked to current mesh\n";
			std::cout.flush();
		}
		// Get linked blend shape deformer
		stat = getBlendShapeDeformer(meshDag,params);
		if (stat != MS::kSuccess)
		{
			std::cout << "Error retrieving blend shape deformer linked to current mesh\n";
			std::cout.flush();
		}
		// Set blend shape deformer envelope to 0, to get base mesh
		if (pBlendShape)
		{
			pBlendShape->setEnvelope(0.0f);
		}
		// Get connected shaders
		stat = getShaders(meshDag);
		if (stat != MS::kSuccess)
		{
			std::cout << "Error getting shaders connected to current mesh\n";
			std::cout.flush();
		}
		// Get vertex data
		stat = getVertices(meshDag,params);
		if (stat != MS::kSuccess)
		{
			std::cout << "Error retrieving vertex data for current mesh\n";
			std::cout.flush();
		}
		// Get vertex bone weights
		if (pSkinCluster)
		{
			getVertexBoneWeights(meshDag,params);
			if (stat != MS::kSuccess)
			{
				std::cout << "Error retrieving veretex bone assignements for current mesh\n";
				std::cout.flush();
			}
		}
		// Get faces data
		stat = getFaces(meshDag,params);
		if (stat != MS::kSuccess)
		{
			std::cout << "Error retrieving faces data for current mesh\n";
			std::cout.flush();
		}
		// Build shared geometry
		if (params.useSharedGeom)
		{
			stat = buildSharedGeometry(meshDag,params);
			if (stat != MS::kSuccess)
			{
				std::cout << "Error building shared geometry for current mesh\n";
				std::cout.flush();
			}
		}
		// Create submeshes (a different submesh for every different shader linked to the mesh)
		stat = createSubmeshes(meshDag,params);
		if (stat != MS::kSuccess)
		{
			std::cout << "Error creating submeshes for current mesh\n";
			std::cout.flush();
		}
		// Restore blendshape envelope
		if (pBlendShape)
		{
			pBlendShape->restoreEnvelope();
		}
		// Free up memory
		newvertices.clear();
		newweights.clear();
		newjointIds.clear();
		newpoints.clear();
		newnormals.clear();
		newuvsets.clear();
		shaders.clear();
		shaderPolygonMapping.clear();
		polygonSets.clear();
		if (pSkinCluster)
			delete pSkinCluster;
		pBlendShape = NULL;

		return MS::kSuccess;
	}


	/*******************************************************************************
	 *                    Load mesh animations from Maya                           *
	 *******************************************************************************/
	// Load vertex animations
	MStatus Mesh::loadAnims(ParamList& params)
	{
		MStatus stat;
		// save current time for later restore
		MTime curTime = MAnimControl::currentTime();
		std::cout << "Loading vertex animations...\n";
		std::cout.flush();
		// clear animations data
		m_vertexClips.clear();
		// load the requested clips
		for (int i=0; i<params.vertClipList.size(); i++)
		{
			std::cout << "Loading clip " << params.vertClipList[i].name.asChar() << "\n";
			std::cout.flush();
			stat = loadClip(params.vertClipList[i].name,params.vertClipList[i].start,
				params.vertClipList[i].stop,params.vertClipList[i].rate,params);
			if (stat == MS::kSuccess)
			{
				std::cout << "Clip successfully loaded\n";
				std::cout.flush();
			}
			else
			{
				std::cout << "Failed loading clip\n";
				std::cout.flush();
			}
		}
		//restore current time
		MAnimControl::setCurrentTime(curTime);
		return MS::kSuccess;
	}



	// Load blend shape deformers
	MStatus Mesh::loadBlendShapes(ParamList &params)
	{
		MStatus stat;
		std::cout << "Loading blend shape poses...\n";
		std::cout.flush();
		// Disable constraints, IK, etc...
		MGlobal::executeCommand("doEnableNodeItems false all",true);
		// Set envelopes of all blend shape deformers to 0
		if (params.useSharedGeom)
		{
			for  (int i=0; i<m_sharedGeom.dagMap.size(); i++)
			{
				dagInfo di = m_sharedGeom.dagMap[i];
				if (di.pBlendShape)
					di.pBlendShape->setEnvelope(0);
			}
		}
		else
		{
			for (int i=0; i<m_submeshes.size(); i++)
			{
				Submesh* pSubmesh = m_submeshes[i];
				if (pSubmesh->m_pBlendShape)
					pSubmesh->m_pBlendShape->setEnvelope(0);
			}
		}
		// Get the blend shape poses
		if (params.useSharedGeom)
		{
			for  (int i=0; i<m_sharedGeom.dagMap.size(); i++)
			{
				dagInfo di = m_sharedGeom.dagMap[i];
				if (di.pBlendShape)
					di.pBlendShape->loadPosesShared(di.dagPath,params,m_sharedGeom.vertices,di.numVertices,di.offset);
			}
		}
		else
		{
			for (int i=0; i<m_submeshes.size(); i++)
			{
				Submesh* pSubmesh = m_submeshes[i];
				if (pSubmesh->m_pBlendShape)
					pSubmesh->m_pBlendShape->loadPosesSubmesh(pSubmesh->m_dagPath,params,pSubmesh->m_vertices,
						pSubmesh->m_indices,i+1);
			}
		}
		// Enable constraints, IK, etc...
		MGlobal::executeCommand("doEnableNodeItems true all",true);
		// Restore blend shape deformers original envelopes
		if (params.useSharedGeom)
		{
			for  (int i=0; i<m_sharedGeom.dagMap.size(); i++)
			{
				dagInfo di = m_sharedGeom.dagMap[i];
				if (di.pBlendShape)
					di.pBlendShape->restoreEnvelope();
			}
		}
		else
		{
			for (int i=0; i<m_submeshes.size(); i++)
			{
				Submesh* pSubmesh = m_submeshes[i];
				if (pSubmesh->m_pBlendShape)
					pSubmesh->m_pBlendShape->restoreEnvelope();
			}
		}
		// Read blend shape animations
		if (params.exportBSAnims)
		{
			stat = loadBlendShapeAnimations(params);
		}
		return MS::kSuccess;
	}

	// Load blend shape animations
	MStatus Mesh::loadBlendShapeAnimations(ParamList& params)
	{
		std::cout << "Loading blend shape animations...\n";
		std::cout.flush();
		// Read the list of blend shape clips to export
		for (int i=0; i<params.BSClipList.size(); i++)
		{
			int startPoseId = 0;
			clipInfo ci = params.BSClipList[i];
			// Create a new animation for every clip
			Animation a;
			a.m_name = ci.name;
			a.m_length = ci.stop - ci.start;
			a.m_tracks.clear();
			std::cout << "clip " << ci.name.asChar() << "\n";
			// Read animation tracks from the blend shape deformer
			if (params.useSharedGeom)
			{
				// Create a track for each blend shape
				std::vector<Track> tracks;
				for  (int j=0; j<m_sharedGeom.dagMap.size(); j++)
				{
					dagInfo di = m_sharedGeom.dagMap[j];
					if (di.pBlendShape)
					{
						Track t = di.pBlendShape->loadTrack(ci.start,ci.stop,ci.rate,params,0,startPoseId);
						tracks.push_back(t);
						startPoseId += di.pBlendShape->getPoseGroups().find(0)->second.poses.size();
					}
				}
				// Merge the tracks into a single track (shared geometry must have a single animation track)
				if (tracks.size() > 0)
				{
					Track newTrack;
					// Merge keyframes at the same time position from all tracks
					for (int j=0; j<tracks[0].m_vertexKeyframes.size(); j++)
					{
						// Create a new keyframe
						vertexKeyframe newKeyframe;
						newKeyframe.time = tracks[0].m_vertexKeyframes[j].time;
						// Get keyframe at current position from all tracks
						for (int k=0; k<tracks.size(); k++)
						{
							vertexKeyframe* pSrcKeyframe = &tracks[k].m_vertexKeyframes[j];
							// Add pose references from this keyframe to the new keyframe for the joined track
							for (int pri=0; pri<pSrcKeyframe->poserefs.size(); pri++)
							{
								// Create a new pose reference
								vertexPoseRef poseref;
								// Copy pose reference settings from source keyframe
								poseref.poseIndex = pSrcKeyframe->poserefs[pri].poseIndex;
								poseref.poseWeight = pSrcKeyframe->poserefs[pri].poseWeight;
								// Add the new pose reference to the new keyframe
								newKeyframe.poserefs.push_back(poseref);
							}
						}
						// Add the keyframe to the new joined track
						newTrack.m_vertexKeyframes.push_back(newKeyframe);
					}
					// Add the joined track to current animation clip
					a.addTrack(newTrack);
				}
			}
			else
			{
				// Create a track for each submesh
				std::vector<Track> tracks;
				for (int j=0; j<m_submeshes.size(); j++)
				{
					Submesh* pSubmesh = m_submeshes[j];
					if (pSubmesh->m_pBlendShape)
					{
						int startPoseId = 0;
						Track t = pSubmesh->m_pBlendShape->loadTrack(ci.start,ci.stop,ci.rate,params,j+1,startPoseId);
						a.addTrack(t);
					}
				}
			}
			if (a.m_tracks.size() > 0)
			{
				std::cout << "length: " << a.m_length << "\n";
				std::cout << "num keyframes: " << a.m_tracks[0].m_vertexKeyframes.size() << "\n";
				std::cout.flush();
				m_BSClips.push_back(a);
			}
			else
			{
				std::cout << "no tracks exported: skipped. \n";
				std::cout.flush();
			}
		}
		return MS::kSuccess;
	}


/******************** Methods to parse geometry data from Maya ************************/
	// Get uvsets info from the maya mesh
	MStatus Mesh::getUVSets(const MDagPath& meshDag)
	{
		MFnMesh mesh(meshDag);
		MStatus stat;
		// Get uv texture coordinate sets' names
		if (mesh.numUVSets() > 0)
		{
			stat = mesh.getUVSetNames(newuvsets);
			if (MS::kSuccess != stat)
			{
				std::cout << "Error retrieving UV sets names\n";
				std::cout.flush();
				return MS::kFailure;
			}
		}
		// Save uvsets info
		for (int i=m_uvsets.size(); i<newuvsets.length(); i++)
		{
			uvset uv;
			uv.size = 2;
			m_uvsets.push_back(uv);
		}
		return MS::kSuccess;
	}


	// Get skin cluster linked to the maya mesh
	MStatus Mesh::getSkinCluster(const MDagPath &meshDag, ParamList &params)
	{
		MStatus stat;
		MFnMesh mesh(meshDag);
		pSkinCluster = NULL;
		if (params.exportVBA || params.exportSkeleton)
		{
			// get connected skin clusters (if present)
			MItDependencyNodes kDepNodeIt( MFn::kSkinClusterFilter );            
			for( ;!kDepNodeIt.isDone() && !pSkinCluster; kDepNodeIt.next()) 
			{            
				MObject kObject = kDepNodeIt.item();
				pSkinCluster = new MFnSkinCluster(kObject);
				unsigned int uiNumGeometries = pSkinCluster->numOutputConnections();
				for(uint uiGeometry = 0; uiGeometry < uiNumGeometries; ++uiGeometry ) 
				{
					unsigned int uiIndex = pSkinCluster->indexForOutputConnection(uiGeometry);
					MObject kOutputObject = pSkinCluster->outputShapeAtIndex(uiIndex);
					if(kOutputObject == mesh.object()) 
					{
						std::cout << "Found skin cluster " << pSkinCluster->name().asChar() << " for mesh " 
							<< mesh.name().asChar() << "\n"; 
						std::cout.flush();
					}	
					else
					{
						delete pSkinCluster;
						pSkinCluster = NULL;
					}
				}
			}
			if (pSkinCluster)
			{
				// load the skeleton
				std::cout << "Loading skeleton data...\n";
				std::cout.flush();
				if (!m_pSkeleton)
					m_pSkeleton = new Skeleton();
				stat = m_pSkeleton->load(pSkinCluster,params);
				if (MS::kSuccess != stat)
				{
					std::cout << "Error loading skeleton data\n";
					std::cout.flush();
				}
				else
				{
					std::cout << "OK\n";
					std::cout.flush();
				}
			}
		}
		return MS::kSuccess;
	}


	// Get blend shape deformer linked to the maya mesh
	MStatus Mesh::getBlendShapeDeformer(const MDagPath &meshDag, OgreMayaExporter::ParamList &params)
	{
		MStatus stat;
		MFnMesh mesh(meshDag);
		if (params.exportBlendShapes)
		{
			// get connected blend shape deformer (if present)
			MItDependencyNodes kDepNodeIt( MFn::kBlendShape );            
			for( ;!kDepNodeIt.isDone() && !pBlendShape; kDepNodeIt.next()) 
			{   
				MObject kObject = kDepNodeIt.item();
				MItDependencyGraph itGraph(kObject,MFn::kMesh,MItDependencyGraph::kDownstream,MItDependencyGraph::kDepthFirst);
				for (;!itGraph.isDone() && !pBlendShape; itGraph.next())
				{
					MFnMesh connectedMesh(itGraph.thisNode());
					if (connectedMesh.fullPathName() == mesh.fullPathName())
					{
						pBlendShape = new BlendShape();
						pBlendShape->load(kObject);
						std::cout << "Found blend shape deformer " << pBlendShape->getName().asChar() << " for mesh " 
							<< mesh.name().asChar() << "\n"; 
					}	
				}
			}
		}
		return MS::kSuccess;
	}
	// Get connected shaders
	MStatus Mesh::getShaders(const MDagPath& meshDag)
	{
		MStatus stat;
		MFnMesh mesh(meshDag);
		stat = mesh.getConnectedShaders(0,shaders,shaderPolygonMapping);
		std::cout.flush();
		if (MS::kSuccess != stat)
		{
			std::cout << "Error getting connected shaders\n";
			std::cout.flush();
			return MS::kFailure;
		}
		std::cout << "Found " << shaders.length() << " connected shaders\n";
		std::cout.flush();
		if (shaders.length() <= 0)
		{
			std::cout << "No connected shaders, skipping mesh\n";
			std::cout.flush();
			return MS::kFailure;
		}
		// create a series of arrays of faces for each different submesh
		polygonSets.clear();
		polygonSets.resize(shaders.length());
		return MS::kSuccess;
	}


	// Get vertex data
	MStatus Mesh::getVertices(const MDagPath &meshDag, OgreMayaExporter::ParamList &params)
	{
		MFnMesh mesh(meshDag);
		// prepare vertex table
		newvertices.resize(mesh.numVertices());
		newweights.resize(mesh.numVertices());
		newjointIds.resize(mesh.numVertices());
		for (int i=0; i<newvertices.size(); i++)
		{
			newvertices[i].pointIdx = -1;
			newvertices[i].normalIdx = -1;
			newvertices[i].next = -2;
		}
		//get vertex positions from mesh
		if (params.exportWorldCoords || (pSkinCluster && params.exportSkeleton))
			mesh.getPoints(newpoints,MSpace::kWorld);
		else
			mesh.getPoints(newpoints,MSpace::kTransform);
		//get list of normals from mesh data
		if (params.exportWorldCoords)
			mesh.getNormals(newnormals,MSpace::kWorld);
		else
			mesh.getNormals(newnormals,MSpace::kTransform);
		//check the "opposite" attribute to see if we have to flip normals
		mesh.findPlug("opposite",true).getValue(opposite);
		return MS::kSuccess;
	}


	// Get vertex bone assignements
	MStatus Mesh::getVertexBoneWeights(const MDagPath& meshDag, OgreMayaExporter::ParamList &params)
	{
		unsigned int numWeights;
		MStatus stat;
		std::cout << "Get vbas\n";
		std::cout.flush();
		MItGeometry iterGeom(meshDag);
		for (int i=0; !iterGeom.isDone(); iterGeom.next(), i++)
		{
			MObject component = iterGeom.component();
			MFloatArray vertexWeights;
			stat=pSkinCluster->getWeights(meshDag,component,vertexWeights,numWeights);
			// save the normalized weights
			newweights[i]=vertexWeights;
			if (MS::kSuccess != stat)
			{
				std::cout << "Error retrieving vertex weights\n";
				std::cout.flush();
			}
			// get ids for the joints
			if (m_pSkeleton)
			{
				MDagPathArray influenceObjs;
				pSkinCluster->influenceObjects(influenceObjs,&stat);
				if (MS::kSuccess != stat)
				{
					std::cout << "Error retrieving influence objects for given skin cluster\n";
					std::cout.flush();
				}
				newjointIds[i].setLength(newweights[i].length());
				for (int j=0; j<influenceObjs.length(); j++)
				{
					bool foundJoint = false;
					MString partialPathName = influenceObjs[j].partialPathName(); 
					for (int k=0; k<m_pSkeleton->getJoints().size() && !foundJoint; k++)
					{
						if (partialPathName == m_pSkeleton->getJoints()[k].name)
						{
							foundJoint=true;
							newjointIds[i][j] = m_pSkeleton->getJoints()[k].id;
						}
					}
				}
			}
		}
		return MS::kSuccess;
	}


	// Get faces data
	MStatus Mesh::getFaces(const MDagPath &meshDag, ParamList &params)
	{
		MStatus stat;
		MFnMesh mesh(meshDag);
		// create an iterator to go through mesh polygons
		if (mesh.numPolygons() > 0)
		{
			std::cout << "Iterate over mesh polygons\n";
			std::cout.flush();
			MItMeshPolygon faceIter(mesh.object(),&stat);
			if (MS::kSuccess != stat)
			{
				std::cout << "Error accessing mesh polygons\n";
				std::cout.flush();
				return MS::kFailure;
			}
			std::cout << "num polygons = " << mesh.numPolygons() << "\n";
			std::cout.flush();
			// iterate over mesh polygons
			for (; !faceIter.isDone(); faceIter.next())
			{
				int numTris=0;
				bool different;
				int vtxIdx, nrmIdx;
				faceIter.numTriangles(numTris);
				// for every triangle composing current polygon extract triangle info
				for (int iTris=0; iTris<numTris; iTris++)
				{
					MPointArray triPoints;
					MIntArray tempTriVertexIdx,triVertexIdx;
					int idx;
					// create a new face to store triangle info
					face newFace;
					// extract triangle vertex indices
					faceIter.getTriangle(iTris,triPoints,tempTriVertexIdx);
					// convert indices to face-relative indices
					MIntArray polyIndices;
					faceIter.getVertices(polyIndices);
					for (uint iObj=0; iObj < tempTriVertexIdx.length(); ++iObj)
					{
						// iPoly is face-relative vertex index
						for (uint iPoly=0; iPoly < polyIndices.length(); ++iPoly)
						{
							if (tempTriVertexIdx[iObj] == polyIndices[iPoly]) 
							{
								triVertexIdx.append(iPoly);
								break;
							}
						}
					}
					// iterate over triangle's vertices
					for (int i=0; i<3; i++)
					{
						different = true;
						vtxIdx = faceIter.vertexIndex(triVertexIdx[i],&stat);
						if (stat != MS::kSuccess)
						{
							std::cout << "Could not access vertex position\n";
							std::cout.flush();
						}
						nrmIdx = faceIter.normalIndex(triVertexIdx[i],&stat);
						if (stat != MS::kSuccess)
						{
							std::cout << "Could not access vertex normal\n";
							std::cout.flush();
						}

						// get vertex color
						MColor color;
						if (faceIter.hasColor(triVertexIdx[i]))
						{
							stat = faceIter.getColor(color,triVertexIdx[i]);
							if (MS::kSuccess != stat)
							{
								color = MColor(1,1,1,1);
							}
							if (color.r > 1)
								color.r = 1;
							else if (color.r < PRECISION)
								color.r = 0;
							if (color.g > 1)
								color.g = 1;
							else if (color.g < PRECISION)
								color.g = 0;
							if (color.b > 1)
								color.b = 1;
							else if (color.b < PRECISION)
								color.b = 0;
							if (color.a > 1)
								color.a = 1;
							else if (color.a < PRECISION)
								color.a = 0;
						}
						else
						{
							color = MColor(1,1,1,1);
						}
						if (newvertices[vtxIdx].next == -2)	// first time we encounter a vertex in this position
						{
							// save vertex position
							newpoints[vtxIdx].cartesianize();
							newvertices[vtxIdx].pointIdx = vtxIdx;
							// save vertex normal
							newvertices[vtxIdx].normalIdx = nrmIdx;
							// save vertex colour
							newvertices[vtxIdx].r = color.r;
							newvertices[vtxIdx].g = color.g;
							newvertices[vtxIdx].b = color.b;
							newvertices[vtxIdx].a = color.a;
							// save vertex texture coordinates
							newvertices[vtxIdx].u.resize(newuvsets.length());
							newvertices[vtxIdx].v.resize(newuvsets.length());
							// save vbas
							newvertices[vtxIdx].vba.resize(newweights[vtxIdx].length());
							for (int j=0; j<newweights[vtxIdx].length(); j++)
							{
								newvertices[vtxIdx].vba[j] = (newweights[vtxIdx])[j];
							}
							// save joint ids
							newvertices[vtxIdx].jointIds.resize(newjointIds[vtxIdx].length());
							for (int j=0; j<newjointIds[vtxIdx].length(); j++)
							{
								newvertices[vtxIdx].jointIds[j] = (newjointIds[vtxIdx])[j];
							}
							// save uv sets data
							for (int j=0; j<newuvsets.length(); j++)
							{
								float2 uv;
								stat = faceIter.getUV(triVertexIdx[i],uv,&newuvsets[j]);
								if (MS::kSuccess != stat)
								{
									uv[0] = 0;
									uv[1] = 0;
								}
								newvertices[vtxIdx].u[j] = uv[0];
								newvertices[vtxIdx].v[j] = (-1)*(uv[1]-1);
							}
							// save vertex index in face info
							newFace.v[i] = m_sharedGeom.vertices.size() + vtxIdx;
							// update value of index to next vertex info (-1 means nothing next)
							newvertices[vtxIdx].next = -1;
						}
						else	// already found at least 1 vertex in this position
						{
							// check if a vertex with same attributes has been saved already
							for (int k=vtxIdx; k!=-1 && different; k=newvertices[k].next)
							{
								different = false;

								if (params.exportVertNorm)
								{
									MFloatVector n1 = newnormals[newvertices[k].normalIdx];
									MFloatVector n2 = newnormals[nrmIdx];
									if (n1.x!=n2.x || n1.y!=n2.y || n1.z!=n2.z)
									{
										different = true;
									}
								}

								if ((params.exportVertCol) &&
									(newvertices[k].r!=color.r || newvertices[k].g!=color.g || newvertices[k].b!= color.b || newvertices[k].a!=color.a))
								{
									different = true;
								}

								if (params.exportTexCoord)
								{
									for (int j=0; j<newuvsets.length(); j++)
									{
										float2 uv;
										stat = faceIter.getUV(triVertexIdx[i],uv,&newuvsets[j]);
										if (MS::kSuccess != stat)
										{
											uv[0] = 0;
											uv[1] = 0;
										}
										uv[1] = (-1)*(uv[1]-1);
										if (newvertices[k].u[j]!=uv[0] || newvertices[k].v[j]!=uv[1])
										{
											different = true;
										}
									}
								}

								idx = k;
							}
							// if no identical vertex has been saved, then save the vertex info
							if (different)
							{
								vertexInfo vtx;
								// save vertex position
								vtx.pointIdx = vtxIdx;
								// save vertex normal
								vtx.normalIdx = nrmIdx;
								// save vertex colour
								vtx.r = color.r;
								vtx.g = color.g;
								vtx.b = color.b;
								vtx.a = color.a;
								// save vertex vba
								vtx.vba.resize(newweights[vtxIdx].length());
								for (int j=0; j<newweights[vtxIdx].length(); j++)
								{
									vtx.vba[j] = (newweights[vtxIdx])[j];
								}
								// save joint ids
								vtx.jointIds.resize(newjointIds[vtxIdx].length());
								for (int j=0; j<newjointIds[vtxIdx].length(); j++)
								{
									vtx.jointIds[j] = (newjointIds[vtxIdx])[j];
								}
								// save vertex texture coordinates
								vtx.u.resize(newuvsets.length());
								vtx.v.resize(newuvsets.length());
								for (int j=0; j<newuvsets.length(); j++)
								{
									float2 uv;
									stat = faceIter.getUV(triVertexIdx[i],uv,&newuvsets[j]);
									if (MS::kSuccess != stat)
									{
										uv[0] = 0;
										uv[1] = 0;
									}
									if (fabs(uv[0]) < PRECISION)
										uv[0] = 0;
									if (fabs(uv[1]) < PRECISION)
										uv[1] = 0;
									vtx.u[j] = uv[0];
									vtx.v[j] = (-1)*(uv[1]-1);
								}
								vtx.next = -1;
								newvertices.push_back(vtx);
								// save vertex index in face info
								newFace.v[i] = m_sharedGeom.vertices.size() + newvertices.size()-1;
								newvertices[idx].next = newvertices.size()-1;
							}
							else
							{
								newFace.v[i] = m_sharedGeom.vertices.size() + idx;
							}
						}
					} // end iteration of triangle vertices
					// add face info to the array corresponding to the submesh it belongs
					// skip faces with no shaders assigned
					if (shaderPolygonMapping[faceIter.index()] >= 0)
						polygonSets[shaderPolygonMapping[faceIter.index()]].push_back(newFace);
				} // end iteration of triangles
			}
		}
		std::cout << "done reading mesh triangles\n";
		std::cout.flush();
		return MS::kSuccess;
	}


	// Build shared geometry
	MStatus Mesh::buildSharedGeometry(const MDagPath &meshDag,ParamList& params)
	{
		std::cout << "Create list of shared vertices\n";
		std::cout.flush();
		// save a new entry in the shared geometry map: we associate the index of the first 
		// vertex we're loading with the dag path from which it has been read
		dagInfo di;
		di.offset = m_sharedGeom.vertices.size();
		di.dagPath = meshDag;
		di.pBlendShape = pBlendShape;
		// load shared vertices
		for (int i=0; i<newvertices.size(); i++)
		{
			vertex v;
			vertexInfo vInfo = newvertices[i];
			// save vertex coordinates (rescale to desired length unit)
			MPoint point = newpoints[vInfo.pointIdx] * params.lum;
			if (fabs(point.x) < PRECISION)
				point.x = 0;
			if (fabs(point.y) < PRECISION)
				point.y = 0;
			if (fabs(point.z) < PRECISION)
				point.z = 0;
			v.x = point.x;
			v.y = point.y;
			v.z = point.z;
			// save vertex normal
			MFloatVector normal = newnormals[vInfo.normalIdx];
			if (fabs(normal.x) < PRECISION)
				normal.x = 0;
			if (fabs(normal.y) < PRECISION)
				normal.y = 0;
			if (fabs(normal.z) < PRECISION)
				normal.z = 0;
			if (opposite)
			{
				v.n.x = -normal.x;
				v.n.y = -normal.y;
				v.n.z = -normal.z;
			}
			else
			{
				v.n.x = normal.x;
				v.n.y = normal.y;
				v.n.z = normal.z;
			}
			v.n.normalize();
			// save vertex color
			v.r = vInfo.r;
			v.g = vInfo.g;
			v.b = vInfo.b;
			v.a = vInfo.a;
			// save vertex bone assignements
			for (int k=0; k<vInfo.vba.size(); k++)
			{
				vba newVba;
				newVba.jointIdx = vInfo.jointIds[k];
				newVba.weight = vInfo.vba[k];
				v.vbas.push_back(newVba);
			}
			// save texture coordinates
			for (int k=0; k<vInfo.u.size(); k++)
			{
				texcoord newTexCoords;
				newTexCoords.u = vInfo.u[k];
				newTexCoords.v = vInfo.v[k];
				newTexCoords.w = 0;
				v.texcoords.push_back(newTexCoords);
			}
			// save vertex index in maya mesh, to retrieve future positions of the same vertex
			v.index = vInfo.pointIdx;
			// add newly created vertex to vertices list
			m_sharedGeom.vertices.push_back(v);
		}
		// Make sure all vertices have the same number of texture coordinates
		for (int i=0; i<m_sharedGeom.vertices.size(); i++)
		{
			vertex* pV = &m_sharedGeom.vertices[i];
			for (int j=pV->texcoords.size(); j<m_uvsets.size(); j++)
			{
				texcoord newTexCoords;
				newTexCoords.u = 0;
				newTexCoords.v = 0;
				newTexCoords.w = 0;
				pV->texcoords.push_back(newTexCoords);
			}
		}
		// save number of vertices referring to this mesh dag in the dag path map
		di.numVertices = m_sharedGeom.vertices.size() - di.offset;
		m_sharedGeom.dagMap.push_back(di);
		std::cout << "done creating vertices list\n";
		std::cout.flush();
		return MS::kSuccess;
	}


	// Create submeshes
	MStatus Mesh::createSubmeshes(const MDagPath& meshDag,ParamList& params)
	{
		MStatus stat;
		MFnMesh mesh(meshDag);
		for (int i=0; i<shaders.length(); i++)
		{
			// check if the submesh has at least 1 triangle
			if (polygonSets[i].size() > 0)
			{
				//create a name for the submesh
				MString submesh_name = meshDag.partialPathName();
				MFnDependencyNode shader_node(shaders[i]);
				if (shaders.length()>1)
				{
					submesh_name += "_";
					submesh_name += shader_node.name();
				}
				//create new submesh
				Submesh* pSubmesh = new Submesh(submesh_name);
				//load linked shader
				stat = pSubmesh->loadMaterial(shaders[i],newuvsets,params);
				if (stat != MS::kSuccess)
				{
					MFnDependencyNode shadingGroup(shaders[i]);
					std::cout << "Error loading material for submesh: " << submesh_name.asChar() << "\n";
					std::cout.flush();
					return MS::kFailure;
				}
				//load vertex and face data
				stat = pSubmesh->load(meshDag,polygonSets[i],newvertices,newpoints,newnormals,newuvsets,params,opposite);
				//if we're not using shared geometry, save a pointer to the blend shape deformer
				if (pBlendShape && !params.useSharedGeom)
					pSubmesh->m_pBlendShape = pBlendShape;
				//add submesh to current mesh
				m_submeshes.push_back(pSubmesh);
				//update number of triangles composing the mesh
				m_numTriangles += pSubmesh->numTriangles();
			}
		}
		return MS::kSuccess;
	}


/******************** Methods to read vertex animations from Maya ************************/
	//load a vertex animation clip
	MStatus Mesh::loadClip(MString& clipName,float start,float stop,float rate,ParamList& params)
	{
		MStatus stat;
		MString msg;
		std::vector<float> times;
		// calculate times from clip sample rate
		times.clear();
		for (float t=start; t<stop; t+=rate)
			times.push_back(t);
		times.push_back(stop);
		// get animation length
		float length=0;
		if (times.size() >= 0)
			length = times[times.size()-1] - times[0];
		if (length < 0)
		{
			std::cout << "invalid time range for the clip, we skip it\n";
			std::cout.flush();
			return MS::kFailure;
		}
		// create a new animation
		Animation a;
		a.m_name = clipName;
		a.m_length = length;
		a.m_tracks.clear();
		// if we're using shared geometry, create a single animation track for the whole mesh
		if (params.useSharedGeom)
		{
			// load the animation track
			stat = loadMeshTrack(a,times,params);
			if (stat != MS::kSuccess)
			{
				std::cout << "Error loading mesh vertex animation\n";
				std::cout.flush();
			}
		}
		// else creae a different animation track for each submesh
		else
		{
			// load all tracks (one for each submesh)
			stat = loadSubmeshTracks(a,times,params);
			if (stat != MS::kSuccess)
			{
				std::cout << "Error loading submeshes vertex animation\n";
				std::cout.flush();
				return MS::kFailure;
			}
		}
		// add newly created animation to animations list
		m_vertexClips.push_back(a);
		// display info
		std::cout << "length: " << a.m_length << "\n";
		std::cout << "num keyframes: " << a.m_tracks[0].m_vertexKeyframes.size() << "\n";
		std::cout.flush();
		// clip successfully loaded
		return MS::kSuccess;
	}


	//load an animation track for the whole mesh (using shared geometry)
	MStatus Mesh::loadMeshTrack(Animation& a,std::vector<float> &times, OgreMayaExporter::ParamList &params)
	{
		MStatus stat;
		// create a new track
		Track t;
		t.m_type = TT_MORPH;
		t.m_target = T_MESH;
		t.m_vertexKeyframes.clear();
		// get keyframes at given times
		for (int i=0; i<times.size(); i++)
		{
			//set time to wanted sample time
			MAnimControl::setCurrentTime(MTime(times[i],MTime::kSeconds));
			//load a keyframe for the mesh at current time
			stat = loadKeyframe(t,times[i]-times[0],params);
			if (stat != MS::kSuccess)
			{
				std::cout << "Error reading animation keyframe at time: " << times[i] << "\n";
				std::cout.flush();
			}
		}
		// add track to given animation
		a.addTrack(t);
		// track sucessfully loaded
		return MS::kSuccess;
	}


	//load all submesh animation tracks (one for each submesh)
	MStatus Mesh::loadSubmeshTracks(Animation& a,std::vector<float> &times, OgreMayaExporter::ParamList &params)
	{
		MStatus stat;
		// create a new track for each submesh
		std::vector<Track> tracks;
		for (int i=0; i<m_submeshes.size(); i++)
		{
			Track t;
			t.m_type = TT_MORPH;
			t.m_target = T_SUBMESH;
			t.m_index = i;
			t.m_vertexKeyframes.clear();
			tracks.push_back(t);
		}
		// get keyframes at given times
		for (int i=0; i<times.size(); i++)
		{
			//set time to wanted sample time
			MAnimControl::setCurrentTime(MTime(times[i],MTime::kSeconds));
			//load a keyframe for each submesh at current time
			for (int j=0; j<m_submeshes.size(); j++)
			{
				stat = m_submeshes[j]->loadKeyframe(tracks[j],times[i]-times[0],params);
				if (stat != MS::kSuccess)
				{
					std::cout << "Error reading animation keyframe at time: " << times[i] << " for submesh: " << j << "\n";
					std::cout.flush();
				}
			}
		}
		// add tracks to given animation
		for (int i=0; i< tracks.size(); i++)
			a.addTrack(tracks[i]);
		// track sucessfully loaded
		return MS::kSuccess;
		return MS::kSuccess;
	}


	// Load a keyframe for the whole mesh
	MStatus Mesh::loadKeyframe(Track& t,float time,ParamList& params)
	{
		// create a new keyframe
		vertexKeyframe k;
		// set keyframe time
		k.time = time;
		for (int i=0; i<m_sharedGeom.dagMap.size(); i++)
		{
			// get the mesh Fn
			dagInfo di = m_sharedGeom.dagMap[i];
			MFnMesh mesh(di.dagPath);
			// get vertex positions
			MFloatPointArray points;
			if (params.exportWorldCoords)
				mesh.getPoints(points,MSpace::kWorld);
			else
				mesh.getPoints(points,MSpace::kObject);
			// calculate vertex offsets
			for (int j=0; j<di.numVertices; j++)
			{
				vertexPosition pos;
				vertex v = m_sharedGeom.vertices[di.offset+j];
				pos.x = points[v.index].x * params.lum;
				pos.y = points[v.index].y * params.lum;
				pos.z = points[v.index].z * params.lum;
				if (fabs(pos.x) < PRECISION)
					pos.x = 0;
				if (fabs(pos.y) < PRECISION)
					pos.y = 0;
				if (fabs(pos.z) < PRECISION)
					pos.z = 0;
				k.positions.push_back(pos);
			}
		}
		// add keyframe to given track
		t.addVertexKeyframe(k);
		// keyframe successfully loaded
		return MS::kSuccess;
	}

/*********************************** Export mesh data **************************************/
	// Write to a OGRE binary mesh
	MStatus Mesh::writeOgreBinary(ParamList &params)
	{
		MStatus stat;
		// If no mesh have been exported, skip mesh creation
		if (m_submeshes.size() <= 0)
		{
			std::cout << "Warning: No meshes selected for export\n";
			std::cout.flush();
			return MS::kFailure;
		}
		// Construct mesh
		Ogre::MeshPtr pMesh = Ogre::MeshManager::getSingleton().createManual(m_name.asChar(), 
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		// Write shared geometry data
		if (params.useSharedGeom)
		{
			createOgreSharedGeometry(pMesh,params);
		}
		// Write submeshes data
		for (int i=0; i<m_submeshes.size(); i++)
		{
			m_submeshes[i]->createOgreSubmesh(pMesh,params);
		}
		// Set skeleton link (if present)
		if (m_pSkeleton && params.exportSkeleton)
		{
			int ri = params.skeletonFilename.rindex('\\');
			int end = params.skeletonFilename.length() - 1;
			MString filename = params.skeletonFilename.substring(ri+1,end);
			pMesh->setSkeletonName(filename.asChar());
		}
		// Write poses
		if (params.exportBlendShapes)
		{
			createOgrePoses(pMesh,params);
		}
		// Write vertex animations
		if (params.exportVertAnims)
		{
			createOgreVertexAnimations(pMesh,params);
		}
		// Write pose animations
		if (params.exportBSAnims)
		{
			createOgrePoseAnimations(pMesh,params);
		}
		// Create a bounding box for the mesh
		Ogre::AxisAlignedBox bbox = pMesh->getBounds();
		for(int i=0; i<m_submeshes.size(); i++)
		{
			MPoint min1 = m_submeshes[i]->m_boundingBox.min();
			MPoint max1 = m_submeshes[i]->m_boundingBox.max();
			Ogre::Vector3 min2(min1.x,min1.y,min1.z);
			Ogre::Vector3 max2(max1.x,max1.y,max1.z);
			Ogre::AxisAlignedBox newbbox;
			newbbox.setExtents(min2,max2);
			bbox.merge(newbbox);
		}
		// Define mesh bounds
		pMesh->_setBounds(bbox,false);
		// Build edges list
		if (params.buildEdges)
		{
			pMesh->buildEdgeList();
		}
		// Build tangents
		if (params.buildTangents)
		{
			Ogre::VertexElementSemantic targetSemantic = params.tangentSemantic == TS_TANGENT ? 
				Ogre::VES_TANGENT : Ogre::VES_TEXTURE_COORDINATES;
			bool canBuild = true;
			unsigned short srcTex, destTex;
			try {
				canBuild = !pMesh->suggestTangentVectorBuildParams(targetSemantic, srcTex, destTex);
			} catch(Ogre::Exception e) {
				canBuild = false;
			}
			if (canBuild)
				pMesh->buildTangentVectors(targetSemantic, srcTex, destTex, 
					params.tangentsSplitMirrored, params.tangentsSplitRotated, params.tangentsUseParity);
		}
		// Export the binary mesh
		Ogre::MeshSerializer serializer;
		serializer.exportMesh(pMesh.getPointer(),params.meshFilename.asChar());
		pMesh.setNull();
		return MS::kSuccess;
	}

	// Create shared geometry data for an Ogre mesh
	MStatus Mesh::createOgreSharedGeometry(Ogre::MeshPtr pMesh,ParamList& params)
	{
		MStatus stat;
		pMesh->sharedVertexData = new Ogre::VertexData();
		pMesh->sharedVertexData->vertexCount = m_sharedGeom.vertices.size();
		// Define a new vertex declaration
		Ogre::VertexDeclaration* pDecl = new Ogre::VertexDeclaration();
		pMesh->sharedVertexData->vertexDeclaration = pDecl;
		unsigned buf = 0;
		size_t offset = 0;
		// Add vertex position
		pDecl->addElement(buf,offset,Ogre::VET_FLOAT3,Ogre::VES_POSITION);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
		// Add vertex normal
		if (params.exportVertNorm)
		{
			pDecl->addElement(buf, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
			offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
		}
		// Add vertex colour
		if (params.exportVertCol)
		{
			pDecl->addElement(buf, offset, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
            offset += Ogre::VertexElement::getTypeSize(Ogre::VET_COLOUR);
		}
		// Add texture coordinates
		for (int i=0; i<m_sharedGeom.vertices[0].texcoords.size(); i++)
		{
			Ogre::VertexElementType uvType = Ogre::VertexElement::multiplyTypeCount(Ogre::VET_FLOAT1, 2);
			pDecl->addElement(buf, offset, uvType, Ogre::VES_TEXTURE_COORDINATES, i);
			offset += Ogre::VertexElement::getTypeSize(uvType);
		}
		// Get optimal vertex declaration
		Ogre::VertexDeclaration* pOptimalDecl = pDecl->getAutoOrganisedDeclaration(params.exportVBA,params.exportBlendShapes || params.exportVertAnims);
		// Create the vertex buffer using the newly created vertex declaration
		stat = createOgreVertexBuffer(pMesh,pDecl,m_sharedGeom.vertices);
		// Write vertex bone assignements list
		if (params.exportVBA)
		{
			// Create a new vertex bone assignements list
			Ogre::Mesh::VertexBoneAssignmentList vbas;
			// Scan list of shared geometry vertices
			for (int i=0; i<m_sharedGeom.vertices.size(); i++)
			{
				vertex v = m_sharedGeom.vertices[i];
				// Add all bone assignements for every vertex to the bone assignements list
				for (int j=0; j<v.vbas.size(); j++)
				{
					Ogre::VertexBoneAssignment vba;
					vba.vertexIndex = i;
					vba.boneIndex = v.vbas[j].jointIdx;
					vba.weight = v.vbas[j].weight;
					if (vba.weight > 0.0f)
						vbas.insert(Ogre::Mesh::VertexBoneAssignmentList::value_type(i, vba));
				}
			}
			// Rationalise the bone assignements list
			pMesh->_rationaliseBoneAssignments(pMesh->sharedVertexData->vertexCount,vbas);
			// Add bone assignements to the mesh
			for (Ogre::Mesh::VertexBoneAssignmentList::iterator bi = vbas.begin(); bi != vbas.end(); bi++)
			{
				pMesh->addBoneAssignment(bi->second);
			}
			pMesh->_compileBoneAssignments();
			pMesh->_updateCompiledBoneAssignments();
		}
		// Reorganize vertex buffers
		pMesh->sharedVertexData->reorganiseBuffers(pOptimalDecl);
		
		return MS::kSuccess;
	}

	// Create an Ogre compatible vertex buffer
	MStatus Mesh::createOgreVertexBuffer(Ogre::MeshPtr pMesh,Ogre::VertexDeclaration* pDecl,const std::vector<vertex>& vertices)
	{
		Ogre::HardwareVertexBufferSharedPtr vbuf = 
			Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(pDecl->getVertexSize(0),
			pMesh->sharedVertexData->vertexCount, 
			Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		pMesh->sharedVertexData->vertexBufferBinding->setBinding(0, vbuf);
		size_t vertexSize = pDecl->getVertexSize(0);
		char* pBase = static_cast<char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
		Ogre::VertexDeclaration::VertexElementList elems = pDecl->findElementsBySource(0);
		Ogre::VertexDeclaration::VertexElementList::iterator ei, eiend;
		eiend = elems.end();
		float* pFloat;
		Ogre::RGBA* pRGBA;
		// Fill the vertex buffer with shared geometry data
		Ogre::ColourValue col;
		float ucoord, vcoord;
		for (long vi=0; vi<vertices.size(); vi++)
		{
			int iTexCoord = 0;
			vertex v = vertices[vi];
			for (ei = elems.begin(); ei != eiend; ++ei)
			{
				Ogre::VertexElement& elem = *ei;
				switch(elem.getSemantic())
				{
				case Ogre::VES_POSITION:
					elem.baseVertexPointerToElement(pBase, &pFloat);
					*pFloat++ = v.x;
					*pFloat++ = v.y;
					*pFloat++ = v.z;
					break;
				case Ogre::VES_NORMAL:
					elem.baseVertexPointerToElement(pBase, &pFloat);
					*pFloat++ = v.n.x;
					*pFloat++ = v.n.y;
					*pFloat++ = v.n.z;
					break;
				case Ogre::VES_DIFFUSE:
					{
						elem.baseVertexPointerToElement(pBase, &pRGBA);
						Ogre::ColourValue col(v.r, v.g, v.b, v.a);
						*pRGBA = Ogre::VertexElement::convertColourValue(col, 
							Ogre::VertexElement::getBestColourVertexElementType());
					}
					break;
				case Ogre::VES_TEXTURE_COORDINATES:
					elem.baseVertexPointerToElement(pBase, &pFloat);
					ucoord = v.texcoords[iTexCoord].u;
					vcoord = v.texcoords[iTexCoord].v;
					*pFloat++ = ucoord;
					*pFloat++ = vcoord;
					iTexCoord++;
					break;
				}
			}
			pBase += vertexSize;
		}
		vbuf->unlock();
		return MS::kSuccess;
	}
	// Create mesh poses for an Ogre mesh
	MStatus Mesh::createOgrePoses(Ogre::MeshPtr pMesh,ParamList& params)
	{
		int poseCounter = 0;
		if (params.useSharedGeom)
		{
			// Create an entry in the submesh pose remapping table for the shared geometry
			submeshPoseRemapping new_sbr;
			m_poseRemapping.insert(std::pair<int,submeshPoseRemapping>(0,new_sbr));
			submeshPoseRemapping& sbr = m_poseRemapping.find(0)->second;
			// Read poses associated from all blendshapes associated to the shared geometry
			for (int i=0; i<m_sharedGeom.dagMap.size(); i++)
			{
				BlendShape* pBS = m_sharedGeom.dagMap[i].pBlendShape;
				// Check if we have a blend shape associated to this subset of the shared geometry
				if (pBS)
				{
					// Get all poses from current blend shape deformer
					poseGroup& pg = pBS->getPoseGroups().find(0)->second;
					for (int j=0; j<pg.poses.size(); j++)
					{
						// Get the pose
						pose* p = &(pg.poses[j]);
						if (p->name == "")
						{
							p->name = "pose";
							p->name += poseCounter;
						}
						// Create a new pose for the ogre mesh
						Ogre::Pose* pPose = pMesh->createPose(0,p->name.asChar());
						// Set the pose attributes
						for (int k=0; k<p->offsets.size(); k++)
						{
							Ogre::Vector3 offset(p->offsets[k].x,p->offsets[k].y,p->offsets[k].z);
							pPose->addVertex(p->offsets[k].index,offset);
						}
						// Add a pose remapping for current pose
						sbr.insert(std::pair<int,int>(poseCounter,poseCounter));
						poseCounter++;
					}
				}
			}
		}
		else
		{
			// Get poses associated to the submeshes
			for (int i=0; i<m_submeshes.size(); i++)
			{
				BlendShape* pBS = m_submeshes[i]->m_pBlendShape;
				// Check if this submesh has a blend shape deformer associated
				if (pBS)
				{
					// Create an entry in the submesh pose remapping table for this submesh
					submeshPoseRemapping new_sbr;
					m_poseRemapping.insert(std::pair<int,submeshPoseRemapping>(i+1,new_sbr));
					submeshPoseRemapping& sbr = m_poseRemapping.find(i+1)->second;
					// Get the pose group corresponding to the current submesh
					poseGroup& pg = pBS->getPoseGroups().find(i+1)->second;
					// Get all poses from current blend shape deformer and current pose group
					for (int j=0; j<pg.poses.size(); j++)
					{
						// Get the pose
						pose* p = &(pg.poses[j]);
						if (p->name == "")
						{
							p->name = "pose";
							p->name += poseCounter;
						}
						// Create a new pose for the ogre mesh
						Ogre::Pose* pPose = pMesh->createPose(p->index,p->name.asChar());
						// Set the pose attributes
						for (int k=0; k<p->offsets.size(); k++)
						{
							Ogre::Vector3 offset(p->offsets[k].x,p->offsets[k].y,p->offsets[k].z);
							pPose->addVertex(p->offsets[k].index,offset);
						}
						// Add a pose remapping for current pose
						sbr.insert(std::pair<int,int>(j,poseCounter));
						
						poseCounter++;
					}
				}
			}
		}
		return MS::kSuccess;
	}
	// Create vertex animations for an Ogre mesh
	MStatus Mesh::createOgreVertexAnimations(Ogre::MeshPtr pMesh,ParamList& params)
	{
		// Read the list of vertex animation clips
		for (int i=0; i<m_vertexClips.size(); i++)
		{
			// Create a new animation
			Ogre::Animation* pAnimation = pMesh->createAnimation(m_vertexClips[i].m_name.asChar(),m_vertexClips[i].m_length);
			// Create all tracks for current animation
			for (int j=0; j<m_vertexClips[i].m_tracks.size(); j++)
			{
				Track* t = &(m_vertexClips[i].m_tracks[j]);
				// Create a new track
				Ogre::VertexAnimationTrack* pTrack;
				if (t->m_target == T_MESH)
					pTrack = pAnimation->createVertexTrack(0,pMesh->sharedVertexData,Ogre::VAT_MORPH);
				else
				{
					pTrack = pAnimation->createVertexTrack(t->m_index+1,pMesh->getSubMesh(t->m_index)->vertexData,
						Ogre::VAT_MORPH);
				}
				// Create keyframes for current track
				for (int k=0; k<t->m_vertexKeyframes.size(); k++)
				{
					// Create a new keyframe
					Ogre::VertexMorphKeyFrame* pKeyframe = pTrack->createVertexMorphKeyFrame(t->m_vertexKeyframes[k].time);
					// Create vertex buffer for current keyframe
					Ogre::HardwareVertexBufferSharedPtr pBuffer = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
						Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3),
						t->m_vertexKeyframes[k].positions.size(),
						Ogre::HardwareBuffer::HBU_STATIC, true);
					float* pFloat = static_cast<float*>(pBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));
					// Fill the vertex buffer with vertex positions
					std::vector<vertexPosition>& positions = t->m_vertexKeyframes[k].positions;
					for (int vi=0; vi<positions.size(); vi++)
					{
						*pFloat++ = static_cast<float>(positions[vi].x);
						*pFloat++ = static_cast<float>(positions[vi].y);
						*pFloat++ = static_cast<float>(positions[vi].z);
					}
					// Unlock vertex buffer
					pBuffer->unlock();
					// Set vertex buffer for current keyframe
					pKeyframe->setVertexBuffer(pBuffer);
				}
			}
		}
		return MS::kSuccess;
	}
	// Create pose animations for an Ogre mesh
	MStatus Mesh::createOgrePoseAnimations(Ogre::MeshPtr pMesh,ParamList& params)
	{
		// Get all loaded blend shape clips
		for (int i=0; i<m_BSClips.size(); i++)
		{
			// Create a new animation for each clip
			Ogre::Animation* pAnimation = pMesh->createAnimation(m_BSClips[i].m_name.asChar(),m_BSClips[i].m_length);
			// Create animation tracks for this animation
			for (int j=0; j<m_BSClips[i].m_tracks.size(); j++)
			{
				Track* t = &m_BSClips[i].m_tracks[j];
				// Create a new track
				Ogre::VertexAnimationTrack* pTrack;
				if (t->m_target == T_MESH)
					pTrack = pAnimation->createVertexTrack(0,pMesh->sharedVertexData,Ogre::VAT_POSE);
				else
				{
					pTrack = pAnimation->createVertexTrack(t->m_index,pMesh->getSubMesh(t->m_index-1)->vertexData,
						Ogre::VAT_POSE);
				}
				// Create keyframes for current track
				for (int k=0; k<t->m_vertexKeyframes.size(); k++)
				{
					Ogre::VertexPoseKeyFrame* pKeyframe = pTrack->createVertexPoseKeyFrame(t->m_vertexKeyframes[k].time);
					for (int pri=0; pri<t->m_vertexKeyframes[k].poserefs.size(); pri++)
					{
						vertexPoseRef* pr = &t->m_vertexKeyframes[k].poserefs[pri];
						// Get the correct absolute index of the pose from the remapping
						int poseIndex = m_poseRemapping.find(t->m_index)->second.find(pr->poseIndex)->second;
						pKeyframe->addPoseReference(poseIndex,pr->poseWeight);
					}
				}
			}
		}
		return MS::kSuccess;
	}

}; //end of namespace
