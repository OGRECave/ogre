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
#include "OgreVector.h"
#include "OgreMatrix4.h"

using namespace Ogre;

//--------------------------------------------------------------------------
TEST(VectorTests,Vector2Scaler)
{
    EXPECT_EQ(Vector2(1, 1) + Vector2(2, 2), Vector2(3, 3));
    EXPECT_EQ(1 + Vector2(2), Vector2(3, 3));
    Vector2 v1;
    v1 = Vector2(1);
    EXPECT_EQ(v1, Vector2(1, 1));
    v1 = Vector2(0.0);
    EXPECT_EQ(v1, Vector2::ZERO);
    v1 += 3;
    EXPECT_EQ(v1, Vector2(3, 3));

    v1 = 3 - Vector2(2);
    EXPECT_EQ(v1, Vector2(1));
    v1 = Vector2(5) - 7;
    EXPECT_EQ(v1, Vector2(-2));
    v1 -= 4;
    EXPECT_EQ(v1, Vector2(-6));
}
//--------------------------------------------------------------------------
TEST(VectorTests,Vector3Scaler)
{
    EXPECT_EQ(Vector3(1, 1, 1) + Vector3(2, 2, 2), Vector3(3, 3, 3));
    EXPECT_EQ(1 + Vector3(2), Vector3(3, 3, 3));
    Vector3 v1;
    v1 = Vector3(1);
    EXPECT_EQ(v1, Vector3(1));
    v1 = Vector3(0.0);
    EXPECT_EQ(v1, Vector3::ZERO);
    v1 += 3;
    EXPECT_EQ(v1, Vector3(3));

    v1 = 3 - Vector3(2);
    EXPECT_EQ(v1, Vector3(1));
    v1 = Vector3(5) - 7;
    EXPECT_EQ(v1, Vector3(-2));
    v1 -= 4;
    EXPECT_EQ(v1, Vector3(-6));
}
//--------------------------------------------------------------------------
TEST(VectorTests,Vector4Scaler)
{
    EXPECT_EQ(Vector4(1, 1, 1, 1) + Vector4(2, 2, 2, 2), Vector4(3, 3, 3, 3));
    EXPECT_EQ(1 + Vector4(2, 2, 2, 2), Vector4(3, 3, 3, 3));
    Vector4 v1;
    v1 = Vector4(1);
    EXPECT_EQ(v1, Vector4(1, 1, 1, 1));
    v1 = Vector4(0.0);
    EXPECT_EQ(v1, Vector4(0,0,0,0));
    v1 += 3;
    EXPECT_EQ(v1, Vector4(3,3,3,3));

    v1 = 3 - Vector4(2,2,2,2);
    EXPECT_EQ(v1, Vector4(1,1,1,1));
    v1 = Vector4(5,5,5,5) - 7;
    EXPECT_EQ(v1, Vector4(-2,-2,-2,-2));
    v1 -= 4;
    EXPECT_EQ(v1, Vector4(-6,-6,-6,-6));
}
//--------------------------------------------------------------------------
TEST(VectorTests,PerpendicularVector)
{
    Vector3 v1(1, 0, 0);
    Vector3 v2(0, 1, 0);
    Vector3 v3(0, 0, 1);

    EXPECT_EQ(v1.dotProduct(v1.perpendicular()), (Real)0);
    EXPECT_EQ(v2.dotProduct(v2.perpendicular()), (Real)0);
    EXPECT_EQ(v3.dotProduct(v3.perpendicular()), (Real)0);

    EXPECT_EQ((v1 + v2).dotProduct((v1 + v2).perpendicular()), (Real)0);
    EXPECT_EQ((v2 + v3).dotProduct((v2 + v3).perpendicular()), (Real)0);
    EXPECT_EQ((v3 + v1).dotProduct((v3 + v1).perpendicular()), (Real)0);
}
//--------------------------------------------------------------------------
TEST(VectorTests, Matrix4ArrayLoading)
{
    Real arr0[16] = { 1, 0, 0, 0,
                      0, 1, 0, 0,
                      0, 0, 1, 0,
                      0, 0, 0, 1 };
    
    Matrix4 mat0(arr0);
    
    EXPECT_EQ(1, mat0[0][0]);
    EXPECT_EQ(1, mat0[1][1]);
    EXPECT_EQ(1, mat0[2][2]);
    EXPECT_EQ(1, mat0[3][3]);
    
    Real arr1[16] = { 0,    1,   2,   3,
                      4,    5,   6,   7,
                      8,    9,  10,  11,
                      12,  13,  14,  15};

    Matrix4 mat1(arr1);

    EXPECT_EQ(0, mat1[0][0]);
    EXPECT_EQ(1, mat1[0][1]);
    EXPECT_EQ(2, mat1[0][2]);
    EXPECT_EQ(3, mat1[0][3]);

    EXPECT_EQ(4, mat1[1][0]);
    EXPECT_EQ(5, mat1[1][1]);
    EXPECT_EQ(6, mat1[1][2]);
    EXPECT_EQ(7, mat1[1][3]);

    EXPECT_EQ(8, mat1[2][0]);
    EXPECT_EQ(9, mat1[2][1]);
    EXPECT_EQ(10, mat1[2][2]);
    EXPECT_EQ(11, mat1[2][3]);

    EXPECT_EQ(12, mat1[3][0]);
    EXPECT_EQ(13, mat1[3][1]);
    EXPECT_EQ(14, mat1[3][2]);
    EXPECT_EQ(15, mat1[3][3]);
}
//--------------------------------------------------------------------------
TEST(VectorTests, Matrix3SVD)
{
    Matrix3 U, V, mat;
    Vector3 w;

    // bidiagonal matrix
    Matrix3 ref(3, 1, 0,
                0, 2, 0,
                0, 0, 1);

    ref.SingularValueDecomposition(U, w, V);
    mat.SingularValueComposition(U, w, V);

    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            EXPECT_NEAR(ref[i][j], mat[i][j], Matrix3::EPSILON);
}
//--------------------------------------------------------------------------
TEST(VectorTests, Matrix3QDU)
{
    Matrix3 linear = Matrix3::IDENTITY * -2; // some scaling and reflection
    Matrix3 rot;
    Vector3 scale, shear;

    linear.QDUDecomposition(rot, scale, shear);

    Matrix3 ref_rot = Matrix3::IDENTITY;
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            EXPECT_FLOAT_EQ(ref_rot[i][j], rot[i][j]);

    for(int i = 0; i < 3; i++)
        EXPECT_FLOAT_EQ(scale[i], -2);
}
//--------------------------------------------------------------------------
TEST(VectorTests, TransformBaseArrayLoading)
{
    typedef TransformBase<3, float> Matrix3x4f;

    double arr1[12] = { 0,    1,   2,   3,
                        4,    5,   6,   7,
                        8,    9,  10,  11};

    Matrix3x4f mat1(arr1);

    EXPECT_EQ(0, mat1[0][0]);
    EXPECT_EQ(1, mat1[0][1]);
    EXPECT_EQ(2, mat1[0][2]);
    EXPECT_EQ(3, mat1[0][3]);

    EXPECT_EQ(4, mat1[1][0]);
    EXPECT_EQ(5, mat1[1][1]);
    EXPECT_EQ(6, mat1[1][2]);
    EXPECT_EQ(7, mat1[1][3]);

    EXPECT_EQ(8, mat1[2][0]);
    EXPECT_EQ(9, mat1[2][1]);
    EXPECT_EQ(10, mat1[2][2]);
    EXPECT_EQ(11, mat1[2][3]);
}
//--------------------------------------------------------------------------
TEST(VectorTests, TypeCasts)
{
    int arr[16] = { 0 };

    Affine3 affine(arr);
    Matrix4 matrix(arr);

    EXPECT_EQ(affine, Affine3::ZERO);
    EXPECT_EQ(matrix, Matrix4::ZERO);

    typedef TransformBase<4, int> Matrix4i; // something that is neither float nor double
    Matrix4i imat;

    Vector3i vec(1, 2, 3);
    imat.setTrans(vec);

    EXPECT_EQ(imat.getTrans(), vec);
}
