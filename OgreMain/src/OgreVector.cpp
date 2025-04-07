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

namespace Ogre
{
namespace
{
constexpr Vector2 gVector2Zero( 0, 0 );
constexpr Vector2 gVector2UnitX( 1, 0);
constexpr Vector2 gVector2UnitY( 0, 1);
constexpr Vector2 gVector2NegativeUnitX( -1,  0);
constexpr Vector2 gVector2NegativeUnitY(  0, -1);
constexpr Vector2 gVector2UnitScale( 1, 1 );

constexpr Vector3 gVector3Zero( 0, 0, 0 );
constexpr Vector3 gVector3UnitX( 1, 0, 0 );
constexpr Vector3 gVector3UnitY( 0, 1, 0 );
constexpr Vector3 gVector3UnitZ( 0, 0, 1 );
constexpr Vector3 gVector3NegativeUnitX( -1,  0,  0 );
constexpr Vector3 gVector3NegativeUnitY(  0, -1,  0 );
constexpr Vector3 gVector3NegativeUnitZ(  0,  0, -1 );
constexpr Vector3 gVector3UnitScale( 1, 1, 1 );

constexpr Vector4 gVector4Zero( 0, 0, 0, 0 );
}  // namespace
const Vector2 &VectorBase<2, Real>::ZERO = gVector2Zero;
const Vector2 &VectorBase<2, Real>::UNIT_X = gVector2UnitX;
const Vector2 &VectorBase<2, Real>::UNIT_Y = gVector2UnitY;
const Vector2 &VectorBase<2, Real>::NEGATIVE_UNIT_X = gVector2NegativeUnitX;
const Vector2 &VectorBase<2, Real>::NEGATIVE_UNIT_Y = gVector2NegativeUnitY;
const Vector2 &VectorBase<2, Real>::UNIT_SCALE = gVector2UnitScale;

const Vector3 &VectorBase<3, Real>::ZERO = gVector3Zero;
const Vector3 &VectorBase<3, Real>::UNIT_X = gVector3UnitX;
const Vector3 &VectorBase<3, Real>::UNIT_Y = gVector3UnitY;
const Vector3 &VectorBase<3, Real>::UNIT_Z = gVector3UnitZ;
const Vector3 &VectorBase<3, Real>::NEGATIVE_UNIT_X = gVector3NegativeUnitX;
const Vector3 &VectorBase<3, Real>::NEGATIVE_UNIT_Y = gVector3NegativeUnitY;
const Vector3 &VectorBase<3, Real>::NEGATIVE_UNIT_Z = gVector3NegativeUnitZ;
const Vector3 &VectorBase<3, Real>::UNIT_SCALE = gVector3UnitScale;

const Vector4 &VectorBase<4, Real>::ZERO = gVector4Zero;
}
