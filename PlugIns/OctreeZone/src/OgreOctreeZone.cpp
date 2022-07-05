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
OctreeZone.cpp  -  Octree Zone implementation
-----------------------------------------------------------------------------
begin                : Tue Feb 20 2007
author               : Eric Cha
email                : ericc@xenopi.com

-----------------------------------------------------------------------------
*/

#include "OgreOctreeZone.h"
#include "OgreSceneNode.h"
#include "OgreAntiPortal.h"
#include "OgrePortal.h"
#include "OgreEntity.h"
#include "OgreOctreeZoneOctree.h"
#include "OgrePCZCamera.h"
#include "OgrePCZLight.h"
#include "OgrePCZSceneNode.h"
#include "OgrePCZSceneManager.h"

namespace Ogre
{
    OctreeZone::OctreeZone( PCZSceneManager * creator, const String& name ) 
        : PCZone(creator, name)
    {
        mZoneTypeName = "ZoneType_Octree";
        // init octree
        AxisAlignedBox b( -10000, -10000, -10000, 10000, 10000, 10000 );
        int depth = 8; 
        mOctree = 0;
        init( b, depth );
    }

    OctreeZone::~OctreeZone()
    {
        // portals & nodelist are deleted in PCZone destructor.

        // delete octree
        if ( mOctree )
        {
            OGRE_DELETE mOctree;
            mOctree = 0;
        }
    }

    /** Set the enclosure node for this OctreeZone
    */
    void OctreeZone::setEnclosureNode(PCZSceneNode * node)
    {
        mEnclosureNode = node;
        if (node)
        {
            // anchor the node to this zone
            node->anchorToHomeZone(this);
            // make sure node world bounds are up to date
            node->_updateBounds();
            // resize the octree to the same size as the enclosure node bounding box
            resize(node->_getWorldAABB());
        }
    }

    // this call adds the given node to either the zone's list
    // of nodes at home in the zone, or to the list of visiting nodes
    // NOTE: The list is decided by the node's homeZone value, so 
    // that must be set correctly before calling this function.
    void OctreeZone::_addNode( PCZSceneNode * n )
    {
        if (n->getHomeZone() == this)
        {
            // add a reference to this node in the "nodes at home in this zone" list
            mHomeNodeList.insert( n );
        }
        else
        {
            // add a reference to this node in the "nodes visiting this zone" list
            mVisitorNodeList.insert( n );
        }
    }

    void OctreeZone::removeNode( PCZSceneNode * n )
    {
        if ( n != 0 )
        {
            removeNodeFromOctree( n );

            if (n->getHomeZone() == this)
            {
                mHomeNodeList.erase( n );

            }
            else
            {
                mVisitorNodeList.erase( n );
            }
        }
    }

    /** Remove all nodes from the node reference list and clear it
    */
    void OctreeZone::_clearNodeLists(short nodeListTypes)
    {
        if (nodeListTypes & HOME_NODE_LIST)
        {
            PCZSceneNodeList::iterator it = mHomeNodeList.begin();
            while( it != mHomeNodeList.end())
            {
                PCZSceneNode * sn = *it;
                removeNodeFromOctree( sn );
                ++it;
            }
            mHomeNodeList.clear();
        }
        if (nodeListTypes & VISITOR_NODE_LIST)
        {
            PCZSceneNodeList::iterator it = mVisitorNodeList.begin();
            while( it != mVisitorNodeList.end())
            {
                PCZSceneNode * sn = *it;
                removeNodeFromOctree( sn );
                ++it;
            }
            mVisitorNodeList.clear();
        }
    }

    /** Indicates whether or not this zone requires zone-specific data for 
        *  each scene node
        */
    bool OctreeZone::requiresZoneSpecificNodeData(void)
    {
        // Octree Zones have zone specific node data
        return true;
    }

    /** create zone specific data for a node
    */
    void OctreeZone::createNodeZoneData(PCZSceneNode * node)
    {
        OctreeZoneData * ozd = OGRE_NEW OctreeZoneData(node, this);
        if (ozd)
        {
            node->setZoneData(this, ozd);
        }
    }

    /* Recursively check for intersection of the given scene node
     * with zone portals.  If the node touches a portal, then the
     * connected zone is assumed to be touched.  The zone adds
     * the node to its node list and the node adds the zone to 
     * its visiting zone list. 
     *
     * NOTE: This function assumes that the home zone of the node 
     *       is correct.  The function "_updateHomeZone" in PCZSceneManager
     *       takes care of this and should have been called before 
     *       this function.
     */

    void OctreeZone::_checkNodeAgainstPortals(PCZSceneNode * pczsn, Portal * ignorePortal)
    {
        if (pczsn == mEnclosureNode ||
            pczsn->allowedToVisit() == false)
        {
            // don't do any checking of enclosure node versus portals
            return;
        }

        PCZone * connectedZone;
        for ( PortalList::iterator it = mPortals.begin(); it != mPortals.end(); ++it )
        {
            Portal * p = *it;
            //Check if the portal intersects the node
            if (p != ignorePortal && p->intersects(pczsn) != Portal::NO_INTERSECT)
            {
                // node is touching this portal
                connectedZone = p->getTargetZone();
                // add zone to the nodes visiting zone list unless it is the home zone of the node
                if (connectedZone != pczsn->getHomeZone() &&
                    !pczsn->isVisitingZone(connectedZone))
                {
                    pczsn->addZoneToVisitingZonesMap(connectedZone);
                    // tell the connected zone that the node is visiting it
                    connectedZone->_addNode(pczsn);
                    //recurse into the connected zone
                    connectedZone->_checkNodeAgainstPortals(pczsn, p->getTargetPortal());
                }
            }
        }
    }

