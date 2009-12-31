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
#include "OgreXSIMeshExporter.h"
#include <xsi_model.h>
#include <xsi_primitive.h>
#include <xsi_polygonnode.h>
#include <xsi_material.h>
#include <xsi_vertex.h>
#include <xsi_trianglevertex.h>
#include <xsi_cluster.h>
#include <xsi_kinematics.h>
#include <xsi_kinematicstate.h>
#include <xsi_selection.h>
#include <xsi_envelope.h>
#include <xsi_time.h>
#include <xsi_source.h>
#include <xsi_edge.h>
#include <xsi_vector3.h>
#include <xsi_matrix4.h>
#include <xsi_mixer.h>
#include <xsi_clip.h>
#include <xsi_timecontrol.h>
#include <xsi_actionsource.h>
#include <xsi_fcurve.h>
#include <xsi_fcurvekey.h>

#include "OgreException.h"
#include "OgreXSIHelper.h"
#include "OgreLogManager.h"
#include "OgreMeshManager.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreMeshManager.h"
#include "OgreMeshSerializer.h"
#include "OgreHardwareBufferManager.h"
#include "OgreVertexBoneAssignment.h"
#include "OgrePose.h"
#include "OgreAnimation.h"
#include "OgreAnimationTrack.h"

using namespace XSI;


