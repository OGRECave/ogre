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
/***************************************************************************
octreescenemanager.cpp  -  description
-------------------
begin                : Fri Sep 27 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net
 
Enhancements 2003 - 2004 (C) The OGRE Team
 
***************************************************************************/

#include <OgreOctreeSceneManager.h>
#include <OgreOctreeSceneQuery.h>
#include <OgreOctreeNode.h>
#include <OgreOctreeCamera.h>
#include <OgreRenderSystem.h>


extern "C"
{
    void findNodesInBox( Ogre::SceneManager *sm,
                         const Ogre::AxisAlignedBox &box,
						 Ogre::list< Ogre::SceneNode * >::type &list,
                         Ogre::SceneNode *exclude )
    {
        static_cast<Ogre::OctreeSceneManager*>( sm ) -> findNodesIn( box, list, exclude );
    }
    void findNodesInSphere( Ogre::SceneManager *sm,
                            const Ogre::Sphere &sphere,
							Ogre::list< Ogre::SceneNode * >::type &list,
                            Ogre::SceneNode *exclude )
    {
        static_cast<Ogre::OctreeSceneManager*>( sm ) -> findNodesIn( sphere, list, exclude );
    }
}

namespace Ogre
{
enum Intersection
{
    OUTSIDE=0,
    INSIDE=1,
    INTERSECT=2
};
int OctreeSceneManager::intersect_call = 0;

Intersection intersect( const Ray &one, const AxisAlignedBox &two )
{
    OctreeSceneManager::intersect_call++;
    // Null box?
    if (two.isNull()) return OUTSIDE;
	// Infinite box?
	if (two.isInfinite()) return INTERSECT;

    bool inside = true;
    const Vector3& twoMin = two.getMinimum();
    const Vector3& twoMax = two.getMaximum();
    Vector3 origin = one.getOrigin();
    Vector3 dir = one.getDirection();

    Vector3 maxT(-1, -1, -1);

    int i = 0;
    for(i=0; i<3; i++ )
    {
        if( origin[i] < twoMin[i] )
        {
            inside = false;
            if( dir[i] > 0 )
            {
                maxT[i] = (twoMin[i] - origin[i])/ dir[i];
            }
        }
        else if( origin[i] > twoMax[i] )
        {
            inside = false;
            if( dir[i] < 0 )
            {
                maxT[i] = (twoMax[i] - origin[i]) / dir[i];
            }
        }
    }

    if( inside )
    {
        return INTERSECT;
    }
    int whichPlane = 0;
    if( maxT[1] > maxT[whichPlane])
        whichPlane = 1;
    if( maxT[2] > maxT[whichPlane])
        whichPlane = 2;

    if( ((int)maxT[whichPlane]) & 0x80000000 )
    {
        return OUTSIDE;
    }
    for(i=0; i<3; i++ )
    {
        if( i!= whichPlane )
        {
            float f = origin[i] + maxT[whichPlane] * dir[i];
            if ( f < (twoMin[i] - 0.00001f) ||
                    f > (twoMax[i] +0.00001f ) )
            {
                return OUTSIDE;
            }
        }
    }

    return INTERSECT;

}


/** Checks how the second box intersects with the first.
*/
Intersection intersect( const PlaneBoundedVolume &one, const AxisAlignedBox &two )
{
    OctreeSceneManager::intersect_call++;
    // Null box?
    if (two.isNull()) return OUTSIDE;
	// Infinite box?
	if (two.isInfinite()) return INTERSECT;

    // Get centre of the box
    Vector3 centre = two.getCenter();
    // Get the half-size of the box
    Vector3 halfSize = two.getHalfSize();

    // For each plane, see if all points are on the negative side
    // If so, object is not visible.
    // If one or more are, it's partial.
    // If all aren't, full
    bool all_inside = true;
    PlaneList::const_iterator i, iend;
    iend = one.planes.end();
    for (i = one.planes.begin(); i != iend; ++i)
    {
        const Plane& plane = *i;

        Plane::Side side = plane.getSide(centre, halfSize);
        if(side == one.outside)
                return OUTSIDE;
        if(side == Plane::BOTH_SIDE)
                all_inside = false; 
    }

    if ( all_inside )
        return INSIDE;
    else
        return INTERSECT;

}


/** Checks how the second box intersects with the first.
*/
Intersection intersect( const AxisAlignedBox &one, const AxisAlignedBox &two )
{
    OctreeSceneManager::intersect_call++;
    // Null box?
    if (one.isNull() || two.isNull()) return OUTSIDE;
	if (one.isInfinite()) return INSIDE;
	if (two.isInfinite()) return INTERSECT;


    const Vector3& insideMin = two.getMinimum();
    const Vector3& insideMax = two.getMaximum();

    const Vector3& outsideMin = one.getMinimum();
    const Vector3& outsideMax = one.getMaximum();

    if (    insideMax.x < outsideMin.x ||
            insideMax.y < outsideMin.y ||
            insideMax.z < outsideMin.z ||
            insideMin.x > outsideMax.x ||
            insideMin.y > outsideMax.y ||
            insideMin.z > outsideMax.z )
    {
        return OUTSIDE;
    }

    bool full = ( insideMin.x > outsideMin.x &&
                  insideMin.y > outsideMin.y &&
                  insideMin.z > outsideMin.z &&
                  insideMax.x < outsideMax.x &&
                  insideMax.y < outsideMax.y &&
                  insideMax.z < outsideMax.z );

    if ( full )
        return INSIDE;
    else
        return INTERSECT;

}

/** Checks how the box intersects with the sphere.
*/
Intersection intersect( const Sphere &one, const AxisAlignedBox &two )
{
    OctreeSceneManager::intersect_call++;
    // Null box?
    if (two.isNull()) return OUTSIDE;
	if (two.isInfinite()) return INTERSECT;

    float sradius = one.getRadius();

    sradius *= sradius;

    Vector3 scenter = one.getCenter();

    const Vector3& twoMin = two.getMinimum();
    const Vector3& twoMax = two.getMaximum();

    float s, d = 0;

    Vector3 mndistance = ( twoMin - scenter );
    Vector3 mxdistance = ( twoMax - scenter );

    if ( mndistance.squaredLength() < sradius &&
            mxdistance.squaredLength() < sradius )
    {
        return INSIDE;
    }

    //find the square of the distance
    //from the sphere to the box
    for ( int i = 0 ; i < 3 ; i++ )
    {
        if ( scenter[ i ] < twoMin[ i ] )
        {
            s = scenter[ i ] - twoMin[ i ];
            d += s * s;
        }

        else if ( scenter[ i ] > twoMax[ i ] )
        {
            s = scenter[ i ] - twoMax[ i ];
            d += s * s;
        }

    }

    bool partial = ( d <= sradius );

    if ( !partial )
    {
        return OUTSIDE;
    }

    else
    {
        return INTERSECT;
    }


}

unsigned long white = 0xFFFFFFFF;

unsigned short OctreeSceneManager::mIndexes[ 24 ] = {0, 1, 1, 2, 2, 3, 3, 0,       //back
        0, 6, 6, 5, 5, 1,             //left
        3, 7, 7, 4, 4, 2,             //right
        6, 7, 5, 4 };          //front
unsigned long OctreeSceneManager::mColors[ 8 ] = {white, white, white, white, white, white, white, white };


OctreeSceneManager::OctreeSceneManager(const String& name) : SceneManager(name)
{
    AxisAlignedBox b( -10000, -10000, -10000, 10000, 10000, 10000 );
    int depth = 8; 
    mOctree = 0;
    init( b, depth );
}

OctreeSceneManager::OctreeSceneManager(const String& name, AxisAlignedBox &box, int max_depth ) 
: SceneManager(name)
{
    mOctree = 0;
    init( box, max_depth );
}

const String& OctreeSceneManager::getTypeName(void) const
{
	return OctreeSceneManagerFactory::FACTORY_TYPE_NAME;
}

void OctreeSceneManager::init( AxisAlignedBox &box, int depth )
{

    if ( mOctree != 0 )
        OGRE_DELETE mOctree;

    mOctree = OGRE_NEW Octree( 0 );

    mMaxDepth = depth;
    mBox = box;

    mOctree -> mBox = box;

    Vector3 min = box.getMinimum();

    Vector3 max = box.getMaximum();

    mOctree -> mHalfSize = ( max - min ) / 2;


    mShowBoxes = false;

    mNumObjects = 0;

    Vector3 v( 1.5, 1.5, 1.5 );

    mScaleFactor.setScale( v );



    // setDisplaySceneNodes( true );
    // setShowBoxes( true );

    //
    //mSceneRoot isn't put into the octree since it has no volume.

}

OctreeSceneManager::~OctreeSceneManager()
{

    if ( mOctree )
	{
        OGRE_DELETE mOctree;
		mOctree = 0;
	}
}

Camera * OctreeSceneManager::createCamera( const String &name )
{
	// Check name not used
	if (mCameras.find(name) != mCameras.end())
	{
		OGRE_EXCEPT(
			Exception::ERR_DUPLICATE_ITEM,
			"A camera with the name " + name + " already exists",
			"OctreeSceneManager::createCamera" );
	}

	Camera * c = OGRE_NEW OctreeCamera( name, this );
    mCameras.insert( CameraList::value_type( name, c ) );

	// create visible bounds aab map entry
	mCamVisibleObjectsMap[c] = VisibleObjectsBoundsInfo();
	
    return c;
}

void OctreeSceneManager::destroySceneNode( const String &name )
{
    OctreeNode * on = static_cast < OctreeNode* > ( getSceneNode( name ) );

    if ( on != 0 )
        _removeOctreeNode( on );

    SceneManager::destroySceneNode( name );
}

bool OctreeSceneManager::getOptionValues( const String & key, StringVector  &refValueList )
{
    return SceneManager::getOptionValues( key, refValueList );
}

bool OctreeSceneManager::getOptionKeys( StringVector & refKeys )
{
    SceneManager::getOptionKeys( refKeys );
    refKeys.push_back( "Size" );
    refKeys.push_back( "ShowOctree" );
    refKeys.push_back( "Depth" );

    return true;
}


void OctreeSceneManager::_updateOctreeNode( OctreeNode * onode )
{
    const AxisAlignedBox& box = onode -> _getWorldAABB();

    if ( box.isNull() )
        return ;

	// Skip if octree has been destroyed (shutdown conditions)
	if (!mOctree)
		return;

    if ( onode -> getOctant() == 0 )
    {
        //if outside the octree, force into the root node.
        if ( ! onode -> _isIn( mOctree -> mBox ) )
            mOctree->_addNode( onode );
        else
            _addOctreeNode( onode, mOctree );
        return ;
    }

    if ( ! onode -> _isIn( onode -> getOctant() -> mBox ) )
    {
        _removeOctreeNode( onode );

        //if outside the octree, force into the root node.
        if ( ! onode -> _isIn( mOctree -> mBox ) )
            mOctree->_addNode( onode );
        else
            _addOctreeNode( onode, mOctree );
    }
}

/** Only removes the node from the octree.  It leaves the octree, even if it's empty.
*/
void OctreeSceneManager::_removeOctreeNode( OctreeNode * n )
{
	// Skip if octree has been destroyed (shutdown conditions)
	if (!mOctree)
		return;

    Octree * oct = n -> getOctant();

    if ( oct )
    {
        oct -> _removeNode( n );
    }

    n->setOctant(0);
}


void OctreeSceneManager::_addOctreeNode( OctreeNode * n, Octree *octant, int depth )
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
            octant -> mChildren[ x ][ y ][ z ] = OGRE_NEW Octree( octant );
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

