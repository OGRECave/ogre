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
#ifndef __BSPSCENENODE_H__
#define __BSPSCENENODE_H__

#include "OgreBspPrerequisites.h"
#include "OgreSceneNode.h"

namespace Ogre {

    /** Specialisation of SceneNode for the BspSceneManager.
    @remarks
        This specialisation of SceneNode is to enable information about the
        leaf node in which any attached objects are held is stored for
        use in the visibility determination. 
    @par
        Do not confuse this class with BspNode, which reflects nodes in the
        BSP tree itself. This class is just like a regular SceneNode, except that
        it should be locating BspNode leaf elements which objects should be included
        in. Note that because objects are movable, and thus may very well be overlapping
        the boundaries of more than one leaf, that it is possible that an object attached
        to one BspSceneNode may actually be associated with more than one BspNode.
    */
    class BspSceneNode : public SceneNode
    {
	protected:
		/// Overridden from SceneNode
		void setInSceneGraph(bool inGraph);		
    public:
        BspSceneNode(SceneManager* creator) : SceneNode(creator) {}
        BspSceneNode(SceneManager* creator, const String& name) 
            : SceneNode(creator, name) {}
        /// Overridden from Node
        void _update(bool updateChildren, bool parentHasChanged);
        /** Detaches the indexed object from this scene node.
        @remarks
            Detaches by index, see the alternate version to detach by name. Object indexes
            may change as other objects are added / removed.
        */
        MovableObject* detachObject(unsigned short index);

        /** Detaches the named object from this node and returns a pointer to it. */
        MovableObject* detachObject(const String& name);

        /** Detaches all objects attached to this node.
        */
        void detachAllObjects(void);


    };

}

#endif
