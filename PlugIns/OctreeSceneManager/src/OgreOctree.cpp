/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
octree.cpp  -  description
-------------------
begin                : Mon Sep 30 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net

Enhancements 2003 - 2004 (C) The OGRE Team

***************************************************************************/

#include <OgreOctree.h>
#include <OgreOctreeNode.h>

namespace Ogre
{

/** Returns true is the box will fit in a child.
*/
bool Octree::_isTwiceSize( const AxisAlignedBox &box ) const
{
	// infinite boxes never fit in a child - always root node
	if (box.isInfinite())
		return false;

    Vector3 halfMBoxSize = mBox.getHalfSize();
    Vector3 boxSize = box.getSize();
    return ((boxSize.x <= halfMBoxSize.x) && (boxSize.y <= halfMBoxSize.y) && (boxSize.z <= halfMBoxSize.z));

}

/** It's assumed the the given box has already been proven to fit into
* a child.  Since it's a loose octree, only the centers need to be
* compared to find the appropriate node.
*/
void Octree::_getChildIndexes( const AxisAlignedBox &box, int *x, int *y, int *z ) const
{
    Vector3 max = mBox.getMaximum();
    Vector3 min = box.getMinimum();

    Vector3 center = mBox.getMaximum().midPoint( mBox.getMinimum() );

    Vector3 ncenter = box.getMaximum().midPoint( box.getMinimum() );

    if ( ncenter.x > center.x )
        * x = 1;
    else
        *x = 0;

    if ( ncenter.y > center.y )
        * y = 1;
    else
        *y = 0;

    if ( ncenter.z > center.z )
        * z = 1;
    else
        *z = 0;

}

Octree::Octree( Octree * parent ) 
    : mWireBoundingBox(0),
      mHalfSize( 0, 0, 0 )
{
    //initialize all children to null.
    for ( int i = 0; i < 2; i++ )
    {
        for ( int j = 0; j < 2; j++ )
        {
            for ( int k = 0; k < 2; k++ )
            {
                mChildren[ i ][ j ][ k ] = 0;
            }
        }
    }

    mParent = parent;
    mNumNodes = 0;
}

Octree::~Octree()
{
    //initialize all children to null.
    for ( int i = 0; i < 2; i++ )
    {
        for ( int j = 0; j < 2; j++ )
        {
            for ( int k = 0; k < 2; k++ )
            {
                if ( mChildren[ i ][ j ][ k ] != 0 )
                    OGRE_DELETE mChildren[ i ][ j ][ k ];
            }
        }
    }

    if(mWireBoundingBox)
        OGRE_DELETE mWireBoundingBox;

    mParent = 0;
}

void Octree::_addNode( OctreeNode * n )
{
    mNodes.push_back( n );
    n -> setOctant( this );

    //update total counts.
    _ref();

}

void Octree::_removeNode( OctreeNode * n )
{
    mNodes.erase( std::find( mNodes.begin(), mNodes.end(), n ) );
    n -> setOctant( 0 );

    //update total counts.
    _unref();
}

void Octree::_getCullBounds( AxisAlignedBox *b ) const
{
    b -> setExtents( mBox.getMinimum() - mHalfSize, mBox.getMaximum() + mHalfSize );
}

WireBoundingBox* Octree::getWireBoundingBox()
{
    // Create a WireBoundingBox if needed
    if(mWireBoundingBox == 0)
        mWireBoundingBox = OGRE_NEW WireBoundingBox();

    mWireBoundingBox->setupBoundingBox(mBox);
    return mWireBoundingBox;
}

}
