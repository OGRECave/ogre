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
#include "OgreStableHeaders.h"

#include "OgreBillboard.h"

#include "OgreBillboardSet.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    Billboard::Billboard():
        mOwnDimensions(false),
        mUseTexcoordRect(false),
        mTexcoordIndex(0),
		mPosition(Vector3::ZERO),
        mDirection(Vector3::ZERO),        
        mParentSet(0),
        mColour(ColourValue::White),
		mRotation(0)
    {
    }
    //-----------------------------------------------------------------------
    Billboard::~Billboard()
    {
    }
    //-----------------------------------------------------------------------
    Billboard::Billboard(const Vector3& position, BillboardSet* owner, const ColourValue& colour)
        : mOwnDimensions(false)
        , mUseTexcoordRect(false)
        , mTexcoordIndex(0)
        , mPosition(position)
        , mDirection(Vector3::ZERO)
        , mParentSet(owner)
        , mColour(colour)
        , mRotation(0)
    {
    }
    //-----------------------------------------------------------------------
    void Billboard::setRotation(const Radian& rotation)
    {
        mRotation = rotation;
        if (mRotation != Radian(0))
            mParentSet->_notifyBillboardRotated();
    }
    //-----------------------------------------------------------------------
    void Billboard::setPosition(const Vector3& position)
    {
        mPosition = position;
    }
    //-----------------------------------------------------------------------
    void Billboard::setPosition(Real x, Real y, Real z)
    {
        mPosition.x = x;
        mPosition.y = y;
        mPosition.z = z;
    }
    //-----------------------------------------------------------------------
    const Vector3& Billboard::getPosition(void) const
    {
        return mPosition;
    }
    //-----------------------------------------------------------------------
    void Billboard::setDimensions(Real width, Real height)
    {
        mOwnDimensions = true;
        mWidth = width;
        mHeight = height;
        mParentSet->_notifyBillboardResized();
    }
    //-----------------------------------------------------------------------
    bool Billboard::hasOwnDimensions(void) const
    {
        return mOwnDimensions;
    }
    //-----------------------------------------------------------------------
    void Billboard::_notifyOwner(BillboardSet* owner)
    {
        mParentSet = owner;
    }
    //-----------------------------------------------------------------------
    void Billboard::setColour(const ColourValue& colour)
    {
        mColour = colour;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Billboard::getColour(void) const
    {
        return mColour;
    }
    //-----------------------------------------------------------------------
    Real Billboard::getOwnWidth(void) const
    {
        return mWidth;
    }
    //-----------------------------------------------------------------------
    Real Billboard::getOwnHeight(void) const
    {
        return mHeight;
    }
    //-----------------------------------------------------------------------
    void Billboard::setTexcoordIndex(uint16 texcoordIndex)
    {
        mTexcoordIndex = texcoordIndex;
        mUseTexcoordRect = false;
    }
    //-----------------------------------------------------------------------
    void Billboard::setTexcoordRect(const FloatRect& texcoordRect)
    {
        mTexcoordRect = texcoordRect;
        mUseTexcoordRect = true;
    }
    //-----------------------------------------------------------------------
    void Billboard::setTexcoordRect(Real u0, Real v0, Real u1, Real v1)
    {
        setTexcoordRect(FloatRect(u0, v0, u1, v1));
    }


}

