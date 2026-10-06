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
#include "OgreAnimable.h"

namespace Ogre {
    void AnimableValue::resetToBaseValue(void)
    {
        switch(mType)
        {
        case INT:
            setValue(mBaseValueInt);
            break;
        case REAL:
            setValue(mBaseValueReal[0]);
            break;
        case VECTOR2:
            setValue(Vector2(mBaseValueReal));
            break;
        case VECTOR3:
            setValue(Vector3(mBaseValueReal));
            break;
        case VECTOR4:
            setValue(Vector4(mBaseValueReal));
            break;
        case QUATERNION:
            setValue(Quaternion(mBaseValueReal));
            break;
        case COLOUR:
            setValue(ColourValue(mBaseValueReal[0], mBaseValueReal[1],
                mBaseValueReal[2], mBaseValueReal[3]));
            break;
        case RADIAN:        
            setValue(Radian(mBaseValueReal[0]));
            break;
        case CUSTOM:
        default:
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Override this function to provide custom behavior");
        }
    }
    //--------------------------------------------------------------------------
    void AnimableValue::setAsBaseValue(const Any& val)
    {
        switch(mType)
        {
        case INT:
            setAsBaseValue(any_cast<int>(val));
            break;
        case REAL:
            setAsBaseValue(any_cast<Real>(val));
            break;
        case VECTOR2:
            setAsBaseValue(any_cast<Vector2>(val));
            break;
        case VECTOR3:
            setAsBaseValue(any_cast<Vector3>(val));
            break;
        case VECTOR4:
            setAsBaseValue(any_cast<Vector4>(val));
            break;
        case QUATERNION:
            setAsBaseValue(any_cast<Quaternion>(val));
            break;
        case COLOUR:
            setAsBaseValue(any_cast<ColourValue>(val));
            break;
        case RADIAN:
            setAsBaseValue(any_cast<Radian>(val));
            break;
        case CUSTOM:
        default:
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Override this function to provide custom behavior");
        }
    }
    //--------------------------------------------------------------------------
    void AnimableValue::setValue(const Any& val)
    {
        switch(mType)
        {
        case INT:
            setValue(any_cast<int>(val));
            break;
        case REAL:
            setValue(any_cast<Real>(val));
            break;
        case VECTOR2:
            setValue(any_cast<Vector2>(val));
            break;
        case VECTOR3:
            setValue(any_cast<Vector3>(val));
            break;
        case VECTOR4:
            setValue(any_cast<Vector4>(val));
            break;
        case QUATERNION:
            setValue(any_cast<Quaternion>(val));
            break;
        case COLOUR:
            setValue(any_cast<ColourValue>(val));
            break;
        case RADIAN:
            setValue(any_cast<Radian>(val));
            break;
        case CUSTOM:
        default:
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Override this function to provide custom behavior");
        }
    }
    //--------------------------------------------------------------------------
    void AnimableValue::applyDeltaValue(const Any& val)
    {
        switch(mType)
        {
        case INT:
            applyDeltaValue(any_cast<int>(val));
            break;
        case REAL:
            applyDeltaValue(any_cast<Real>(val));
            break;
        case VECTOR2:
            applyDeltaValue(any_cast<Vector2>(val));
            break;
        case VECTOR3:
            applyDeltaValue(any_cast<Vector3>(val));
            break;
        case VECTOR4:
            applyDeltaValue(any_cast<Vector4>(val));
            break;
        case QUATERNION:
            applyDeltaValue(any_cast<Quaternion>(val));
            break;
        case COLOUR:
            applyDeltaValue(any_cast<ColourValue>(val));
            break;
        case RADIAN:
            applyDeltaValue(any_cast<Radian>(val));
            break;
        case CUSTOM:
        default:
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Override this function to provide custom behavior");
        }
    }
    //--------------------------------------------------------------------------
    Any AnimableValue::lerpAny(const Any& v0, const Any& v1, Real t)
    {
        switch(mType)
        {
        case INT:
            return Math::lerp(any_cast<int>(v0), any_cast<int>(v1), t);
        case REAL:
            return Math::lerp(any_cast<Real>(v0), any_cast<Real>(v1), t);
        case VECTOR2:
            return Math::lerp(any_cast<Vector2>(v0), any_cast<Vector2>(v1), t);
        case VECTOR3:
            return Math::lerp(any_cast<Vector3>(v0), any_cast<Vector3>(v1), t);
        case VECTOR4:
            return Math::lerp(any_cast<Vector4>(v0), any_cast<Vector4>(v1), t);
        case QUATERNION:
            return Math::lerp(any_cast<Quaternion>(v0), any_cast<Quaternion>(v1), t);
        case COLOUR:
            return Math::lerp(any_cast<ColourValue>(v0), any_cast<ColourValue>(v1), t);
        case RADIAN:
            return Math::lerp(any_cast<Radian>(v0), any_cast<Radian>(v1), t);
        case CUSTOM:
        default:
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Override this function to provide custom behavior");
        }
    }
    //--------------------------------------------------------------------------
    Any AnimableValue::scaleAny(const Any& v, Real s)
    {
        switch(mType)
        {
        case INT:
            return any_cast<int>(v) * s;
        case REAL:
            return any_cast<Real>(v) * s;
        case VECTOR2:
            return any_cast<Vector2>(v) * s;
        case VECTOR3:
            return any_cast<Vector3>(v) * s;
        case VECTOR4:
            return any_cast<Vector4>(v) * s;
        case QUATERNION:
            return any_cast<Quaternion>(v) * s;
        case COLOUR:
            return any_cast<ColourValue>(v) * s;
        case RADIAN:
            return any_cast<Radian>(v) * s;
        case CUSTOM:
        default:
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Override this function to provide custom behavior");
        }
    }
}
