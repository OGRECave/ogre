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
#include "OgreSceneQuery.h"
#include "OgreException.h"
#include "OgreSceneManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    SceneQuery::SceneQuery(SceneManager* mgr)
        : mParentSceneMgr(mgr), mQueryMask(0xFFFFFFFF), 
		mWorldFragmentType(SceneQuery::WFT_NONE)
    {
		// default type mask to everything except lights & fx (previous behaviour)
		mQueryTypeMask = (0xFFFFFFFF & ~SceneManager::FX_TYPE_MASK) 
			& ~SceneManager::LIGHT_TYPE_MASK;

    }
    //-----------------------------------------------------------------------
    SceneQuery::~SceneQuery()
    {
    }
    //-----------------------------------------------------------------------
    void SceneQuery::setQueryMask(uint32 mask)
    {
        mQueryMask = mask;
    }
    //-----------------------------------------------------------------------
    uint32 SceneQuery::getQueryMask(void) const
    {
        return mQueryMask;
    }
	//-----------------------------------------------------------------------
	void SceneQuery::setQueryTypeMask(uint32 mask)
	{
		mQueryTypeMask = mask;
	}
	//-----------------------------------------------------------------------
	uint32 SceneQuery::getQueryTypeMask(void) const
	{
		return mQueryTypeMask;
	}
    //-----------------------------------------------------------------------
    void SceneQuery::setWorldFragmentType(enum SceneQuery::WorldFragmentType wft)
    {
        // Check supported
        if (mSupportedWorldFragments.find(wft) == mSupportedWorldFragments.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "This world fragment type is not supported.",
                "SceneQuery::setWorldFragmentType");
        }
        mWorldFragmentType = wft;
    }
    //-----------------------------------------------------------------------
    SceneQuery::WorldFragmentType 
    SceneQuery::getWorldFragmentType(void) const
    {
        return mWorldFragmentType;
    }
    //-----------------------------------------------------------------------
    RegionSceneQuery::RegionSceneQuery(SceneManager* mgr)
        :SceneQuery(mgr), mLastResult(NULL)
    {
    }
    //-----------------------------------------------------------------------
    RegionSceneQuery::~RegionSceneQuery()
    {
        clearResults();
    }
    //-----------------------------------------------------------------------
    SceneQueryResult& RegionSceneQuery::getLastResults(void) const
    {
        assert(mLastResult);
        return *mLastResult;
    }
    //-----------------------------------------------------------------------
    void RegionSceneQuery::clearResults(void)
    {
        if (mLastResult)
        {
            OGRE_DELETE mLastResult;
        }
        mLastResult = NULL;
    }
	//---------------------------------------------------------------------
    SceneQueryResult&
    RegionSceneQuery::execute(void)
    {
        clearResults();
        mLastResult = OGRE_NEW SceneQueryResult();
        // Call callback version with self as listener
        execute(this);
        return *mLastResult;
    }
	//---------------------------------------------------------------------
    bool RegionSceneQuery::
        queryResult(MovableObject* obj)
    {
        // Add to internal list
        mLastResult->movables.push_back(obj);
        // Continue
        return true;
    }
	//---------------------------------------------------------------------
    bool RegionSceneQuery::queryResult(SceneQuery::WorldFragment* fragment)
    {
        // Add to internal list
        mLastResult->worldFragments.push_back(fragment);
        // Continue
        return true;
    }
    //-----------------------------------------------------------------------
    AxisAlignedBoxSceneQuery::AxisAlignedBoxSceneQuery(SceneManager* mgr)
        : RegionSceneQuery(mgr)
    {
    }
    //-----------------------------------------------------------------------
    AxisAlignedBoxSceneQuery::~AxisAlignedBoxSceneQuery()
    {
    }
    //-----------------------------------------------------------------------
    void AxisAlignedBoxSceneQuery::setBox(const AxisAlignedBox& box)
    {
        mAABB = box;
    }
    //-----------------------------------------------------------------------
    const AxisAlignedBox& AxisAlignedBoxSceneQuery::getBox(void) const
    {
        return mAABB;
    }
    //-----------------------------------------------------------------------
    SphereSceneQuery::SphereSceneQuery(SceneManager* mgr)
        : RegionSceneQuery(mgr)
    {
    }
    //-----------------------------------------------------------------------
    SphereSceneQuery::~SphereSceneQuery()
    {
    }
    //-----------------------------------------------------------------------
    void SphereSceneQuery::setSphere(const Sphere& sphere)
    {
        mSphere = sphere;
    }
    //-----------------------------------------------------------------------
    const Sphere& SphereSceneQuery::getSphere() const
    {
        return mSphere;
    }

    //-----------------------------------------------------------------------
    PlaneBoundedVolumeListSceneQuery::PlaneBoundedVolumeListSceneQuery(SceneManager* mgr)
        : RegionSceneQuery(mgr)
    {
    }
    //-----------------------------------------------------------------------
    PlaneBoundedVolumeListSceneQuery::~PlaneBoundedVolumeListSceneQuery()
    {
    }
    //-----------------------------------------------------------------------
    void PlaneBoundedVolumeListSceneQuery::setVolumes(const PlaneBoundedVolumeList& volumes)
    {
        mVolumes = volumes;
    }
    //-----------------------------------------------------------------------
    const PlaneBoundedVolumeList& PlaneBoundedVolumeListSceneQuery::getVolumes() const
    {
        return mVolumes;
    }

    //-----------------------------------------------------------------------
    RaySceneQuery::RaySceneQuery(SceneManager* mgr) : SceneQuery(mgr)
    {
        mSortByDistance = false;
        mMaxResults = 0;
    }
    //-----------------------------------------------------------------------
    RaySceneQuery::~RaySceneQuery()
    {
    }
    //-----------------------------------------------------------------------
    void RaySceneQuery::setRay(const Ray& ray)
    {
        mRay = ray;
    }
    //-----------------------------------------------------------------------
    const Ray& RaySceneQuery::getRay(void) const
    {
        return mRay;
    }
    //-----------------------------------------------------------------------
    void RaySceneQuery::setSortByDistance(bool sort, ushort maxresults)
    {
        mSortByDistance = sort;
        mMaxResults = maxresults;
    }
    //-----------------------------------------------------------------------
    bool RaySceneQuery::getSortByDistance(void) const
    {
        return mSortByDistance;
    }
    //-----------------------------------------------------------------------
    ushort RaySceneQuery::getMaxResults(void) const
    {
        return mMaxResults;
    }
    //-----------------------------------------------------------------------
    RaySceneQueryResult& RaySceneQuery::execute(void)
    {
        // Clear without freeing the vector buffer
        mResult.clear();
        
        // Call callback version with self as listener
        this->execute(this);

        if (mSortByDistance)
        {
            if (mMaxResults != 0 && mMaxResults < mResult.size())
            {
                // Partially sort the N smallest elements, discard others
                std::partial_sort(mResult.begin(), mResult.begin()+mMaxResults, mResult.end());
                mResult.resize(mMaxResults);
            }
            else
            {
                // Sort entire result array
                std::sort(mResult.begin(), mResult.end());
            }
        }

        return mResult;
    }
    //-----------------------------------------------------------------------
    RaySceneQueryResult& RaySceneQuery::getLastResults(void)
    {
        return mResult;
    }
    //-----------------------------------------------------------------------
    void RaySceneQuery::clearResults(void)
    {
        // C++ idiom to free vector buffer: swap with empty vector
        RaySceneQueryResult().swap(mResult);
    }
    //-----------------------------------------------------------------------
    bool RaySceneQuery::queryResult(MovableObject* obj, Real distance)
    {
        // Add to internal list
        RaySceneQueryResultEntry dets;
        dets.distance = distance;
        dets.movable = obj;
        dets.worldFragment = NULL;
        mResult.push_back(dets);
        // Continue
        return true;
    }
    //-----------------------------------------------------------------------
    bool RaySceneQuery::queryResult(SceneQuery::WorldFragment* fragment, Real distance)
    {
        // Add to internal list
        RaySceneQueryResultEntry dets;
        dets.distance = distance;
        dets.movable = NULL;
        dets.worldFragment = fragment;
        mResult.push_back(dets);
        // Continue
        return true;
    }
    //-----------------------------------------------------------------------
    /*
    PyramidSceneQuery::PyramidSceneQuery(SceneManager* mgr) : RegionSceneQuery(mgr)
    {
    }
    //-----------------------------------------------------------------------
    PyramidSceneQuery::~PyramidSceneQuery()
    {
    }
    */
    //-----------------------------------------------------------------------
    IntersectionSceneQuery::IntersectionSceneQuery(SceneManager* mgr)
    : SceneQuery(mgr), mLastResult(NULL)
    {
    }
    //-----------------------------------------------------------------------
    IntersectionSceneQuery::~IntersectionSceneQuery()
    {
        clearResults();
    }
    //-----------------------------------------------------------------------
    IntersectionSceneQueryResult& IntersectionSceneQuery::getLastResults(void) const
    {
        assert(mLastResult);
        return *mLastResult;
    }
    //-----------------------------------------------------------------------
    void IntersectionSceneQuery::clearResults(void)
    {
        if (mLastResult)
        {
            OGRE_DELETE mLastResult;
        }
        mLastResult = NULL;
    }
	//---------------------------------------------------------------------
    IntersectionSceneQueryResult&
    IntersectionSceneQuery::execute(void)
    {
        clearResults();
        mLastResult = OGRE_NEW IntersectionSceneQueryResult();
        // Call callback version with self as listener
        execute(this);
        return *mLastResult;
    }
	//---------------------------------------------------------------------
    bool IntersectionSceneQuery::
        queryResult(MovableObject* first, MovableObject* second)
    {
        // Add to internal list
        mLastResult->movables2movables.push_back(
            SceneQueryMovableObjectPair(first, second)
            );
        // Continue
        return true;
    }
	//---------------------------------------------------------------------
    bool IntersectionSceneQuery::
        queryResult(MovableObject* movable, SceneQuery::WorldFragment* fragment)
    {
        // Add to internal list
        mLastResult->movables2world.push_back(
            SceneQueryMovableObjectWorldFragmentPair(movable, fragment)
            );
        // Continue
        return true;
    }




}
    



