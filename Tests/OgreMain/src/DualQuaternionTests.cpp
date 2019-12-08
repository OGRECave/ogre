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
#include <gtest/gtest.h>
#include "OgreDualQuaternion.h"
#include "OgreVector.h"
#include "OgreMatrix4.h"


using namespace Ogre;

//--------------------------------------------------------------------------
TEST(DualQuaternionTests,Conversion)
{
    DualQuaternion dQuat;
    Quaternion quat(Radian(Degree(60)), Vector3::UNIT_Y);
    Vector3 translation(0, 0, 10);
    dQuat.fromRotationTranslation(quat, translation);
        
    Quaternion result;
    Vector3 resTrans;
    dQuat.toRotationTranslation(result, resTrans);

    EXPECT_EQ(result, quat);
    EXPECT_TRUE(resTrans.positionEquals(translation));
}
//--------------------------------------------------------------------------
TEST(DualQuaternionTests,DefaultValue)
{
    DualQuaternion dQuatDefault;
    
    Quaternion quatDefault;
    Vector3 transDefault;
    
    dQuatDefault.toRotationTranslation(quatDefault, transDefault);

    EXPECT_EQ(quatDefault, Quaternion::IDENTITY); 
    EXPECT_TRUE(transDefault.positionEquals(Vector3::ZERO));
}
//--------------------------------------------------------------------------
TEST(DualQuaternionTests,Matrix)
{
    Affine3 transform;
    Vector3 translation(10, 4, 0);
    Vector3 scale = Vector3::UNIT_SCALE;
    Quaternion rotation;
    rotation.FromAngleAxis(Radian(Math::PI), Vector3::UNIT_Z);
    transform.makeTransform(translation, scale, rotation);

    DualQuaternion dQuat;
    dQuat.fromTransformationMatrix(transform);
    Affine3 transformResult;
    dQuat.toTransformationMatrix(transformResult);

    Vector3 translationResult;
    Vector3 scaleResult;
    Quaternion rotationResult;
    transformResult.decomposition(translationResult, scaleResult, rotationResult);

    EXPECT_TRUE(translationResult.positionEquals(translation));
    EXPECT_TRUE(scaleResult.positionEquals(scale));
    EXPECT_TRUE(rotationResult.equals(rotation, Radian(0.001)));
}
//--------------------------------------------------------------------------
