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
#include "OgreTerrainGroup.h"
#include "OgreRoot.h"
#include "OgreWorkQueue.h"


namespace Ogre
{
	const uint16 TerrainGroup::WORKQUEUE_LOAD_REQUEST = 1;
	//---------------------------------------------------------------------
	TerrainGroup::TerrainGroup(SceneManager* sm, Terrain::Alignment align, 
		uint16 terrainSize, Real terrainWorldSize)
		: mSceneManager(sm)
		, mAlignment(align)
		, mTerrainSize(terrainSize)
		, mTerrainWorldSize(terrainWorldSize)
		, mFilenamePrefix("terrain")
		, mFilenameExtension("dat")
		, mResourceGroup(ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME)
	{
		mDefaultImportData.terrainAlign = align;
		mDefaultImportData.terrainSize = terrainSize;
		mDefaultImportData.worldSize = terrainWorldSize;
		// by default we delete input data because we copy it, unless user
		// passes us an ImportData where they explicitly don't want it copied
		mDefaultImportData.deleteInputData = true;

		WorkQueue* wq = Root::getSingleton().getWorkQueue();
		mWorkQueueChannel = wq->getChannel("Ogre/TerrainGroup");
		wq->addRequestHandler(mWorkQueueChannel, this);
		wq->addResponseHandler(mWorkQueueChannel, this);

	}
	//---------------------------------------------------------------------
	TerrainGroup::~TerrainGroup()
	{
		removeAllTerrains();

		WorkQueue* wq = Root::getSingleton().getWorkQueue();
		wq->removeRequestHandler(mWorkQueueChannel, this);
		wq->removeResponseHandler(mWorkQueueChannel, this);

	}
	//---------------------------------------------------------------------
	void TerrainGroup::setOrigin(const Vector3& pos)
	{
		if (pos != mOrigin)
		{
			mOrigin = pos;
			for (TerrainSlotMap::iterator i = mTerrainSlots.begin(); i != mTerrainSlots.end(); ++i)
			{
				TerrainSlot* slot = i->second;
				if (slot->instance)
				{
					slot->instance->setPosition(getTerrainSlotPosition(slot->x, slot->y));
				}
			}
		}

	}
	//---------------------------------------------------------------------
	void TerrainGroup::setFilenameConvention(const String& prefix, const String& extension)
	{
		mFilenamePrefix = prefix;
		mFilenameExtension = extension;
	}
	//---------------------------------------------------------------------
	void TerrainGroup::setFilenamePrefix(const String& prefix)
	{
		mFilenamePrefix = prefix;
	}
	//---------------------------------------------------------------------
	void TerrainGroup::setFilenameExtension(const String& extension)
	{
		mFilenameExtension = extension;
	}
	//---------------------------------------------------------------------
	void TerrainGroup::defineTerrain(long x, long y)
	{
		defineTerrain(x, y, generateFilename(x, y));
	}
	//---------------------------------------------------------------------
	void TerrainGroup::defineTerrain(long x, long y, float constantHeight)
	{
		TerrainSlot* slot = getTerrainSlot(x, y, true);

		slot->def.useImportData();

		// Copy all settings, but make sure our primary settings are immutable
		*slot->def.importData = mDefaultImportData;
		slot->def.importData->constantHeight = constantHeight;
		slot->def.importData->terrainAlign = mAlignment;
		slot->def.importData->terrainSize = mTerrainSize;
		slot->def.importData->worldSize = mTerrainWorldSize;
	}
	//---------------------------------------------------------------------
	void TerrainGroup::defineTerrain(long x, long y, const Terrain::ImportData* importData)
	{
		TerrainSlot* slot = getTerrainSlot(x, y, true);

		slot->def.useImportData();

		// Copy all settings, but make sure our primary settings are immutable
		*slot->def.importData = *importData;
		slot->def.importData->terrainAlign = mAlignment;
		slot->def.importData->terrainSize = mTerrainSize;
		slot->def.importData->worldSize = mTerrainWorldSize;

	}
	//---------------------------------------------------------------------
	void TerrainGroup::defineTerrain(long x, long y, const Image* img, 
		const Terrain::LayerInstanceList* layers /*= 0*/)
	{
		TerrainSlot* slot = getTerrainSlot(x, y, true);

		slot->freeInstance();
		slot->def.useImportData();

		*slot->def.importData = mDefaultImportData;

		// Copy all settings, but make sure our primary settings are immutable
		// copy image - this will get deleted by importData
		slot->def.importData->inputImage = OGRE_NEW Image(*img);
		if (layers)
		{
			// copy (held by value)
			slot->def.importData->layerList = *layers;
		}
		slot->def.importData->terrainAlign = mAlignment;
		slot->def.importData->terrainSize = mTerrainSize;
		slot->def.importData->worldSize = mTerrainWorldSize;

	}
	//---------------------------------------------------------------------
	void TerrainGroup::defineTerrain(long x, long y, const float* pFloat /*= 0*/, 
		const Terrain::LayerInstanceList* layers /*= 0*/)
	{
		TerrainSlot* slot = getTerrainSlot(x, y, true);

		slot->freeInstance();
		slot->def.useImportData();

		*slot->def.importData = mDefaultImportData;

		// Copy all settings, but make sure our primary settings are immutable
		if(pFloat)
		{
			// copy data - this will get deleted by importData
			slot->def.importData->inputFloat = OGRE_ALLOC_T(float, mTerrainSize*mTerrainSize, MEMCATEGORY_GEOMETRY);
			memcpy(slot->def.importData->inputFloat, pFloat, sizeof(float) * mTerrainSize*mTerrainSize);
		}
		if (layers)
		{
			// copy (held by value)
			slot->def.importData->layerList = *layers;
		}
		slot->def.importData->terrainAlign = mAlignment;
		slot->def.importData->terrainSize = mTerrainSize;
		slot->def.importData->worldSize = mTerrainWorldSize;

	}
	//---------------------------------------------------------------------
	void TerrainGroup::defineTerrain(long x, long y, const String& filename)
	{
		TerrainSlot* slot = getTerrainSlot(x, y, true);

		slot->freeInstance();

		slot->def.useFilename();
		slot->def.filename = filename;

	}
	//---------------------------------------------------------------------
	void TerrainGroup::loadAllTerrains(bool synchronous /*= false*/)
	{
		// Just a straight iteration - for the numbers involved not worth 
		// keeping a loaded / unloaded list
		for (TerrainSlotMap::iterator i = mTerrainSlots.begin(); i != mTerrainSlots.end(); ++i)
		{
			TerrainSlot* slot = i->second;
			loadTerrainImpl(slot, synchronous);
		}

	}
	//---------------------------------------------------------------------
	void TerrainGroup::saveAllTerrains(bool onlyIfModified, bool replaceFilenames)
	{
		for (TerrainSlotMap::iterator i = mTerrainSlots.begin(); i != mTerrainSlots.end(); ++i)
		{
			TerrainSlot* slot = i->second;
			if (slot->instance)
			{
				Terrain* t = slot->instance;
				if (t->isLoaded() && 
					(!onlyIfModified || t->isModified()))
				{
					// Overwrite the file names?
					if (replaceFilenames)
						slot->def.filename = generateFilename(slot->x, slot->y);

					String filename;
					if (!slot->def.filename.empty())
						filename = slot->def.filename;
					else
						filename = generateFilename(slot->x, slot->y);

					t->save(filename);

				}
			}
			
		}

	}
	//---------------------------------------------------------------------
	void TerrainGroup::loadTerrain(long x, long y, bool synchronous /*= false*/)
	{
		TerrainSlot* slot = getTerrainSlot(x, y, false);
		if (slot)
		{
			loadTerrainImpl(slot, synchronous);
		}

	}
	//---------------------------------------------------------------------
	void TerrainGroup::loadTerrainImpl(TerrainSlot* slot, bool synchronous)
	{
		if (!slot->instance && 
			(!slot->def.filename.empty() || slot->def.importData))
		{
			// Allocate in main thread so no race conditions
			slot->instance = OGRE_NEW Terrain(mSceneManager);
			slot->instance->setResourceGroup(mResourceGroup);

			LoadRequest req;
			req.slot = slot;
			req.origin = this;
			Root::getSingleton().getWorkQueue()->addRequest(
				mWorkQueueChannel, WORKQUEUE_LOAD_REQUEST, 
				Any(req), 0, synchronous);

		}
	}
	//---------------------------------------------------------------------
	void TerrainGroup::unloadTerrain(long x, long y)
	{
		TerrainSlot* slot = getTerrainSlot(x, y, false);
		if (slot)
		{
			slot->freeInstance();
		}


	}
	//---------------------------------------------------------------------
	void TerrainGroup::removeTerrain(long x, long y)
	{
		uint32 key = packIndex(x, y);
		TerrainSlotMap::iterator i = mTerrainSlots.find(key);
		if (i != mTerrainSlots.end())
		{
			OGRE_DELETE i->second;
			mTerrainSlots.erase(i);
		}

	}
	//---------------------------------------------------------------------
	void TerrainGroup::removeAllTerrains()
	{
		for (TerrainSlotMap::iterator i = mTerrainSlots.begin(); i != mTerrainSlots.end(); ++i)
		{
			delete i->second;
		}
		mTerrainSlots.clear();
	}
	//---------------------------------------------------------------------
	TerrainGroup::TerrainSlotDefinition* TerrainGroup::getTerrainDefinition(long x, long y) const
	{
		TerrainSlot* slot = getTerrainSlot(x, y);
		if (slot)
			return &slot->def;
		else
			return 0;

	}
	//---------------------------------------------------------------------
	Terrain* TerrainGroup::getTerrain(long x, long y) const
	{
		TerrainSlot* slot = getTerrainSlot(x, y);
		if (slot)
			return slot->instance;
		else
			return 0;
	}
	//---------------------------------------------------------------------
	float TerrainGroup::getHeightAtWorldPosition(Real x, Real y, Real z, Terrain** ppTerrain /* = 0*/)
	{
		return getHeightAtWorldPosition(Vector3(x, y, z), ppTerrain);

	}
	//---------------------------------------------------------------------
	float TerrainGroup::getHeightAtWorldPosition(const Vector3& pos, Terrain** ppTerrain /*= 0*/)
	{
		long x, y;
		convertWorldPositionToTerrainSlot(pos, &x, &y);
		TerrainSlot* slot = getTerrainSlot(x, y);
		if (slot && slot->instance && slot->instance->isLoaded())
		{
			if (ppTerrain)
				*ppTerrain = slot->instance;
			return slot->instance->getHeightAtWorldPosition(pos);
		}
		else
		{
			if (ppTerrain)
				*ppTerrain = 0;
			return 0;
		}
	}
	//---------------------------------------------------------------------
	TerrainGroup::RayResult TerrainGroup::rayIntersects(const Ray& ray, Real distanceLimit /* = 0*/) const 
	{
		long curr_x, curr_z;
		convertWorldPositionToTerrainSlot(ray.getOrigin(), &curr_x, &curr_z);
		TerrainSlot* slot = getTerrainSlot(curr_x, curr_z);
		Real dist = 0;
		RayResult result(false, 0, Vector3::ZERO);

		Vector3 tmp, localRayDir, centreOrigin, offset;
		// get the middle of the current tile
		convertTerrainSlotToWorldPosition(curr_x, curr_z, &centreOrigin);
		offset = ray.getOrigin() - centreOrigin;
		localRayDir = ray.getDirection();
		switch (getAlignment())
		{
		case Terrain::ALIGN_X_Y:
			std::swap(localRayDir.y, localRayDir.z);
			std::swap(offset.y, offset.z);
			break;
		case Terrain::ALIGN_Y_Z:
			// x = z, z = y, y = -x
			tmp.x = localRayDir.z; 
			tmp.z = localRayDir.y; 
			tmp.y = -localRayDir.x; 
			localRayDir = tmp;
			tmp.x = offset.z; 
			tmp.z = offset.y; 
			tmp.y = -offset.x; 
			offset = tmp;
			break;
		case Terrain::ALIGN_X_Z:
			// already in X/Z but values increase in -Z
			localRayDir.z = -localRayDir.z;
			offset.z = -offset.z;
			break;
		}
		// Normalise the offset  based on the world size of a square, and rebase to the bottom left
		offset /= mTerrainWorldSize;
		offset += 0.5f;
		// this is our counter moving away from the 'current' square
		Vector3 inc(Math::Abs(localRayDir.x), Math::Abs(localRayDir.y), Math::Abs(localRayDir.z));
		long xdir = localRayDir.x > 0.0f ? 1 : -1;
		long zdir = localRayDir.z > 0.0f ? 1 : -1;

		// We're always counting from 0 to 1 regardless of what direction we're heading
		if (xdir < 0)
			offset.x = 1.0f - offset.x;
		if (zdir < 0)
			offset.z = 1.0f - offset.z;

		// find next slot
		bool keepSearching = true;
		int numGaps = 0;
		while(keepSearching)
		{
			if (Math::RealEqual(inc.x, 0.0f) && Math::RealEqual(inc.z, 0.0f))
				keepSearching = false;

			while (!slot && keepSearching)
			{
				++numGaps;
				/// if we don't find any filled slot in 6 traversals, give up
				if (numGaps > 6)
				{
					keepSearching = false;
					break;
				}
				// find next slot
				while (offset.x < 1.0f && offset.z < 1.0f)
					offset += inc;
				if (offset.x >= 1.0f)
				{
					curr_x += xdir;
					offset.x -= 1.0f;
				}
				else if (offset.z >= 1.0f)
				{
					curr_z += zdir;
					offset.z -= 1.0f;
				}
				if (distanceLimit)
				{
					Vector3 worldPos;
					convertTerrainSlotToWorldPosition(curr_x, curr_z, &worldPos);
					if (ray.getOrigin().distance(worldPos) > distanceLimit)
					{
						keepSearching = false;
						break;
					}
				}
				slot = getTerrainSlot(curr_x, curr_z);
			}
			if (slot && slot->instance)
			{
				numGaps = 0;
				// don't cascade into neighbours
				std::pair<bool, Vector3> raypair = slot->instance->rayIntersects(ray, false, distanceLimit);
				if (raypair.first)
				{
					keepSearching = false;
					result.hit = true;
					result.terrain = slot->instance;
					result.position = raypair.second;
					break;
				}
				else
				{
					// not this one, trigger search for another slot
					slot = 0;
				}
			}

		}


		return result;

	}
	//---------------------------------------------------------------------
	void TerrainGroup::boxIntersects(const AxisAlignedBox& box, TerrainList* resultList) const
	{
		resultList->clear();
		// Much simpler test
		for (TerrainSlotMap::const_iterator i = mTerrainSlots.begin(); i != mTerrainSlots.end(); ++i)
		{
			if (i->second->instance && box.intersects(i->second->instance->getWorldAABB()))
				resultList->push_back(i->second->instance);
		}

	}
	//---------------------------------------------------------------------
	void TerrainGroup::sphereIntersects(const Sphere& sphere, TerrainList* resultList) const
	{
		resultList->clear();
		// Much simpler test
		for (TerrainSlotMap::const_iterator i = mTerrainSlots.begin(); i != mTerrainSlots.end(); ++i)
		{
			if (i->second->instance && sphere.intersects(i->second->instance->getWorldAABB()))
				resultList->push_back(i->second->instance);
		}

	}
	//---------------------------------------------------------------------
	void TerrainGroup::convertWorldPositionToTerrainSlot(const Vector3& pos, long *x, long *y) const
	{
		// 0,0 terrain is centred at the origin
		Vector3 terrainPos;
		// convert to standard xy base (z up), make relative to origin
		Terrain::convertWorldToTerrainAxes(mAlignment, pos - mOrigin, &terrainPos);

		Real offset = mTerrainWorldSize * 0.5f;
		terrainPos.x += offset;
		terrainPos.y += offset;

		*x = static_cast<long>(floor(terrainPos.x / mTerrainWorldSize));
		*y = static_cast<long>(floor(terrainPos.y / mTerrainWorldSize));


	}
	//---------------------------------------------------------------------
	void TerrainGroup::convertTerrainSlotToWorldPosition(long x, long y, Vector3* pos) const
	{
		Vector3 terrainPos(x * mTerrainWorldSize, y * mTerrainWorldSize, 0);

		Terrain::convertTerrainToWorldAxes(mAlignment, terrainPos, pos);
		*pos += mOrigin;


	}
	//---------------------------------------------------------------------
	bool TerrainGroup::isDerivedDataUpdateInProgress() const
	{
		for (TerrainSlotMap::const_iterator i = mTerrainSlots.begin(); i != mTerrainSlots.end(); ++i)
		{
			if (i->second->instance && i->second->instance->isDerivedDataUpdateInProgress())
				return true;
		}
		return false;
	}
	//---------------------------------------------------------------------
	bool TerrainGroup::canHandleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ)
	{
		LoadRequest lreq = any_cast<LoadRequest>(req->getData());
		// only deal with own requests
		if (lreq.origin != this)
			return false;
		else
			return true;

	}
	//---------------------------------------------------------------------
	WorkQueue::Response* TerrainGroup::handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ)
	{
		LoadRequest lreq = any_cast<LoadRequest>(req->getData());

		TerrainSlotDefinition& def = lreq.slot->def;
		Terrain* t = lreq.slot->instance;
		assert(t && "Terrain instance should have been constructed in the main thread");
		WorkQueue::Response* response = 0;
		try
		{
			if (!def.filename.empty())
				t->prepare(def.filename);
			else
			{
				assert(def.importData && "No import data or file name");
				t->prepare(*def.importData);
				// if this worked, we can destroy the input data to save space
				def.freeImportData();
			}
			response = OGRE_NEW WorkQueue::Response(req, true, Any());
		}
		catch (Exception& e)
		{
			// oops
			response = OGRE_NEW WorkQueue::Response(req, false, Any(), 
				e.getFullDescription());
		}

		return response;


	}
	//---------------------------------------------------------------------
	bool TerrainGroup::canHandleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ)
	{
		LoadRequest lreq = any_cast<LoadRequest>(res->getRequest()->getData());
		// only deal with own requests
		if (lreq.origin != this)
			return false;
		else
			return true;

	}
	//---------------------------------------------------------------------
	void TerrainGroup::handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ)
	{
		// No response data, just request
		LoadRequest lreq = any_cast<LoadRequest>(res->getRequest()->getData());

		if (res->succeeded())
		{
			TerrainSlot* slot = lreq.slot;
			Terrain* terrain = slot->instance;
			// do final load now we've prepared in the background
			// we must set the position
			terrain->setPosition(getTerrainSlotPosition(slot->x, slot->y));

			terrain->load();

			// hook up with neighbours
			for (int i = -1; i <= 1; ++i)
			{
				for (int j = -1; j <= 1; ++j)
				{
					if (i != 0 || j != 0)
						connectNeighbour(slot, i, j);
				}

			}
		}
		else
		{
			// oh dear
			LogManager::getSingleton().stream(LML_CRITICAL) <<
				"We failed to prepare the terrain at (" << lreq.slot->x << ", " <<
				lreq.slot->y <<") with the error '" << res->getMessages() << "'";
			lreq.slot->freeInstance();
		}

	}
	//---------------------------------------------------------------------
	void TerrainGroup::connectNeighbour(TerrainSlot* slot, long offsetx, long offsety)
	{
		TerrainSlot* neighbourSlot = getTerrainSlot(slot->x + offsetx, slot->y + offsety);
		if (neighbourSlot && neighbourSlot->instance && neighbourSlot->instance->isLoaded())
		{
			// reclaculate if imported
			slot->instance->setNeighbour(Terrain::getNeighbourIndex(offsetx, offsety), neighbourSlot->instance, 
				slot->def.importData != 0);
		}
	}
	//---------------------------------------------------------------------
	uint32 TerrainGroup::packIndex(long x, long y) const
	{
		// Convert to signed 16-bit so sign bit is in bit 15
		int16 xs16 = static_cast<int16>(x);
		int16 ys16 = static_cast<int16>(y);

		// convert to unsigned because we do not want to propagate sign bit to 32-bits
		uint16 x16 = static_cast<uint16>(xs16);
		uint16 y16 = static_cast<uint16>(ys16);

		uint32 key = 0;
		key = (x16 << 16) | y16;

		return key;


	}
	//---------------------------------------------------------------------
	String TerrainGroup::generateFilename(long x, long y) const
	{
		StringUtil::StrStreamType str;
		str << mFilenamePrefix << "_" << 
			std::setw(8) << std::setfill('0') << std::hex << packIndex(x, y) << 
			"." << mFilenameExtension;
		return str.str();
	}
	//---------------------------------------------------------------------
	Vector3 TerrainGroup::getTerrainSlotPosition(long x, long y)
	{
		Vector3 pos;
		convertTerrainSlotToWorldPosition(x, y, &pos);
		return pos;
	}
	//---------------------------------------------------------------------
	TerrainGroup::TerrainSlot* TerrainGroup::getTerrainSlot(long x, long y, bool createIfMissing)
	{
		uint32 key = packIndex(x, y);
		TerrainSlotMap::iterator i = mTerrainSlots.find(key);
		if (i != mTerrainSlots.end())
			return i->second;
		else if (createIfMissing)
		{
			TerrainSlot* slot = OGRE_NEW TerrainSlot(x, y);
			mTerrainSlots[key] = slot;
			return slot;
		}
		return 0;
	}
	//---------------------------------------------------------------------
	TerrainGroup::TerrainSlot* TerrainGroup::getTerrainSlot(long x, long y) const
	{
		uint32 key = packIndex(x, y);
		TerrainSlotMap::const_iterator i = mTerrainSlots.find(key);
		if (i != mTerrainSlots.end())
			return i->second;
		else
			return 0;

	}
	//---------------------------------------------------------------------
	void TerrainGroup::freeTemporaryResources()
	{
		for (TerrainSlotMap::iterator i = mTerrainSlots.begin(); i != mTerrainSlots.end(); ++i)
		{
			if (i->second->instance)
				i->second->instance->freeTemporaryResources();
		}

	}
	//---------------------------------------------------------------------
	void TerrainGroup::update(bool synchronous)
	{
		for (TerrainSlotMap::iterator i = mTerrainSlots.begin(); i != mTerrainSlots.end(); ++i)
		{
			if (i->second->instance)
				i->second->instance->update();
		}

	}
	//---------------------------------------------------------------------
	void TerrainGroup::updateGeometry()
	{
		for (TerrainSlotMap::iterator i = mTerrainSlots.begin(); i != mTerrainSlots.end(); ++i)
		{
			if (i->second->instance)
				i->second->instance->updateGeometry();
		}

	}
	//---------------------------------------------------------------------
	void TerrainGroup::updateDerivedData(bool synchronous, uint8 typeMask)
	{
		for (TerrainSlotMap::iterator i = mTerrainSlots.begin(); i != mTerrainSlots.end(); ++i)
		{
			if (i->second->instance)
				i->second->instance->updateDerivedData();
		}

	}
	//---------------------------------------------------------------------
	TerrainGroup::TerrainIterator TerrainGroup::getTerrainIterator()
	{
		return TerrainIterator(mTerrainSlots.begin(), mTerrainSlots.end());
	}
	//---------------------------------------------------------------------
	TerrainGroup::ConstTerrainIterator TerrainGroup::getTerrainIterator() const
	{
		return ConstTerrainIterator(mTerrainSlots.begin(), mTerrainSlots.end());
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	TerrainGroup::TerrainSlotDefinition::~TerrainSlotDefinition()
	{
		freeImportData();
	}
	//---------------------------------------------------------------------
	void TerrainGroup::TerrainSlotDefinition::freeImportData()
	{
		OGRE_DELETE_T(importData, ImportData, MEMCATEGORY_GEOMETRY);
		importData = 0;
	}
	//---------------------------------------------------------------------
	void TerrainGroup::TerrainSlotDefinition::useImportData()
	{
		filename.clear();
		freeImportData();
		importData = OGRE_NEW_T(Terrain::ImportData, MEMCATEGORY_GEOMETRY);
		// we're going to own all the data in the def
		importData->deleteInputData = true;

	}
	//---------------------------------------------------------------------
	void TerrainGroup::TerrainSlotDefinition::useFilename()
	{
		freeImportData();
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	TerrainGroup::TerrainSlot::~TerrainSlot()
	{
		freeInstance();
	}
	//---------------------------------------------------------------------
	void TerrainGroup::TerrainSlot::freeInstance()
	{
		OGRE_DELETE instance;
		instance = 0;
	}

}

