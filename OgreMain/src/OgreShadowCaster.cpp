/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#include "OgreStableHeaders.h"
#include "OgreLight.h"
#include "OgreEdgeListBuilder.h"
#include "OgreOptimisedUtil.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"

namespace Ogre {
	const LightList& ShadowRenderable::getLights(void) const 
	{
		// return empty
		static LightList ll;
		return ll;
	}
	// ------------------------------------------------------------------------
	void ShadowCaster::updateEdgeListLightFacing(EdgeData* edgeData, 
		const Vector4& lightPos)
	{
		edgeData->updateTriangleLightFacing(lightPos);
	}
	// ------------------------------------------------------------------------
	void ShadowCaster::generateShadowVolume(EdgeData* edgeData, 
		const HardwareIndexBufferSharedPtr& indexBuffer, size_t& indexBufferUsedSize, 
		const Light* light, ShadowRenderableList& shadowRenderables, unsigned long flags)
	{
		// Edge groups should be 1:1 with shadow renderables
		assert(edgeData->edgeGroups.size() == shadowRenderables.size());

		Light::LightTypes lightType = light->getType();

		// Whether to use the McGuire method, a triangle fan covering all silhouette
		// This won't work properly with multiple separate edge groups (should be one fan per group, not implemented)
        // or when light position is inside light cap bound as extrusion could be in opposite directions
        // and McGuire cap could intersect near clip plane of camera frustum without being noticed.
		bool useMcGuire = edgeData->edgeGroups.size() <= 1 && 
            (lightType == Light::LT_DIRECTIONAL || !getLightCapBounds().contains(light->getDerivedPosition()));
		EdgeData::EdgeGroupList::const_iterator egi, egiend;
		ShadowRenderableList::const_iterator si;

		// pre-count the size of index data we need since it makes a big perf difference
		// to GL in particular if we lock a smaller area of the index buffer
		size_t preCountIndexes = 0;

		si = shadowRenderables.begin();
		egiend = edgeData->edgeGroups.end();
		for (egi = edgeData->edgeGroups.begin(); egi != egiend; ++egi, ++si)
		{
			const EdgeData::EdgeGroup& eg = *egi;
			bool  firstDarkCapTri = true;

			EdgeData::EdgeList::const_iterator i, iend;
			iend = eg.edges.end();
			for (i = eg.edges.begin(); i != iend; ++i)
			{
				const EdgeData::Edge& edge = *i;

				// Silhouette edge, when two tris has opposite light facing, or
				// degenerate edge where only tri 1 is valid and the tri light facing
				char lightFacing = edgeData->triangleLightFacings[edge.triIndex[0]];
				if ((edge.degenerate && lightFacing) ||
					(!edge.degenerate && (lightFacing != edgeData->triangleLightFacings[edge.triIndex[1]])))
				{

					preCountIndexes += 3;

					// Are we extruding to infinity?
					if (!(lightType == Light::LT_DIRECTIONAL &&
						flags & SRF_EXTRUDE_TO_INFINITY))
					{
						preCountIndexes += 3;
					}

					if(useMcGuire)
					{
						// Do dark cap tri
						// Use McGuire et al method, a triangle fan covering all silhouette
						// edges and one point (taken from the initial tri)
						if (flags & SRF_INCLUDE_DARK_CAP)
						{
							if (firstDarkCapTri)
							{
								firstDarkCapTri = false;
							}
							else
							{
								preCountIndexes += 3;
							}
						}
					}
				}

			}

			if(useMcGuire)
			{
				// Do light cap
				if (flags & SRF_INCLUDE_LIGHT_CAP) 
				{
					// Iterate over the triangles which are using this vertex set
					EdgeData::TriangleList::const_iterator ti, tiend;
					EdgeData::TriangleLightFacingList::const_iterator lfi;
					ti = edgeData->triangles.begin() + eg.triStart;
					tiend = ti + eg.triCount;
					lfi = edgeData->triangleLightFacings.begin() + eg.triStart;
					for ( ; ti != tiend; ++ti, ++lfi)
					{
						assert(ti->vertexSet == eg.vertexSet);
						// Check it's light facing
						if (*lfi)
						{
							preCountIndexes += 3;
						}
					}

				}
			}
			else
			{
				// Do both caps
				int increment = ((flags & SRF_INCLUDE_DARK_CAP) ? 3 : 0) + ((flags & SRF_INCLUDE_LIGHT_CAP) ? 3 : 0);
				if(increment != 0)
				{
					// Iterate over the triangles which are using this vertex set
					EdgeData::TriangleList::const_iterator ti, tiend;
					EdgeData::TriangleLightFacingList::const_iterator lfi;
					ti = edgeData->triangles.begin() + eg.triStart;
					tiend = ti + eg.triCount;
					lfi = edgeData->triangleLightFacings.begin() + eg.triStart;
					for ( ; ti != tiend; ++ti, ++lfi)
					{
						assert(ti->vertexSet == eg.vertexSet);
						// Check it's light facing
						if (*lfi)
							preCountIndexes += increment;
					}
				}
			}
		}
		// End pre-count
		
		//Check if index buffer is to small 
		if (preCountIndexes > indexBuffer->getNumIndexes())
		{
			LogManager::getSingleton().logMessage(LML_CRITICAL, 
				String("Warning: shadow index buffer size to small. Auto increasing buffer size to") + 
				StringConverter::toString(sizeof(unsigned short) * preCountIndexes));
			
			SceneManager* pManager = Root::getSingleton()._getCurrentSceneManager();
			if (pManager)
			{
				pManager->setShadowIndexBufferSize(preCountIndexes);
			}
			
			//Check that the index buffer size has actually increased
			if (preCountIndexes > indexBuffer->getNumIndexes())
			{
				//increasing index buffer size has failed
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
					"Lock request out of bounds.",
					"ShadowCaster::generateShadowVolume");
			}
		}
		else if(indexBufferUsedSize + preCountIndexes > indexBuffer->getNumIndexes())
		{
			indexBufferUsedSize = 0;
		}

