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
#include "OgreBspSceneNode.h"
#include "OgreBspSceneManager.h"

namespace Ogre {


    void BspSceneNode::_update(bool updateChildren, bool parentHasChanged)
    {
        bool checkMovables = false;

        if (mNeedParentUpdate || parentHasChanged)
        {
            // This means we've moved
            checkMovables = true;
        }

        // Call superclass
        SceneNode::_update(updateChildren, parentHasChanged);

        if (checkMovables)
        {
            // Check membership of attached objects
            ObjectMap::const_iterator it, itend;
            itend = mObjectsByName.end();
            for (it = mObjectsByName.begin(); it != itend; ++it)
            {
                MovableObject* mov = it->second;

                static_cast<BspSceneManager*>(mCreator)->_notifyObjectMoved(
                    mov, this->_getDerivedPosition());

            }
        }

    }
	//-------------------------------------------------------------------------
	MovableObject* BspSceneNode::detachObject(unsigned short index)
	{
		MovableObject* ret = SceneNode::detachObject(index);
		static_cast<BspSceneManager*>(mCreator)->_notifyObjectDetached(ret);
		return ret;
		
	}
	//-------------------------------------------------------------------------
	MovableObject* BspSceneNode::detachObject(const String& name)
	{
		MovableObject* ret = SceneNode::detachObject(name);
		static_cast<BspSceneManager*>(mCreator)->_notifyObjectDetached(ret);
		return ret;
	}
	//-------------------------------------------------------------------------
	void BspSceneNode::detachAllObjects(void)
	{
		ObjectMap::const_iterator i, iend;
		iend = mObjectsByName.end();
		for (i = mObjectsByName.begin(); i != iend; ++i)
		{
			static_cast<BspSceneManager*>(mCreator)
				->_notifyObjectDetached(i->second);
		}
		SceneNode::detachAllObjects();
	}
	//-------------------------------------------------------------------------
	void BspSceneNode::setInSceneGraph(bool inGraph)
	{
		if (mIsInSceneGraph != inGraph)
		{
			ObjectMap::const_iterator i, iend;
			iend = mObjectsByName.end();
			for (i = mObjectsByName.begin(); i != iend; ++i)
			{
				if (!inGraph)
				{
					// Equivalent to detaching
					static_cast<BspSceneManager*>(mCreator)
						->_notifyObjectDetached(i->second);
				}
				else
				{
					// move deals with re-adding
	                static_cast<BspSceneManager*>(mCreator)->_notifyObjectMoved(
    	                i->second, this->_getDerivedPosition());
				}
			}
		}
		mIsInSceneGraph = inGraph;
	}

}



