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
OgrePCZSceneQuery.cpp  -  Scene Query implementations for PCZSceneManager
-----------------------------------------------------------------------------
begin                : Wed Feb 21, 2007
author               : Eric Cha
email                : ericc@xenopi.com
current TODO's       : none known
-----------------------------------------------------------------------------
*/

#include "OgreRoot.h"

#include "OgrePCZSceneQuery.h"
#include "OgrePCZSceneManager.h"
#include "OgrePCZSceneNode.h"
#include "OgreEntity.h"

namespace Ogre
{

    //---------------------------------------------------------------------
    PCZIntersectionSceneQuery::PCZIntersectionSceneQuery(SceneManager* creator)
            : DefaultIntersectionSceneQuery(creator)
    {

    }
    //---------------------------------------------------------------------
    PCZIntersectionSceneQuery::~PCZIntersectionSceneQuery()
    {}
    //---------------------------------------------------------------------
    void PCZIntersectionSceneQuery::execute(IntersectionSceneQueryListener* listener)
    {
        typedef std::pair<MovableObject *, MovableObject *> MovablePair;
        typedef std::set
            < std::pair<MovableObject *, MovableObject *> > MovableSet;

        MovableSet set;

        // Iterate over all movable types
        Root::MovableObjectFactoryIterator factIt = 
            Root::getSingleton().getMovableObjectFactoryIterator();
        while(factIt.hasMoreElements())
        {
            SceneManager::MovableObjectIterator it = 
                mParentSceneMgr->getMovableObjectIterator(
                factIt.getNext()->getType());
            while( it.hasMoreElements() )
            {

                MovableObject * e = it.getNext();
                PCZone * zone = ((PCZSceneNode*)(e->getParentSceneNode()))->getHomeZone();
                PCZSceneNodeList list;
                //find the nodes that intersect the AAB
                static_cast<PCZSceneManager*>( mParentSceneMgr ) -> findNodesIn( e->getWorldBoundingBox(), list, zone, 0 );
                //grab all moveables from the node that intersect...
                PCZSceneNodeList::iterator nit = list.begin();
                while( nit != list.end() )
                {
                    SceneNode::ObjectIterator oit = (*nit) -> getAttachedObjectIterator();
                    while( oit.hasMoreElements() )
                    {
                        MovableObject * m = oit.getNext();

                        if( m != e &&
                            set.find( MovablePair(e,m)) == set.end() &&
                            set.find( MovablePair(m,e)) == set.end() &&
                            (m->getQueryFlags() & mQueryMask) &&
                            (m->getTypeFlags() & mQueryTypeMask) &&
                            m->isInScene() && 
                            e->getWorldBoundingBox().intersects( m->getWorldBoundingBox() ) )
                        {
                            listener -> queryResult( e, m );
                            // deal with attached objects, since they are not directly attached to nodes
                            if (m->getMovableType() == "Entity")
                            {
                                Entity* e2 = static_cast<Entity*>(m);
                                Entity::ChildObjectListIterator childIt = e2->getAttachedObjectIterator();
                                while(childIt.hasMoreElements())
                                {
                                    MovableObject* c = childIt.getNext();
                                    if (c->getQueryFlags() & mQueryMask && 
                                        e->getWorldBoundingBox().intersects( c->getWorldBoundingBox() ))
                                    {
                                        listener->queryResult(e, c);
                                    }
                                }
                            }
                        }
                        set.insert( MovablePair(e,m) );

                    }
                    ++nit;
                }

            }
        }
    }
    /** Creates a custom PCZ AAB query */
    PCZAxisAlignedBoxSceneQuery::PCZAxisAlignedBoxSceneQuery(SceneManager* creator)
            : DefaultAxisAlignedBoxSceneQuery(creator)
    {
        mStartZone = 0;
        mExcludeNode = 0;
    }
    /** Deletes the custom PCZ query */
    PCZAxisAlignedBoxSceneQuery::~PCZAxisAlignedBoxSceneQuery()
    {}