        _addOctreeNode( n, octant -> mChildren[ x ][ y ][ z ], ++depth );

    }

    else
    {
        octant -> _addNode( n );
    }
}


SceneNode * OctreeSceneManager::createSceneNodeImpl( void )
{
    return OGRE_NEW OctreeNode( this );
}

SceneNode * OctreeSceneManager::createSceneNodeImpl( const String &name )
{
    return OGRE_NEW OctreeNode( this, name );
}

void OctreeSceneManager::_updateSceneGraph( Camera * cam )
{
    SceneManager::_updateSceneGraph( cam );
}

void OctreeSceneManager::_alertVisibleObjects( void )
{
    OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
        "Function doesn't do as advertised",
        "OctreeSceneManager::_alertVisibleObjects" );

    Octree::NodeList::iterator it = mVisible.begin();

    while ( it != mVisible.end() )
    {
        OctreeNode * node = *it;

        ++it;
    }
}

void OctreeSceneManager::_findVisibleObjects(Camera * cam, 
	VisibleObjectsBoundsInfo* visibleBounds, bool onlyShadowCasters )
{

    getRenderQueue()->clear();
    mBoxes.clear();
    mVisible.clear();

    mNumObjects = 0;

    //walk the octree, adding all visible Octreenodes nodes to the render queue.
    walkOctree( static_cast < OctreeCamera * > ( cam ), getRenderQueue(), mOctree, 
				visibleBounds, false, onlyShadowCasters );

    // Show the octree boxes & cull camera if required
    if ( mShowBoxes )
    {
        for ( BoxList::iterator it = mBoxes.begin(); it != mBoxes.end(); ++it )
        {
            getRenderQueue()->addRenderable(*it);
        }
    }
}

