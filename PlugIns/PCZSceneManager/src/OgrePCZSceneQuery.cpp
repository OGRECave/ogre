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
OgrePCZSceneQuery.cpp  -  Scene Query implementations for PCZSceneManager
-----------------------------------------------------------------------------
begin                : Wed Feb 21, 2007
author               : Eric Cha
email                : ericc@xenopi.com
current TODO's       : none known
-----------------------------------------------------------------------------
*/

#include <OgreEntity.h>
#include <OgreRoot.h>

#include "OgrePCZSceneQuery.h"
#include "OgrePCZSceneManager.h"

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