    /** Finds any entities that intersect the AAB for the query. */
    void PCZAxisAlignedBoxSceneQuery::execute(SceneQueryListener* listener)
    {
        PCZSceneNodeList list;
        //find the nodes that intersect the AAB
        static_cast<PCZSceneManager*>( mParentSceneMgr ) -> findNodesIn( mAABB, list, mStartZone, (PCZSceneNode*)mExcludeNode );

        //grab all moveables from the node that intersect...
        PCZSceneNodeList::iterator it = list.begin();
        while( it != list.end() )
        {
            SceneNode::ObjectIterator oit = (*it) -> getAttachedObjectIterator();
            while( oit.hasMoreElements() )
            {
                MovableObject * m = oit.getNext();
                if( (m->getQueryFlags() & mQueryMask) && 
                    (m->getTypeFlags() & mQueryTypeMask) && 
                    m->isInScene() &&
                    mAABB.intersects( m->getWorldBoundingBox() ) )
                {
                    listener -> queryResult( m );
                    // deal with attached objects, since they are not directly attached to nodes
                    if (m->getMovableType() == "Entity")
                    {
                        Entity* e = static_cast<Entity*>(m);
                        Entity::ChildObjectListIterator childIt = e->getAttachedObjectIterator();
                        while(childIt.hasMoreElements())
                        {
                            MovableObject* c = childIt.getNext();
                            if (c->getQueryFlags() & mQueryMask)
                            {
                                listener->queryResult(c);
                            }
                        }
                    }
                }

            }
            ++it;
        }
        // reset startzone and exclude node
        mStartZone = 0;
        mExcludeNode = 0;
    }
    //---------------------------------------------------------------------
    PCZRaySceneQuery::
    PCZRaySceneQuery(SceneManager* creator) : DefaultRaySceneQuery(creator)
    {
        mStartZone = 0;
        mExcludeNode = 0;
    }
    //---------------------------------------------------------------------
    PCZRaySceneQuery::~PCZRaySceneQuery()
    {}
    //---------------------------------------------------------------------
    void PCZRaySceneQuery::execute(RaySceneQueryListener* listener)
    {
        PCZSceneNodeList list;
        //find the nodes that intersect the Ray
        static_cast<PCZSceneManager*>( mParentSceneMgr ) -> findNodesIn( mRay, list, mStartZone, (PCZSceneNode*)mExcludeNode );

        //grab all moveables from the node that intersect...
        PCZSceneNodeList::iterator it = list.begin();
        while( it != list.end() )
        {
            SceneNode::ObjectIterator oit = (*it) -> getAttachedObjectIterator();
            while( oit.hasMoreElements() )
            {
                MovableObject * m = oit.getNext();
                if( (m->getQueryFlags() & mQueryMask) && 
                    (m->getTypeFlags() & mQueryTypeMask) && m->isInScene() )
                {
                    std::pair<bool, Real> result = mRay.intersects(m->getWorldBoundingBox());

                    if( result.first )
                    {
                        listener -> queryResult( m, result.second );
                        // deal with attached objects, since they are not directly attached to nodes
                        if (m->getMovableType() == "Entity")
                        {
                            Entity* e = static_cast<Entity*>(m);
                            Entity::ChildObjectListIterator childIt = e->getAttachedObjectIterator();
                            while(childIt.hasMoreElements())
                            {
                                MovableObject* c = childIt.getNext();
                                if (c->getQueryFlags() & mQueryMask)
                                {
                                    result = mRay.intersects(c->getWorldBoundingBox());
                                    if (result.first)
                                    {
                                        listener->queryResult(c, result.second);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ++it;
        }
        // reset startzone and exclude node
        mStartZone = 0;
        mExcludeNode = 0;
    }


    //---------------------------------------------------------------------
    PCZSphereSceneQuery::
    PCZSphereSceneQuery(SceneManager* creator) : DefaultSphereSceneQuery(creator)
    {
        mStartZone = 0;
        mExcludeNode = 0;
    }
    //---------------------------------------------------------------------
    PCZSphereSceneQuery::~PCZSphereSceneQuery()
    {}
    //---------------------------------------------------------------------
    void PCZSphereSceneQuery::execute(SceneQueryListener* listener)
    {
        PCZSceneNodeList list;
        //find the nodes that intersect the Sphere
        static_cast<PCZSceneManager*>( mParentSceneMgr ) -> findNodesIn( mSphere, list, mStartZone, (PCZSceneNode*)mExcludeNode );

        //grab all moveables from the node that intersect...
        PCZSceneNodeList::iterator it = list.begin();
        while( it != list.end() )
        {
            SceneNode::ObjectIterator oit = (*it) -> getAttachedObjectIterator();
            while( oit.hasMoreElements() )
            {
                MovableObject * m = oit.getNext();
                if( (m->getQueryFlags() & mQueryMask) && 
                    (m->getTypeFlags() & mQueryTypeMask) && 
                    m->isInScene() && 
                    mSphere.intersects( m->getWorldBoundingBox() ) )
                {
                    listener -> queryResult( m );
                    // deal with attached objects, since they are not directly attached to nodes
                    if (m->getMovableType() == "Entity")
                    {
                        Entity* e = static_cast<Entity*>(m);
                        Entity::ChildObjectListIterator childIt = e->getAttachedObjectIterator();
                        while(childIt.hasMoreElements())
                        {
                            MovableObject* c = childIt.getNext();
                            if (c->getQueryFlags() & mQueryMask &&
                                mSphere.intersects( c->getWorldBoundingBox()))
                            {
                                listener->queryResult(c);
                            }
                        }
                    }
                }
            }
            ++it;
        }
        // reset startzone and exclude node
        mStartZone = 0;
        mExcludeNode = 0;
    }
    //---------------------------------------------------------------------
    PCZPlaneBoundedVolumeListSceneQuery::
    PCZPlaneBoundedVolumeListSceneQuery(SceneManager* creator)
            : DefaultPlaneBoundedVolumeListSceneQuery(creator)
    {
        mStartZone = 0;
        mExcludeNode = 0;
    }
    //---------------------------------------------------------------------
    PCZPlaneBoundedVolumeListSceneQuery::~PCZPlaneBoundedVolumeListSceneQuery()
    {}
    //---------------------------------------------------------------------
    void PCZPlaneBoundedVolumeListSceneQuery::execute(SceneQueryListener* listener)
    {
        set<SceneNode*>::type checkedSceneNodes;

        PlaneBoundedVolumeList::iterator pi, piend;
        piend = mVolumes.end();
        for (pi = mVolumes.begin(); pi != piend; ++pi)
        {
            PCZSceneNodeList list;
            //find the nodes that intersect the Plane bounded Volume
            static_cast<PCZSceneManager*>( mParentSceneMgr ) -> findNodesIn( *pi, list, mStartZone, (PCZSceneNode*)mExcludeNode );

            //grab all moveables from the node that intersect...
            PCZSceneNodeList::iterator it, itend;
            itend = list.end();
            for (it = list.begin(); it != itend; ++it)
            {
                // avoid double-check same scene node
                if (!checkedSceneNodes.insert(*it).second)
                    continue;
                SceneNode::ObjectIterator oit = (*it) -> getAttachedObjectIterator();
                while( oit.hasMoreElements() )
                {
                    MovableObject * m = oit.getNext();
                    if( (m->getQueryFlags() & mQueryMask) && 
                        (m->getTypeFlags() & mQueryTypeMask) && 
                        m->isInScene() &&
                        (*pi).intersects( m->getWorldBoundingBox() ) )
                    {
                        listener -> queryResult( m );
                        // deal with attached objects, since they are not directly attached to nodes
                        if (m->getMovableType() == "Entity")
                        {
                            Entity* e = static_cast<Entity*>(m);
                            Entity::ChildObjectListIterator childIt = e->getAttachedObjectIterator();
                            while(childIt.hasMoreElements())
                            {
                                MovableObject* c = childIt.getNext();
                                if (c->getQueryFlags() & mQueryMask &&
                                    (*pi).intersects( c->getWorldBoundingBox()))
                                {
                                    listener->queryResult(c);
                                }
                            }
                        }
                    }
                }
            }
        }//for
        // reset startzone and exclude node
        mStartZone = 0;
        mExcludeNode = 0;
    }


}
