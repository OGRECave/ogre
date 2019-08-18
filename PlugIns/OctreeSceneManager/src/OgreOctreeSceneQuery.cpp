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
/***************************************************************************
OgreOctreeSceneQuery.cpp  -  description
-------------------
begin                : Tues July 20, 2004
copyright            : (C) 2004by Jon Anderson
email                : janders@users.sf.net
 
 
 
***************************************************************************/

#include "OgreOctreeSceneQuery.h"
#include "OgreRoot.h"
#include "OgreSceneNode.h"
#include "OgreOctreeSceneManager.h"
#include "OgreEntity.h"

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
    for(const auto& factIt : Root::getSingleton().getMovableObjectFactories())
    {
        for (const auto& it : mParentSceneMgr->getMovableObjects(factIt.first))
        {

            MovableObject * e = it.second;

            std::list< SceneNode * > list;
            //find the nodes that intersect the AAB
            static_cast<OctreeSceneManager*>( mParentSceneMgr ) -> findNodesIn( e->getWorldBoundingBox(), list, 0 );
            //grab all moveables from the node that intersect...
            std::list< SceneNode * >::iterator nit = list.begin();
            while( nit != list.end() )
            {
                for (auto m : (*nit)->getAttachedObjects())
                {
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
                            for(auto c : e2->getAttachedObjects())
                            {
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
    std::list< SceneNode * > _list;
    //find the nodes that intersect the AAB
    static_cast<OctreeSceneManager*>( mParentSceneMgr ) -> findNodesIn( mAABB, _list, 0 );

    //grab all moveables from the node that intersect...
    std::list< SceneNode * >::iterator it = _list.begin();
    while( it != _list.end() )
    {
        for (auto m : (*it)->getAttachedObjects())
        {
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
                    for (auto c : e->getAttachedObjects())
                    {
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
    std::list< SceneNode * > _list;
    //find the nodes that intersect the AAB
    static_cast<OctreeSceneManager*>( mParentSceneMgr ) -> findNodesIn( mRay, _list, 0 );

    //grab all moveables from the node that intersect...
    std::list< SceneNode * >::iterator it = _list.begin();
    while( it != _list.end() )
    {
        for (auto m : (*it)->getAttachedObjects())
        {
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
                        for(auto c : e->getAttachedObjects())
                        {
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
    std::list< SceneNode * > _list;
    //find the nodes that intersect the AAB
    static_cast<OctreeSceneManager*>( mParentSceneMgr ) -> findNodesIn( mSphere, _list, 0 );

    //grab all moveables from the node that intersect...
    std::list< SceneNode * >::iterator it = _list.begin();
    while( it != _list.end() )
    {
        for (auto m : (*it)->getAttachedObjects())
        {
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
                    for(auto c : e->getAttachedObjects())
                    {
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
    std::set<SceneNode*> checkedSceneNodes;

    PlaneBoundedVolumeList::iterator pi, piend;
    piend = mVolumes.end();
    for (pi = mVolumes.begin(); pi != piend; ++pi)
    {
        std::list< SceneNode * > _list;
        //find the nodes that intersect the AAB
        static_cast<OctreeSceneManager*>( mParentSceneMgr ) -> findNodesIn( *pi, _list, 0 );

        //grab all moveables from the node that intersect...
        std::list< SceneNode * >::iterator it, itend;
        itend = _list.end();
        for (it = _list.begin(); it != itend; ++it)
        {
            // avoid double-check same scene node
            if (!checkedSceneNodes.insert(*it).second)
                continue;
            for (auto m : (*it)->getAttachedObjects())
            {
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
                        for(auto c : e->getAttachedObjects())
                        {
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