		// Lock index buffer for writing, just enough length as we need
		unsigned short* pIdx = static_cast<unsigned short*>(
			indexBuffer->lock(sizeof(unsigned short) * indexBufferUsedSize, sizeof(unsigned short) * preCountIndexes,
			indexBufferUsedSize == 0 ? HardwareBuffer::HBL_DISCARD : HardwareBuffer::HBL_NO_OVERWRITE));
		size_t numIndices = indexBufferUsedSize;
		
		// Iterate over the groups and form renderables for each based on their
		// lightFacing
		si = shadowRenderables.begin();
		egiend = edgeData->edgeGroups.end();
		for (egi = edgeData->edgeGroups.begin(); egi != egiend; ++egi, ++si)
		{
			const EdgeData::EdgeGroup& eg = *egi;
			// Initialise the index start for this shadow renderable
			IndexData* indexData = (*si)->getRenderOperationForUpdate()->indexData;

			if (indexData->indexBuffer != indexBuffer)
			{
				(*si)->rebindIndexBuffer(indexBuffer);
				indexData = (*si)->getRenderOperationForUpdate()->indexData;
			}

			indexData->indexStart = numIndices;
			// original number of verts (without extruded copy)
			size_t originalVertexCount = eg.vertexData->vertexCount;
			bool  firstDarkCapTri = true;
			unsigned short darkCapStart = 0;

			EdgeData::EdgeList::const_iterator i, iend;
			iend = eg.edges.end();
			for (i = eg.edges.begin(); i != iend; ++i)
			{
				const EdgeData::Edge& edge = *i;

				// Silhouette edge, when two tris has opposite light facing, or
				// degenerate edge where only tri 1 is valid and the tri light facing
				char lightFacing = edgeData->triangleLightFacings[edge.triIndex[0]];
				if ((edge.degenerate && lightFacing) ||
					(!edge.degenerate && (lightFacing != edgeData->triangleLightFacings[edge.triIndex[1]])))
				{
					size_t v0 = edge.vertIndex[0];
					size_t v1 = edge.vertIndex[1];
					if (!lightFacing)
					{
						// Inverse edge indexes when t1 is light away
						std::swap(v0, v1);
					}

					/* Note edge(v0, v1) run anticlockwise along the edge from
					the light facing tri so to point shadow volume tris outward,
					light cap indexes have to be backwards

					We emit 2 tris if light is a point light, 1 if light 
					is directional, because directional lights cause all
					points to converge to a single point at infinity.

					First side tri = near1, near0, far0
					Second tri = far0, far1, near1

					'far' indexes are 'near' index + originalVertexCount
					because 'far' verts are in the second half of the 
					buffer
					*/
					assert(v1 < 65536 && v0 < 65536 && (v0 + originalVertexCount) < 65536 &&
						"Vertex count exceeds 16-bit index limit!");
					*pIdx++ = static_cast<unsigned short>(v1);
					*pIdx++ = static_cast<unsigned short>(v0);
					*pIdx++ = static_cast<unsigned short>(v0 + originalVertexCount);
					numIndices += 3;

					// Are we extruding to infinity?
					if (!(lightType == Light::LT_DIRECTIONAL &&
						flags & SRF_EXTRUDE_TO_INFINITY))
					{
						// additional tri to make quad
						*pIdx++ = static_cast<unsigned short>(v0 + originalVertexCount);
						*pIdx++ = static_cast<unsigned short>(v1 + originalVertexCount);
						*pIdx++ = static_cast<unsigned short>(v1);
						numIndices += 3;
					}

					if(useMcGuire)
					{
						// Do dark cap tri
						// Use McGuire et al method, a triangle fan covering all silhouette
						// edges and one point (taken from the initial tri)
						if (flags & SRF_INCLUDE_DARK_CAP)
						{
							if (firstDarkCapTri)
							{
								darkCapStart = static_cast<unsigned short>(v0 + originalVertexCount);
								firstDarkCapTri = false;
							}
							else
							{
								*pIdx++ = darkCapStart;
								*pIdx++ = static_cast<unsigned short>(v1 + originalVertexCount);
								*pIdx++ = static_cast<unsigned short>(v0 + originalVertexCount);
								numIndices += 3;
							}

						}
					}
				}

			}

			if(!useMcGuire)
			{
				// Do dark cap
				if (flags & SRF_INCLUDE_DARK_CAP) 
				{
					// Iterate over the triangles which are using this vertex set
					EdgeData::TriangleList::const_iterator ti, tiend;
					EdgeData::TriangleLightFacingList::const_iterator lfi;
					ti = edgeData->triangles.begin() + eg.triStart;
					tiend = ti + eg.triCount;
					lfi = edgeData->triangleLightFacings.begin() + eg.triStart;
					for ( ; ti != tiend; ++ti, ++lfi)
					{
						const EdgeData::Triangle& t = *ti;
						assert(t.vertexSet == eg.vertexSet);
						// Check it's light facing
						if (*lfi)
						{
							assert(t.vertIndex[0] < 65536 && t.vertIndex[1] < 65536 &&
								t.vertIndex[2] < 65536 && 
								"16-bit index limit exceeded!");
							*pIdx++ = static_cast<unsigned short>(t.vertIndex[1] + originalVertexCount);
							*pIdx++ = static_cast<unsigned short>(t.vertIndex[0] + originalVertexCount);
							*pIdx++ = static_cast<unsigned short>(t.vertIndex[2] + originalVertexCount);
							numIndices += 3;
						}
					}

				}
			}

			// Do light cap
			if (flags & SRF_INCLUDE_LIGHT_CAP) 
			{
				// separate light cap?
				if ((*si)->isLightCapSeparate())
				{
					// update index count for this shadow renderable
					indexData->indexCount = numIndices - indexData->indexStart;

					// get light cap index data for update
					indexData = (*si)->getLightCapRenderable()->getRenderOperationForUpdate()->indexData;
					// start indexes after the current total
					indexData->indexStart = numIndices;
				}

				// Iterate over the triangles which are using this vertex set
				EdgeData::TriangleList::const_iterator ti, tiend;
				EdgeData::TriangleLightFacingList::const_iterator lfi;
				ti = edgeData->triangles.begin() + eg.triStart;
				tiend = ti + eg.triCount;
				lfi = edgeData->triangleLightFacings.begin() + eg.triStart;
				for ( ; ti != tiend; ++ti, ++lfi)
				{
					const EdgeData::Triangle& t = *ti;
					assert(t.vertexSet == eg.vertexSet);
					// Check it's light facing
					if (*lfi)
					{
						assert(t.vertIndex[0] < 65536 && t.vertIndex[1] < 65536 &&
							t.vertIndex[2] < 65536 && 
							"16-bit index limit exceeded!");
						*pIdx++ = static_cast<unsigned short>(t.vertIndex[0]);
						*pIdx++ = static_cast<unsigned short>(t.vertIndex[1]);
						*pIdx++ = static_cast<unsigned short>(t.vertIndex[2]);
						numIndices += 3;
					}
				}

			}

			// update index count for current index data (either this shadow renderable or its light cap)
			indexData->indexCount = numIndices - indexData->indexStart;

		}

