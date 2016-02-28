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
octreenode.h  -  description
-------------------
begin                : Fri Sep 27 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net

***************************************************************************/

#ifndef OCTREENODE_H
#define OCTREENODE_H

#include "OgreSceneNode.h"

#include "OgreOctreePrerequisites.h"
#include "OgreOctree.h"

namespace Ogre
{

/** Specialized SceneNode that is customized for working within an Octree. Each node
* maintains its own bounding box, rather than merging it with all the children.
*
*/

class _OgreOctreePluginExport OctreeNode : public SceneNode
{
public:
    /** Standard constructor */
    OctreeNode( SceneManager* creator );
    /** Standard constructor */
    OctreeNode( SceneManager* creator, const String& name );
    /** Standard destructor */
    ~OctreeNode();

    /** Overridden from Node to remove any reference to octants */
    Node * removeChild( unsigned short index );
    
    /** Overridden from Node to remove any reference to octants */
    Node * removeChild( const String & name );

    /** Overridden from Node to remove any reference to octants */
    Node * removeChild( Node* child);

    /** Overridden from Node to remove any reference to octants */
    void removeAllChildren(void);

    /** Returns the Octree in which this OctreeNode resides
    */
    Octree * getOctant()
    {
        return mOctant;
    };

    /** Sets the Octree in which this OctreeNode resides
    */
    void setOctant( Octree *o )
    {
        mOctant = o;
    };

    /** Determines if the center of this node is within the given box
    */
    bool _isIn( AxisAlignedBox &box );

    /** Adds all the attached scenenodes to the render queue
    */
    virtual void _addToRenderQueue( Camera* cam, RenderQueue * q, bool onlyShadowCasters, 
        VisibleObjectsBoundsInfo* visibleBounds);

    /** Sets up the LegacyRenderOperation for rendering this scene node as geometry.
    @remarks
    This will render the scenenode as a bounding box.
    */
    virtual void getRenderOperation( RenderOperation& op );

    /** Returns the local bounding box of this OctreeNode.
    @remarks
    This is used to render the bounding box, rather then the global.
    */
    AxisAlignedBox & _getLocalAABB()
    {
        return mLocalAABB;
    };




protected:

    /** Internal method for updating the bounds for this OctreeNode.
    @remarks
    This method determines the bounds solely from the attached objects, not
    any children. If the node has changed its bounds, it is removed from its
    current octree, and reinserted into the tree.
    */
    void _updateBounds( void );

    void _removeNodeAndChildren( );

    /// Local bounding box
    AxisAlignedBox mLocalAABB;

    ///Octree this node is attached to.
    Octree *mOctant;

    /// Preallocated corners for rendering
    Real mCorners[ 24 ];
    /// Shared colors for rendering
    static unsigned long mColors[ 8 ];
    /// Shared indexes for rendering
    static unsigned short mIndexes[ 24 ];


};

}


#endif
