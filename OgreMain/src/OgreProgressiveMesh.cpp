/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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
#include "OgreStableHeaders.h"

// The algorithm in this file is based heavily on:
/*
*  Progressive Mesh type Polygon Reduction Algorithm
*  by Stan Melax (c) 1998
*/

#include "OgreProgressiveMesh.h"
#include "OgreString.h"
#include "OgreHardwareBufferManager.h"
#include <algorithm>

#include <iostream>

#if OGRE_DEBUG_MODE 
std::ofstream ofdebug;
#endif 

namespace Ogre {
	#define NEVER_COLLAPSE_COST 99999.9f


    /** Comparator for unique vertex list
    */
    struct vectorLess
    {
		_OgreExport bool operator()(const Vector3& v1, const Vector3& v2) const
        {
			if (v1.x < v2.x) return true;
			if (v1.x == v2.x && v1.y < v2.y) return true;
			if (v1.x == v2.x && v1.y == v2.y && v1.z < v2.z) return true;

			return false;
		}
	};
    //---------------------------------------------------------------------
    ProgressiveMesh::ProgressiveMesh(const VertexData* vertexData, 
        const IndexData* indexData)
    {
        addWorkingData(vertexData, indexData);
        mpVertexData = vertexData;
        mpIndexData = indexData;
        mWorstCosts.resize(vertexData->vertexCount);



    }
    //---------------------------------------------------------------------
    ProgressiveMesh::~ProgressiveMesh()
    {
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::addExtraVertexPositionBuffer(const VertexData* vertexData)
    {
        addWorkingData(vertexData, mpIndexData);
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::build(ushort numLevels, LODFaceList* outList, 
			VertexReductionQuota quota, Real reductionValue)
    {
        IndexData* newLod;

        computeAllCosts();

#if OGRE_DEBUG_MODE
		dumpContents("pm_before.log");
#endif

        // Init
        mCurrNumIndexes = mpIndexData->indexCount;
        size_t numVerts, numCollapses;
        // Use COMMON vert count, not original vert count
        // Since collapsing 1 common vert position is equivalent to collapsing them all
        numVerts = mNumCommonVertices;
		
#if OGRE_DEBUG_MODE 
		ofdebug.open("progressivemesh.log");
#endif
		numCollapses = 0;
		bool abandon = false;
		while (numLevels--)
        {
            // NB idf 'abandon' is set, we stop reducing 
            // However, we still bake the number of LODs requested, even if it 
            // means they are the same
            if (!abandon)
            {
			    if (quota == VRQ_PROPORTIONAL)
			    {
				    numCollapses = static_cast<size_t>(numVerts * reductionValue);
			    }
			    else 
			    {
				    numCollapses = static_cast<size_t>(reductionValue);
			    }
                // Minimum 3 verts!
                if ( (numVerts - numCollapses) < 3) 
                    numCollapses = numVerts - 3;
			    // Store new number of verts
			    numVerts = numVerts - numCollapses;

			    while(numCollapses-- && !abandon)
                {
                    size_t nextIndex = getNextCollapser();
                    // Collapse on every buffer
                    WorkingDataList::iterator idata, idataend;
                    idataend = mWorkingData.end();
                    for (idata = mWorkingData.begin(); idata != idataend; ++idata)
                    {
                        PMVertex* collapser = &( idata->mVertList.at( nextIndex ) );
                        // This will reduce mCurrNumIndexes and recalc costs as required
					    if (collapser->collapseTo == NULL)
					    {
						    // Must have run out of valid collapsables
						    abandon = true;
						    break;
					    }
#if OGRE_DEBUG_MODE 
					    ofdebug << "Collapsing index " << (unsigned int)collapser->index << "(border: "<< collapser->isBorder() <<
						    ") to " << (unsigned int)collapser->collapseTo->index << "(border: "<< collapser->collapseTo->isBorder() <<
						    ")" << std::endl;
#endif
					    assert(collapser->collapseTo->removed == false);

                        collapse(collapser);
                    }

                }
            }
#if OGRE_DEBUG_MODE
			StringUtil::StrStreamType logname;
			logname << "pm_level" << numLevels << ".log";
			dumpContents(logname.str());
#endif

            // Bake a new LOD and add it to the list
            newLod = OGRE_NEW IndexData();
            bakeNewLOD(newLod);
            outList->push_back(newLod);
			
        }



    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::addWorkingData(const VertexData * vertexData, 
        const IndexData * indexData)
    {
        // Insert blank working data, then fill 
        mWorkingData.push_back(PMWorkingData());

        PMWorkingData& work = mWorkingData.back();

        // Build vertex list
		// Resize face list (this will always be this big)
		work.mFaceVertList.resize(vertexData->vertexCount);
		// Also resize common vert list to max, to avoid reallocations
		work.mVertList.resize(vertexData->vertexCount);

		// locate position element & hte buffer to go with it
		const VertexElement* posElem = vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
		HardwareVertexBufferSharedPtr vbuf = 
			vertexData->vertexBufferBinding->getBuffer(posElem->getSource());
		// lock the buffer for reading
		unsigned char* pVertex = static_cast<unsigned char*>(
			vbuf->lock(HardwareBuffer::HBL_READ_ONLY));
		float* pFloat;
		Vector3 pos;
		// Map for identifying duplicate position vertices
		typedef std::map<Vector3, size_t, vectorLess> CommonVertexMap;
		CommonVertexMap commonVertexMap;
		CommonVertexMap::iterator iCommonVertex;
		size_t numCommon = 0;
        size_t i = 0;
        for (i = 0; i < vertexData->vertexCount; ++i, pVertex += vbuf->getVertexSize())
        {
			posElem->baseVertexPointerToElement(pVertex, &pFloat);

            pos.x = *pFloat++;
            pos.y = *pFloat++;
            pos.z = *pFloat++;

			// Try to find this position in the existing map 
			iCommonVertex = commonVertexMap.find(pos);
			if (iCommonVertex == commonVertexMap.end())
			{
				// Doesn't exist, so create it
				PMVertex* commonVert = &(work.mVertList[numCommon]);
				commonVert->setDetails(pos, numCommon);
				commonVert->removed = false;
				commonVert->toBeRemoved = false;
				commonVert->seam = false;

				// Enter it in the map
				commonVertexMap.insert(CommonVertexMap::value_type(pos, numCommon) );
				// Increment common index
				++numCommon;

				work.mFaceVertList[i].commonVertex = commonVert;
				work.mFaceVertList[i].realIndex = i;
			}
			else
			{
				// Exists already, reference it
				PMVertex* existingVert = &(work.mVertList[iCommonVertex->second]);
				work.mFaceVertList[i].commonVertex = existingVert;
				work.mFaceVertList[i].realIndex = i;

				// Also tag original as a seam since duplicates at this location
				work.mFaceVertList[i].commonVertex->seam = true;

			}
			
        }
		vbuf->unlock();

		mNumCommonVertices = numCommon;

        // Build tri list
        size_t numTris = indexData->indexCount / 3;
		unsigned short* pShort;
		unsigned int* pInt;
		HardwareIndexBufferSharedPtr ibuf = indexData->indexBuffer;
		bool use32bitindexes = (ibuf->getType() == HardwareIndexBuffer::IT_32BIT);
		if (use32bitindexes)
		{
			pInt = static_cast<unsigned int*>(
				ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
		}
		else
		{
			pShort = static_cast<unsigned short*>(
				ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
		}
        work.mTriList.resize(numTris); // assumed tri list
        for (i = 0; i < numTris; ++i)
        {
			PMFaceVertex *v0, *v1, *v2;
			// use 32-bit index always since we're not storing
			unsigned int vindex = use32bitindexes? *pInt++ : *pShort++;
			v0 = &(work.mFaceVertList[vindex]);
			vindex = use32bitindexes? *pInt++ : *pShort++;
			v1 = &(work.mFaceVertList[vindex]);
			vindex = use32bitindexes? *pInt++ : *pShort++;
			v2 = &(work.mFaceVertList[vindex]);

			work.mTriList[i].setDetails(i, v0, v1, v2);

            work.mTriList[i].removed = false;

        }
		ibuf->unlock();

    }
    //---------------------------------------------------------------------
    Real ProgressiveMesh::computeEdgeCollapseCost(PMVertex *src, PMVertex *dest)
    {
        // if we collapse edge uv by moving src to dest then how 
        // much different will the model change, i.e. how much "error".
        // The method of determining cost was designed in order 
        // to exploit small and coplanar regions for
        // effective polygon reduction.
        Vector3 edgeVector = src->position - dest->position;

        Real cost;
		Real curvature = 0.001f;

        // find the "sides" triangles that are on the edge uv
        PMVertex::FaceList sides;
        PMVertex::FaceList::iterator srcface, srcfaceEnd;
        srcfaceEnd = src->face.end();
        // Iterate over src's faces and find 'sides' of the shared edge which is being collapsed
        for(srcface = src->face.begin(); srcface != srcfaceEnd; ++srcface)
        {
            // Check if this tri also has dest in it (shared edge)
            if( (*srcface)->hasCommonVertex(dest) )
            {
                sides.insert(*srcface);
            }
        }

		// Special cases
		// If we're looking at a border vertex
        if(src->isBorder())
        {
			if (sides.size() > 1) 
			{
				// src is on a border, but the src-dest edge has more than one tri on it
				// So it must be collapsing inwards
				// Mark as very high-value cost
				// curvature = 1.0f;
				cost = 1.0f;
			}
			else
			{
				// Collapsing ALONG a border
				// We can't use curvature to measure the effect on the model
				// Instead, see what effect it has on 'pulling' the other border edges
				// The more colinear, the less effect it will have
				// So measure the 'kinkiness' (for want of a better term)
				// Normally there can be at most 1 other border edge attached to this
				// However in weird cases there may be more, so find the worst
				Vector3 collapseEdge, otherBorderEdge;
				Real kinkiness, maxKinkiness;
				PMVertex::NeighborList::iterator n, nend;
				nend = src->neighbor.end();
				maxKinkiness = 0.0f;
				edgeVector.normalise();
				collapseEdge = edgeVector;
				for (n = src->neighbor.begin(); n != nend; ++n)
				{
					if (*n != dest && (*n)->isManifoldEdgeWith(src))
					{
						otherBorderEdge = src->position - (*n)->position;
						otherBorderEdge.normalise();
						// This time, the nearer the dot is to -1, the better, because that means
						// the edges are opposite each other, therefore less kinkiness
						// Scale into [0..1]
						kinkiness = (otherBorderEdge.dotProduct(collapseEdge) + 1.002f) * 0.5f;
						maxKinkiness = std::max(kinkiness, maxKinkiness);

					}
				}

				cost = maxKinkiness; 

			}
        } 
		else // not a border
		{

			// Standard inner vertex
			// Calculate curvature
			// use the triangle facing most away from the sides 
			// to determine our curvature term
			// Iterate over src's faces again
			for(srcface = src->face.begin(); srcface != srcfaceEnd; ++srcface) 
			{
				Real mincurv = 1.0f; // curve for face i and closer side to it
				// Iterate over the sides
				PMVertex::FaceList::iterator sidesFace, sidesFaceEnd;
				sidesFaceEnd = sides.end();
				for(sidesFace = sides.begin(); sidesFace != sidesFaceEnd; ++sidesFace) 
				{
					// Dot product of face normal gives a good delta angle
					Real dotprod = (*srcface)->normal.dotProduct( (*sidesFace)->normal );
					// NB we do (1-..) to invert curvature where 1 is high curvature [0..1]
					// Whilst dot product is high when angle difference is low
					mincurv =  std::min(mincurv,(1.002f - dotprod) * 0.5f);
				}
				curvature = std::max(curvature, mincurv);
			}
			cost = curvature;
		}

        // check for texture seam ripping
		if (src->seam && !dest->seam)
		{
			cost = 1.0f;
		}

        // Check for singular triangle destruction
        // If src and dest both only have 1 triangle (and it must be a shared one)
        // then this would destroy the shape, so don't do this
        if (src->face.size() == 1 && dest->face.size() == 1)
        {
            cost = NEVER_COLLAPSE_COST;
        }


		// Degenerate case check
		// Are we going to invert a face normal of one of the neighbouring faces?
		// Can occur when we have a very small remaining edge and collapse crosses it
		// Look for a face normal changing by > 90 degrees
		for(srcface = src->face.begin(); srcface != srcfaceEnd; ++srcface) 
		{
			// Ignore the deleted faces (those including src & dest)
			if( !(*srcface)->hasCommonVertex(dest) )
			{
				// Test the new face normal
				PMVertex *v0, *v1, *v2;
				// Replace src with dest wherever it is
				v0 = ( (*srcface)->vertex[0]->commonVertex == src) ? dest : (*srcface)->vertex[0]->commonVertex;
				v1 = ( (*srcface)->vertex[1]->commonVertex == src) ? dest : (*srcface)->vertex[1]->commonVertex;
				v2 = ( (*srcface)->vertex[2]->commonVertex == src) ? dest : (*srcface)->vertex[2]->commonVertex;

				// Cross-product 2 edges
				Vector3 e1 = v1->position - v0->position; 
				Vector3 e2 = v2->position - v1->position;

				Vector3 newNormal = e1.crossProduct(e2);
				newNormal.normalise();

				// Dot old and new face normal
				// If < 0 then more than 90 degree difference
				if (newNormal.dotProduct( (*srcface)->normal ) < 0.0f )
				{
					// Don't do it!
					cost = NEVER_COLLAPSE_COST;
					break; // No point continuing
				}


			}
		}
		

		assert (cost >= 0);
		return cost;
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::initialiseEdgeCollapseCosts(void)
    {
        WorkingDataList::iterator i, iend;
        iend = mWorkingData.end();
        for (i = mWorkingData.begin(); i != iend; ++i)
        {
            CommonVertexList::iterator v, vend;
            vend = i->mVertList.end();
            for (v = i->mVertList.begin(); v != vend; ++v)
            {
                v->collapseTo = NULL;
                v->collapseCost = NEVER_COLLAPSE_COST;
            }
        }

        
    }
    //---------------------------------------------------------------------
    Real ProgressiveMesh::computeEdgeCostAtVertexForBuffer(WorkingDataList::iterator idata, size_t vertIndex)
    {
        // compute the edge collapse cost for all edges that start
        // from vertex v.  Since we are only interested in reducing
        // the object by selecting the min cost edge at each step, we
        // only cache the cost of the least cost edge at this vertex
        // (in member variable collapse) as well as the value of the 
        // cost (in member variable objdist).

        CommonVertexList::iterator v = idata->mVertList.begin();
        v += vertIndex;

        if(v->neighbor.empty()) {
            // v doesn't have neighbors so nothing to collapse
            v->notifyRemoved();
            return v->collapseCost;
        }

        // Init metrics
        v->collapseCost = NEVER_COLLAPSE_COST;
        v->collapseTo = NULL;

        // search all neighboring edges for "least cost" edge
        PMVertex::NeighborList::iterator n, nend;
        nend = v->neighbor.end();
        Real cost;
        for(n = v->neighbor.begin(); n != nend; ++n) 
        {
            cost = computeEdgeCollapseCost(&(*v), *n);
            if( (!v->collapseTo) || cost < v->collapseCost) 
            {
                v->collapseTo = *n;  // candidate for edge collapse
                v->collapseCost = cost;             // cost of the collapse
            }
        }

        return v->collapseCost;
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::computeAllCosts(void)
    {
        initialiseEdgeCollapseCosts();
        size_t i;
        for (i = 0; i < mpVertexData->vertexCount; ++i)
        {
            computeEdgeCostAtVertex(i);
        }
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::collapse(ProgressiveMesh::PMVertex *src)
    {
        PMVertex *dest = src->collapseTo;
		std::set<PMVertex*> recomputeSet;

		// Abort if we're never supposed to collapse
		if (src->collapseCost == NEVER_COLLAPSE_COST) 
			return;

		// Remove this vertex from the running for the next check
		src->collapseTo = NULL;
		src->collapseCost = NEVER_COLLAPSE_COST;
		mWorstCosts[src->index] = NEVER_COLLAPSE_COST;

		// Collapse the edge uv by moving vertex u onto v
	    // Actually remove tris on uv, then update tris that
	    // have u to have v, and then remove u.
	    if(!dest) {
		    // src is a vertex all by itself 
#if OGRE_DEBUG_MODE 
			ofdebug << "Aborting collapse, orphan vertex. " << std::endl;
#endif
			return;
	    }

		// Add dest and all the neighbours of source and dest to recompute list
		recomputeSet.insert(dest);
		PMVertex::NeighborList::iterator n, nend;
        nend = src->neighbor.end();

		PMVertex* temp;

	    for(n = src->neighbor.begin(); n != nend; ++n)
        {
			temp = *n;
			recomputeSet.insert( *n );
		}
        nend = dest->neighbor.end();
	    for(n = dest->neighbor.begin(); n != nend; ++n)
        {
			temp = *n;
			recomputeSet.insert( *n );
		}

	    // delete triangles on edge src-dest
        // Notify others to replace src with dest
        PMVertex::FaceList::iterator f, fend;
        fend = src->face.end();
		// Queue of faces for removal / replacement
		// prevents us screwing up the iterators while we parse
		PMVertex::FaceList faceRemovalList, faceReplacementList;
	    for(f = src->face.begin(); f != fend; ++f) 
        {
		    if((*f)->hasCommonVertex(dest)) 
            {
                // Tri is on src-dest therefore is gone
				faceRemovalList.insert(*f);
                // Reduce index count by 3 (useful for quick allocation later)
                mCurrNumIndexes -= 3;
		    }
            else
            {
                // Only src involved, replace with dest
				faceReplacementList.insert(*f);
            }
	    }

		src->toBeRemoved = true;
		// Replace all the faces queued for replacement
	    for(f = faceReplacementList.begin(); f != faceReplacementList.end(); ++f) 
		{
			/* Locate the face vertex which corresponds with the common 'dest' vertex
			To to this, find a removed face which has the FACE vertex corresponding with
			src, and use it's FACE vertex version of dest.
			*/
			PMFaceVertex* srcFaceVert = (*f)->getFaceVertexFromCommon(src);
			PMFaceVertex* destFaceVert = NULL;
			PMVertex::FaceList::iterator iremoved;
			for(iremoved = faceRemovalList.begin(); iremoved != faceRemovalList.end(); ++iremoved) 
			{
				//if ( (*iremoved)->hasFaceVertex(srcFaceVert) )
				//{
					destFaceVert = (*iremoved)->getFaceVertexFromCommon(dest); 
				//}
			}
			
			assert(destFaceVert);

#if OGRE_DEBUG_MODE 
			ofdebug << "Replacing vertex on face " << (unsigned int)(*f)->index << std::endl;
#endif
            (*f)->replaceVertex(srcFaceVert, destFaceVert);
		}
		// Remove all the faces queued for removal
	    for(f = faceRemovalList.begin(); f != faceRemovalList.end(); ++f) 
		{
#if OGRE_DEBUG_MODE 
			ofdebug << "Removing face " << (unsigned int)(*f)->index << std::endl;
#endif
			(*f)->notifyRemoved();
		}

        // Notify the vertex that it is gone
        src->notifyRemoved();

        // recompute costs
		std::set<PMVertex*>::iterator irecomp, irecompend;
		irecompend = recomputeSet.end();
		for (irecomp = recomputeSet.begin(); irecomp != irecompend; ++irecomp)
		{
			temp = (*irecomp);
			computeEdgeCostAtVertex( (*irecomp)->index );
		}
		
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::computeEdgeCostAtVertex(size_t vertIndex)
    {
		// Call computer for each buffer on this vertex
        Real worstCost = -0.01f;
        WorkingDataList::iterator i, iend;
        iend = mWorkingData.end();
        for (i = mWorkingData.begin(); i != iend; ++i)
        {
            worstCost = std::max(worstCost, 
                computeEdgeCostAtVertexForBuffer(i, vertIndex));
        }
        // Save the worst cost
        mWorstCosts[vertIndex] = worstCost;
    }
    //---------------------------------------------------------------------
    size_t ProgressiveMesh::getNextCollapser(void)
    {
        // Scan
        // Not done as a sort because want to keep the lookup simple for now
        Real bestVal = NEVER_COLLAPSE_COST;
        size_t i, bestIndex;
		bestIndex = 0; // NB this is ok since if nothing is better than this, nothing will collapse
        for (i = 0; i < mNumCommonVertices; ++i)
        {
            if (mWorstCosts[i] < bestVal)
            {
                bestVal = mWorstCosts[i];
                bestIndex = i;
            }
        }
        return bestIndex;
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::bakeNewLOD(IndexData* pData)
    {
        assert(mCurrNumIndexes > 0 && "No triangles to bake!");
        // Zip through the tri list of any working data copy and bake
        pData->indexCount = mCurrNumIndexes;
		pData->indexStart = 0;
		// Base size of indexes on original 
		bool use32bitindexes = 
			(mpIndexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT);

		// Create index buffer, we don't need to read it back or modify it a lot
		pData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
			use32bitindexes? HardwareIndexBuffer::IT_32BIT : HardwareIndexBuffer::IT_16BIT,
			pData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

        unsigned short* pShort;
		unsigned int* pInt;
		if (use32bitindexes)
		{
			pInt = static_cast<unsigned int*>(
				pData->indexBuffer->lock( 0,
					pData->indexBuffer->getSizeInBytes(),
					HardwareBuffer::HBL_DISCARD));
		}
		else
		{
			pShort = static_cast<unsigned short*>(
				pData->indexBuffer->lock( 0,
					pData->indexBuffer->getSizeInBytes(),
					HardwareBuffer::HBL_DISCARD));
		}
        TriangleList::iterator tri, triend;
        // Use the first working data buffer, they are all the same index-wise
        WorkingDataList::iterator pWork = mWorkingData.begin();
        triend = pWork->mTriList.end();
        for (tri = pWork->mTriList.begin(); tri != triend; ++tri)
        {
            if (!tri->removed)
            {
				if (use32bitindexes)
				{
					*pInt++ = static_cast<unsigned int>(tri->vertex[0]->realIndex);
					*pInt++ = static_cast<unsigned int>(tri->vertex[1]->realIndex);
					*pInt++ = static_cast<unsigned int>(tri->vertex[2]->realIndex);
				}
				else
				{
					*pShort++ = static_cast<unsigned short>(tri->vertex[0]->realIndex);
					*pShort++ = static_cast<unsigned short>(tri->vertex[1]->realIndex);
					*pShort++ = static_cast<unsigned short>(tri->vertex[2]->realIndex);
				}
            }
        }
		pData->indexBuffer->unlock();

    }
    //---------------------------------------------------------------------
    ProgressiveMesh::PMTriangle::PMTriangle() : removed(false)
    {
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMTriangle::setDetails(size_t newindex, 
		ProgressiveMesh::PMFaceVertex *v0, ProgressiveMesh::PMFaceVertex *v1, 
        ProgressiveMesh::PMFaceVertex *v2)
    {
        assert(v0!=v1 && v1!=v2 && v2!=v0);

        index = newindex;
		vertex[0]=v0;
        vertex[1]=v1;
        vertex[2]=v2;

        computeNormal();

        // Add tri to vertices
        // Also tell vertices they are neighbours
        for(int i=0;i<3;i++) {
            vertex[i]->commonVertex->face.insert(this);
            for(int j=0;j<3;j++) if(i!=j) {
                vertex[i]->commonVertex->neighbor.insert(vertex[j]->commonVertex);
            }
        }
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMTriangle::notifyRemoved(void)
    {
        int i;
        for(i=0; i<3; i++) {
            // remove this tri from the vertices
            if(vertex[i]) vertex[i]->commonVertex->face.erase(this);
        }
        for(i=0; i<3; i++) {
            int i2 = (i+1)%3;
            if(!vertex[i] || !vertex[i2]) continue;
            // Check remaining vertices and remove if not neighbours anymore
            // NB May remain neighbours if other tris link them
            vertex[i ]->commonVertex->removeIfNonNeighbor(vertex[i2]->commonVertex);
            vertex[i2]->commonVertex->removeIfNonNeighbor(vertex[i ]->commonVertex);
        }

        removed = true;
    }
    //---------------------------------------------------------------------
    bool ProgressiveMesh::PMTriangle::hasCommonVertex(ProgressiveMesh::PMVertex *v) const
    {
        return (v == vertex[0]->commonVertex ||
			v == vertex[1]->commonVertex || 
			v == vertex[2]->commonVertex);
    }
    //---------------------------------------------------------------------
	bool ProgressiveMesh::PMTriangle::hasFaceVertex(ProgressiveMesh::PMFaceVertex *v) const
	{
		return (v == vertex[0] ||
				v == vertex[1] || 
				v == vertex[2]);
	}
    //---------------------------------------------------------------------
	ProgressiveMesh::PMFaceVertex* 
	ProgressiveMesh::PMTriangle::getFaceVertexFromCommon(ProgressiveMesh::PMVertex* commonVert)
	{
		if (vertex[0]->commonVertex == commonVert) return vertex[0];
		if (vertex[1]->commonVertex == commonVert) return vertex[1];
		if (vertex[2]->commonVertex == commonVert) return vertex[2];

		return NULL;

	}
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMTriangle::computeNormal()
    {
        Vector3 v0=vertex[0]->commonVertex->position;
        Vector3 v1=vertex[1]->commonVertex->position;
        Vector3 v2=vertex[2]->commonVertex->position;
        // Cross-product 2 edges
        Vector3 e1 = v1 - v0; 
        Vector3 e2 = v2 - v1;

        normal = e1.crossProduct(e2);
        normal.normalise();
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMTriangle::replaceVertex(
		ProgressiveMesh::PMFaceVertex *vold, ProgressiveMesh::PMFaceVertex *vnew) 
    {
        assert(vold && vnew);
        assert(vold==vertex[0] || vold==vertex[1] || vold==vertex[2]);
        assert(vnew!=vertex[0] && vnew!=vertex[1] && vnew!=vertex[2]);
        if(vold==vertex[0]){
            vertex[0]=vnew;
        }
        else if(vold==vertex[1]){
            vertex[1]=vnew;
        }
        else {
            assert(vold==vertex[2]);
            vertex[2]=vnew;
        }
        int i;
        vold->commonVertex->face.erase(this);
        vnew->commonVertex->face.insert(this);
        for(i=0;i<3;i++) {
            vold->commonVertex->removeIfNonNeighbor(vertex[i]->commonVertex);
            vertex[i]->commonVertex->removeIfNonNeighbor(vold->commonVertex);
        }
        for(i=0;i<3;i++) {
            assert(vertex[i]->commonVertex->face.find(this) != vertex[i]->commonVertex->face.end());
            for(int j=0;j<3;j++) if(i!=j) {
#if OGRE_DEBUG_MODE 
				ofdebug << "Adding vertex " << (unsigned int)vertex[j]->commonVertex->index << " to the neighbor list "
					"of vertex " << (unsigned int)vertex[i]->commonVertex->index << std::endl;
#endif 
                vertex[i]->commonVertex->neighbor.insert(vertex[j]->commonVertex);
            }
        }
        computeNormal();
    }
    //---------------------------------------------------------------------
    ProgressiveMesh::PMVertex::PMVertex() : removed(false)
    {
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMVertex::setDetails(const Vector3& v, size_t newindex)
    {
        position = v;
        index = newindex;
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMVertex::notifyRemoved(void)
    {
        NeighborList::iterator i, iend;
        iend = neighbor.end();
        for (i = neighbor.begin(); i != iend; ++i)
        {
            // Remove me from neighbor
            (*i)->neighbor.erase(this);
        }
        removed = true;
		this->collapseTo = NULL;
        this->collapseCost = NEVER_COLLAPSE_COST;
    }
    //---------------------------------------------------------------------
    bool ProgressiveMesh::PMVertex::isBorder() 
    {
        // Look for edges which only have one tri attached, this is a border

        NeighborList::iterator i, iend;
        iend = neighbor.end();
        // Loop for each neighbor
        for(i = neighbor.begin(); i != iend; ++i) 
        {
            // Count of tris shared between the edge between this and neighbor
            ushort count = 0;
            // Loop over each face, looking for shared ones
            FaceList::iterator j, jend;
            jend = face.end();
            for(j = face.begin(); j != jend; ++j) 
            {
                if((*j)->hasCommonVertex(*i))
                {
                    // Shared tri
                    count ++;
                }
            }
            //assert(count>0); // Must be at least one!
            // This edge has only 1 tri on it, it's a border
            if(count == 1) 
				return true;
        }
        return false;
    } 
	//---------------------------------------------------------------------
	bool ProgressiveMesh::PMVertex::isManifoldEdgeWith(ProgressiveMesh::PMVertex* v)
	{
		// Check the sides involving both these verts
		// If there is only 1 this is a manifold edge
		ushort sidesCount = 0;
		FaceList::iterator i, iend;
		iend = face.end();
		for (i = face.begin(); i != iend; ++i)
		{
			if ((*i)->hasCommonVertex(v))
			{
				sidesCount++;
			}
		}

		return (sidesCount == 1);
	}
	//---------------------------------------------------------------------
    void ProgressiveMesh::PMVertex::removeIfNonNeighbor(ProgressiveMesh::PMVertex *n) 
    {
        // removes n from neighbor list if n isn't a neighbor.
        NeighborList::iterator i = neighbor.find(n);
        if (i == neighbor.end())
            return; // Not in neighbor list anyway

        FaceList::iterator f, fend;
        fend = face.end();
        for(f = face.begin(); f != fend; ++f) 
        {
            if((*f)->hasCommonVertex(n)) return; // Still a neighbor
        }

#if OGRE_DEBUG_MODE 
		ofdebug << "Vertex " << (unsigned int)n->index << " is no longer a neighbour of vertex " << (unsigned int)this->index <<
			" so has been removed from the latter's neighbor list." << std::endl;
#endif
        neighbor.erase(n);

		if (neighbor.empty() && !toBeRemoved)
		{
			// This vertex has been removed through isolation (collapsing around it)
			this->notifyRemoved();
		}
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::dumpContents(const String& log)
	{
		std::ofstream ofdump(log.c_str());

		// Just dump 1st working data for now
		WorkingDataList::iterator worki = mWorkingData.begin();

		CommonVertexList::iterator vi, vend;
		vend = worki->mVertList.end();
		ofdump << "-------== VERTEX LIST ==-----------------" << std::endl;
		size_t i;
		for (vi = worki->mVertList.begin(), i = 0; i < mNumCommonVertices; ++vi, ++i)
		{
			ofdump << "Vertex " << (unsigned int)vi->index << " pos: " << vi->position << " removed: " 
				<< vi->removed << " isborder: " << vi->isBorder() << std::endl;
			ofdump << "    Faces:" << std::endl;
			PMVertex::FaceList::iterator f, fend;
			fend = vi->face.end();
			for(f = vi->face.begin(); f != fend; ++f)
			{
				ofdump << "    Triangle index " << (unsigned int)(*f)->index << std::endl;
			}
			ofdump << "    Neighbours:" << std::endl;
			PMVertex::NeighborList::iterator n, nend;
			nend = vi->neighbor.end();
			for (n = vi->neighbor.begin(); n != nend; ++n)
			{
				ofdump << "    Vertex index " << (unsigned int)(*n)->index << std::endl;
			}

		}

		TriangleList::iterator ti, tend;
		tend = worki->mTriList.end();
		ofdump << "-------== TRIANGLE LIST ==-----------------" << std::endl;
		for(ti = worki->mTriList.begin(); ti != tend; ++ti)
		{
			ofdump << "Triangle " << (unsigned int)ti->index << " norm: " << ti->normal << " removed: " << ti->removed << std::endl;
			ofdump << "    Vertex 0: " << (unsigned int)ti->vertex[0]->realIndex << std::endl;
			ofdump << "    Vertex 1: " << (unsigned int)ti->vertex[1]->realIndex << std::endl;
			ofdump << "    Vertex 2: " << (unsigned int)ti->vertex[2]->realIndex << std::endl;
		}

		ofdump << "-------== COLLAPSE COST LIST ==-----------------" << std::endl;
		for (size_t ci = 0; ci < mNumCommonVertices; ++ci)
		{
			ofdump << "Vertex " << (unsigned int)ci << ": " << mWorstCosts[ci] << std::endl;
		}

		ofdump.close();
	}


}
