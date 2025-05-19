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

namespace Ogre {

    //---------------------------------------------------------------------
    Bone::Bone(unsigned short handle, Skeleton* creator) 
        : Node(), mCreator(creator), mHandle(handle), mManuallyControlled(false)
    {
    }
    //---------------------------------------------------------------------
    Bone::Bone(const String& name, unsigned short handle, Skeleton* creator) 
        : Node(name), mCreator(creator), mHandle(handle), mManuallyControlled(false)
    {
    }
    //---------------------------------------------------------------------
    Bone::~Bone()
    {
    }
    //---------------------------------------------------------------------
    Bone* Bone::createChild(unsigned short handle, const Vector3& inTranslate, 
        const Quaternion& inRotate)
    {
        Bone* retBone = mCreator->createBone(handle);
        retBone->translate(inTranslate);
        retBone->rotate(inRotate);
        this->addChild(retBone);
        return retBone;
    }
    //---------------------------------------------------------------------
    Node* Bone::createChildImpl(void)
    {
        return mCreator->createBone();
    }
    //---------------------------------------------------------------------
    Node* Bone::createChildImpl(const String& name)
    {
        return mCreator->createBone(name);
    }
    //---------------------------------------------------------------------
    void Bone::setBindingPose(void)
    {
        setInitialState();

        // Save inverse derived position/scale/orientation, used for calculate offset transform later
        mBindDerivedInversePosition = - _getDerivedPosition();
        mBindDerivedInverseScale = Vector3::UNIT_SCALE / _getDerivedScale();
        mBindDerivedInverseOrientation = _getDerivedOrientation().Inverse();
    }
    //---------------------------------------------------------------------
    void Bone::reset(void)
    {
        resetToInitialState();
    }
    //---------------------------------------------------------------------
    void Bone::setManuallyControlled(bool manuallyControlled) 
    {
        mManuallyControlled = manuallyControlled;
        mCreator->_notifyManualBoneStateChange(this);
    }
    //---------------------------------------------------------------------
    void Bone::_getOffsetTransform(Affine3& m) const
    {
        // Combine scale with binding pose inverse scale,
        // NB just combine as equivalent axes, no shearing
        Vector3 locScale = _getDerivedScale() * mBindDerivedInverseScale;

        // Combine orientation with binding pose inverse orientation
        Quaternion locRotate = _getDerivedOrientation() * mBindDerivedInverseOrientation;

        // Combine position with binding pose inverse position,
        // Note that translation is relative to scale & rotation,
        // so first reverse transform original derived position to
        // binding pose bone space, and then transform to current
        // derived bone space.
        Vector3 locTranslate = _getDerivedPosition() + locRotate * (locScale * mBindDerivedInversePosition);

        m.makeTransform(locTranslate, locScale, locRotate);
    }
    //---------------------------------------------------------------------
    void Bone::needUpdate(bool forceParentUpdate)
    {
        Node::needUpdate(forceParentUpdate);

        if (isManuallyControlled())
        {
            // Dirty the skeleton if manually controlled so animation can be updated
            mCreator->_notifyManualBonesDirty();
        }

    }





}

