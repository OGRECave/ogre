/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreBspSceneManager.h"
#include "OgreBspNode.h"
#include "OgreException.h"
#include "OgreRenderSystem.h"
#include "OgreCamera.h"
#include "OgreMaterial.h"
#include "OgrePatchSurface.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreMath.h"
#include "OgreControllerManager.h"
#include "OgreLogManager.h"
#include "OgreBspSceneNode.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreMaterialManager.h"
#include "OgreCodec.h"
#include "OgreRoot.h"

#include <fstream>

namespace Ogre {

    //-----------------------------------------------------------------------
    BspSceneManager::BspSceneManager(const String& name)
        : SceneManager(name)
    {
        // Set features for debugging render
        mShowNodeAABs = false;

        mLevel.reset();

    }
    //-----------------------------------------------------------------------
    const String& BspSceneManager::getTypeName(void) const
    {
        return BspSceneManagerFactory::FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    BspSceneManager::~BspSceneManager() {}
    //-----------------------------------------------------------------------
    size_t BspSceneManager::estimateWorldGeometry(const String& filename)
    {
        return BspLevel::calculateLoadingStages(filename);
        
    }
    //-----------------------------------------------------------------------
    size_t BspSceneManager::estimateWorldGeometry(DataStreamPtr& stream, 
        const String& typeName)
    {
        return BspLevel::calculateLoadingStages(stream);
        
    }
    //-----------------------------------------------------------------------
    void BspSceneManager::setWorldGeometry(const String& filename)
    {
        auto stream = Root::openFileStream(
            filename, ResourceGroupManager::getSingleton().getWorldResourceGroupName());
        setWorldGeometry(stream);
    }
    //-----------------------------------------------------------------------
    void BspSceneManager::setWorldGeometry(DataStreamPtr& stream, 
        const String& typeName)
    {
        Codec::getCodec("bsp")->decode(stream, getRootSceneNode());
    }

    void BspSceneManager::setLevel(const BspLevelPtr& level)
    {
        mLevel = level;

        if(!mLevel)
            return;

        if (mLevel->isSkyEnabled())
        {
            // Quake3 is always aligned with Z upwards
            Quaternion q;
            q.FromAngleAxis(Radian(Math::HALF_PI), Vector3::UNIT_X);
            // Also draw last, and make close to camera (far clip plane is shorter)
            setSkyDome(true, mLevel->getSkyMaterialName(),
                mLevel->getSkyCurvature(), 12, 2000, false, q);
        }
        else
        {
            setSkyDome(false, BLANKSTRING);
        }
    }
    //-----------------------------------------------------------------------
    void BspSceneManager::_findVisibleObjects(Camera* cam, 
        VisibleObjectsBoundsInfo* visibleBounds, bool onlyShadowCasters)
    {
        // Clear unique list of movables for this frame
        mMovablesForRendering.clear();

        // Assemble an AAB on the fly which contains the scene elements visible
        // by the camera.
        CamVisibleObjectsMap::iterator findIt = mCamVisibleObjectsMap.find( cam );

        // Walk the tree, tag static geometry, return camera's node (for info only)
        // Movables are now added to the render queue in processVisibleLeaf
        walkTree(cam, &(findIt->second), onlyShadowCasters);
    }
    //---------------------------------------------------------------------
    bool BspSceneManager::fireRenderQueueEnded(uint8 id, const String& invocation)
    {
        bool repeat = SceneManager::fireRenderQueueEnded(id, invocation);
        // Trigger level render just after skies
        // can't trigger on mWorldGeometryRenderQueue because we're not queueing there
        if (id == RENDER_QUEUE_SKIES_EARLY)
        {
            renderStaticGeometry();
        }
        return repeat;

    }
    //-----------------------------------------------------------------------
    void BspSceneManager::renderStaticGeometry(void)
    {
        // Check we should be rendering
        if (!isRenderQueueToBeProcessed(getWorldGeometryRenderQueue()))
            return;

        // For each material in turn, cache rendering data & render
        MaterialFaceGroupMap::const_iterator mati;
        for (mati = mMatFaceGroupMap.begin(); mati != mMatFaceGroupMap.end(); ++mati)
        {
            // Get Material
            Material* thisMaterial = mati->first;
            thisMaterial->touch();

            // Skip if no faces to process (we're not doing flare types yet)
            if (!mLevel->cacheGeometry(mati->second))
                continue;

            const Technique::Passes& passes = thisMaterial->getBestTechnique ()->getPasses();
            for (size_t p = 0; p < passes.size(); p++)
            {
                _injectRenderWithPass(passes[p], mLevel.get());
            } 


        } // for each material

        /*
        if (mShowNodeAABs)
        {
            mDestRenderSystem->_render(mAABGeometry);
        }
        */
    }
    //-----------------------------------------------------------------------
    // REMOVE THIS CRAP
    //-----------------------------------------------------------------------
    // Temp debug lines
    bool firstTime = true;
    std::ofstream of;
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    BspNode* BspSceneManager::walkTree(Camera* camera, 
        VisibleObjectsBoundsInfo *visibleBounds, bool onlyShadowCasters)
    {
        if (!mLevel) return 0;

        // Locate the leaf node where the camera is located
        BspNode* cameraNode = mLevel->findLeaf(camera->getDerivedPosition());

        mMatFaceGroupMap.clear();
        mFaceGroupSet.clear();

        // Scan through all the other leaf nodes looking for visibles
        int i = mLevel->mNumNodes - mLevel->mLeafStart;
        BspNode* nd = mLevel->mRootNode + mLevel->mLeafStart;

        /*
        if (firstTime)
        {
            camera->getViewMatrix(); // Force update view before report
            of.open("BspSceneManager.log");
            of << *camera << std::endl;
            of << "Camera Node: " << *cameraNode << std::endl;
            of << "Vertex Data: " << std::endl;
            for (int testi = 0; testi < mLevel->mNumVertices; ++testi)
            {
                of << " " << testi << ": pos(" <<
                  mLevel->mVertices[testi].position[0] << ", " <<
                    mLevel->mVertices[testi].position[1] << ", " << mLevel->mVertices[testi].position[2] << ")" <<
                    " uv(" << mLevel->mVertices[testi].texcoords[0] << ", " << mLevel->mVertices[testi].texcoords[1] << ")" <<
                    " lm(" << mLevel->mVertices[testi].lightmap[0] << ", " << mLevel->mVertices[testi].lightmap[1] << ")" << std::endl;
            }
            of << "Element data:" << std::endl;
            for (testi = 0; testi < mLevel->mNumElements; ++testi)
            {
                of << " " << testi << ": " << mLevel->mElements[testi] << std::endl;

            }
        }
        */

        while (i--)
        {
            if (mLevel->isLeafVisible(cameraNode, nd))
            {

                // Visible according to PVS, check bounding box against frustum
                FrustumPlane plane;
                if (camera->isVisible(nd->getBoundingBox(), &plane))
                {
                    //if (firstTime)
                    //{
                    //    of << "Visible Node: " << *nd << std::endl;
                    //}
                    processVisibleLeaf(nd, camera, visibleBounds, onlyShadowCasters);
                    if (mShowNodeAABs)
                        addBoundingBox(nd->getBoundingBox(), true);
                }
            }
            nd++;
        }


        // TEST
        //if (firstTime)
        //{
        //    of.close();
        //    firstTime = false;
        //}
        return cameraNode;

    }
    //-----------------------------------------------------------------------
    void BspSceneManager::processVisibleLeaf(BspNode* leaf, Camera* cam, 
        VisibleObjectsBoundsInfo* visibleBounds, bool onlyShadowCasters)
    {
        MaterialPtr pMat;
        // Skip world geometry if we're only supposed to process shadow casters
        // World is pre-lit
        if (!onlyShadowCasters)
        {
            // Parse the leaf node's faces, add face groups to material map
            int numGroups = leaf->getNumFaceGroups();
            int idx = leaf->getFaceGroupStart();

            while (numGroups--)
            {
                int realIndex = mLevel->mLeafFaceGroups[idx++];
                // Check not already included
                if (mFaceGroupSet.find(realIndex) != mFaceGroupSet.end())
                    continue;
                StaticFaceGroup* faceGroup = mLevel->mFaceGroups + realIndex;
                // Get Material pointer by handle
                pMat = static_pointer_cast<Material>(MaterialManager::getSingleton().getByHandle(faceGroup->materialHandle));
                assert (pMat);
                // Check normal (manual culling)
                ManualCullingMode cullMode = pMat->getTechnique(0)->getPass(0)->getManualCullingMode();
                if (cullMode != MANUAL_CULL_NONE)
                {
                    Real dist = faceGroup->plane.getDistance(cam->getDerivedPosition());
                    if ( (dist < 0 && cullMode == MANUAL_CULL_BACK) ||
                        (dist > 0 && cullMode == MANUAL_CULL_FRONT) )
                        continue; // skip
                }
                mFaceGroupSet.insert(realIndex);
                // Try to insert, will find existing if already there
                std::pair<MaterialFaceGroupMap::iterator, bool> matgrpi;
                matgrpi = mMatFaceGroupMap.emplace(pMat.get(), std::vector<StaticFaceGroup*>());
                // Whatever happened, matgrpi.first is map iterator
                // Need to get second part of that to get vector
                matgrpi.first->second.push_back(faceGroup);

                //if (firstTime)
                //{
                //    of << "  Emitting faceGroup: index=" << realIndex << ", " << *faceGroup << std::endl;
                //}
            }
        }

        // Add movables to render queue, provided it hasn't been seen already
        const BspNode::IntersectingObjectSet& objects = leaf->getObjects();
        BspNode::IntersectingObjectSet::const_iterator oi, oiend;
        oiend = objects.end();
        for (oi = objects.begin(); oi != oiend; ++oi)
        {
            if (mMovablesForRendering.find(*oi) == mMovablesForRendering.end())
            {
                // It hasn't been seen yet
                MovableObject *mov = const_cast<MovableObject*>(*oi); // hacky

                getRenderQueue()->processVisibleObject(mov, cam, onlyShadowCasters, visibleBounds);

            }
        }


    }
    //-----------------------------------------------------------------------
    void BspSceneManager::showNodeBoxes(bool show)
    {
        mShowNodeAABs = show;
    }
    //-----------------------------------------------------------------------
    void BspSceneManager::addBoundingBox(const AxisAlignedBox& aab, bool visible)
    {
        /*
        unsigned long visibleColour;
        unsigned long nonVisibleColour;
        Root& r = Root::getSingleton();

        r.convertColourValue(ColourValue::White, &visibleColour);
        r.convertColourValue(ColourValue::Blue, &nonVisibleColour);
        if (mShowNodeAABs)
        {
            // Add set of lines
            float* pVertices = (float*)mAABGeometry.pVertices + (mAABGeometry.numVertices*3);
            unsigned short* pIndexes = mAABGeometry.pIndexes + mAABGeometry.numIndexes;
            unsigned long* pColours = (unsigned long*)mAABGeometry.pDiffuseColour + mAABGeometry.numVertices;

            const Vector3* pCorner = aab.getAllCorners();

            int i;
            for (i = 0; i < 8; ++i)
            {
                *pVertices++ = pCorner->x;
                *pVertices++ = pCorner->y;
                *pVertices++ = pCorner->z;
                pCorner++;

                if (visible)
                {
                    *pColours++ = visibleColour;
                }
                else
                {
                    *pColours++ = nonVisibleColour;
                }

            }

            *pIndexes++ = 0 + mAABGeometry.numVertices;
            *pIndexes++ = 1 + mAABGeometry.numVertices;
            *pIndexes++ = 1 + mAABGeometry.numVertices;
            *pIndexes++ = 2 + mAABGeometry.numVertices;
            *pIndexes++ = 2 + mAABGeometry.numVertices;
            *pIndexes++ = 3 + mAABGeometry.numVertices;
            *pIndexes++ = 3 + mAABGeometry.numVertices;
            *pIndexes++ = 1 + mAABGeometry.numVertices;
            *pIndexes++ = 4 + mAABGeometry.numVertices;
            *pIndexes++ = 5 + mAABGeometry.numVertices;
            *pIndexes++ = 5 + mAABGeometry.numVertices;
            *pIndexes++ = 6 + mAABGeometry.numVertices;
            *pIndexes++ = 6 + mAABGeometry.numVertices;
            *pIndexes++ = 7 + mAABGeometry.numVertices;
            *pIndexes++ = 7 + mAABGeometry.numVertices;
            *pIndexes++ = 4 + mAABGeometry.numVertices;
            *pIndexes++ = 1 + mAABGeometry.numVertices;
            *pIndexes++ = 5 + mAABGeometry.numVertices;
            *pIndexes++ = 2 + mAABGeometry.numVertices;
            *pIndexes++ = 4 + mAABGeometry.numVertices;
            *pIndexes++ = 0 + mAABGeometry.numVertices;
            *pIndexes++ = 6 + mAABGeometry.numVertices;
            *pIndexes++ = 3 + mAABGeometry.numVertices;
            *pIndexes++ = 7 + mAABGeometry.numVertices;


            mAABGeometry.numVertices += 8;
            mAABGeometry.numIndexes += 24;


        }
        */

    }
    //-----------------------------------------------------------------------
    ViewPoint BspSceneManager::getSuggestedViewpoint(bool random)
    {
        if (!mLevel || mLevel->mPlayerStarts.empty())
        {
            // No level, use default
            return SceneManager::getSuggestedViewpoint(random);
        }
        else
        {
            if (random)
            {
                size_t idx = (size_t)( Math::UnitRandom() * mLevel->mPlayerStarts.size() );
                return mLevel->mPlayerStarts[idx];
            }
            else
            {
                return mLevel->mPlayerStarts[0];
            }


        }

    }
    //-----------------------------------------------------------------------
    SceneNode * BspSceneManager::createSceneNodeImpl( void )
    {
        return OGRE_NEW BspSceneNode( this );
    }
    //-----------------------------------------------------------------------
    SceneNode * BspSceneManager::createSceneNodeImpl( const String &name )
    {
        return OGRE_NEW BspSceneNode( this, name );
    }
    //-----------------------------------------------------------------------
    void BspSceneManager::_notifyObjectMoved(const MovableObject* mov, 
        const Vector3& pos)
    {
        if (mLevel)
        {
            mLevel->_notifyObjectMoved(mov, pos);
        }
    }
    //-----------------------------------------------------------------------
    void BspSceneManager::_notifyObjectDetached(const MovableObject* mov)
    {
        if (mLevel)
        {
            mLevel->_notifyObjectDetached(mov);
        }
    }
    //-----------------------------------------------------------------------
    void BspSceneManager::clearScene(void)
    {
        SceneManager::clearScene();
        // Clear level
        mLevel.reset();
    }
    //-----------------------------------------------------------------------
    /*
    AxisAlignedBoxSceneQuery* BspSceneManager::
    createAABBQuery(const AxisAlignedBox& box, unsigned long mask)
    {
        // TODO
        return NULL;
    }
    //-----------------------------------------------------------------------
    SphereSceneQuery* BspSceneManager::
    createSphereQuery(const Sphere& sphere, unsigned long mask)
    {
        // TODO
        return NULL;
    }
    */
    //-----------------------------------------------------------------------
    RaySceneQuery* BspSceneManager::
    createRayQuery(const Ray& ray, uint32 mask)
    {
        BspRaySceneQuery* q = OGRE_NEW BspRaySceneQuery(this);
        q->setRay(ray);
        q->setQueryMask(mask);
        return q;
    }
    //-----------------------------------------------------------------------
    IntersectionSceneQuery* BspSceneManager::
    createIntersectionQuery(uint32 mask)
    {
        BspIntersectionSceneQuery* q = OGRE_NEW BspIntersectionSceneQuery(this);
        q->setQueryMask(mask);
        return q;
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    BspIntersectionSceneQuery::BspIntersectionSceneQuery(SceneManager* creator) 
        : DefaultIntersectionSceneQuery(creator), mWorldFragmentType(WFT_NONE)
    {
        mSupportedWorldFragments.insert(WFT_NONE);
        // Add bounds fragment type
        mSupportedWorldFragments.insert(WFT_PLANE_BOUNDED_REGION);
    }
    void BspIntersectionSceneQuery::execute(IntersectionSceneQueryListener* listener)
    {
        /*
        Go through each leaf node in BspLevel and check movables against each other and world
        Issue: some movable-movable intersections could be reported twice if 2 movables
        overlap 2 leaves?
        */
        const BspLevelPtr& lvl = ((BspSceneManager*)mParentSceneMgr)->getLevel();
        if (!lvl) return;

        BspNode* leaf = lvl->getLeafStart();
        int numLeaves = lvl->getNumLeaves();
        
        while (numLeaves--)
        {
            const BspNode::IntersectingObjectSet& objects = leaf->getObjects();
            int numObjects = (int)objects.size();

            BspNode::IntersectingObjectSet::const_iterator a, b, theEnd;
            theEnd = objects.end();
            a = objects.begin();
            for (int oi = 0; oi < numObjects; ++oi, ++a)
            {
                const MovableObject* aObj = *a;
                // Skip this object if collision not enabled
                if (!(aObj->getQueryFlags() & mQueryMask) ||
                    !(aObj->getTypeFlags() & mQueryTypeMask) ||
                    !aObj->isInScene())
                    continue;

                if (oi < (numObjects-1))
                {
                    // Check object against others in this node
                    b = a;
                    for (++b; b != theEnd; ++b)
                    {
                        const MovableObject* bObj = *b;
                        // Apply mask to b (both must pass)
                        if ((bObj->getQueryFlags() & mQueryMask) && 
                            (bObj->getTypeFlags() & mQueryTypeMask) && 
                            bObj->isInScene())
                        {
                            const AxisAlignedBox& box1 = aObj->getWorldBoundingBox();
                            const AxisAlignedBox& box2 = bObj->getWorldBoundingBox();

                            if (box1.intersects(box2))
                            {
                                if (!listener->queryResult(const_cast<MovableObject*>(aObj), 
                                    const_cast<MovableObject*>(bObj)))
                                    return; 
                            }
                        }
                    }
                }
                // Check object against brushes
                if (mQueryTypeMask & SceneManager::WORLD_GEOMETRY_TYPE_MASK)
                {
                    const BspNode::NodeBrushList& brushes = leaf->getSolidBrushes();
                    BspNode::NodeBrushList::const_iterator bi, biend;
                    biend = brushes.end();
                    Real radius = aObj->getBoundingRadius();
                    const Vector3& pos = aObj->getParentNode()->_getDerivedPosition();

                    for (bi = brushes.begin(); bi != biend; ++bi)
                    {
                        std::vector<Plane>::const_iterator planeit, planeitend;
                        planeitend = (*bi)->planes.end();
                        bool brushIntersect = true; // Assume intersecting for now

                        for (planeit = (*bi)->planes.begin(); planeit != planeitend; ++planeit)
                        {
                            Real dist = planeit->getDistance(pos);
                            if (dist > radius)
                            {
                                // Definitely excluded
                                brushIntersect = false;
                                break;
                            }
                        }
                        if (brushIntersect)
                        {
                            // report this brush as it's WorldFragment
                            assert((*bi)->fragment.fragmentType == WFT_PLANE_BOUNDED_REGION);
                            if (!listener->queryResult(const_cast<MovableObject*>(aObj), 
                                    const_cast<WorldFragment*>(&((*bi)->fragment))))
                                return; 
                        }

                    }
                }


            }

            ++leaf;
        }



    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    BspRaySceneQuery::BspRaySceneQuery(SceneManager* creator)
        :DefaultRaySceneQuery(creator), mWorldFragmentType(WFT_NONE)
    {
        // Add supported fragment types
        mSupportedWorldFragments.insert(WFT_NONE);
        mSupportedWorldFragments.insert(WFT_SINGLE_INTERSECTION);
        mSupportedWorldFragments.insert(WFT_PLANE_BOUNDED_REGION);
    }
    //-----------------------------------------------------------------------
    void BspRaySceneQuery::execute(RaySceneQueryListener* listener)
    {
        clearTemporaries();
        BspLevelPtr lvl = static_cast<BspSceneManager*>(mParentSceneMgr)->getLevel();
        if (lvl)
        {
            processNode(
                lvl->getRootNode(), 
                mRay, listener);
        }
    }
    //-----------------------------------------------------------------------
    BspRaySceneQuery::~BspRaySceneQuery()
    {
        clearTemporaries();
    }
    //-----------------------------------------------------------------------
    void BspRaySceneQuery::clearTemporaries(void)
    {
        mObjsThisQuery.clear();
        std::vector<WorldFragment*>::iterator i;
        for (i = mSingleIntersections.begin(); i != mSingleIntersections.end(); ++i)
        {
            OGRE_FREE(*i, MEMCATEGORY_SCENE_CONTROL);
        }
        mSingleIntersections.clear();
    }
    //-----------------------------------------------------------------------
    bool BspRaySceneQuery::processNode(const BspNode* node, const Ray& tracingRay, 
        RaySceneQueryListener* listener, Real maxDistance, Real traceDistance)
    {
        if (node->isLeaf())
        {
            return processLeaf(node, tracingRay, listener, maxDistance, traceDistance);
        }

        bool res = true;
        std::pair<bool, Real> result = tracingRay.intersects(node->getSplitPlane());
        if (result.first && result.second < maxDistance)
        {
            // Crosses the split plane, need to perform 2 queries
            // Calculate split point ray
            Vector3 splitPoint = tracingRay.getOrigin() 
                + tracingRay.getDirection() * result.second;
            Ray splitRay(splitPoint, tracingRay.getDirection());

            if (node->getSide(tracingRay.getOrigin()) == Plane::NEGATIVE_SIDE)
            {
                // Intersects from -ve side, so do back then front
                res = processNode(
                    node->getBack(), tracingRay, listener, result.second, traceDistance);
                if (!res) return res;
                
                res = processNode(
                    node->getFront(), splitRay, listener, 
                    maxDistance - result.second, 
                    traceDistance + result.second);
            }
            else
            {
                // Intersects from +ve side, so do front then back
                res = processNode(node->getFront(), tracingRay, listener, 
                    result.second, traceDistance);
                if (!res) return res;
                res = processNode(node->getBack(), splitRay, listener,
                    maxDistance - result.second, 
                    traceDistance + result.second);
            }
        }
        else
        {
            // Does not cross the splitting plane, just cascade down one side
            res = processNode(node->getNextNode(tracingRay.getOrigin()),
                tracingRay, listener, maxDistance, traceDistance);
        }

        return res;
    }
    //-----------------------------------------------------------------------
    bool BspRaySceneQuery::processLeaf(const BspNode* leaf, const Ray& tracingRay, 
        RaySceneQueryListener* listener, Real maxDistance, Real traceDistance)
    {
        const BspNode::IntersectingObjectSet& objects = leaf->getObjects();

        BspNode::IntersectingObjectSet::const_iterator i, iend;
        iend = objects.end();
        //Check ray against objects
        for(i = objects.begin(); i != iend; ++i)
        {
            // cast away constness, constness of node is nothing to do with objects
            MovableObject* obj = const_cast<MovableObject*>(*i);
            // Skip this object if not enabled
            if(!(obj->getQueryFlags() & mQueryMask) ||
                !((obj->getTypeFlags() & mQueryTypeMask)))
                continue;

            // check we haven't reported this one already
            // (objects can be intersecting more than one node)
            if (mObjsThisQuery.find(obj) != mObjsThisQuery.end())
                continue;

            //Test object as bounding box
            std::pair<bool, Real> result = 
                tracingRay.intersects(obj->getWorldBoundingBox());
            // if the result came back positive and intersection point is inside
            // the node, fire the event handler
            if(result.first && result.second <= maxDistance)
            {
                if (!listener->queryResult(obj, result.second + traceDistance))
                    return false;
            }
        }


        // Check ray against brushes
        if (mQueryTypeMask & SceneManager::WORLD_GEOMETRY_TYPE_MASK)
        {
            const BspNode::NodeBrushList& brushList = leaf->getSolidBrushes();
            BspNode::NodeBrushList::const_iterator bi, biend;
            biend = brushList.end();
            bool intersectedBrush = false;
            for (bi = brushList.begin(); bi != biend; ++bi)
            {
                BspNode::Brush* brush = *bi;
                

                std::pair<bool, Real> result = Math::intersects(tracingRay, brush->planes, true);
                // if the result came back positive and intersection point is inside
                // the node, check if this brush is closer
                if(result.first && result.second <= maxDistance)
                {
                    intersectedBrush = true;
                    if(mWorldFragmentType == WFT_SINGLE_INTERSECTION)
                    {
                        // We're interested in a single intersection
                        // Have to create these 
                        SceneQuery::WorldFragment* wf = OGRE_ALLOC_T(SceneQuery::WorldFragment, 1, MEMCATEGORY_SCENE_CONTROL);
                        wf->fragmentType = WFT_SINGLE_INTERSECTION;
                        wf->singleIntersection = tracingRay.getPoint(result.second);
                        // save this so we can clean up later
                        mSingleIntersections.push_back(wf);
                        if (!listener->queryResult(wf, result.second + traceDistance))
                            return false;
                    }
                    else if (mWorldFragmentType ==  WFT_PLANE_BOUNDED_REGION)
                    {
                        // We want the whole bounded volume
                        assert((*bi)->fragment.fragmentType == WFT_PLANE_BOUNDED_REGION);
                        if (!listener->queryResult(const_cast<WorldFragment*>(&(brush->fragment)), 
                            result.second + traceDistance))
                            return false; 

                    }
                }
            }
            if (intersectedBrush)
            {
                return false; // stop here
            }
        }

        return true;

    } 
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    const String BspSceneManagerFactory::FACTORY_TYPE_NAME = "BspSceneManager";
    //-----------------------------------------------------------------------
    void BspSceneManagerFactory::initMetaData(void) const
    {
        mMetaData.typeName = FACTORY_TYPE_NAME;
        mMetaData.worldGeometrySupported = true;
    }
    //-----------------------------------------------------------------------
    SceneManager* BspSceneManagerFactory::createInstance(
        const String& instanceName)
    {
        return OGRE_NEW BspSceneManager(instanceName);
    }

}

