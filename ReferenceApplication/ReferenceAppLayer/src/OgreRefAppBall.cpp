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
#include "OgreRefAppBall.h"
#include "OgreRefAppWorld.h"

namespace OgreRefApp
{

    //-------------------------------------------------------------------------
    Ball::Ball(const String& name, Real radius) : ApplicationObject(name)
    {
        mRadius = radius;
        setUp(name);
    }
    //-------------------------------------------------------------------------
    Ball::~Ball()
    {

    }
    //-------------------------------------------------------------------------
    void Ball::setUp(const String& name)
    {
        // Create visual presence
        SceneManager* sm = World::getSingleton().getSceneManager();
        mEntity = sm->createEntity(name, "sphere.mesh");
        mSceneNode = sm->getRootSceneNode()->createChildSceneNode(name);
        // Scale down, default size is 100
        Real scale = mRadius / 100.0f;
        mSceneNode->scale(scale, scale, scale);

        mSceneNode->attachObject(mEntity);
        // Add reverse reference
        mEntity->setUserObject(this);

        // Create mass body
        mOdeBody = new dBody(World::getSingleton().getOdeWorld()->id());
        // Set reverse reference
        mOdeBody->setData(this);
        // Set mass 
        setMassSphere(0.1, mRadius); // TODO change to more realistic values

        this->setBounceParameters(0.7, 0.1);
        this->setSoftness(0.0f);
        this->setFriction(Math::POS_INFINITY);

        // Create collision proxy
        dSphere* odeSphere = new dSphere(0, mRadius);
        mCollisionProxies.push_back(odeSphere);
        updateCollisionProxies();


    }


}