void OctreeSceneManager::walkOctree( OctreeCamera *camera, RenderQueue *queue, 
	Octree *octant, VisibleObjectsBoundsInfo* visibleBounds, 
	bool foundvisible, bool onlyShadowCasters )
{

    //return immediately if nothing is in the node.
    if ( octant -> numNodes() == 0 )
        return ;

    OctreeCamera::Visibility v = OctreeCamera::NONE;

    if ( foundvisible )
    {
        v = OctreeCamera::FULL;
    }

    else if ( octant == mOctree )
    {
        v = OctreeCamera::PARTIAL;
    }

    else
    {
        AxisAlignedBox box;
        octant -> _getCullBounds( &box );
        v = camera -> getVisibility( box );
    }


    // if the octant is visible, or if it's the root node...
    if ( v != OctreeCamera::NONE )
    {

        //Add stuff to be rendered;
        Octree::NodeList::iterator it = octant -> mNodes.begin();

        if ( mShowBoxes )
        {
            mBoxes.push_back( octant->getWireBoundingBox() );
        }

        bool vis = true;

        while ( it != octant -> mNodes.end() )
        {
            OctreeNode * sn = *it;

            // if this octree is partially visible, manually cull all
            // scene nodes attached directly to this level.

            if ( v == OctreeCamera::PARTIAL )
                vis = camera -> isVisible( sn -> _getWorldAABB() );

            if ( vis )
            {

                mNumObjects++;
                sn -> _addToRenderQueue(camera, queue, onlyShadowCasters, visibleBounds );

                mVisible.push_back( sn );

                if ( mDisplayNodes )
                    queue -> addRenderable( sn );

                // check if the scene manager or this node wants the bounding box shown.
                if (sn->getShowBoundingBox() || mShowBoundingBoxes)
                    sn->_addBoundingBoxToQueue(queue);
            }

            ++it;
        }

        Octree* child;
        bool childfoundvisible = (v == OctreeCamera::FULL);
        if ( (child = octant -> mChildren[ 0 ][ 0 ][ 0 ]) != 0 )
            walkOctree( camera, queue, child, visibleBounds, childfoundvisible, onlyShadowCasters );

        if ( (child = octant -> mChildren[ 1 ][ 0 ][ 0 ]) != 0 )
            walkOctree( camera, queue, child, visibleBounds, childfoundvisible, onlyShadowCasters );

        if ( (child = octant -> mChildren[ 0 ][ 1 ][ 0 ]) != 0 )
            walkOctree( camera, queue, child, visibleBounds, childfoundvisible, onlyShadowCasters );

        if ( (child = octant -> mChildren[ 1 ][ 1 ][ 0 ]) != 0 )
            walkOctree( camera, queue, child, visibleBounds, childfoundvisible, onlyShadowCasters );

        if ( (child = octant -> mChildren[ 0 ][ 0 ][ 1 ]) != 0 )
            walkOctree( camera, queue, child, visibleBounds, childfoundvisible, onlyShadowCasters );

        if ( (child = octant -> mChildren[ 1 ][ 0 ][ 1 ]) != 0 )
            walkOctree( camera, queue, child, visibleBounds, childfoundvisible, onlyShadowCasters );

        if ( (child = octant -> mChildren[ 0 ][ 1 ][ 1 ]) != 0 )
            walkOctree( camera, queue, child, visibleBounds, childfoundvisible, onlyShadowCasters );

        if ( (child = octant -> mChildren[ 1 ][ 1 ][ 1 ]) != 0 )
            walkOctree( camera, queue, child, visibleBounds, childfoundvisible, onlyShadowCasters );

    }

}

