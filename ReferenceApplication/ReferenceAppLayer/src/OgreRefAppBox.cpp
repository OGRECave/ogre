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
#include "OgreRefAppBox.h"
#include "OgreRefAppWorld.h"

namespace OgreRefApp
{

    //-------------------------------------------------------------------------
    Box::Box(const String& name, Real width, Real height, Real depth) : ApplicationObject(name)
    {
        mDimensions.x = width;
        mDimensions.y = height;
        mDimensions.z = depth;
        setUp(name);
    }
    //-------------------------------------------------------------------------
    Box::~Box()
    {

    }
    //-------------------------------------------------------------------------
    void Box::setUp(const String& name)
    {
        // Create visual presence
        SceneManager* sm = World::getSingleton().getSceneManager();
        mEntity = sm->createEntity(name, "cube.mesh");
        mSceneNode = sm->getRootSceneNode()->createChildSceneNode(name);
        // Scale down, default size is 100x100x100
        mSceneNode->scale(mDimensions.x / 100.f, 
            mDimensions.y / 100.f, mDimensions.z / 100.f);

        mSceneNode->attachObject(mEntity);
        // Add reverse reference
        mEntity->setUserObject(this);

        // Create mass body
        mOdeBody = new dBody(World::getSingleton().getOdeWorld()->id());
        // Set reverse reference
        mOdeBody->setData(this);
        // Set mass 
        setMassBox(0.05, mDimensions);

        this->setBounceParameters(0.0, 0.0);
        this->setSoftness(0.0f);
        this->setFriction(Math::POS_INFINITY);

        // Create collision proxy
        dBox* odeBox = new dBox(0, mDimensions.x, mDimensions.y, mDimensions.z);
        mCollisionProxies.push_back(odeBox);
        updateCollisionProxies();


    }


}