		// Unlock index buffer
		indexBuffer->unlock();

		// In debug mode, check we didn't overrun the index buffer
		assert(numIndices == indexBufferUsedSize + preCountIndexes);
		assert(numIndices <= indexBuffer->getNumIndexes() &&
			"Index buffer overrun while generating shadow volume!! "
			"You must increase the size of the shadow index buffer.");

		indexBufferUsedSize = numIndices;
	}
	// ------------------------------------------------------------------------
	void ShadowCaster::extrudeVertices(
		const HardwareVertexBufferSharedPtr& vertexBuffer, 
		size_t originalVertexCount, const Vector4& light, Real extrudeDist)
	{
		assert (vertexBuffer->getVertexSize() == sizeof(float) * 3
			&& "Position buffer should contain only positions!");

		// Extrude the first area of the buffer into the second area
		// Lock the entire buffer for writing, even though we'll only be
		// updating the latter because you can't have 2 locks on the same
		// buffer
		float* pSrc = static_cast<float*>(
			vertexBuffer->lock(HardwareBuffer::HBL_NORMAL));

		// TODO: We should add extra (ununsed) vertices ensure source and
		// destination buffer have same alignment for slight performance gain.
		float* pDest = pSrc + originalVertexCount * 3;

		OptimisedUtil::getImplementation()->extrudeVertices(
			light, extrudeDist,
			pSrc, pDest, originalVertexCount);

		vertexBuffer->unlock();

	}
	// ------------------------------------------------------------------------
	void ShadowCaster::extrudeBounds(AxisAlignedBox& box, const Vector4& light, Real extrudeDist) const
	{
		Vector3 extrusionDir;

		if (light.w == 0)
		{
			// Parallel projection guarantees min/max relationship remains the same
			extrusionDir.x = -light.x;
			extrusionDir.y = -light.y;
			extrusionDir.z = -light.z;
			extrusionDir.normalise();
			extrusionDir *= extrudeDist;
			box.setExtents(box.getMinimum() + extrusionDir, 
				box.getMaximum() + extrusionDir);
		}
		else
		{
			Vector3 oldMin, oldMax, currentCorner;
			// Getting the original values
			oldMin = box.getMinimum();
			oldMax = box.getMaximum();
			// Starting the box again with a null content
			box.setNull();

			// merging all the extruded corners

			// 0 : min min min
			currentCorner = oldMin;
			extrusionDir.x = currentCorner.x - light.x;
			extrusionDir.y = currentCorner.y - light.y;
			extrusionDir.z = currentCorner.z - light.z;
			box.merge(currentCorner + extrudeDist * extrusionDir.normalisedCopy());

			// 6 : min min max
			// only z has changed
			currentCorner.z = oldMax.z;
			extrusionDir.z = currentCorner.z - light.z;
			box.merge(currentCorner + extrudeDist * extrusionDir.normalisedCopy());

			// 5 : min max max
			currentCorner.y = oldMax.y;
			extrusionDir.y = currentCorner.y - light.y;
			box.merge(currentCorner + extrudeDist * extrusionDir.normalisedCopy());

			// 1 : min max min
			currentCorner.z = oldMin.z;
			extrusionDir.z = currentCorner.z - light.z;
			box.merge(currentCorner + extrudeDist * extrusionDir.normalisedCopy());

			// 2 : max max min
			currentCorner.x = oldMax.x;
			extrusionDir.x = currentCorner.x - light.x;
			box.merge(currentCorner + extrudeDist * extrusionDir.normalisedCopy());

			// 4 : max max max
			currentCorner.z = oldMax.z;
			extrusionDir.z = currentCorner.z - light.z;
			box.merge(currentCorner + extrudeDist * extrusionDir.normalisedCopy());

			// 7 : max min max
			currentCorner.y = oldMin.y;
			extrusionDir.y = currentCorner.y - light.y;
			box.merge(currentCorner + extrudeDist * extrusionDir.normalisedCopy());

			// 3 : max min min
			currentCorner.z = oldMin.z;
			extrusionDir.z = currentCorner.z - light.z;
			box.merge(currentCorner + extrudeDist * extrusionDir.normalisedCopy());

		}

	}
	// ------------------------------------------------------------------------
	Real ShadowCaster::getExtrusionDistance(const Vector3& objectPos, const Light* light) const
	{
		Vector3 diff = objectPos - light->getDerivedPosition();
		return light->getAttenuationRange() - diff.length();
	}

}
