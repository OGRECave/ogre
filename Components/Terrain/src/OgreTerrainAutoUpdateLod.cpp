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
#include "OgreTerrain.h"
#include "OgreTerrainQuadTreeNode.h"
#include "OgreCamera.h"
#include "OgreSceneManager.h"
#include "OgreViewport.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRay.h"
#include "OgreTerrainAutoUpdateLod.h"

/*
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
// we do lots of conversions here, casting them all is tedious & cluttered, we know what we're doing
#   pragma warning (disable : 4244)
#endif
*/

namespace Ogre
{
    void TerrainAutoUpdateLodByDistance::autoUpdateLod(Terrain *terrain, bool synchronous, const Any &data)
    {
        if( terrain )
            autoUpdateLodByDistance(terrain, synchronous, any_cast<Real>(data));
    }

    void TerrainAutoUpdateLodByDistance::autoUpdateLodByDistance(Terrain *terrain, bool synchronous, const Real holdDistance)
    {
        if (!terrain->isLoaded())
            return;

        // calculate error terms
        const Viewport* vp = terrain->getSceneManager()->getCurrentViewport();
        if(!vp)
            return;
        const Camera* cam = vp->getCamera()->getLodCamera();

        // W. de Boer 2000 calculation
        // A = vp_near / abs(vp_top)
        // A = 1 / tan(fovy*0.5)    (== 1 for fovy=45*2)
        Real A = 1.0f / Math::Tan(cam->getFOVy() * 0.5f);
        // T = 2 * maxPixelError / vertRes
        Real maxPixelError = TerrainGlobalOptions::getSingleton().getMaxPixelError() * cam->_getLodBiasInverse();
        Real T = 2.0f * maxPixelError / (Real)vp->getActualHeight();

        // CFactor = A / T
        Real cFactor = A / T;

        int maxLod = traverseTreeByDistance(terrain->getQuadTree(), cam, cFactor, holdDistance);
        if (maxLod >= 0)
            terrain->load(maxLod,synchronous);
    }

    int TerrainAutoUpdateLodByDistance::traverseTreeByDistance(TerrainQuadTreeNode *node,
            const Camera *cam, Real cFactor, const Real holdDistance)
    {
        if (!node->isLeaf())
        {
            int tmp = -1;
            for (int i = 0; i < 4; ++i)
            {
                int ret = traverseTreeByDistance(node->getChild(i), cam, cFactor, holdDistance);
                if (ret != -1)
                {
                    if (tmp == -1 || ret < tmp)
                        tmp = ret;
                }
            }

            if (tmp != -1)
                return tmp;
        }

        Vector3 localPos = cam->getDerivedPosition() - node->getLocalCentre() - node->getTerrain()->getPosition();
        Real dist;
        if (TerrainGlobalOptions::getSingleton().getUseRayBoxDistanceCalculation())
        {
            // Get distance to this terrain node (to closest point of the box)
            // head towards centre of the box (note, box may not cover mLocalCentre because of height)
            Vector3 dir(node->getBoundingBox().getCenter() - localPos);
            dir.normalise();
            Ray ray(localPos, dir);
            std::pair<bool, Real> intersectRes = Math::intersects(ray, node->getBoundingBox());

            // ray will always intersect, we just want the distance
            dist = intersectRes.second;
        }
        else
        {
            // distance to tile centre
            dist = localPos.length();
            // deduct half the radius of the box, assume that on average the
            // worst case is best approximated by this
            dist -= (node->getBoundingRadius() * 0.5f);
        }

        // For each LOD, the distance at which the LOD will transition *downwards*
        // is given by
        // distTransition = maxDelta * cFactor;
        for (uint16 lodLevel = 0; lodLevel < node->getLodCount(); ++lodLevel)
        {
            // If we have no parent, and this is the lowest LOD, we always render
            // this is the 'last resort' so to speak, we always enoucnter this last
            if (lodLevel+1 == node->getLodCount() && !node->getParent())
                return lodLevel + node->getBaseLod();
            else
            {
                // Calculate or reuse transition distance
                Real distTransition;
                if (Math::RealEqual(cFactor, node->getLodLevel(lodLevel)->lastCFactor))
                    distTransition = node->getLodLevel(lodLevel)->lastTransitionDist;
                else
                    distTransition = node->getLodLevel(lodLevel)->maxHeightDelta * cFactor;

                if ((dist - holdDistance) < distTransition)
                    return lodLevel + node->getBaseLod();
            }
        }

        return -1;
    }
}