// --- non template versions
void _findNodes( const AxisAlignedBox &t, list< SceneNode * >::type &list, SceneNode *exclude, bool full, Octree *octant )
{

	if ( !full )
	{
		AxisAlignedBox obox;
		octant -> _getCullBounds( &obox );

		Intersection isect = intersect( t, obox );

		if ( isect == OUTSIDE )
			return ;

		full = ( isect == INSIDE );
	}


	Octree::NodeList::iterator it = octant -> mNodes.begin();

	while ( it != octant -> mNodes.end() )
	{
		OctreeNode * on = ( *it );

		if ( on != exclude )
		{
			if ( full )
			{
				list.push_back( on );
			}

			else
			{
				Intersection nsect = intersect( t, on -> _getWorldAABB() );

				if ( nsect != OUTSIDE )
				{
					list.push_back( on );
				}
			}

		}

		++it;
	}

	Octree* child;

	if ( (child=octant -> mChildren[ 0 ][ 0 ][ 0 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 1 ][ 0 ][ 0 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 0 ][ 1 ][ 0 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 1 ][ 1 ][ 0 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 0 ][ 0 ][ 1 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 1 ][ 0 ][ 1 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 0 ][ 1 ][ 1 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 1 ][ 1 ][ 1 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

}

void _findNodes( const Sphere &t, list< SceneNode * >::type &list, SceneNode *exclude, bool full, Octree *octant )
{

	if ( !full )
	{
		AxisAlignedBox obox;
		octant -> _getCullBounds( &obox );

		Intersection isect = intersect( t, obox );

		if ( isect == OUTSIDE )
			return ;

		full = ( isect == INSIDE );
	}


	Octree::NodeList::iterator it = octant -> mNodes.begin();

	while ( it != octant -> mNodes.end() )
	{
		OctreeNode * on = ( *it );

		if ( on != exclude )
		{
			if ( full )
			{
				list.push_back( on );
			}

			else
			{
				Intersection nsect = intersect( t, on -> _getWorldAABB() );

				if ( nsect != OUTSIDE )
				{
					list.push_back( on );
				}
			}

		}

		++it;
	}

	Octree* child;

	if ( (child=octant -> mChildren[ 0 ][ 0 ][ 0 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 1 ][ 0 ][ 0 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 0 ][ 1 ][ 0 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 1 ][ 1 ][ 0 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 0 ][ 0 ][ 1 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 1 ][ 0 ][ 1 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 0 ][ 1 ][ 1 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 1 ][ 1 ][ 1 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

}


void _findNodes( const PlaneBoundedVolume &t, list< SceneNode * >::type &list, SceneNode *exclude, bool full, Octree *octant )
{

	if ( !full )
	{
		AxisAlignedBox obox;
		octant -> _getCullBounds( &obox );

		Intersection isect = intersect( t, obox );

		if ( isect == OUTSIDE )
			return ;

		full = ( isect == INSIDE );
	}


	Octree::NodeList::iterator it = octant -> mNodes.begin();

	while ( it != octant -> mNodes.end() )
	{
		OctreeNode * on = ( *it );

		if ( on != exclude )
		{
			if ( full )
			{
				list.push_back( on );
			}

			else
			{
				Intersection nsect = intersect( t, on -> _getWorldAABB() );

				if ( nsect != OUTSIDE )
				{
					list.push_back( on );
				}
			}

		}

		++it;
	}

	Octree* child;

	if ( (child=octant -> mChildren[ 0 ][ 0 ][ 0 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 1 ][ 0 ][ 0 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 0 ][ 1 ][ 0 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 1 ][ 1 ][ 0 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 0 ][ 0 ][ 1 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 1 ][ 0 ][ 1 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 0 ][ 1 ][ 1 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 1 ][ 1 ][ 1 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

}

void _findNodes( const Ray &t, list< SceneNode * >::type &list, SceneNode *exclude, bool full, Octree *octant )
{

	if ( !full )
	{
		AxisAlignedBox obox;
		octant -> _getCullBounds( &obox );

		Intersection isect = intersect( t, obox );

		if ( isect == OUTSIDE )
			return ;

		full = ( isect == INSIDE );
	}


	Octree::NodeList::iterator it = octant -> mNodes.begin();

	while ( it != octant -> mNodes.end() )
	{
		OctreeNode * on = ( *it );

		if ( on != exclude )
		{
			if ( full )
			{
				list.push_back( on );
			}

			else
			{
				Intersection nsect = intersect( t, on -> _getWorldAABB() );

				if ( nsect != OUTSIDE )
				{
					list.push_back( on );
				}
			}

		}

		++it;
	}

	Octree* child;

	if ( (child=octant -> mChildren[ 0 ][ 0 ][ 0 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 1 ][ 0 ][ 0 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 0 ][ 1 ][ 0 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 1 ][ 1 ][ 0 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 0 ][ 0 ][ 1 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 1 ][ 0 ][ 1 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 0 ][ 1 ][ 1 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

	if ( (child=octant -> mChildren[ 1 ][ 1 ][ 1 ]) != 0 )
		_findNodes( t, list, exclude, full, child );

}

void OctreeSceneManager::findNodesIn( const AxisAlignedBox &box, list< SceneNode * >::type &list, SceneNode *exclude )
{
    _findNodes( box, list, exclude, false, mOctree );
}

void OctreeSceneManager::findNodesIn( const Sphere &sphere, list< SceneNode * >::type &list, SceneNode *exclude )
{
    _findNodes( sphere, list, exclude, false, mOctree );
}

void OctreeSceneManager::findNodesIn( const PlaneBoundedVolume &volume, list< SceneNode * >::type &list, SceneNode *exclude )
{
    _findNodes( volume, list, exclude, false, mOctree );
}

void OctreeSceneManager::findNodesIn( const Ray &r, list< SceneNode * >::type &list, SceneNode *exclude )
{
    _findNodes( r, list, exclude, false, mOctree );
}

void OctreeSceneManager::resize( const AxisAlignedBox &box )
{
    list< SceneNode * >::type nodes;
    list< SceneNode * >::type ::iterator it;

    _findNodes( mOctree->mBox, nodes, 0, true, mOctree );

    OGRE_DELETE mOctree;

    mOctree = OGRE_NEW Octree( 0 );
    mOctree->mBox = box;

	const Vector3 min = box.getMinimum();
	const Vector3 max = box.getMaximum();
	mOctree->mHalfSize = ( max - min ) * 0.5f;

    it = nodes.begin();

    while ( it != nodes.end() )
    {
        OctreeNode * on = static_cast < OctreeNode * > ( *it );
        on -> setOctant( 0 );
        _updateOctreeNode( on );
        ++it;
    }

}

bool OctreeSceneManager::setOption( const String & key, const void * val )
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

    else if ( key == "ShowOctree" )
    {
        mShowBoxes = * static_cast < const bool * > ( val );
        return true;
    }


    return SceneManager::setOption( key, val );


}

bool OctreeSceneManager::getOption( const String & key, void *val )
{
    if ( key == "Size" )
    {
        AxisAlignedBox * b = static_cast < AxisAlignedBox * > ( val );
        b -> setExtents( mOctree->mBox.getMinimum(), mOctree->mBox.getMaximum() );
        return true;
    }

    else if ( key == "Depth" )
    {
        * static_cast < int * > ( val ) = mMaxDepth;
        return true;
    }

    else if ( key == "ShowOctree" )
    {

        * static_cast < bool * > ( val ) = mShowBoxes;
        return true;
    }


    return SceneManager::getOption( key, val );

}

void OctreeSceneManager::clearScene(void)
{
    SceneManager::clearScene();
    init(mBox, mMaxDepth);

}

//---------------------------------------------------------------------
AxisAlignedBoxSceneQuery*
OctreeSceneManager::createAABBQuery(const AxisAlignedBox& box, unsigned long mask)
{
    OctreeAxisAlignedBoxSceneQuery* q = OGRE_NEW OctreeAxisAlignedBoxSceneQuery(this);
    q->setBox(box);
    q->setQueryMask(mask);
    return q;
}
//---------------------------------------------------------------------
SphereSceneQuery*
OctreeSceneManager::createSphereQuery(const Sphere& sphere, unsigned long mask)
{
    OctreeSphereSceneQuery* q = OGRE_NEW OctreeSphereSceneQuery(this);
    q->setSphere(sphere);
    q->setQueryMask(mask);
    return q;
}
//---------------------------------------------------------------------
PlaneBoundedVolumeListSceneQuery*
OctreeSceneManager::createPlaneBoundedVolumeQuery(const PlaneBoundedVolumeList& volumes,
        unsigned long mask)
{
    OctreePlaneBoundedVolumeListSceneQuery* q = OGRE_NEW OctreePlaneBoundedVolumeListSceneQuery(this);
    q->setVolumes(volumes);
    q->setQueryMask(mask);
    return q;
}

//---------------------------------------------------------------------
RaySceneQuery*
OctreeSceneManager::createRayQuery(const Ray& ray, unsigned long mask)
{
    OctreeRaySceneQuery* q = OGRE_NEW OctreeRaySceneQuery(this);
    q->setRay(ray);
    q->setQueryMask(mask);
    return q;
}
//---------------------------------------------------------------------
IntersectionSceneQuery*
OctreeSceneManager::createIntersectionQuery(unsigned long mask)
{

    // Octree implementation performs WORSE for < 500 objects
    // TODO: optimise it so it's better in all cases
    //OctreeIntersectionSceneQuery* q = OGRE_NEW OctreeIntersectionSceneQuery(this);
    DefaultIntersectionSceneQuery* q = OGRE_NEW DefaultIntersectionSceneQuery(this);
    q->setQueryMask(mask);
    return q;
}
//-----------------------------------------------------------------------
const String OctreeSceneManagerFactory::FACTORY_TYPE_NAME = "OctreeSceneManager";
//-----------------------------------------------------------------------
void OctreeSceneManagerFactory::initMetaData(void) const
{
	mMetaData.typeName = FACTORY_TYPE_NAME;
	mMetaData.description = "Scene manager organising the scene on the basis of an octree.";
	mMetaData.sceneTypeMask = 0xFFFF; // support all types
	mMetaData.worldGeometrySupported = false;
}
//-----------------------------------------------------------------------
SceneManager* OctreeSceneManagerFactory::createInstance(
	const String& instanceName)
{
	return OGRE_NEW OctreeSceneManager(instanceName);
}
//-----------------------------------------------------------------------
void OctreeSceneManagerFactory::destroyInstance(SceneManager* instance)
{
	OGRE_DELETE instance;
}


}
