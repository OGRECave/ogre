/*
-----------------------------------------------------------------------------
This source file is part of the OGRE Reference Application, a layer built
on top of OGRE(Object-oriented Graphics Rendering Engine)
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
#include "OgreRefAppPlane.h"
#include "OgreRefAppWorld.h"

namespace OgreRefApp
{
    //-------------------------------------------------------------------------
    FinitePlane::FinitePlane(const String& name, Real width, Real height) : ApplicationObject(name)
    {
        mWidth = width;
        mHeight = height;
        setUp(name);
    }
    //-------------------------------------------------------------------------
    FinitePlane::~FinitePlane()
    {

    }
    //-------------------------------------------------------------------------
    void FinitePlane::setUp(const String& name)
    {
        // Create visual presence
        SceneManager* sm = World::getSingleton().getSceneManager();
        mEntity = sm->createEntity(name, "Prefab_Plane");
        mSceneNode = sm->getRootSceneNode()->createChildSceneNode(name);
        mSceneNode->attachObject(mEntity);
        // Add reverse reference
        mEntity->setUserObject(this);

        // Default plane is 100x100
        mSceneNode->scale(mWidth / 100.0f, mHeight / 100.0f, 1.0);

        // No mass body (considered static)

        // Create collision proxy
        // SpaceID is irrelevant, we're doing our own spacial partitioning

        // WARNING: an ODE dPlane seems to barf badly when you setPosition on it! Use a box.
        // The other benefit here is that it reflects the width / height of the plane, not infinite
        dBox* odeBox = new dBox(0, mWidth*2, mHeight*2, 1); 
        mCollisionProxies.push_back(odeBox);
        updateCollisionProxies();



    }

}