    /** (recursive) check the given light against all portals in the zone
    * NOTE: This is the default implementation, which doesn't take advantage
    *       of any zone-specific optimizations for checking portal visibility
    */
    void OctreeZone::_checkLightAgainstPortals(PCZLight *light, 
                                               unsigned long frameCount, 
                                               PCZFrustum *portalFrustum,
                                               Portal * ignorePortal)
    {
        for ( PortalList::iterator it = mPortals.begin(); it != mPortals.end(); ++it )
        {
            Portal * p = *it;
            if (p != ignorePortal)
            {
                // calculate the direction vector from light to portal
                Vector3 lightToPortal = p->getDerivedCP() - light->getDerivedPosition();
                if (portalFrustum->isVisible(p))
                {
                    // portal is facing the light, but some light types need to
                    // check illumination radius too.
                    PCZone * targetZone = p->getTargetZone();
                    switch(light->getType())
                    {
                    case Light::LT_POINT:
                        // point lights - just check if within illumination range
                        if (lightToPortal.length() <= light->getAttenuationRange())
                        {
                            // if portal is quad portal it must be pointing towards the light 
                            if ((p->getType() == Portal::PORTAL_TYPE_QUAD && lightToPortal.dotProduct(p->getDerivedDirection()) < 0.0) ||
                                (p->getType() != Portal::PORTAL_TYPE_QUAD))
                            {
                                if (!light->affectsZone(targetZone))
                                {
                                    light->addZoneToAffectedZonesList(targetZone);
                                    if (targetZone->getLastVisibleFrame() == frameCount)
                                    {
                                        light->setAffectsVisibleZone(true);
                                    }
                                    // set culling frustum from the portal
                                    portalFrustum->addPortalCullingPlanes(p);
                                    // recurse into the target zone of the portal
                                    p->getTargetZone()->_checkLightAgainstPortals(light, 
                                                                                frameCount, 
                                                                                portalFrustum,
                                                                                p->getTargetPortal());
                                    // remove the planes added by this portal
                                    portalFrustum->removePortalCullingPlanes(p);
                                }
                            }
                        }
                        break;
                    case Light::LT_DIRECTIONAL:
                        // directionals have infinite range, so just make sure
                        // the direction is facing the portal
                        if (lightToPortal.dotProduct(light->getDerivedDirection()) >= 0.0)
                        {
                            // if portal is quad portal it must be pointing towards the light 
                            if ((p->getType() == Portal::PORTAL_TYPE_QUAD && lightToPortal.dotProduct(p->getDerivedDirection()) < 0.0) ||
                                (p->getType() != Portal::PORTAL_TYPE_QUAD))
                            {
                                if (!light->affectsZone(targetZone))
                                {
                                    light->addZoneToAffectedZonesList(targetZone);
                                    if (targetZone->getLastVisibleFrame() == frameCount)
                                    {
                                        light->setAffectsVisibleZone(true);
                                    }
                                    // set culling frustum from the portal
                                    portalFrustum->addPortalCullingPlanes(p);
                                    // recurse into the target zone of the portal
                                    p->getTargetZone()->_checkLightAgainstPortals(light, 
                                                                                frameCount, 
                                                                                portalFrustum,
                                                                                p->getTargetPortal());
                                    // remove the planes added by this portal
                                    portalFrustum->removePortalCullingPlanes(p);
                                }
                            }
                        }
                        break;
                    case Light::LT_SPOTLIGHT:
                        // spotlights - just check if within illumination range
                        // Technically, we should check if the portal is within
                        // the cone of illumination, but for now, we'll leave that
                        // as a future optimisation.
                        if (lightToPortal.length() <= light->getAttenuationRange())
                        {
                            // if portal is quad portal it must be pointing towards the light 
                            if ((p->getType() == Portal::PORTAL_TYPE_QUAD && lightToPortal.dotProduct(p->getDerivedDirection()) < 0.0) ||
                                (p->getType() != Portal::PORTAL_TYPE_QUAD))
                            {
                                if (!light->affectsZone(targetZone))
                                {
                                    light->addZoneToAffectedZonesList(targetZone);
                                    if (targetZone->getLastVisibleFrame() == frameCount)
                                    {
                                        light->setAffectsVisibleZone(true);
                                    }
                                    // set culling frustum from the portal
                                    portalFrustum->addPortalCullingPlanes(p);
                                    // recurse into the target zone of the portal
                                    p->getTargetZone()->_checkLightAgainstPortals(light, 
                                                                                frameCount, 
                                                                                portalFrustum,
                                                                                p->getTargetPortal());
                                    // remove the planes added by this portal
                                    portalFrustum->removePortalCullingPlanes(p);
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }           
    }

    /** Update the zone data for the portals in the zone
    * NOTE: All portal spatial data must be up-to-date before calling this routine.
    */
    void OctreeZone::updatePortalsZoneData(void)
    {
        PortalList transferPortalList;
        AntiPortalList transferAntiPortalList;
        // check each portal to see if it's intersecting another portal of smaller size
        for ( PortalList::iterator it = mPortals.begin(); it != mPortals.end(); ++it )
        {
            Portal * p = *it;
            bool portalNeedUpdate = p->needUpdate();

            Real pRadius = p->getDerivedRadius();

            // First we check against portals in the SAME zone (and only if they have a 
            // target zone different from the home zone)
            // Here we check only against portals that moved and of smaller size.

            // We do not need to check portal againts previous portals 
            // since it would have been already checked.
            // Hence we start with the next portal after the current portal.
            PortalList::iterator it2 = it;
            for ( ++it2; it2 != mPortals.end(); ++it2 )
            {
                Portal * p2 = (*it2);

                // Skip portal if it doesn't need updating.
                // If both portals are not moving, then there's no need to check between them.
                if (!portalNeedUpdate && !p2->needUpdate()) continue;

                // Skip portal if it's not pointing to another zone.
                if (p2->getTargetZone() == this) continue;

                // Skip portal if it's pointing to the same target zone as this portal points to
                if (p2->getTargetZone() == p->getTargetZone()) continue;

                if (pRadius > p2->getDerivedRadius())
                {
                    // Portal#1 is bigger than Portal#2, check for crossing
                    if (p2->getCurrentHomeZone() != p->getTargetZone() && p2->crossedPortal(p))
                    {
                        // portal#2 crossed portal#1 - flag portal#2 to be moved to portal#1's target zone
                        p2->setNewHomeZone(p->getTargetZone());
                        transferPortalList.push_back(p2);
                    }
                }
                else if (pRadius < p2->getDerivedRadius())
                {
                    // Portal #2 is bigger than Portal #1, check for crossing
                    if (p->getCurrentHomeZone() != p2->getTargetZone() && p->crossedPortal(p2))
                    {
                        // portal#1 crossed portal#2 - flag portal#1 to be moved to portal#2's target zone
                        p->setNewHomeZone(p2->getTargetZone());
                        transferPortalList.push_back(p);
                        continue;
                    }
                }
            }

            // Secondly we check againts the antiportals of this zone.
            for (AntiPortalList::iterator ait = mAntiPortals.begin(); ait != mAntiPortals.end(); ++ait)
            {
                AntiPortal* ap = (*ait);

                // Skip portal if it doesn't need updating.
                // If both portals are not moving, then there's no need to check between them.
                if (!portalNeedUpdate && !ap->needUpdate()) continue;

                // only check for crossing if AntiPortal smaller than portal.
                if (pRadius > ap->getDerivedRadius())
                {
                    // Portal#1 is bigger than AntiPortal, check for crossing
                    if (ap->crossedPortal(p))
                    {
                        // AntiPortal crossed Portal#1 - flag AntiPortal to be moved to Portal#1's target zone
                        ap->setNewHomeZone(p->getTargetZone());
                        transferAntiPortalList.push_back(ap);
                    }
                }
            }

            // Skip portal if it doesn't need updating.
            if (!portalNeedUpdate) continue;

            // Thirdly we check against portals in the target zone (and only if that target
            // zone is different from the home zone)
            PCZone * tzone = p->getTargetZone();
            if (tzone != this)
            {
                for ( PortalList::iterator it3 = tzone->mPortals.begin(); it3 != tzone->mPortals.end(); ++it3 )
                {
                    Portal * p3 = (*it3);
                    // only check against bigger regular portals
                    if (pRadius < p3->getDerivedRadius())
                    {
                        // Portal#3 is bigger than Portal#1, check for crossing
                        if (p->getCurrentHomeZone() != p3->getTargetZone() && p->crossedPortal(p3))
                        {
                            // Portal#1 crossed Portal#3 - switch target zones for Portal#1
                            p->setTargetZone(p3->getTargetZone());
                            break;
                        }
                    }
                }
            }
        }
        // transfer any portals to new zones that have been flagged
        for ( PortalList::iterator it = transferPortalList.begin(); it != transferPortalList.end(); ++it )
        {
            Portal * p = *it;
            if (p->getNewHomeZone() != 0)
            {
                _removePortal(p);
                p->getNewHomeZone()->_addPortal(p);
                p->setNewHomeZone(0);
            }
        }
        // transfer any anti portals to new zones that have been flagged
        for (AntiPortalList::iterator it = transferAntiPortalList.begin(); it != transferAntiPortalList.end(); ++it)
        {
            AntiPortal* p = *it;
            if (p->getNewHomeZone() != 0)
            {
                _removeAntiPortal(p);
                p->getNewHomeZone()->_addAntiPortal(p);
                p->setNewHomeZone(0);
            }
        }
    }

    /** Mark nodes dirty base on moving portals. */
    void OctreeZone::dirtyNodeByMovingPortals(void)
    {
        // Octree zone is a space partitioned zone.
        // Hence we can smartly grab nodes of interest and flag them.
        for ( PortalList::iterator it = mPortals.begin(); it != mPortals.end(); ++it )
        {
            Portal* p = *it;
            if (p->needUpdate())
            {
                PCZSceneNodeList nodeList;
                mOctree->_findNodes(p->getAAB(), nodeList, NULL, true, false);
                PCZSceneNodeList::iterator i = nodeList.begin();
                while ( i != nodeList.end() )
                {
                    (*i)->setMoved(true);
                    ++i;
                }
            }
        }
    }

    /* The following function checks if a node has left it's current home zone.
    * This is done by checking each portal in the zone.  If the node has crossed
    * the portal, then the current zone is no longer the home zone of the node.  The
    * function then recurses into the connected zones.  Once a zone is found where
    * the node does NOT cross out through a portal, that zone is the new home zone.
    NOTE: For this function to work, the node must start out in the proper zone to
          begin with!
    */
    PCZone* OctreeZone::updateNodeHomeZone( PCZSceneNode * pczsn, bool allowBackTouches )
    {
        // default to newHomeZone being the current home zone
        PCZone * newHomeZone = pczsn->getHomeZone();

        // Check all portals of the start zone for crossings!
        Portal* portal;
        PortalList::iterator pi, piend;
        piend = mPortals.end();
        for (pi = mPortals.begin(); pi != piend; pi++)
        {
            portal = *pi;

            Portal::PortalIntersectResult pir = portal->intersects(pczsn);
            switch (pir)
            {
            default:
            case Portal::NO_INTERSECT: // node does not intersect portal - do nothing
            case Portal::INTERSECT_NO_CROSS:// node intersects but does not cross portal - do nothing               
                break;
            case Portal::INTERSECT_BACK_NO_CROSS:// node intersects but on the back of the portal
                if (allowBackTouches)
                {
                    // node is on wrong side of the portal - fix if we're allowing backside touches
                    if (portal->getTargetZone() != this &&
                        portal->getTargetZone() != pczsn->getHomeZone())
                    {
                        // set the home zone of the node to the target zone of the portal
                        pczsn->setHomeZone(portal->getTargetZone());
                        // continue checking for portal crossings in the new zone
                        newHomeZone = portal->getTargetZone()->updateNodeHomeZone(pczsn, false);
                    }
                }
                break;
            case Portal::INTERSECT_CROSS:
                // node intersects and crosses the portal - recurse into that zone as new home zone
                if (portal->getTargetZone() != this &&
                    portal->getTargetZone() != pczsn->getHomeZone())
                {
                    // set the home zone of the node to the target zone of the portal
                    pczsn->setHomeZone(portal->getTargetZone());
                    // continue checking for portal crossings in the new zone
                    newHomeZone = portal->getTargetZone()->updateNodeHomeZone(pczsn, true);
                }
                break;
            }
        }

        // return the new home zone
        return newHomeZone;

    }

    /*
    // Recursively walk the zones, adding all visible SceneNodes to the list of visible nodes.
    */
    void OctreeZone::findVisibleNodes(PCZCamera *camera, 
                                  NodeList & visibleNodeList,
                                  RenderQueue * queue,
                                  VisibleObjectsBoundsInfo* visibleBounds, 
                                  bool onlyShadowCasters,
                                  bool displayNodes,
                                  bool showBoundingBoxes)
    {

        //return immediately if nothing is in the zone.
        if (mHomeNodeList.empty() &&
            mVisitorNodeList.empty() &&
            mPortals.empty())
            return ;

        // Else, the zone is automatically assumed to be visible since either
        // it is the camera the zone is in, or it was reached because
        // a connecting portal was deemed visible to the camera.  

        // enable sky if called to do so for this zone
        if (mHasSky)
        {
            // enable sky 
            mPCZSM->enableSky(true);
        }

        // Recursively find visible nodes in the zone
        walkOctree(camera, 
                   visibleNodeList,
                   queue, 
                   mOctree, 
                   visibleBounds, 
                   false, 
                   onlyShadowCasters,
                   displayNodes,
                   showBoundingBoxes);

        // Here we merge both portal and antiportal visible to the camera into one list.
        // Then we sort them in the order from nearest to furthest from camera.
        PortalBaseList sortedPortalList;
        for (AntiPortalList::iterator iter = mAntiPortals.begin(); iter != mAntiPortals.end(); ++iter)
        {
            AntiPortal* portal = *iter;
            if (camera->isVisible(portal))
            {
                sortedPortalList.push_back(portal);
            }
        }
        for (PortalList::iterator iter = mPortals.begin(); iter != mPortals.end(); ++iter)
        {
            Portal* portal = *iter;
            if (camera->isVisible(portal))
            {
                sortedPortalList.push_back(portal);
            }
        }
        const Vector3& cameraOrigin(camera->getDerivedPosition());
        std::sort(sortedPortalList.begin(), sortedPortalList.end(),
            PortalSortDistance(cameraOrigin));

        // create a standalone frustum for anti portal use.
        // we're doing this instead of using camera because we don't need
        // to do camera frustum check again.
        PCZFrustum antiPortalFrustum;
        antiPortalFrustum.setOrigin(cameraOrigin);
        antiPortalFrustum.setProjectionType(camera->getProjectionType());

        // now we do culling check and remove hidden portals.
        // whenever we get a portal in the main loop, we can be sure that it is not
        // occluded by AntiPortal. So we do traversal right there and then.
        // This is because the portal list has been sorted.
        size_t sortedPortalListCount = sortedPortalList.size();
        for (size_t i = 0; i < sortedPortalListCount; ++i)
        {
            PortalBase* portalBase = sortedPortalList[i];
            if (!portalBase) continue; // skip removed portal.

            if (portalBase->getTypeFlags() == PortalFactory::FACTORY_TYPE_FLAG)
            {
                Portal* portal = static_cast<Portal*>(portalBase);
                // portal is visible. Add the portal as extra culling planes to camera
                int planes_added = camera->addPortalCullingPlanes(portal);
                // tell target zone it's visible this frame
                portal->getTargetZone()->setLastVisibleFrame(mLastVisibleFrame);
                portal->getTargetZone()->setLastVisibleFromCamera(camera);
                // recurse into the connected zone 
                portal->getTargetZone()->findVisibleNodes(camera,
                                                          visibleNodeList,
                                                          queue,
                                                          visibleBounds,
                                                          onlyShadowCasters,
                                                          displayNodes,
                                                          showBoundingBoxes);
                if (planes_added > 0)
                {
                    // Then remove the extra culling planes added before going to the next portal in the list.
                    camera->removePortalCullingPlanes(portal);
                }
            }
            else if (i < sortedPortalListCount) // skip antiportal test if it is the last item in the list.
            {
                // this is an anti portal. So we use it to test preceding portals in the list.
                AntiPortal* antiPortal = static_cast<AntiPortal*>(portalBase);
                int planes_added = antiPortalFrustum.addPortalCullingPlanes(antiPortal);

                for (size_t j = i + 1; j < sortedPortalListCount; ++j)
                {
                    PortalBase* otherPortal = sortedPortalList[j];
                    // Since this is an antiportal, we are doing the inverse of the test.
                    // Here if the portal is fully visible in the anti portal fustrum, it means it's hidden.
                    if (otherPortal && antiPortalFrustum.isFullyVisible(otherPortal))
                        sortedPortalList[j] = NULL;
                }

                if (planes_added > 0)
                {
                    // Then remove the extra culling planes added before going to the next portal in the list.
                    antiPortalFrustum.removePortalCullingPlanes(antiPortal);
                }
            }
        }
    }

    void OctreeZone::walkOctree(PCZCamera *camera, 
                                NodeList & visibleNodeList,
                                RenderQueue *queue, 
                                Octree *octant, 
                                VisibleObjectsBoundsInfo* visibleBounds, 
                                bool foundvisible, 
                                bool onlyShadowCasters,
                                bool displayNodes,
                                bool showBoundingBoxes)
    {

        //return immediately if nothing is in the node.
        if ( octant -> numNodes() == 0 )
            return ;

        PCZCamera::Visibility v = PCZCamera::NONE;

        if ( foundvisible )
        {
            v = PCZCamera::FULL;
        }

        else if ( octant == mOctree )
        {
            v = PCZCamera::PARTIAL;
        }

        else
        {
            AxisAlignedBox box;
            octant -> _getCullBounds( &box );
            v = camera -> getVisibility( box );
        }


        // if the octant is visible, or if it's the root node...
        if ( v != PCZCamera::NONE )
        {
            //Add stuff to be rendered;
            PCZSceneNodeList::iterator it = octant -> mNodes.begin();

            bool vis = true;

            while ( it != octant -> mNodes.end() )
            {
                PCZSceneNode * sn = *it;
                // if the scene node is already visible, then we can skip it
                if (sn->getLastVisibleFrame() != mLastVisibleFrame ||
                    sn->getLastVisibleFromCamera() != camera)
                {
                    // if this octree is partially visible, manually cull all
                    // scene nodes attached directly to this level.
                    if ( v == PCZCamera::PARTIAL )
                    {
                        vis = camera -> isVisible( sn -> _getWorldAABB() );
                    }
                    if ( vis )
                    {
                        // add the node to the render queue
                        sn -> _addToRenderQueue(camera, queue, onlyShadowCasters, visibleBounds );
                        // add it to the list of visible nodes
                        visibleNodeList.push_back( sn );
                        // if we are displaying nodes, add the node renderable to the queue
                        if ( mPCZSM->getDebugDrawer() )
                        {
                            mPCZSM->getDebugDrawer()->drawSceneNode(sn);
                        }
                        // flag the node as being visible this frame
                        sn->setLastVisibleFrame(mLastVisibleFrame);
                        sn->setLastVisibleFromCamera(camera);
                    }
                }
                ++it;
            }

            Octree* child;
            bool childfoundvisible = (v == PCZCamera::FULL);
            if ( (child = octant -> mChildren[ 0 ][ 0 ][ 0 ]) != 0 )
                walkOctree( camera, visibleNodeList, queue, child, visibleBounds, childfoundvisible, onlyShadowCasters, displayNodes, showBoundingBoxes );

            if ( (child = octant -> mChildren[ 1 ][ 0 ][ 0 ]) != 0 )
                walkOctree( camera, visibleNodeList, queue, child, visibleBounds, childfoundvisible, onlyShadowCasters, displayNodes, showBoundingBoxes );

            if ( (child = octant -> mChildren[ 0 ][ 1 ][ 0 ]) != 0 )
                walkOctree( camera, visibleNodeList, queue, child, visibleBounds, childfoundvisible, onlyShadowCasters, displayNodes, showBoundingBoxes );

            if ( (child = octant -> mChildren[ 1 ][ 1 ][ 0 ]) != 0 )
                walkOctree( camera, visibleNodeList, queue, child, visibleBounds, childfoundvisible, onlyShadowCasters, displayNodes, showBoundingBoxes );

            if ( (child = octant -> mChildren[ 0 ][ 0 ][ 1 ]) != 0 )
                walkOctree( camera, visibleNodeList, queue, child, visibleBounds, childfoundvisible, onlyShadowCasters, displayNodes, showBoundingBoxes );

            if ( (child = octant -> mChildren[ 1 ][ 0 ][ 1 ]) != 0 )
                walkOctree( camera, visibleNodeList, queue, child, visibleBounds, childfoundvisible, onlyShadowCasters, displayNodes, showBoundingBoxes );

            if ( (child = octant -> mChildren[ 0 ][ 1 ][ 1 ]) != 0 )
                walkOctree( camera, visibleNodeList, queue, child, visibleBounds, childfoundvisible, onlyShadowCasters, displayNodes, showBoundingBoxes );

            if ( (child = octant -> mChildren[ 1 ][ 1 ][ 1 ]) != 0 )
                walkOctree( camera, visibleNodeList, queue, child, visibleBounds, childfoundvisible, onlyShadowCasters, displayNodes, showBoundingBoxes );

        }
    }

    // --- find nodes which intersect various types of BV's ---

    void OctreeZone::_findNodes(const AxisAlignedBox &t, 
                                PCZSceneNodeList &list, 
                                PortalList &visitedPortals,
                                bool includeVisitors,
                                bool recurseThruPortals,
                                PCZSceneNode *exclude )
    {
        // if this zone has an enclosure, check against the enclosure AABB first
        if (mEnclosureNode)
        {
            if (!mEnclosureNode->_getWorldAABB().intersects(t))
            {
                // AABB of zone does not intersect t, just return.
                return;
            }
        }

        // use the Octree to more efficiently find nodes intersecting the aab
        mOctree->_findNodes(t, list, exclude, includeVisitors, false);

        // if asked to, recurse through portals
        if (recurseThruPortals)
        {
            PortalList::iterator pit = mPortals.begin();
            while ( pit != mPortals.end() )
            {
                Portal * portal = *pit;
                // check portal versus boundign box
                if (portal->intersects(t))
                {
                    // make sure portal hasn't already been recursed through
                    PortalList::iterator pit2 = std::find(visitedPortals.begin(), visitedPortals.end(), portal);
                    if (pit2 == visitedPortals.end())
                    {
                        // save portal to the visitedPortals list
                        visitedPortals.push_front(portal);
                        // recurse into the connected zone 
                        portal->getTargetZone()->_findNodes(t, 
                                                            list, 
                                                            visitedPortals,
                                                            includeVisitors, 
                                                            recurseThruPortals, 
                                                            exclude);
                    }
                }
                pit++;
            }
        }

    }

    void OctreeZone::_findNodes(const Sphere &t, 
                                PCZSceneNodeList &list, 
                                PortalList &visitedPortals,
                                bool includeVisitors,
                                bool recurseThruPortals,
                                PCZSceneNode *exclude )
    {
        // if this zone has an enclosure, check against the enclosure AABB first
        if (mEnclosureNode)
        {
            if (!mEnclosureNode->_getWorldAABB().intersects(t))
            {
                // AABB of zone does not intersect t, just return.
                return;
            }
        }

        // use the Octree to more efficiently find nodes intersecting the sphere
        mOctree->_findNodes(t, list, exclude, includeVisitors, false);

        // if asked to, recurse through portals
        if (recurseThruPortals)
        {
            PortalList::iterator pit = mPortals.begin();
            while ( pit != mPortals.end() )
            {
                Portal * portal = *pit;
                // check portal versus boundign box
                if (portal->intersects(t))
                {
                    // make sure portal hasn't already been recursed through
                    PortalList::iterator pit2 = std::find(visitedPortals.begin(), visitedPortals.end(), portal);
                    if (pit2 == visitedPortals.end())
                    {
                        // save portal to the visitedPortals list
                        visitedPortals.push_front(portal);
                        // recurse into the connected zone 
                        portal->getTargetZone()->_findNodes(t, 
                                                            list, 
                                                            visitedPortals,
                                                            includeVisitors, 
                                                            recurseThruPortals, 
                                                            exclude);
                    }
                }
                pit++;
            }
        }

    }

    void OctreeZone::_findNodes(const PlaneBoundedVolume &t, 
                                PCZSceneNodeList &list, 
                                PortalList &visitedPortals,
                                bool includeVisitors,
                                bool recurseThruPortals,
                                PCZSceneNode *exclude)
    {
        // if this zone has an enclosure, check against the enclosure AABB first
        if (mEnclosureNode)
        {
            if (!t.intersects(mEnclosureNode->_getWorldAABB()))
            {
                // AABB of zone does not intersect t, just return.
                return;
            }
        }

        // use the Octree to more efficiently find nodes intersecting the plane bounded volume
        mOctree->_findNodes(t, list, exclude, includeVisitors, false);

        // if asked to, recurse through portals
        if (recurseThruPortals)
        {
            PortalList::iterator pit = mPortals.begin();
            while ( pit != mPortals.end() )
            {
                Portal * portal = *pit;
                // check portal versus boundign box
                if (portal->intersects(t))
                {
                    // make sure portal hasn't already been recursed through
                    PortalList::iterator pit2 = std::find(visitedPortals.begin(), visitedPortals.end(), portal);
                    if (pit2 == visitedPortals.end())
                    {
                        // save portal to the visitedPortals list
                        visitedPortals.push_front(portal);
                        // recurse into the connected zone 
                        portal->getTargetZone()->_findNodes(t, 
                                                            list, 
                                                            visitedPortals,
                                                            includeVisitors, 
                                                            recurseThruPortals, 
                                                            exclude);
                    }
                }
                pit++;
            }
        }

    }

    void OctreeZone::_findNodes(const Ray &t, 
                                PCZSceneNodeList &list, 
                                PortalList &visitedPortals,
                                bool includeVisitors,
                                bool recurseThruPortals,
                                PCZSceneNode *exclude )
    {
        // if this zone has an enclosure, check against the enclosure AABB first
        if (mEnclosureNode)
        {
            std::pair<bool, Real> nsect = t.intersects(mEnclosureNode->_getWorldAABB());
            if (!nsect.first)
            {
                // AABB of zone does not intersect t, just return.
                return;
            }
        }

        // use the Octree to more efficiently find nodes intersecting the ray
        mOctree->_findNodes(t, list, exclude, includeVisitors, false);

        // if asked to, recurse through portals
        if (recurseThruPortals)
        {
            PortalList::iterator pit = mPortals.begin();
            while ( pit != mPortals.end() )
            {
                Portal * portal = *pit;
                // check portal versus boundign box
                if (portal->intersects(t))
                {
                    // make sure portal hasn't already been recursed through
                    PortalList::iterator pit2 = std::find(visitedPortals.begin(), visitedPortals.end(), portal);
                    if (pit2 == visitedPortals.end())
                    {
                        // save portal to the visitedPortals list
                        visitedPortals.push_front(portal);
                        // recurse into the connected zone 
                        portal->getTargetZone()->_findNodes(t, 
                                                            list, 
                                                            visitedPortals,
                                                            includeVisitors, 
                                                            recurseThruPortals, 
                                                            exclude);
                    }
                }
                pit++;
            }
        }
    }

    /** called when the scene manager creates a camera because
        some zone managers (like TerrainZone) need the camera info.
    */
    void OctreeZone::notifyCameraCreated( Camera* c )
    {
    }
    //-------------------------------------------------------------------------
    void OctreeZone::notifyWorldGeometryRenderQueue(uint8 qid)
    {
    }
    //-------------------------------------------------------------------------
    void OctreeZone::notifyBeginRenderScene(void)
    {
    }
    //-------------------------------------------------------------------------
    void OctreeZone::setZoneGeometry(const String &filename, PCZSceneNode * parentNode)
    {
        String entityName, nodeName;
        entityName = this->getName() + "_entity";
        nodeName = this->getName() + "_Node";
        Entity *ent = mPCZSM->createEntity(entityName , filename );
        // create a node for the entity
        PCZSceneNode * node;
        node = (PCZSceneNode*)(parentNode->createChildSceneNode(nodeName));
        // attach the entity to the node
        node->attachObject(ent);
        // set the node as the enclosure node
        setEnclosureNode(node);
    }
    //-------------------------------------------------------------------------
    void OctreeZone::getAABB(AxisAlignedBox & aabb)
    {
        // get the Octree bounding box
        aabb = mOctree->mBox;
    }
    //-------------------------------------------------------------------------
    void OctreeZone::init(AxisAlignedBox &box, int depth)
    {
        if ( mOctree != 0 )
            OGRE_DELETE mOctree;

        mOctree = OGRE_NEW Octree( this, 0 );

        mMaxDepth = depth;
        mBox = box;

        mOctree -> mBox = box;

        Vector3 min = box.getMinimum();

        Vector3 max = box.getMaximum();

        mOctree -> mHalfSize = ( max - min ) / 2;
    }

    void OctreeZone::resize( const AxisAlignedBox &box )
    {
        // delete the octree
        OGRE_DELETE mOctree;
        // create a new octree
        mOctree = OGRE_NEW Octree( this, 0 );
        // set the octree bounding box 
        mOctree->mBox = box;
        const Vector3 &min = box.getMinimum();
        const Vector3 &max = box.getMaximum();
        mOctree->mHalfSize = ( max - min ) * 0.5f;

        OctreeZoneData * ozd;
        PCZSceneNodeList::iterator it = mHomeNodeList.begin();
        while ( it != mHomeNodeList.end() )
        {
            PCZSceneNode * on = ( *it );
            ozd = (OctreeZoneData*)(on->getZoneData(this));
            ozd -> setOctant( 0 );
            updateNodeOctant( ozd );
            ++it;
        }

        it = mVisitorNodeList.begin();
        while ( it != mVisitorNodeList.end() )
        {
            PCZSceneNode * on = ( *it );
            ozd = (OctreeZoneData*)(on->getZoneData(this));
            ozd -> setOctant( 0 );
            updateNodeOctant( ozd );
            ++it;
        }

    }
    bool OctreeZone::setOption( const String & key, const void * val )
    {
        if ( key == "Size" )
        {
            resize( * static_cast < const AxisAlignedBox * > ( val ) );
            return true;
        }

        else if ( key == "Depth" )
        {
            mMaxDepth = * static_cast < const int * > ( val );
            // copy the box since resize will delete mOctree and reference won't work
            AxisAlignedBox box = mOctree->mBox;
            resize(box);
            return true;
        }

/*      else if ( key == "ShowOctree" )
        {
            mShowBoxes = * static_cast < const bool * > ( val );
            return true;
        }*/
        return false;
    }

    void OctreeZone::updateNodeOctant( OctreeZoneData * zoneData )
    {
        const AxisAlignedBox& box = zoneData -> mOctreeWorldAABB;

        if ( box.isNull() )
            return ;

        // Skip if octree has been destroyed (shutdown conditions)
        if (!mOctree)
            return;

        PCZSceneNode* node = zoneData->mAssociatedNode;
        if ( zoneData->getOctant() == 0 )
        {
            //if outside the octree, force into the root node.
            if ( ! zoneData->_isIn( mOctree -> mBox ) )
                mOctree->_addNode( node  );
            else
                addNodeToOctree( node, mOctree );
            return ;
        }

        if ( ! zoneData->_isIn( zoneData->getOctant()->mBox ) )
        {

            //if outside the octree, force into the root node.
            if ( !zoneData->_isIn( mOctree -> mBox ) )
            {
                // skip if it's already in the root node.
                if (((OctreeZoneData*)node->getZoneData(this))->getOctant() == mOctree)
                    return;

                removeNodeFromOctree( node );
                mOctree->_addNode( node );
            }
            else
                addNodeToOctree( node, mOctree );
        }
    }

    /** Only removes the node from the octree.  It leaves the octree, even if it's empty.
    */
    void OctreeZone::removeNodeFromOctree( PCZSceneNode * n )
    {
        // Skip if octree has been destroyed (shutdown conditions)
        if (!mOctree)
            return;

        Octree * oct = ((OctreeZoneData*)n->getZoneData(this)) -> getOctant();

        if ( oct )
        {
            oct -> _removeNode( n );
        }

        ((OctreeZoneData*)n->getZoneData(this))->setOctant(0);
    }


    void OctreeZone::addNodeToOctree( PCZSceneNode * n, Octree *octant, int depth )
    {

        // Skip if octree has been destroyed (shutdown conditions)
        if (!mOctree)
            return;

        const AxisAlignedBox& bx = n -> _getWorldAABB();


        //if the octree is twice as big as the scene node,
        //we will add it to a child.
        if ( ( depth < mMaxDepth ) && octant -> _isTwiceSize( bx ) )
        {
            int x, y, z;
            octant -> _getChildIndexes( bx, &x, &y, &z );

            if ( octant -> mChildren[ x ][ y ][ z ] == 0 )
            {
                octant -> mChildren[ x ][ y ][ z ] = OGRE_NEW Octree( this, octant );
                const Vector3& octantMin = octant -> mBox.getMinimum();
                const Vector3& octantMax = octant -> mBox.getMaximum();
                Vector3 min, max;

                if ( x == 0 )
                {
                    min.x = octantMin.x;
                    max.x = ( octantMin.x + octantMax.x ) / 2;
                }

                else
                {
                    min.x = ( octantMin.x + octantMax.x ) / 2;
                    max.x = octantMax.x;
                }

                if ( y == 0 )
                {
                    min.y = octantMin.y;
                    max.y = ( octantMin.y + octantMax.y ) / 2;
                }

                else
                {
                    min.y = ( octantMin.y + octantMax.y ) / 2;
                    max.y = octantMax.y;
                }

                if ( z == 0 )
                {
                    min.z = octantMin.z;
                    max.z = ( octantMin.z + octantMax.z ) / 2;
                }

                else
                {
                    min.z = ( octantMin.z + octantMax.z ) / 2;
                    max.z = octantMax.z;
                }

                octant -> mChildren[ x ][ y ][ z ] -> mBox.setExtents( min, max );
                octant -> mChildren[ x ][ y ][ z ] -> mHalfSize = ( max - min ) / 2;
            }

            addNodeToOctree( n, octant -> mChildren[ x ][ y ][ z ], ++depth );

        }
        else
        {
            if (((OctreeZoneData*)n->getZoneData(this))->getOctant() == octant)
                return;

            removeNodeFromOctree( n );
            octant -> _addNode( n );
        }
    }

    /***********************************************************************\
    OctreeZoneData - OctreeZone-specific Data structure for Scene Nodes
    ************************************************************************/

    OctreeZoneData::OctreeZoneData(PCZSceneNode * node, PCZone * zone)
        : ZoneData(node, zone)
    {
        mOctant = 0;
    }

    OctreeZoneData::~OctreeZoneData()
    {
    }

    /* Update the octreezone specific data for a node */
    void OctreeZoneData::update(void)
    {
        mOctreeWorldAABB.setNull();

        // need to use object iterator here.
        for (auto m : mAssociatedNode->getAttachedObjects())
        {
            // merge world bounds of object
            mOctreeWorldAABB.merge( m->getWorldBoundingBox(true) );
        }


        // update the Octant for the node because things might have moved.
        // if it hasn't been added to the octree, add it, and if has moved
        // enough to leave it's current node, we'll update it.
        if ( ! mOctreeWorldAABB.isNull() )
        {
            static_cast < OctreeZone * > ( mAssociatedZone ) -> updateNodeOctant( this );
        }
    }

    /** Since we are loose, only check the center.
    */
    bool OctreeZoneData::_isIn( AxisAlignedBox &box )
    {
        // Always fail if not in the scene graph or box is null
        if (!mAssociatedNode->isInSceneGraph() || box.isNull()) return false;

        // Always succeed if AABB is infinite
        if (box.isInfinite())
            return true;

        Vector3 center = mAssociatedNode->_getWorldAABB().getMaximum().midPoint( mAssociatedNode->_getWorldAABB().getMinimum() );

        Vector3 bmin = box.getMinimum();
        Vector3 bmax = box.getMaximum();

        bool centre = ( bmax > center && bmin < center );
        if (!centre)
            return false;

        // Even if covering the centre line, need to make sure this BB is not large
        // enough to require being moved up into parent. When added, bboxes would
        // end up in parent due to cascade but when updating need to deal with
        // bbox growing too large for this child
        Vector3 octreeSize = bmax - bmin;
        Vector3 nodeSize = mAssociatedNode->_getWorldAABB().getMaximum() - mAssociatedNode->_getWorldAABB().getMinimum();
        return nodeSize < octreeSize;
    }

    //-------------------------------------------------------------------------
    // OctreeZoneFactory functions
    //String octreeZoneString = String("ZoneType_Octree"); 
    OctreeZoneFactory::OctreeZoneFactory() : PCZoneFactory("ZoneType_Octree")
    {
    }
    OctreeZoneFactory::~OctreeZoneFactory()
    {
    }
    bool OctreeZoneFactory::supportsPCZoneType(const String& zoneType)
    {
        if (mFactoryTypeName == zoneType)
        {
            return true;
        }
        return false;
    }
    PCZone* OctreeZoneFactory::createPCZone(PCZSceneManager * pczsm, const String& zoneName)
    {
        return OGRE_NEW OctreeZone(pczsm, zoneName);
    }

}