namespace Ogre {
    //-----------------------------------------------------------------------
    XsiMeshExporter::UniqueVertex::UniqueVertex()
        : position(Vector3::ZERO), normal(Vector3::ZERO), colour(0), nextIndex(0)
    {
        for (int i = 0; i < OGRE_MAX_TEXTURE_COORD_SETS; ++i)
            uv[i] = Vector3::ZERO;
    }
    //-----------------------------------------------------------------------
    bool XsiMeshExporter::UniqueVertex::operator==(const UniqueVertex& rhs) const
    {
        bool ret = position == rhs.position && 
            normal == rhs.normal && 
            colour == rhs.colour;
        if (!ret) return ret;

        for (int i = 0; i < OGRE_MAX_TEXTURE_COORD_SETS && ret; ++i)
        {
            ret = ret && (uv[i] == rhs.uv[i]);
        }

        return ret;
        

    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    XsiMeshExporter::XsiMeshExporter()
    {
    }
    //-----------------------------------------------------------------------
    XsiMeshExporter::~XsiMeshExporter()
    {
		/// Tidy up
		cleanupDeformerMap();
		cleanupMaterialMap();
    }
    //-----------------------------------------------------------------------
	DeformerMap& XsiMeshExporter::buildMeshForExport(
		bool mergeSubMeshes, bool exportChildren, 
		bool edgeLists, bool tangents, VertexElementSemantic tangentSemantic, 
		bool tangentsSplitMirrored, bool tangentsSplitRotated, bool tangentsUseParity,
		bool vertexAnimation, AnimationList& animList, Real fps, const String& materialPrefix, 
		LodData* lod, const String& skeletonName)
    {

		LogOgreAndXSI(L"** Begin OGRE Mesh Export **");
        // Derive the scene root
        X3DObject sceneRoot(mXsiApp.GetActiveSceneRoot());

        // Construct mesh
        mMesh = MeshManager::getSingleton().createManual("XSIExport", 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		mMaterialPrefix = materialPrefix;

		cleanupDeformerMap();
		cleanupMaterialMap();
		mShapeKeyMapping.clear();

		// Find all PolygonMesh objects
		buildPolygonMeshList(exportChildren);
		// progress report
		ProgressManager::getSingleton().progress();

		// notify of skeleton beforehand
		if (!skeletonName.empty())
		{
			mMesh->setSkeletonName(skeletonName);
		}

		// write the data into a mesh
		buildMesh(mMesh.getPointer(), mergeSubMeshes, !skeletonName.empty(), 
			vertexAnimation, animList, fps);

		// progress report
		ProgressManager::getSingleton().progress();

		if (lod)
		{
			mMesh->generateLodLevels(lod->distances, lod->quota, lod->reductionValue);
			// progress report
			ProgressManager::getSingleton().progress();
		}

        if(edgeLists)
        {
            LogOgreAndXSI(L"Calculating edge lists");
            mMesh->buildEdgeList();
			// progress report
			ProgressManager::getSingleton().progress();
        }

        if(tangents)
        {
            LogOgreAndXSI(L"Calculating tangents");
            unsigned short src, dest;
            if (!mMesh->suggestTangentVectorBuildParams(tangentSemantic, src, dest))
            {
                mMesh->buildTangentVectors(tangentSemantic, src, dest, tangentsSplitMirrored, 
					tangentsSplitRotated, tangentsUseParity);
            }
            else
            {
                LogOgreAndXSI(L"Could not derive tangents parameters");
            }
			// progress report
			ProgressManager::getSingleton().progress();

        }

		cleanupPolygonMeshList();

		LogOgreAndXSI(L"** OGRE Mesh Export Complete **");

		return mXsiDeformerMap;
    }
	//-----------------------------------------------------------------------
	void XsiMeshExporter::exportMesh(const String& fileName, const AxisAlignedBox& skelAABB)
	{

		// Pad bounds
		AxisAlignedBox currBounds = mMesh->getBounds();
		currBounds.merge(skelAABB);
		mMesh->_setBounds(currBounds, false);

		MeshSerializer serializer;
		serializer.exportMesh(mMesh.getPointer(), fileName);

		// progress report
		ProgressManager::getSingleton().progress();

		mMesh.setNull();


	}
	//-----------------------------------------------------------------------
	MaterialMap& XsiMeshExporter::getMaterials(void)
	{
		return mXsiMaterialMap;
	}
	//-----------------------------------------------------------------------
	TextureProjectionMap& XsiMeshExporter::getTextureProjectionMap(void)
	{
		return mTextureProjectionMap;
	}
	//-----------------------------------------------------------------------
	void XsiMeshExporter::buildMesh(Mesh* pMesh, bool mergeSubmeshes, 
		bool lookForBoneAssignments, bool vertexAnimation, AnimationList& animList, 
		Real fps)
	{
		/* Iterate over the list of polygon meshes that we've already located.
			For each one:
				If we're not merging submeshes, bake any protosubmeshes built
				  into the mesh and clear the protosubmesh list
			    Scan the clusters for 'poly' clusters (which can contain material
				  discrepancies). Any that use a different material should be
				  noted in the polycluster list
			    Build ProtoSubMeshes by iterating over the triangles in the mesh, 
				  building the list of UniqueVertices against the material as we 
				  go. We check each triangle to see if the polygon index is in 
				  the list of polyclusters & if so it goes into the other lists
		    
			Finally, we bake any remaining protosubmeshes into submeshes.
		*/
		// Calculate the number of progress updates each mesh must raise
		float progPerMesh = (float)OGRE_XSI_NUM_MESH_STEPS / (float)(mXsiPolygonMeshList.size());
		float currProg = 0.0f;
		for (PolygonMeshList::iterator pm = mXsiPolygonMeshList.begin();
			pm != mXsiPolygonMeshList.end(); ++pm)
		{
			currProg += progPerMesh;
			unsigned short progUpdates = (unsigned short)currProg;
			currProg -= progUpdates;
			// build contents of this polymesh into ProtoSubMesh(es)
			processPolygonMesh(pMesh, *pm, lookForBoneAssignments, progUpdates);

			if (!mergeSubmeshes)
			{
				// export out at the end of every PolygonMesh
				exportProtoSubMeshes(pMesh);
			}
		}
		if (mergeSubmeshes)
		{
			// export out the combined result
			exportProtoSubMeshes(pMesh);
		}

		if (vertexAnimation)
		{
			exportAnimations(pMesh, animList, fps);
		}
	}
	//-----------------------------------------------------------------------
	XsiMeshExporter::ProtoSubMesh* XsiMeshExporter::createOrRetrieveProtoSubMesh(
		const String& materialName, const String& name, 
		TextureCoordDimensionList& texCoordDims, bool hasVertexColours)
	{
		bool createNew = true;
		ProtoSubMesh* ret = 0;
		ProtoSubMeshList* protoList = 0;
		
		MaterialProtoSubMeshMap::iterator pi = mMaterialProtoSubmeshMap.find(materialName);
		if (pi == mMaterialProtoSubmeshMap.end())
		{
			protoList = new ProtoSubMeshList();
			mMaterialProtoSubmeshMap[materialName] = protoList;
		}
		else
		{
			// Iterate over the protos with the same material
			protoList = pi->second;

			for (ProtoSubMeshList::iterator psi = protoList->begin(); psi != protoList->end(); ++psi)
			{
				ProtoSubMesh* candidate = *psi;
				// Check format is compatible
				if (candidate->textureCoordDimensions.size() != texCoordDims.size())
				{
					continue;
				}
				if (candidate->hasVertexColours != hasVertexColours)
				{
					continue;
				}

				bool compat = true;
				std::vector<ushort>::iterator t = texCoordDims.begin();
				std::vector<ushort>::iterator u = candidate->textureCoordDimensions.begin(); 
				for (;t != texCoordDims.end(); ++t,++u)
				{
					if (*t != *u)
					{
						compat = false;
						break;
					}
				}

				if (compat)
				{
					createNew = false;
					ret = candidate;
					break;
				}
			}
		}

		if (createNew)
		{
			ret = new ProtoSubMesh();
			protoList->push_back(ret);
			ret->materialName = materialName;
			ret->name = name;
			ret->textureCoordDimensions = texCoordDims;
			ret->hasVertexColours = hasVertexColours;
		}

		return ret;
		
	}
	//-----------------------------------------------------------------------
	void XsiMeshExporter::processPolygonMesh(Mesh* pMesh, PolygonMeshEntry* xsiMesh, 
		bool lookForBoneAssignments, unsigned short progressUpdates)
	{
		// Pre-process the mesh
		if (!preprocessPolygonMesh(xsiMesh))
		{
			while(progressUpdates--)
				ProgressManager::getSingleton().progress();

			return;
		}
		
        // Retrieve all the XSI relevant summary info
        CPointRefArray pointArray(xsiMesh->mesh.GetPoints());
        MATH::CVector3Array srcPosArray = pointArray.GetPositionArray();
        CPolygonNodeRefArray nodeArray(xsiMesh->mesh.GetNodes());
        MATH::CVector3Array srcNormArray = nodeArray.GetNormalArray();
        CTriangleRefArray triArray = xsiMesh->mesh.GetTriangles();

		StringUtil::StrStreamType msg;
		msg << "-- " << XSItoOgre(xsiMesh->obj.GetName()) << " --" << std::endl;
		msg << "Points: " << pointArray.GetCount() << std::endl;
		msg << "Triangles: " << triArray.GetCount() << std::endl;
		msg << "Normals: " << srcNormArray.GetCount() << std::endl;
		msg << "Num UVs: " << mCurrentTextureCoordDimensions.size() << std::endl;
		String str = msg.str();
		LogOgreAndXSI(str);

		if (mCurrentTextureCoordDimensions.size() > OGRE_MAX_TEXTURE_COORD_SETS)
		{
			// too many texture coordinates!
			StringUtil::StrStreamType str;
			str << "PolygonMesh '" << XSItoOgre(xsiMesh->mesh.GetName()) 
				<< "' has too many texture coordinate sets (" 
				<< mCurrentTextureCoordDimensions.size()
				<< "); the limit is " << OGRE_MAX_TEXTURE_COORD_SETS;

			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, str.str(), 
				"XsiMeshExporter::processPolygonMesh");

		}
		
		// Save transforms
		MATH::CTransformation xsiTransform = xsiMesh->obj.GetKinematics().GetGlobal().GetTransform();
		MATH::CTransformation rotTrans;
		rotTrans.SetRotation(xsiTransform.GetRotation());
		// Bounds calculation
        Real squaredRadius = 0.0f;
        Vector3 min, max;
        bool first = true;
		CPolygonFaceRefArray polys(xsiMesh->mesh.GetPolygons());
        UniqueVertex vertex;


		float progPerTri = (float)progressUpdates / triArray.GetCount();
		float prog = 0.0f;
		// Iterate through all the triangles
        // There will often be less positions than normals and UVs
        // But TrianglePoint
        for (long t = 0; t < triArray.GetCount(); ++t)
        {
            Triangle tri(triArray[t]);
			// derive sampler indices for triangle
			size_t samplerIndices[3];
			deriveSamplerIndices(tri, polys[tri.GetPolygonIndex()], samplerIndices);

			// Decide which ProtoSubMesh we're to add this to (assume main)
			// If we find this triangle relates to a polygon which is in 
			// a cluster which has a different material, we change
			ProtoSubMesh* currentProto = mMainProtoMesh;
			PolygonToProtoSubMeshList::iterator polyi = 
				mPolygonToProtoSubMeshList.find(tri.GetPolygonIndex());
			if (polyi != mPolygonToProtoSubMeshList.end())
			{
				currentProto = polyi->second;
			}
			// has this mesh been used in this proto before? if not set offset
			size_t positionIndexOffset;
			if (currentProto->lastMeshEntry == xsiMesh)
			{
				positionIndexOffset = currentProto->lastMeshIndexOffset;
			}
			else
			{
				// first time this has been used
				// since we assume we 100% process each polygon mesh before the next,
				// just use last pointer since faster in this section
				currentProto->lastMeshEntry = xsiMesh;
				positionIndexOffset = currentProto->indices.size();
				currentProto->lastMeshIndexOffset = positionIndexOffset;
				// Also have to store this for future reference
				currentProto->polygonMeshOffsetMap[xsiMesh] = positionIndexOffset;
			}
			
            CTriangleVertexRefArray points = tri.GetPoints();
            for (long p = 0; p < 3; ++p)
            {
                TriangleVertex point(points[p]);
                long posIndex = point.GetIndex(); // unique position index
				// adjust index per offset, this makes position indices unique
				// per polymesh in teh same protosubmesh
				posIndex += positionIndexOffset;

				// Get position
				MATH::CVector3 xsipos = point.GetPosition();
				// Apply global SRT
				xsipos.MulByTransformationInPlace(xsiTransform);
                vertex.position = XSItoOgre(xsipos);
				// Get normal
				MATH::CVector3 xsinorm = point.GetNormal();
				// Apply global rotation
				xsinorm *= rotTrans;
                vertex.normal = XSItoOgre(xsinorm);

				for (size_t i = 0; i < mCurrentTextureCoordDimensions.size(); ++i)
				{
					// sampler indices can correctly dereference to sampler-order
					// uv sets we built earlier
					vertex.uv[i] = (mCurrentSamplerSets[i])[samplerIndices[p]];
				}
                
                if (mCurrentHasVertexColours)
                    vertex.colour = XSItoOgre(point.GetColor());

                size_t index = createOrRetrieveUniqueVertex(
									currentProto, posIndex, true, vertex);
                currentProto->indices.push_back(index);

				// bounds
				if (first)
				{
					squaredRadius = vertex.position.squaredLength();
					min = max = vertex.position;
					first = false;
				}
				else
				{
					squaredRadius = 
						std::max(squaredRadius, vertex.position.squaredLength());
					min.makeFloor(vertex.position);
					max.makeCeil(vertex.position);
				}
            }
		
			// Progress
			prog += progPerTri;
			while (prog >= 1.0f)
			{
				ProgressManager::getSingleton().progress();
				prog -= 1.0f;
			}

		}

		// Merge bounds
		AxisAlignedBox box;
        box.setExtents(min, max);
        box.merge(pMesh->getBounds());
        pMesh->_setBounds(box);
        pMesh->_setBoundingSphereRadius(
			std::max(
				pMesh->getBoundingSphereRadius(), 
				Math::Sqrt(squaredRadius)));

		// Deal with any bone assignments
		if (lookForBoneAssignments)
		{
			processBoneAssignments(pMesh, xsiMesh);
		}

		// process any shape keys
		processShapeKeys(pMesh, xsiMesh);

		// Post-process the mesh
		postprocessPolygonMesh(xsiMesh);

	}
	//-----------------------------------------------------------------------
	bool XsiMeshExporter::preprocessPolygonMesh(PolygonMeshEntry* xsiMesh)
	{
        // derive number of UVs
        int numUVs = 0;
        CRefArray clusterRefArray;
        // Filter to 'sample' types
        xsiMesh->mesh.GetClusters().Filter(
			siSampledPointCluster,CStringArray(),L"",clusterRefArray);

        Cluster samplePointClusterUV;
        CRefArray uvClusterPropertiesRefArray;
		int i;
        
        for(i = 0; i < clusterRefArray.GetCount(); ++i)
        {
            Cluster cluster(clusterRefArray[i]);		
            // Now filter all the 'uvspace' children
            // there is one of these per UV set
            if(cluster.GetProperties().Filter(
                siClsUVSpaceTxtType, CStringArray(), L"", 
                uvClusterPropertiesRefArray) == CStatus::OK)
            {
                samplePointClusterUV = cluster;			
                break;
            }
        }

        // Ok, we now have our array of UV sets
        numUVs = uvClusterPropertiesRefArray.GetCount();
		size_t numSamplerPoints = xsiMesh->mesh.GetNodes().GetCount();
		// list of UVs stored in order of sampler points (use 3D coords by default)
		mCurrentSamplerSets.reserve(numUVs);
		mCurrentTextureCoordDimensions.reserve(numUVs);
        for(i = 0; i < numUVs; ++i)
        {
			// init sampler points
			Vector3* samplerUVs = new Vector3[numSamplerPoints];
			mCurrentSamplerSets.push_back(samplerUVs);

			// Detect the dimensions by figuring out if any are all 0
			bool hasU, hasV, hasW;
			hasU = hasV = hasW = false;

			// Pull out all the UV data for this set and reorder it based on 
			// samples, we'll need this for reference later
			// get Elements from uvspace Property
            ClusterProperty uvProp(uvClusterPropertiesRefArray[i]);
			CClusterPropertyElementArray uvElements = uvProp.GetElements();

			// Add this to an map from uv set name to index
			String textureProjectionName = XSItoOgre(uvProp.GetName());
			mTextureProjectionMap[textureProjectionName] = i;

			// Now, each Element here is actually a CDoubleArray of the u,v,w values
			// However it's not in order of samplers, we need to use the Array
			// linked to from the Elements collection under the cluster (not the 
			// cluster property, confusing I know) to figure out what sampler 
			// index it is, i.e.
			// samplerUVs[Array[j]] = Element[j]
			// In all casesmCurrentSamplerSets in XSI element properties are referenced back to 
			// their original poly index via this array, ie all cluster properties
			// in the same cluster are dereferenced in the same way. 
			CLongArray derefArray = samplePointClusterUV.GetElements().GetArray();

			for (int j = 0; j < uvElements.GetCount(); ++j)
			{
				CDoubleArray curUVW(uvElements.GetItem(j));
				size_t samplerIndex = derefArray[j];
				samplerUVs[samplerIndex].x = curUVW[0];//u
				samplerUVs[samplerIndex].y = 1.0f - curUVW[1];//v (invert)
				samplerUVs[samplerIndex].z = curUVW[2];//w

				if (!hasU && !Ogre::Math::RealEqual(curUVW[0], 0))
					hasU = true;
				if (!hasV && !Ogre::Math::RealEqual(curUVW[1], 0))
					hasV = true;
				if (!hasW && !Ogre::Math::RealEqual(curUVW[2], 0))
					hasW = true;

			}
			
			// save dimensions
			mCurrentTextureCoordDimensions.push_back(
				(hasU?1:0) + (hasV?1:0) + (hasW?1:0));

        }


        // do we have vertex colours?
        ClusterProperty vertexColourClusterProperty = xsiMesh->mesh.GetCurrentVertexColor();
        if (vertexColourClusterProperty.IsValid())
            mCurrentHasVertexColours = true;
		else
			mCurrentHasVertexColours = false;

		
		/* Create any ProtoSubMeshes which don't exist yet for the 
		 * materials in question, and define the PolygonCluster map
		 */
		// Main material (will never exist if not merging submeshes)
		String materialName = mMaterialPrefix + 
			XSItoOgre(xsiMesh->obj.GetMaterial().GetName());
		registerMaterial(materialName, xsiMesh->obj.GetMaterial());
		
		mMainProtoMesh = createOrRetrieveProtoSubMesh(
			materialName, 
			XSItoOgre(xsiMesh->obj.GetName()),
			mCurrentTextureCoordDimensions, 
			mCurrentHasVertexColours);
		
		// For each polygon cluster
        CRefArray polygonClusters;
        // Filter to 'poly' types
        xsiMesh->mesh.GetClusters().Filter(siPolygonCluster, CStringArray(), L"", 
			polygonClusters);
		mPolygonToProtoSubMeshList.clear();
        for(i = 0; i < polygonClusters.GetCount(); ++i)
        {
			Cluster cluster(polygonClusters[i]);	
			// Is the material different for this poly cluster?
			if (cluster.GetMaterial() != xsiMesh->obj.GetMaterial())
			{
				String submatName = mMaterialPrefix + 
					XSItoOgre(cluster.GetMaterial().GetName());
				registerMaterial(submatName, cluster.GetMaterial());
				ProtoSubMesh* ps = createOrRetrieveProtoSubMesh(
					submatName,
					XSItoOgre(cluster.GetName()),
					mCurrentTextureCoordDimensions, 
					mCurrentHasVertexColours);
				// Each element is a polygon index 
				CLongArray elems = cluster.GetElements().GetArray();
				for (int p = 0; p < elems.GetCount(); ++p)
				{
					mPolygonToProtoSubMeshList[elems[p]] = ps;
				}
			}
		}

		return true;


	}
	//-----------------------------------------------------------------------
	void XsiMeshExporter::postprocessPolygonMesh(PolygonMeshEntry* xsiMesh)
	{
		// clear all position index remaps, incase merged
		for (MaterialProtoSubMeshMap::iterator m = mMaterialProtoSubmeshMap.begin();
			m != mMaterialProtoSubmeshMap.end(); ++m)
		{

			for (ProtoSubMeshList::iterator p = m->second->begin();
				p != m->second->end(); ++p)
			{
				ProtoSubMesh* ps = *p;
				ps->posIndexRemap.clear();
			}
		}

		// free temp UV data now
		for(SamplerSetList::iterator s = mCurrentSamplerSets.begin();
			s != mCurrentSamplerSets.end(); ++s)
		{
			// init sampler points
			delete [] *s;
		}
		mCurrentSamplerSets.clear();
		mCurrentTextureCoordDimensions.clear();
		
	}
	//-----------------------------------------------------------------------
	void XsiMeshExporter::processBoneAssignments(Mesh* pMesh, PolygonMeshEntry* xsiMesh)
	{
		// We have to iterate over the clusters which have envelope assignments
		// then, for each protosubmesh which uses this polymesh, we need to create
		// a bone assignment for each deformer, not forgetting to add one for 
		// each duplicated copy of this vertex too
		// We build up a global list of deformers as we go which will get passed
		// back to the top-level caller to build a skeleton from later
		CRefArray clusterRefArray;
		// Filter to 'vertex' types
		xsiMesh->mesh.GetClusters().Filter(
			siVertexCluster,CStringArray(),L"",clusterRefArray);



		for(int i = 0; i < clusterRefArray.GetCount(); ++i)
		{
			Cluster cluster(clusterRefArray[i]);	

			CRefArray envelopes = cluster.GetEnvelopes();
			for (int e = 0; e < envelopes.GetCount(); ++e)
			{
				Envelope envelope(envelopes[e]);

				// Get mapping from cluster element index to geometry position index
				CLongArray derefArray = envelope.GetElements(CTime().GetTime()).GetArray();

				CRefArray deformers = envelope.GetDeformers();
				for (int d = 0; d < deformers.GetCount(); ++d)
				{
					X3DObject deformer(deformers[d]);
					// Has this deformer been allocated a boneID already?
					String deformerName = XSItoOgre(deformer.GetName());
					DeformerMap::iterator di = 
						mXsiDeformerMap.find(deformerName);
					DeformerEntry* deformerEntry;
					bool newDeformerEntry = false;
					bool atLeastOneAssignment = false;
					if (di == mXsiDeformerMap.end())
					{
						deformerEntry = new DeformerEntry(mXsiDeformerMap.size(), deformer);
						deformerEntry->hasVertexAssignments = true;
						newDeformerEntry = true;
					}
					else
					{
						deformerEntry = di->second;
					}

					// Get the weights for this deformer
					CDoubleArray weights = 
						envelope.GetDeformerWeights(deformer, CTime().GetTime());
					// Weights are in order of cluster elements, we need to dereference
					// those to the original point index using the cluster element array
					for (int w = 0; w < weights.GetCount(); ++w)
					{
						size_t positionIndex = derefArray[w];
						float weight = weights[w];
						// Skip zero weights
						if (weight == 0.0f)
							continue;

						// Locate ProtoSubMeshes which use this mesh
						for (MaterialProtoSubMeshMap::iterator mi = mMaterialProtoSubmeshMap.begin();
							mi != mMaterialProtoSubmeshMap.end(); ++mi)
						{
							for (ProtoSubMeshList::iterator psi = mi->second->begin();
								psi != mi->second->end(); ++psi)
							{
								ProtoSubMesh* ps = *psi;
								ProtoSubMesh::PolygonMeshOffsetMap::iterator poli = 
									ps->polygonMeshOffsetMap.find(xsiMesh);
								if (poli != ps->polygonMeshOffsetMap.end())
								{
									// adjust index based on merging
									size_t adjIndex = positionIndex + poli->second;
									// look up real index
									// If it doesn't exist, it's probably on a seam
									// between clusters and we can safely skip it
									IndexRemap::iterator remi = ps->posIndexRemap.find(adjIndex);
									if (remi != ps->posIndexRemap.end())
									{

										size_t vertIndex = remi->second;
										bool moreVerts = true;
										// add UniqueVertex and clones
										while (moreVerts)
										{
											UniqueVertex& vertex = ps->uniqueVertices[vertIndex];
											VertexBoneAssignment vba;
											vba.boneIndex = deformerEntry->boneID;
											vba.vertexIndex = vertIndex;
											vba.weight = weight;
											ps->boneAssignments.insert(
												Mesh::VertexBoneAssignmentList::value_type(vertIndex, vba));
											atLeastOneAssignment = true;

											if (vertex.nextIndex == 0)
											{
												moreVerts = false;
											}
											else
											{
												vertIndex = vertex.nextIndex;
											}
										}
									}

								}
							}

						}



					}

					// Only add new deformer if we actually had any assignments
					if (newDeformerEntry && atLeastOneAssignment)
					{
						mXsiDeformerMap[deformerName] = deformerEntry;
					}



					
				}
				

			}
		}


	}
	//-----------------------------------------------------------------------
	void XsiMeshExporter::processShapeKeys(Mesh* pMesh, PolygonMeshEntry* xsiMesh)
	{
		// ShapeKeys are kept underneath clusters
		// The clusters are point clusters, not poly clusters like those used 
		// to define materials
		// Each point cluster identifies the list of points involved, and the shape key
		// contains the offsets for that key
		// Like bone assignments, we have to ensure we add keys for duplicated points

		// Get points & vertices incase we need to convert from local reference frame
		CPointRefArray pointsArray = xsiMesh->mesh.GetPoints();
		CVertexRefArray verticesArray = xsiMesh->mesh.GetVertices();

        CRefArray clusterRefArray;
        // Filter to 'pnt' types
        xsiMesh->mesh.GetClusters().Filter(
			siVertexCluster,CStringArray(),L"",clusterRefArray);

		for(int i = 0; i < clusterRefArray.GetCount(); ++i)
		{
			Cluster cluster(clusterRefArray[i]);

			// Cluster elements are the vertex indices affected
			CClusterElementArray vertexIndexArray = cluster.GetElements();

			CRefArray clusterProperties = cluster.GetProperties();
			for (int p = 0; p < clusterProperties.GetCount(); ++p)
			{
				ClusterProperty prop(clusterProperties[p]);
				// Pull out only shape keys
				if (prop.GetPropertyType() == siClusterPropertyShapeKeyType)
				{
					ShapeKey shapeKey(prop);

					Parameter keyTypeParam = shapeKey.GetParameter(L"KeyType");
					CValue currMode = keyTypeParam.GetValue();
					/*
					StringUtil::StrStreamType str;
					str << "KeyType = " << (unsigned short)currMode << 
						" siShapeLocalReferenceMode = " << (unsigned short)siShapeLocalReferenceMode <<
						" siShapeAbsoluteReferenceMode = " << (unsigned short)siShapeAbsoluteReferenceMode <<
						" siShapeObjectReferenceMode = " << (unsigned short)siShapeObjectReferenceMode;
					LogOgreAndXSI(str.str());
					*/

					// XSI bug? siShapeReferenceMode enum doesn't match runtime values
					// Local = 1, Absolute = 0, Object = 2 in real life
					// Logged with Softimage as UDEV00203965 
					bool isLocalSpace = 
						((unsigned short)currMode) == 1; //siShapeLocalReferenceMode;
					bool isGlobalSpace = 
						((unsigned short)currMode) == 0;//siShapeAbsoluteReferenceMode;

					LogOgreAndXSI("Found shape key " + XSItoOgre(shapeKey.GetName()));
					// elements of property are the offsets, a double array of values
					CClusterPropertyElementArray shapeElements = shapeKey.GetElements();

					// Locate ProtoSubMeshes which use this mesh
					for (MaterialProtoSubMeshMap::iterator mi = mMaterialProtoSubmeshMap.begin();
						mi != mMaterialProtoSubmeshMap.end(); ++mi)
					{
						for (ProtoSubMeshList::iterator psi = mi->second->begin();
							psi != mi->second->end(); ++psi)
						{
							ProtoSubMesh* ps = *psi;
							ProtoSubMesh::PolygonMeshOffsetMap::iterator poli = 
								ps->polygonMeshOffsetMap.find(xsiMesh);
							if (poli != ps->polygonMeshOffsetMap.end())
							{
								// remap from mesh vertex index to proto vertex index
								size_t indexAdjustment = poli->second;

								// Create a new pose, target is implied by proto, final
								// index to be determined later including merging
								Pose pose(0, XSItoOgre(shapeKey.GetName()));

								// Iterate per vertex affected
								for (int xi = 0; xi < vertexIndexArray.GetCount(); ++xi)
								{
									// Index
									size_t positionIndex = vertexIndexArray.GetItem(xi);
									// Now get offset
									CDoubleArray xsiOffset = shapeElements.GetItem(xi);
									Vector3 offset(xsiOffset[0], xsiOffset[1], xsiOffset[2]);

									// Skip zero offsets
									if (offset == Vector3::ZERO)
										continue;


									if (isLocalSpace)
									{
										// Local reference mode -> object space
										// Local mode is the most popular since in XSI
										// it plays nice with skeletal animation, but
										// it's relative to the _point's_ local space

										// Get local axes
										// XSI defines local space as:
										// Y = vertex normal
										// X = normalised projection of first edge 
										//     from vertex onto normal plane
										// Z = cross product of above
										Point point(pointsArray[positionIndex]);
										bool normalValid;
										Vector3 localY = XSItoOgre(point.GetNormal(normalValid));
										Vertex vertex(verticesArray.GetItem(positionIndex));
										CEdgeRefArray edgeArray = vertex.GetNeighborEdges();
										if (normalValid && edgeArray.GetCount() > 0)
										{

											Edge edge(edgeArray[0]);
											CVertexRefArray verticesOnEdge = edge.GetNeighborVertices();
											Vertex otherVertex = 
												(verticesOnEdge[0] == vertex) ?
												verticesOnEdge[1] : verticesOnEdge[0];
											Vector3 edgeVector 
												= XSItoOgre(otherVertex.GetPosition())
													- XSItoOgre(vertex.GetPosition());
											// Project the vector onto the normal plane (d irrelevant)
											Plane normPlane(localY, 0);
											Vector3 localX = normPlane.projectVector(edgeVector);
											localX.normalise();

											Vector3 localZ = localX.crossProduct(localY);

											// multiply out position by local axes to form
											// final position
											offset = (localX * offset.x) + 
												(localY * offset.y) + 
												(localZ * offset.z);

										}

									}
									
									if (!isGlobalSpace)
									{
										// shape is in object space, if object is parented
										// by a null or a static bone, we need to adjust the
										// shape offset since this inherited transform is
										// baked into the base OGRE mesh (to preserve
										// relative positioning of parts)
										// This deals with rotation and scaling

										// If object is parented
										// Don't know if anyone really uses this
										// Convert global to object space
										MATH::CTransformation xform = 
											xsiMesh->obj.GetKinematics().GetGlobal().GetTransform();
										MATH::CVector3 off(offset.x, offset.y, offset.z);
										off.MulByTransformationInPlace(xform);
										// now adjust for position since OGRE's poses
										// are relative to original vertex position
										off -= xform.GetTranslation();

										offset = XSItoOgre(off);



									}


									// adjust index based on merging
									size_t adjIndex = positionIndex + indexAdjustment;
									// look up real index
									// If it doesn't exist, it's probably on a seam
									// between clusters and we can safely skip it
									IndexRemap::iterator remi = ps->posIndexRemap.find(adjIndex);
									if (remi != ps->posIndexRemap.end())
									{

										size_t vertIndex = remi->second;
										bool moreVerts = true;


										// add UniqueVertex and clones
										while (moreVerts)
										{
											UniqueVertex& vertex = ps->uniqueVertices[vertIndex];

											// Create a vertex pose entry
											pose.addVertex(vertIndex, offset);

											if (vertex.nextIndex == 0)
											{
												moreVerts = false;
											}
											else
											{
												vertIndex = vertex.nextIndex;
											}
										} // more duplicate verts
									} // found remap?
								} // for each vertex affected

								// Add pose to proto
								ps->poseList.push_back(pose);

								// record that we used this shape key
								ps->shapeKeys.Add(shapeKey);


							} // proto found?
						}// for each proto
					} // for each material/protolist
				} // shape key cluster property?
			} // for each cluster property
		} // for each cluster

	}
	//-----------------------------------------------------------------------
	void XsiMeshExporter::buildShapeClipList(ShapeClipList& listToPopulate)
	{
		// Process all mixers
		Model root = mXsiApp.GetActiveSceneRoot();
		if (root.HasMixer())
		{
			XSI::Mixer mixer = root.GetMixer();
			buildShapeClipList(mixer, listToPopulate);
		}

		// Get all child models (recursive)
		CRefArray models = root.GetModels();
		for (int m = 0; m < models.GetCount(); ++m)
		{
			Model model(models[m]);
			if (model.HasMixer())
			{
				XSI::Mixer mixer = root.GetMixer();
				buildShapeClipList(mixer, listToPopulate);
			}
		}

	}
	//-----------------------------------------------------------------------
	void XsiMeshExporter::buildShapeClipList(ClipContainer& container, 
		ShapeClipList& listToPopulate)
	{
		CRefArray clips = container.GetClips();
		for (int c = 0; c < clips.GetCount(); ++c)
		{
			if (clips[c].IsA(siClipContainerID))
			{
				ClipContainer container(clips[c]);
				// cascade
				buildShapeClipList(container, listToPopulate);
			}
			else
			{

				XSI::Clip clip(clips[c]);
				XSI::CString clipType = clip.GetType();
				if (clip.GetType() == siClipShapeType)
				{
					XSI::TimeControl timeControl = clip.GetTimeControl();
					ShapeClipEntry sce;
					sce.clip = clip;
					sce.startFrame = sce.originalStartFrame = timeControl.GetStartOffset();
					long length = (1.0 / timeControl.GetScale()) * 
						(timeControl.GetClipOut() - timeControl.GetClipIn() + 1);
					sce.endFrame = sce.startFrame + length - 1;

					// Find link to shape
					sce.keytoPose = 0;
					ActionSource source(clip.GetSource());
					CRefArray sourceItems = source.GetItems();
					assert (sourceItems.GetCount() == 1 && "More than one source item on shape clip!");
					AnimationSourceItem sourceItem(sourceItems[0]);
					// Source is the shape key
					// Locate this in the list we built while building poses
					for (ShapeKeyMapping::iterator skm = mShapeKeyMapping.begin();
						skm != mShapeKeyMapping.end(); ++skm)
					{
						ShapeKeyToPoseEntry& mapping = *skm;
						if(mapping.shapeKey == sourceItem.GetSource())
						{
							// bingo
							sce.keytoPose = &(*skm);
						}
					}

					listToPopulate.push_back(sce);

				}
			}

		}


	}
	//-----------------------------------------------------------------------
	void XsiMeshExporter::exportAnimations(Mesh* pMesh, AnimationList& animList, 
		Real fps)
	{
		ShapeClipList clipList;
		buildShapeClipList(clipList);
		// Add all animations
		for (AnimationList::iterator i = animList.begin(); i != animList.end(); ++i)
		{
			// For each animation, we want to search for all shape clips using a
			// shape key which has been used 
			AnimationEntry& animEntry = *i;
			Animation* anim = 0;

			// Iterate per submesh since we want to collect a track per submesh
			// Iterate over the 'target index' which is submesh index + 1
			for (ushort targetIndex = 1; targetIndex <= pMesh->getNumSubMeshes(); ++targetIndex)
			{
				// find all shape clips for this submesh overlapping this time period
				// we'll clamp the clip too
				ShapeClipList submeshClipList;
				std::set<long> keyframeList;

				deriveShapeClipAndKeyframeList(targetIndex, animEntry, clipList, 
					submeshClipList, keyframeList);

				if (!submeshClipList.empty())
				{

					// Create animation if we haven't already
					if (!anim)
					{
						Real len = (float)(animEntry.endFrame - animEntry.startFrame + 1) / fps;
						anim = pMesh->createAnimation(animEntry.animationName, len);
					}
					
					// Create track
					VertexAnimationTrack* track = 
						anim->createVertexTrack(targetIndex, VAT_POSE);
					
					// Now sample all the clips on all the keyframes
					for (std::set<long>::iterator k = keyframeList.begin();
						k != keyframeList.end(); ++k)
					{
						// Create a key
						long frameNum = *k;
						Real keyTime = (float)(frameNum - animEntry.startFrame) / fps;
						VertexPoseKeyFrame* keyFrame = 
							track->createVertexPoseKeyFrame(keyTime);

						// Sample all the clips
						for (ShapeClipList::iterator c = submeshClipList.begin();
							c != submeshClipList.end(); ++c)
						{
							ShapeClipEntry& sce = *c;
							if(frameNum >= sce.startFrame && 
								frameNum <= sce.endFrame)
							{

								// map the keyframe number to a local number
								long localFrameNum = frameNum - sce.originalStartFrame *
									sce.clip.GetTimeControl().GetScale();

								// sample pose influences
								// Iterate over the animated parameters, find a 'weight' fcurve entry
								bool fcurveFound = false;
								CRefArray params = sce.clip.GetAnimatedParameters();
								for (int p = 0; p < params.GetCount(); ++p)
								{
									Parameter param(params[p]);
									if (param.GetSource().IsA(siFCurveID))
									{
										fcurveFound = true;
										FCurve fcurve(param.GetSource());
										CValue val = fcurve.Eval(localFrameNum);
										if ((float)val != 0.0f)
										{
											keyFrame->addPoseReference(
												sce.keytoPose->poseIndex, val);
										}
										
									}
								}

								if (!fcurveFound)
								{
									// No fcurves, so just assume 1.0 weight since that's
									// how XSI deals with it
									keyFrame->addPoseReference(sce.keytoPose->poseIndex, 1.0f);

								}
							}
						}
					}
					
				}

			}
		}

	}
	//-----------------------------------------------------------------------
	void XsiMeshExporter::deriveShapeClipAndKeyframeList(ushort targetIndex, 
		AnimationEntry& animEntry, ShapeClipList& inClipList, 
		ShapeClipList& outClipList, std::set<long>& keyFrameList)
	{
		for (ShapeClipList::iterator sci = inClipList.begin(); 
			sci != inClipList.end(); ++sci)
		{
			ShapeClipEntry& sce = *sci;

			if (sce.startFrame <= animEntry.endFrame &&
				sce.endFrame >= animEntry.startFrame && 
				sce.keytoPose->targetHandle == targetIndex)
			{
				// Clip overlaps with the animation sampling area and 
				// applies to this submesh
				ShapeClipEntry newClipEntry;
				newClipEntry.originalStartFrame = sce.startFrame;
				newClipEntry.startFrame = std::max(sce.startFrame, animEntry.startFrame);
				newClipEntry.endFrame = std::min(sce.endFrame, animEntry.endFrame);
				newClipEntry.clip = sce.clip;
				newClipEntry.keytoPose = sce.keytoPose;
				outClipList.push_back(newClipEntry);

				// Iterate over the animated parameters, find a 'weight' fcurve entry
				CRefArray params = sce.clip.GetAnimatedParameters();
				for (int p = 0; p < params.GetCount(); ++p)
				{
					Parameter param(params[p]);
					if (param.GetSource().IsA(siFCurveID))
					{
						FCurve fcurve(param.GetSource());
						CRefArray keys = fcurve.GetKeys();
						for (int k = 0; k < keys.GetCount(); ++k)
						{
							FCurveKey key(keys[k]);
							// convert from key time to global frame number
							CTime time = key.GetTime();
							long frameNum = (long)((double)(time.GetTime() + sce.startFrame)
								/ sce.clip.GetTimeControl().GetScale());
							keyFrameList.insert(frameNum);
						}
				

					}
				}

			}
		}
		// Always sample start & end frames
		keyFrameList.insert(animEntry.startFrame);
		keyFrameList.insert(animEntry.endFrame);

	}
	//-----------------------------------------------------------------------
	void XsiMeshExporter::exportProtoSubMeshes(Mesh* pMesh)
	{
		// Take the list of ProtoSubMesh instances and bake a SubMesh per
		// instance, then clear the list

		for (MaterialProtoSubMeshMap::iterator mi = mMaterialProtoSubmeshMap.begin();
			mi != mMaterialProtoSubmeshMap.end(); ++mi)
		{
			for (ProtoSubMeshList::iterator psi = mi->second->begin();
				psi != mi->second->end(); ++psi)
			{
				// export each one
				exportProtoSubMesh(pMesh, *psi);

				// free it
				delete *psi;
			}
			// delete proto list
			delete mi->second;
		}
		mMaterialProtoSubmeshMap.clear();
	}
	//-----------------------------------------------------------------------
	void XsiMeshExporter::exportProtoSubMesh(Mesh* pMesh, ProtoSubMesh* proto)
	{
		// Skip protos which have ended up empty
		if (proto->indices.empty())
			return;

        SubMesh* sm = 0;
        if (proto->name.empty())
        {
            // anonymous submesh
            sm = pMesh->createSubMesh();
        }
        else
        {
            // named submesh
            sm = pMesh->createSubMesh(proto->name);
        }

        // Set material
        sm->setMaterialName(proto->materialName);
        // never use shared geometry
        sm->useSharedVertices = false;
        sm->vertexData = new VertexData();
        // always do triangle list
        sm->indexData->indexCount = proto->indices.size();
		
		sm->vertexData->vertexCount = proto->uniqueVertices.size();
        // Determine index size
        bool use32BitIndexes = false;
        if (proto->uniqueVertices.size() > 65536)
        {
            use32BitIndexes = true;
        }

        sm->indexData->indexBuffer = 
            HardwareBufferManager::getSingleton().createIndexBuffer(
            use32BitIndexes ? HardwareIndexBuffer::IT_32BIT : HardwareIndexBuffer::IT_16BIT,
            sm->indexData->indexCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        if (use32BitIndexes)
        {
            uint32* pIdx = static_cast<uint32*>(
                sm->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
            writeIndexes(pIdx, proto->indices);
            sm->indexData->indexBuffer->unlock();
        }
        else
        {
            uint16* pIdx = static_cast<uint16*>(
                sm->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
            writeIndexes(pIdx, proto->indices);
            sm->indexData->indexBuffer->unlock();
        }


        // define vertex declaration
        unsigned buf = 0;
        size_t offset = 0;
		// always add position and normal
        sm->vertexData->vertexDeclaration->addElement(buf, offset, VET_FLOAT3, VES_POSITION);
        offset += VertexElement::getTypeSize(VET_FLOAT3);
		// Split vertex data after position if poses present
		if (!proto->poseList.empty())
		{
			buf++;
			offset = 0;
		}
        sm->vertexData->vertexDeclaration->addElement(buf, offset, VET_FLOAT3, VES_NORMAL);
        offset += VertexElement::getTypeSize(VET_FLOAT3);
        // split vertex data here if animated
        if (pMesh->hasSkeleton())
        {
            buf++;
            offset = 0;
        }
		// Optional vertex colour
        if(proto->hasVertexColours)
        {
            sm->vertexData->vertexDeclaration->addElement(buf, offset, VET_COLOUR, VES_DIFFUSE);
            offset += VertexElement::getTypeSize(VET_COLOUR);
        }
        // Define UVs
        for (unsigned short uvi = 0; uvi < proto->textureCoordDimensions.size(); ++uvi)
        {
			VertexElementType uvType = 
				VertexElement::multiplyTypeCount(
					VET_FLOAT1, proto->textureCoordDimensions[uvi]);
            sm->vertexData->vertexDeclaration->addElement(
				buf, offset, uvType, VES_TEXTURE_COORDINATES, uvi);
            offset += VertexElement::getTypeSize(uvType);
        }

        // create & fill buffer(s)
        for (unsigned short b = 0; b <= sm->vertexData->vertexDeclaration->getMaxSource(); ++b)
        {
            createVertexBuffer(sm->vertexData, b, proto->uniqueVertices);
        }

		// deal with any bone assignments
		if (!proto->boneAssignments.empty())
		{
			// rationalise first (normalises and strips out any excessive bones)
			sm->parent->_rationaliseBoneAssignments(
				sm->vertexData->vertexCount, proto->boneAssignments);

			for (Mesh::VertexBoneAssignmentList::iterator bi = proto->boneAssignments.begin();
				bi != proto->boneAssignments.end(); ++bi)
			{
				sm->addBoneAssignment(bi->second);
			}
		}

		// poses
		// derive target index (current submesh index + 1 since 0 is shared geom)
		ushort targetIndex = pMesh->getNumSubMeshes(); 
		ushort sk = 0;
		for (std::list<Pose>::iterator pi = proto->poseList.begin();
			pi != proto->poseList.end(); ++pi, ++sk)
		{
			Pose* pose = pMesh->createPose(targetIndex, pi->getName());
			Pose::VertexOffsetIterator vertIt = 
				pi->getVertexOffsetIterator();
			while (vertIt.hasMoreElements())
			{
				pose->addVertex(vertIt.peekNextKey(), vertIt.peekNextValue());
				vertIt.getNext();
			}

			// record shape key to pose mapping for animation later
			ShapeKeyToPoseEntry se;
			se.shapeKey = proto->shapeKeys[sk];
			se.poseIndex = pMesh->getPoseCount() - 1;
			se.targetHandle = targetIndex;
			mShapeKeyMapping.push_back(se);

		}

	}
	//-----------------------------------------------------------------------
	void XsiMeshExporter::buildPolygonMeshList(bool includeChildren)
	{
		Selection sel(mXsiApp.GetSelection());
		if (sel.GetCount() == 0)
		{
			// Whole scene
			// Derive the scene root
			X3DObject sceneRoot(mXsiApp.GetActiveSceneRoot());
			findPolygonMeshes(sceneRoot, true);
		}
		else
		{
			// iterate over selection
			for (int i = 0; i < sel.GetCount(); ++i)
			{
				X3DObject obj(sel[i]);
				findPolygonMeshes(obj,includeChildren);
			}
		}
	}
	//-----------------------------------------------------------------------
	void XsiMeshExporter::findPolygonMeshes(X3DObject& x3dObj, bool recurse)
	{
		// Check validity of current object
		if (!x3dObj.IsValid())
		{
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
				"Invalid X3DObject found",
				"XsiMeshExporter::exportX3DObject");
		}
		// Log a message in script window
		CString name = x3dObj.GetName() ;
		LogOgreAndXSI(L"-- Traversing " +  name) ;


		// locate any geometry
		if (!x3dObj.IsA(siCameraID) && 
			!x3dObj.IsA(siLightID) && 
			!x3dObj.IsA(siNullID) && 
			!x3dObj.IsA(siModelID))
		{
			Primitive prim(x3dObj.GetActivePrimitive());
			if (prim.IsValid())
			{
				Geometry geom(prim.GetGeometry());
				if (geom.GetRef().GetClassID() == siPolygonMeshID)
				{
					// add it to the list
					PolygonMesh pmesh(geom);
					mXsiPolygonMeshList.insert(
						new PolygonMeshEntry(pmesh, x3dObj));

					LogOgreAndXSI(L"-- Queueing " +  name) ;
				}
			}

		}

		// Cascade into children
		if (recurse)
		{
			CRefArray children = x3dObj.GetChildren();

			for(long i = 0; i < children.GetCount(); i++)
			{
				X3DObject childObj = children[i];
				findPolygonMeshes(childObj, recurse);
			}
		}

	}
    //-----------------------------------------------------------------------
	void XsiMeshExporter::cleanupPolygonMeshList(void)
	{
		for (PolygonMeshList::iterator pm = mXsiPolygonMeshList.begin();
			pm != mXsiPolygonMeshList.end(); ++pm)
		{
			delete *pm;
		}
		mXsiPolygonMeshList.clear();
	}
	//-----------------------------------------------------------------------
	void XsiMeshExporter::cleanupDeformerMap(void)
	{
		for (DeformerMap::iterator d = mXsiDeformerMap.begin();
			d != mXsiDeformerMap.end(); ++d)
		{
			delete d->second;
		}
		mXsiDeformerMap.clear();
	}
	//-----------------------------------------------------------------------
	void XsiMeshExporter::cleanupMaterialMap(void)
	{
		for (MaterialMap::iterator d = mXsiMaterialMap.begin();
			d != mXsiMaterialMap.end(); ++d)
		{
			delete d->second;
		}
		mXsiMaterialMap.clear();
	}
	//-----------------------------------------------------------------------
	void XsiMeshExporter::deriveSamplerIndices(const Triangle& tri, 
		const PolygonFace& face, size_t* samplerIndices)
	{
		//We want to find what is the SampleIndex associated with 3 
		//vertex in a Triangle
		CPointRefArray facePoints = face.GetPoints();

		//Get the position of the 3 vertex in the triangle
		MATH::CVector3Array triPos = tri.GetPositionArray();

		//Get the position of the N Points in the polygon
		MATH::CVector3Array facePos = facePoints.GetPositionArray();

		//To know if the 3 vertex have a point in the same position
		bool found[3];
		found[0] = false;
		found[1] = false;
		found[2] = false;

		int p,t;
		//For the 3 triangle vertices
		for(t=0; t<3 ; t++)
		{       //for each polygon point
			for(p=0; p<facePos.GetCount() && !found[t]; p++)
			{
				//Check if the position is the same
				if(triPos[t] == facePos[p])
				{
					//if so, we know the PolygonPointIndex of the TriangleVertex
					//then, we must find what is the sample index associated 
					//with this Point
					samplerIndices[t] = 
						getSamplerIndex(Facet(face), facePoints[p]);
					found[t] = true;
				}
			}

		}

		if (!found[0] || !found[1] || !found[2] )
		{
			// Problem!
			LogOgreAndXSI(L"!! Couldn't find a matching UV point!");
		}

	}
	//-----------------------------------------------------------------------
	size_t XsiMeshExporter::getSamplerIndex(const Facet &f, const Point &p)
	{
		//This function check if a Sample is shared by a Facet and a Point
		//just by using the operator=
		//Only one Sample can be shared.

		Sample curFacetSample;
		CSampleRefArray facetSamples( f.GetSamples() );
		CSampleRefArray pointSamples( p.GetSamples() );

		for(int i = 0; i < facetSamples.GetCount(); i++ )
		{

			curFacetSample = Sample( facetSamples[i] );

			for(int j = 0; j < pointSamples.GetCount(); j++)
			{
				if(curFacetSample == Sample(pointSamples[j]))
				{
					return curFacetSample.GetIndex();
				}
			}
		}
		// Problem!
		mXsiApp.LogMessage(L"!! Couldn't find a matching sample point!");
		return 0;
	}
    //-----------------------------------------------------------------------
    template <typename T> 
    void XsiMeshExporter::writeIndexes(T* buf, IndexList& indexes)
    {
        IndexList::const_iterator i, iend;
        iend = indexes.end();
        for (i = indexes.begin(); i != iend; ++i)
        {
            *buf++ = static_cast<T>(*i);
        }
    }
    //-----------------------------------------------------------------------
    void XsiMeshExporter::createVertexBuffer(VertexData* vd, 
		unsigned short bufIdx, UniqueVertexList& uniqueVertexList)
    {
        HardwareVertexBufferSharedPtr vbuf = 
			HardwareBufferManager::getSingleton().createVertexBuffer(
            	vd->vertexDeclaration->getVertexSize(bufIdx),
            	vd->vertexCount, 
            	HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        vd->vertexBufferBinding->setBinding(bufIdx, vbuf);
        size_t vertexSize = vd->vertexDeclaration->getVertexSize(bufIdx);

        char* pBase = static_cast<char*>(
				vbuf->lock(HardwareBuffer::HBL_DISCARD));

        VertexDeclaration::VertexElementList elems = 
			vd->vertexDeclaration->findElementsBySource(bufIdx);
        VertexDeclaration::VertexElementList::iterator ei, eiend;
        eiend = elems.end();
        float* pFloat;
        RGBA* pRGBA;

        UniqueVertexList::iterator srci = uniqueVertexList.begin();

        for (size_t v = 0; v < vd->vertexCount; ++v, ++srci)
        {
            for (ei = elems.begin(); ei != eiend; ++ei)
            {
                VertexElement& elem = *ei;
                switch(elem.getSemantic())
                {
                case VES_POSITION:
                    elem.baseVertexPointerToElement(pBase, &pFloat);
                    *pFloat++ = srci->position.x;
                    *pFloat++ = srci->position.y;
                    *pFloat++ = srci->position.z;
                    break;
                case VES_NORMAL:
                    elem.baseVertexPointerToElement(pBase, &pFloat);
                    *pFloat++ = srci->normal.x;
                    *pFloat++ = srci->normal.y;
                    *pFloat++ = srci->normal.z;
                    break;
                case VES_DIFFUSE:
                    elem.baseVertexPointerToElement(pBase, &pRGBA);
                    *pRGBA = srci->colour;
                    break;
                case VES_TEXTURE_COORDINATES:
                    elem.baseVertexPointerToElement(pBase, &pFloat);
					for (int t = 0; t < VertexElement::getTypeCount(elem.getType()); ++t)
					{
						Real val = srci->uv[elem.getIndex()][t];
						*pFloat++ = val;
					}
                    break;
                }
            }
            pBase += vertexSize;
        }
        vbuf->unlock();

    }
    //-----------------------------------------------------------------------
    size_t XsiMeshExporter::createOrRetrieveUniqueVertex(
		ProtoSubMesh* proto, size_t positionIndex, 
		bool positionIndexIsOriginal, const UniqueVertex& vertex)
    {
		size_t lookupIndex;
		if (positionIndexIsOriginal)
		{
			// look up the original index
			IndexRemap::iterator remapi = 
				proto->posIndexRemap.find(positionIndex);
			if (remapi == proto->posIndexRemap.end())
			{
				// not found, add
				size_t realIndex = proto->uniqueVertices.size();
				// add remap entry so we can find this again
				proto->posIndexRemap[positionIndex] = realIndex;
				proto->uniqueVertices.push_back(vertex);
				return realIndex;
			}
			else
			{
				// Found existing mapping
				lookupIndex = remapi->second;
			}
		}
		else
		{
			// Not an original index, index is real
			lookupIndex = positionIndex;
		}

		// If we get here, either the position isn't an original index (ie
		// we've already found that it doesn't match, and have cascaded)
		// or there is an existing entry
		// Get existing
	    UniqueVertex& orig = proto->uniqueVertices[lookupIndex];
		// Compare, do we have the same details?
		if (orig == vertex)
		{
			// ok, they match
			return lookupIndex;
		}
		else
		{
    	    // no match, go to next or create new
	        if (orig.nextIndex)
        	{
            	// cascade to the next candidate (which is a real index, not an original)
                return createOrRetrieveUniqueVertex(
						proto, orig.nextIndex, false, vertex);
            }
			else
			{
				// No more cascades to check, must be a new one
	            // get new index
    	        size_t realIndex = proto->uniqueVertices.size();
        	    orig.nextIndex = realIndex;
            	// create new (NB invalidates 'orig' reference)
                proto->uniqueVertices.push_back(vertex);
				// note, don't add to remap, that's only for finding the
				// first entry, nextIndex is used to chain to the others

    	        return realIndex;
			}
		}
    }
    //-----------------------------------------------------------------------
	void XsiMeshExporter::registerMaterial(const String& name, 
		XSI::Material mat)
	{
		// Check we have a real-time shader based material first
		XSI::Parameter rtParam = mat.GetParameter(L"RealTime");
		
		if(rtParam.GetSource().IsValid() && 
			rtParam.GetSource().IsA(XSI::siShaderID))
		{
			MaterialMap::iterator i = mXsiMaterialMap.find(name);
			if (i == mXsiMaterialMap.end())
			{
				// Add this one to the list
				MaterialEntry* matEntry = new MaterialEntry();
				matEntry->name = name;
				matEntry->xsiShader = XSI::Shader(rtParam.GetSource());
				mXsiMaterialMap[name] = matEntry;
			}
		}
	}
}
