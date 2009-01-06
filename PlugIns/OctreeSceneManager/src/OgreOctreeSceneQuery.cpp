/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/
 
Copyright  2000-2005 The OGRE Team
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
/***************************************************************************
OgreOctreeSceneQuery.cpp  -  description
-------------------
begin                : Tues July 20, 2004
copyright            : (C) 2004by Jon Anderson
email                : janders@users.sf.net
 
 
 
***************************************************************************/

#include <OgreOctreeSceneQuery.h>
#include <OgreOctreeSceneManager.h>
#include <OgreEntity.h>
#include <OgreRoot.h>

namespace Ogre
{

//---------------------------------------------------------------------
OctreeIntersectionSceneQuery::OctreeIntersectionSceneQuery(SceneManager* creator)
        : DefaultIntersectionSceneQuery(creator)
{

}
//---------------------------------------------------------------------
OctreeIntersectionSceneQuery::~OctreeIntersectionSceneQuery()
{}
//---------------------------------------------------------------------
void OctreeIntersectionSceneQuery::execute(IntersectionSceneQueryListener* listener)
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

			Ogre::list< SceneNode * >::type list;
			//find the nodes that intersect the AAB
			static_cast<OctreeSceneManager*>( mParentSceneMgr ) -> findNodesIn( e->getWorldBoundingBox(), list, 0 );
			//grab all moveables from the node that intersect...
			Ogre::list< SceneNode * >::type::iterator nit = list.begin();
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
/** Creates a custom Octree AAB query */
OctreeAxisAlignedBoxSceneQuery::OctreeAxisAlignedBoxSceneQuery(SceneManager* creator)
        : DefaultAxisAlignedBoxSceneQuery(creator)
{
}
/** Deletes the custom Octree query */
OctreeAxisAlignedBoxSceneQuery::~OctreeAxisAlignedBoxSceneQuery()
{}

/** Finds any entities that intersect the AAB for the query. */
void OctreeAxisAlignedBoxSceneQuery::execute(SceneQueryListener* listener)
{
    list< SceneNode * >::type _list;
    //find the nodes that intersect the AAB
    static_cast<OctreeSceneManager*>( mParentSceneMgr ) -> findNodesIn( mAABB, _list, 0 );

    //grab all moveables from the node that intersect...
    list< SceneNode * >::type::iterator it = _list.begin();
    while( it != _list.end() )
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

}
//---------------------------------------------------------------------
OctreeRaySceneQuery::
OctreeRaySceneQuery(SceneManager* creator) : DefaultRaySceneQuery(creator)
{
}
//---------------------------------------------------------------------
OctreeRaySceneQuery::~OctreeRaySceneQuery()
{}
//---------------------------------------------------------------------
void OctreeRaySceneQuery::execute(RaySceneQueryListener* listener)
{
    list< SceneNode * >::type _list;
    //find the nodes that intersect the AAB
    static_cast<OctreeSceneManager*>( mParentSceneMgr ) -> findNodesIn( mRay, _list, 0 );

    //grab all moveables from the node that intersect...
    list< SceneNode * >::type::iterator it = _list.begin();
    while( it != _list.end() )
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

}


//---------------------------------------------------------------------
OctreeSphereSceneQuery::
OctreeSphereSceneQuery(SceneManager* creator) : DefaultSphereSceneQuery(creator)
{
}
//---------------------------------------------------------------------
OctreeSphereSceneQuery::~OctreeSphereSceneQuery()
{}
//---------------------------------------------------------------------
void OctreeSphereSceneQuery::execute(SceneQueryListener* listener)
{
    list< SceneNode * >::type _list;
    //find the nodes that intersect the AAB
    static_cast<OctreeSceneManager*>( mParentSceneMgr ) -> findNodesIn( mSphere, _list, 0 );

    //grab all moveables from the node that intersect...
    list< SceneNode * >::type::iterator it = _list.begin();
    while( it != _list.end() )
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
}
//---------------------------------------------------------------------
OctreePlaneBoundedVolumeListSceneQuery::
OctreePlaneBoundedVolumeListSceneQuery(SceneManager* creator)
        : DefaultPlaneBoundedVolumeListSceneQuery(creator)
{

}
//---------------------------------------------------------------------
OctreePlaneBoundedVolumeListSceneQuery::~OctreePlaneBoundedVolumeListSceneQuery()
{}
//---------------------------------------------------------------------
void OctreePlaneBoundedVolumeListSceneQuery::execute(SceneQueryListener* listener)
{
    set<SceneNode*>::type checkedSceneNodes;

    PlaneBoundedVolumeList::iterator pi, piend;
    piend = mVolumes.end();
    for (pi = mVolumes.begin(); pi != piend; ++pi)
    {
        list< SceneNode * >::type _list;
        //find the nodes that intersect the AAB
        static_cast<OctreeSceneManager*>( mParentSceneMgr ) -> findNodesIn( *pi, _list, 0 );

        //grab all moveables from the node that intersect...
        list< SceneNode * >::type::iterator it, itend;
        itend = _list.end();
        for (it = _list.begin(); it != itend; ++it)
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
}


}
