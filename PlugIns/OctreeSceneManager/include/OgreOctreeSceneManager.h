/***************************************************************************
octreescenemanager.h  -  description
-------------------
begin                : Fri Sep 27 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net
***************************************************************************/

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

#ifndef OCTREESCENEMANAGER_H
#define OCTREESCENEMANAGER_H

#include "OgreTerrainPrerequisites.h"
#include "OgreSceneManager.h"
#include "OgreRenderOperation.h"
#include "OgreSphere.h"

#include <list>
#include <algorithm>

#include <OgreOctree.h>


namespace Ogre
{

class OctreeNode;

class OctreeCamera;
class OctreeIntersectionSceneQuery;
class OctreeRaySceneQuery;
class OctreeSphereSceneQuery;
class OctreeAxisAlignedBoxSceneQuery;
class OctreePlaneBoundedVolumeListSceneQuery;


typedef list< WireBoundingBox * >::type BoxList;
typedef list< unsigned long >::type ColorList;
//typedef list< SceneNode * >::type SceneNodeList;


/** Specialized SceneManager that divides the geometry into an octree in order to facilitate spatial queries.
@remarks
*/

class _OgreOctreePluginExport OctreeSceneManager : public SceneManager
{
    friend class OctreeIntersectionSceneQuery;
    friend class OctreeRaySceneQuery;
    friend class OctreeSphereSceneQuery;
    friend class OctreeAxisAlignedBoxSceneQuery;
    friend class OctreePlaneBoundedVolumeListSceneQuery;

public:
    static int intersect_call;
    /** Standard Constructor.  Initializes the octree to -10000,-10000,-10000 to 10000,10000,10000 with a depth of 8. */
    OctreeSceneManager(const String& name);
    /** Standard Constructor */
    OctreeSceneManager(const String& name, AxisAlignedBox &box, int max_depth );
    /** Standard destructor */
    ~OctreeSceneManager();

	/// @copydoc SceneManager::getTypeName
	const String& getTypeName(void) const;

    /** Initializes the manager to the given box and depth.
    */
    void init( AxisAlignedBox &box, int d );

    /** Creates a specialized OctreeNode */
    virtual SceneNode * createSceneNodeImpl ( void );
    /** Creates a specialized OctreeNode */
    virtual SceneNode * createSceneNodeImpl ( const String &name );
    /** Creates a specialized OctreeCamera */
    virtual Camera * createCamera( const String &name );

    /** Deletes a scene node */
    virtual void destroySceneNode( const String &name );



    /** Does nothing more */
    virtual void _updateSceneGraph( Camera * cam );
    /** Recurses through the octree determining which nodes are visible. */
    virtual void _findVisibleObjects ( Camera * cam, 
		VisibleObjectsBoundsInfo* visibleBounds, bool onlyShadowCasters );

    /** Alerts each unculled object, notifying it that it will be drawn.
     * Useful for doing calculations only on nodes that will be drawn, prior
     * to drawing them...
     */
    virtual void _alertVisibleObjects( void );

    /** Walks through the octree, adding any visible objects to the render queue.
    @remarks
    If any octant in the octree if completely within the view frustum,
    all subchildren are automatically added with no visibility tests.
    */
    void walkOctree( OctreeCamera *, RenderQueue *, Octree *, 
		VisibleObjectsBoundsInfo* visibleBounds, bool foundvisible, 
		bool onlyShadowCasters);

    /** Checks the given OctreeNode, and determines if it needs to be moved
    * to a different octant.
    */
    void _updateOctreeNode( OctreeNode * );
    /** Removes the given octree node */
    void _removeOctreeNode( OctreeNode * );
    /** Adds the Octree Node, starting at the given octree, and recursing at max to the specified depth.
    */
    void _addOctreeNode( OctreeNode *, Octree *octree, int depth = 0 );

    /** Recurses the octree, adding any nodes intersecting with the box into the given list.
    It ignores the exclude scene node.
    */
    void findNodesIn( const AxisAlignedBox &box, list< SceneNode * >::type &list, SceneNode *exclude = 0 );

    /** Recurses the octree, adding any nodes intersecting with the sphere into the given list.
    It ignores the exclude scene node.
    */
    void findNodesIn( const Sphere &sphere, list< SceneNode * >::type &list, SceneNode *exclude = 0 );

    /** Recurses the octree, adding any nodes intersecting with the volume into the given list.
      It ignores the exclude scene node.
      */
    void findNodesIn( const PlaneBoundedVolume &volume, list< SceneNode * >::type &list, SceneNode *exclude=0 );

    /** Recurses the octree, adding any nodes intersecting with the ray into the given list.
      It ignores the exclude scene node.
      */
    void findNodesIn( const Ray &ray, list< SceneNode * >::type &list, SceneNode *exclude=0 );

    /** Sets the box visibility flag */
    void setShowBoxes( bool b )
    {
        mShowBoxes = b;
    };

    void setLooseOctree( bool b )
    {
        mLoose = b;
    };


    /** Resizes the octree to the given size */
    void resize( const AxisAlignedBox &box );

    /** Sets the given option for the SceneManager
               @remarks
        Options are:
        "Size", AxisAlignedBox *;
        "Depth", int *;
        "ShowOctree", bool *;
    */

    virtual bool setOption( const String &, const void * );
    /** Gets the given option for the Scene Manager.
        @remarks
        See setOption
    */
    virtual bool getOption( const String &, void * );

    bool getOptionValues( const String & key, StringVector &refValueList );
    bool getOptionKeys( StringVector &refKeys );
    /** Overridden from SceneManager */
    void clearScene(void);

    AxisAlignedBoxSceneQuery* createAABBQuery(const AxisAlignedBox& box, unsigned long mask);
    SphereSceneQuery* createSphereQuery(const Sphere& sphere, unsigned long mask);
    PlaneBoundedVolumeListSceneQuery* createPlaneBoundedVolumeQuery(const PlaneBoundedVolumeList& volumes, unsigned long mask);
    RaySceneQuery* createRayQuery(const Ray& ray, unsigned long mask);
    IntersectionSceneQuery* createIntersectionQuery(unsigned long mask);

protected:


	Octree::NodeList mVisible;

    /// The root octree
    Octree *mOctree;

    /// List of boxes to be rendered
    BoxList mBoxes;

    /// Number of rendered objs
    int mNumObjects;

    /// Max depth for the tree
    int mMaxDepth;
    /// Size of the octree
    AxisAlignedBox mBox;

    /// Boxes visibility flag
    bool mShowBoxes;


    bool mLoose;

    Real mCorners[ 24 ];
    static unsigned long mColors[ 8 ];
    static unsigned short mIndexes[ 24 ];

    Matrix4 mScaleFactor;

};

/// Factory for OctreeSceneManager
class OctreeSceneManagerFactory : public SceneManagerFactory
{
protected:
	void initMetaData(void) const;
public:
	OctreeSceneManagerFactory() {}
	~OctreeSceneManagerFactory() {}
	/// Factory type name
	static const String FACTORY_TYPE_NAME;
	SceneManager* createInstance(const String& instanceName);
	void destroyInstance(SceneManager* instance);
};



}

#endif

