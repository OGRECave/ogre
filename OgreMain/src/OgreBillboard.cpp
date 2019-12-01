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
        mColour(ColourValue::White),
        mRotation(0),
        mParentSet(0)
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
        , mColour(colour)
        , mRotation(0)
        , mParentSet(owner)
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

