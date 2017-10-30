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

namespace Ogre
{
    /** HOW THIS WORKS:

        Instead of writing like 12 times the same code, we use macros:
        DEFINE_OPERATION( ArrayVector3, ArrayVector3, +, _mm_add_ps );
        Means: "define '+' operator that takes both arguments as ArrayVector3 and use
        the _mm_add_ps intrinsic to do the job"

        Note that for scalars (i.e. floats) we use DEFINE_L_SCALAR_OPERATION/DEFINE_R_SCALAR_OPERATION
        depending on whether the scalar is on the left or right side of the operation
        (i.e. 2 * a vs a * 2)
        And for ArrayReal scalars we use DEFINE_L_OPERATION/DEFINE_R_OPERATION

        As for division, we use specific scalar versions to increase performance (calculate
        the inverse of the scalar once, then multiply) as well as placing asserts in
        case of trying to divide by zero.

        I've considered using templates, but decided against it because wrong operator usage
        would raise cryptic compile error messages, and templates inability to limit which
        types are being used, leave the slight possibility of mixing completely unrelated
        types (i.e. ArrayQuaternion + ArrayVector4) and quietly compiling wrong code.
        Furthermore some platforms may not have decent compilers handling templates for
        major operator usage.

        Advantages:
            * Increased readability
            * Ease of reading & understanding
        Disadvantages:
            * Debugger can't go inside the operator. See workaround.

        As a workaround to the disadvantage, you can compile this code using cl.exe /EP /P /C to
        generate this file with macros replaced as actual code (very handy!)
    */
    // Arithmetic operations
#define DEFINE_OPERATION( leftClass, rightClass, op )\
    inline ArrayVector3 operator op ( const leftClass &lhs, const rightClass &rhs )\
    {\
        const ArrayReal * RESTRICT_ALIAS lhsChunkBase = lhs.mChunkBase;\
        const ArrayReal * RESTRICT_ALIAS rhsChunkBase = rhs.mChunkBase;\
        return ArrayVector3(\
                lhsChunkBase[0] op rhsChunkBase[0],\
                lhsChunkBase[1] op rhsChunkBase[1],\
                lhsChunkBase[2] op rhsChunkBase[2] );\
       }
#define DEFINE_L_OPERATION( leftType, rightClass, op )\
    inline ArrayVector3 operator op ( const leftType fScalar, const rightClass &rhs )\
    {\
        return ArrayVector3(\
                fScalar op rhs.mChunkBase[0],\
                fScalar op rhs.mChunkBase[1],\
                fScalar op rhs.mChunkBase[2] );\
    }
#define DEFINE_R_OPERATION( leftClass, rightType, op )\
    inline ArrayVector3 operator op ( const leftClass &lhs, const rightType fScalar )\
    {\
        return ArrayVector3(\
                lhs.mChunkBase[0] op fScalar,\
                lhs.mChunkBase[1] op fScalar,\
                lhs.mChunkBase[2] op fScalar );\
    }

#define DEFINE_L_SCALAR_DIVISION( leftType, rightClass, op )\
    inline ArrayVector3 operator op ( const leftType fScalar, const rightClass &rhs )\
    {\
        return ArrayVector3(\
                fScalar op rhs.mChunkBase[0],\
                fScalar op rhs.mChunkBase[1],\
                fScalar op rhs.mChunkBase[2] );\
    }
#define DEFINE_R_SCALAR_DIVISION( leftClass, rightType, op, op_func )\
    inline ArrayVector3 operator op ( const leftClass &lhs, const rightType fScalar )\
    {\
        assert( fScalar != 0.0 );\
        Real fInv = 1.0f / fScalar;\
        return ArrayVector3(\
                lhs.mChunkBase[0] op_func fInv,\
                lhs.mChunkBase[1] op_func fInv,\
                lhs.mChunkBase[2] op_func fInv );\
    }

#ifdef NDEBUG
    #define ASSERT_DIV_BY_ZERO( values ) ((void)0)
#else
    #define ASSERT_DIV_BY_ZERO( values ) {\
                assert( values != 0 && "One of the 4 floats is a zero. Can't divide by zero" ); }
#endif

    // Update operations
#define DEFINE_UPDATE_OPERATION( leftClass, op, op_func )\
    inline void ArrayVector3::operator op ( const leftClass &a )\
    {\
        ArrayReal * RESTRICT_ALIAS chunkBase = mChunkBase;\
        const ArrayReal * RESTRICT_ALIAS aChunkBase = a.mChunkBase;\
        chunkBase[0] = chunkBase[0] op_func aChunkBase[0];\
        chunkBase[1] = chunkBase[1] op_func aChunkBase[1];\
        chunkBase[2] = chunkBase[2] op_func aChunkBase[2];\
    }
#define DEFINE_UPDATE_R_SCALAR_OPERATION( rightType, op, op_func )\
    inline void ArrayVector3::operator op ( const rightType fScalar )\
    {\
        mChunkBase[0] = mChunkBase[0] op_func fScalar;\
        mChunkBase[1] = mChunkBase[1] op_func fScalar;\
        mChunkBase[2] = mChunkBase[2] op_func fScalar;\
    }
#define DEFINE_UPDATE_DIVISION( leftClass, op, op_func )\
    inline void ArrayVector3::operator op ( const leftClass &a )\
    {\
        ArrayReal * RESTRICT_ALIAS chunkBase = mChunkBase;\
        const ArrayReal * RESTRICT_ALIAS aChunkBase = a.mChunkBase;\
        chunkBase[0] = chunkBase[0] op_func aChunkBase[0];\
        chunkBase[1] = chunkBase[1] op_func aChunkBase[1];\
        chunkBase[2] = chunkBase[2] op_func aChunkBase[2];\
    }
#define DEFINE_UPDATE_R_SCALAR_DIVISION( rightType, op, op_func )\
    inline void ArrayVector3::operator op ( const rightType fScalar )\
    {\
        assert( fScalar != 0.0 );\
        Real fInv = 1.0f / fScalar;\
        mChunkBase[0] = mChunkBase[0] op_func fInv;\
        mChunkBase[1] = mChunkBase[1] op_func fInv;\
        mChunkBase[2] = mChunkBase[2] op_func fInv;\
    }

    inline const ArrayVector3& ArrayVector3::operator + () const
    {
        return *this;
    };
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayVector3::operator - () const
    {
        return ArrayVector3( -mChunkBase[0], -mChunkBase[1], -mChunkBase[2] );
    }
    //-----------------------------------------------------------------------------------

    // + Addition
    DEFINE_OPERATION( ArrayVector3, ArrayVector3, + );
    DEFINE_L_OPERATION( Real, ArrayVector3, + );
    DEFINE_R_OPERATION( ArrayVector3, Real, + );

    // - Subtraction
    DEFINE_OPERATION( ArrayVector3, ArrayVector3, - );
    DEFINE_L_OPERATION( Real, ArrayVector3, - );
    DEFINE_R_OPERATION( ArrayVector3, Real, - );

    // * Multiplication
    DEFINE_OPERATION( ArrayVector3, ArrayVector3, * );
    DEFINE_L_OPERATION( Real, ArrayVector3, * );
    DEFINE_R_OPERATION( ArrayVector3, Real, * );

    // / Division (scalar versions use mul instead of div, because they mul against the reciprocal)
    DEFINE_OPERATION( ArrayVector3, ArrayVector3, / );
    DEFINE_L_SCALAR_DIVISION( Real, ArrayVector3, / );
    DEFINE_R_SCALAR_DIVISION( ArrayVector3, Real, /, * );

    inline ArrayVector3 ArrayVector3::Cmov4( const ArrayVector3 &arg1, const ArrayVector3 &arg2, ArrayMaskR mask )
    {
        return ArrayVector3(
                MathlibC::Cmov4( arg1.mChunkBase[0], arg2.mChunkBase[0], mask ),
                MathlibC::Cmov4( arg1.mChunkBase[1], arg2.mChunkBase[1], mask ),
                MathlibC::Cmov4( arg1.mChunkBase[2], arg2.mChunkBase[2], mask ) );
    }

    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------

    // Update operations
    // +=
    DEFINE_UPDATE_OPERATION(            ArrayVector3,       +=, + );
    DEFINE_UPDATE_R_SCALAR_OPERATION(   Real,               +=, + );

    // -=
    DEFINE_UPDATE_OPERATION(            ArrayVector3,       -=, - );
    DEFINE_UPDATE_R_SCALAR_OPERATION(   Real,               -=, - );

    // *=
    DEFINE_UPDATE_OPERATION(            ArrayVector3,       *=, * );
    DEFINE_UPDATE_R_SCALAR_OPERATION(   Real,               *=, * );

    // /=
    DEFINE_UPDATE_DIVISION(             ArrayVector3,       /=, / );
    DEFINE_UPDATE_R_SCALAR_DIVISION(    Real,               /=, * );

    //Functions
    //-----------------------------------------------------------------------------------
    inline ArrayReal ArrayVector3::length() const
    {
        return sqrt( (mChunkBase[0] * mChunkBase[0]) +
                        (mChunkBase[1] * mChunkBase[1]) +
                        (mChunkBase[2] * mChunkBase[2]) );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayReal ArrayVector3::squaredLength() const
    {
        return (mChunkBase[0] * mChunkBase[0]) +
                (mChunkBase[1] * mChunkBase[1]) +
                (mChunkBase[2] * mChunkBase[2]);
    }
    //-----------------------------------------------------------------------------------
    inline ArrayReal ArrayVector3::distance( const ArrayVector3& rhs ) const
    {
        return (*this - rhs).length();
    }
    //-----------------------------------------------------------------------------------
    inline ArrayReal ArrayVector3::squaredDistance( const ArrayVector3& rhs ) const
    {
        return (*this - rhs).squaredLength();
    }
    //-----------------------------------------------------------------------------------
    inline ArrayReal ArrayVector3::dotProduct( const ArrayVector3& vec ) const
    {
        return (mChunkBase[0] * vec.mChunkBase[0]) +
                (mChunkBase[1] * vec.mChunkBase[1]) +
                (mChunkBase[2] * vec.mChunkBase[2]);
    }
    //-----------------------------------------------------------------------------------
    inline ArrayReal ArrayVector3::absDotProduct( const ArrayVector3& vec ) const
    {
        return Math::Abs( (mChunkBase[0] * vec.mChunkBase[0]) ) +
                Math::Abs( (mChunkBase[1] * vec.mChunkBase[1]) ) +
                Math::Abs( (mChunkBase[2] * vec.mChunkBase[2]) );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayVector3::normalise( void )
    {
        ArrayReal sqLength = (mChunkBase[0] * mChunkBase[0]) +
                                (mChunkBase[1] * mChunkBase[1]) +
                                (mChunkBase[2] * mChunkBase[2]);

        //Convert sqLength's 0s into 1, so that zero vectors remain as zero
        //Denormals are treated as 0 during the check.
        //Note: We could create a mask now and nuke nans after InvSqrt, however
        //generating the nans could impact performance in some architectures
        sqLength = MathlibC::Cmov4( sqLength, 1.0f, sqLength > MathlibC::FLOAT_MIN );
        ArrayReal invLength = MathlibC::InvSqrtNonZero4( sqLength );
        mChunkBase[0] = mChunkBase[0] * invLength; //x * invLength
        mChunkBase[1] = mChunkBase[1] * invLength; //y * invLength
        mChunkBase[2] = mChunkBase[2] * invLength; //z * invLength
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayVector3::crossProduct( const ArrayVector3& rkVec ) const
    {
        return ArrayVector3(
                (mChunkBase[1] * rkVec.mChunkBase[2]) -
                (mChunkBase[2] * rkVec.mChunkBase[1]),
                (mChunkBase[2] * rkVec.mChunkBase[0]) -
                (mChunkBase[0] * rkVec.mChunkBase[2]),
                (mChunkBase[0] * rkVec.mChunkBase[1]) -
                (mChunkBase[1] * rkVec.mChunkBase[0]) );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayVector3::midPoint( const ArrayVector3& rkVec ) const
    {
        return ArrayVector3( (mChunkBase[0] + rkVec.mChunkBase[0]) * 0.5f,
                             (mChunkBase[1] + rkVec.mChunkBase[1]) * 0.5f,
                             (mChunkBase[2] + rkVec.mChunkBase[2]) * 0.5f );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayVector3::makeFloor( const ArrayVector3& cmp )
    {
        ArrayReal * RESTRICT_ALIAS aChunkBase = &mChunkBase[0];
        const ArrayReal * RESTRICT_ALIAS bChunkBase = &cmp.mChunkBase[0];
        aChunkBase[0] = Ogre::min( aChunkBase[0], bChunkBase[0] );
        aChunkBase[1] = Ogre::min( aChunkBase[1], bChunkBase[1] );
        aChunkBase[2] = Ogre::min( aChunkBase[2], bChunkBase[2] );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayVector3::makeCeil( const ArrayVector3& cmp )
    {
        ArrayReal * RESTRICT_ALIAS aChunkBase = &mChunkBase[0];
        const ArrayReal * RESTRICT_ALIAS bChunkBase = &cmp.mChunkBase[0];
        aChunkBase[0] = Ogre::max( aChunkBase[0], bChunkBase[0] );
        aChunkBase[1] = Ogre::max( aChunkBase[1], bChunkBase[1] );
        aChunkBase[2] = Ogre::max( aChunkBase[2], bChunkBase[2] );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayReal ArrayVector3::getMinComponent() const
    {
        return Ogre::min( mChunkBase[0], Ogre::min( mChunkBase[1], mChunkBase[2] ) );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayReal ArrayVector3::getMaxComponent() const
    {
        return Ogre::max( mChunkBase[0], Ogre::max( mChunkBase[1], mChunkBase[2] ) );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayVector3::setToSign()
    {
        mChunkBase[0] = mChunkBase[0] >= 0 ? 1.0f : -1.0f;
        mChunkBase[1] = mChunkBase[1] >= 0 ? 1.0f : -1.0f;
        mChunkBase[2] = mChunkBase[2] >= 0 ? 1.0f : -1.0f;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayVector3::perpendicular( void ) const
    {
        ArrayVector3 perp = this->crossProduct( ArrayVector3::UNIT_X );

        const ArrayMaskR mask = perp.squaredLength() <= MathlibC::fSqEpsilon;
        // Check length
        if( mask )
        {
            /* One or more of these vectors are the X axis multiplied by a scalar,
               so we have to use another axis for those.
            */
            ArrayVector3 perp1 = this->crossProduct( ArrayVector3::UNIT_Y );
            perp.mChunkBase[0] = MathlibC::Cmov4( perp1.mChunkBase[0], perp.mChunkBase[0], mask );
            perp.mChunkBase[1] = MathlibC::Cmov4( perp1.mChunkBase[1], perp.mChunkBase[1], mask );
            perp.mChunkBase[2] = MathlibC::Cmov4( perp1.mChunkBase[2], perp.mChunkBase[2], mask );
        }
        perp.normalise();

        return perp;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayVector3::normalisedCopy( void ) const
    {
        ArrayReal sqLength = (mChunkBase[0] * mChunkBase[0]) +
                                (mChunkBase[1] * mChunkBase[1]) +
                                (mChunkBase[2] * mChunkBase[2]);

        //Convert sqLength's 0s into 1, so that zero vectors remain as zero
        //Denormals are treated as 0 during the check.
        //Note: We could create a mask now and nuke nans after InvSqrt, however
        //generating the nans could impact performance in some architectures
        sqLength = MathlibC::Cmov4( sqLength, MathlibC::ONE, sqLength > MathlibC::FLOAT_MIN );
        ArrayReal invLength = MathlibC::InvSqrtNonZero4( sqLength );

        return ArrayVector3( mChunkBase[0] * invLength, //x * invLength
                             mChunkBase[1] * invLength, //y * invLength
                             mChunkBase[2] * invLength );   //z * invLength
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayVector3::reflect( const ArrayVector3& normal ) const
    {
        return ( *this - ( 2.0f * this->dotProduct( normal ) ) * normal );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayVector3::inverseLeaveZeroes( void )
    {
        mChunkBase[0] = MathlibC::CmovRobust( mChunkBase[0], MathlibC::InvNonZero4(mChunkBase[0]),
                                                mChunkBase[0] == 0.0f );
        mChunkBase[1] = MathlibC::CmovRobust( mChunkBase[1], MathlibC::InvNonZero4(mChunkBase[1]),
                                                mChunkBase[1] == 0.0f );
        mChunkBase[2] = MathlibC::CmovRobust( mChunkBase[2], MathlibC::InvNonZero4(mChunkBase[2]),
                                                mChunkBase[2] == 0.0f );
    }
    //-----------------------------------------------------------------------------------
    inline int ArrayVector3::isNaN( void ) const
    {
        return Math::isNaN( mChunkBase[0] ) | Math::isNaN( mChunkBase[1] ) | Math::isNaN( mChunkBase[2] );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayVector3::primaryAxis( void ) const
    {
        // We could've used some operators, i.e.
        // xVec = MathlibC::Cmov( ArrayVector3::UNIT_X, ArrayVector3::NEGATIVE_UNIT_X )
        // and so forth, which would've increased readability considerably. However,
        // MSVC's compiler ability to do constant propagation & remove dead code sucks,
        // which means it would try to cmov the Y & Z component even though we already
        // know it's always zero for both +x & -x. Therefore, we do it the manual
        // way. Doing this the "human readable way" results in massive amounts of wasted
        // instructions and stack memory abuse.
        // See Vector3::primaryAxis() to understand what's actually going on.
        ArrayReal absx = Math::Abs( mChunkBase[0] );
        ArrayReal absy = Math::Abs( mChunkBase[1] );
        ArrayReal absz = Math::Abs( mChunkBase[2] );

        //xVec = x > 0 ? Vector3::UNIT_X : Vector3::NEGATIVE_UNIT_X;
        ArrayReal sign = MathlibC::Cmov4( 1.0f, -1.0f, mChunkBase[0] > 0 );
        ArrayVector3 xVec( sign, 0, 0 );

        //yVec = y > 0 ? Vector3::UNIT_Y : Vector3::NEGATIVE_UNIT_Y;
        sign = MathlibC::Cmov4( 1.0f, -1.0f, mChunkBase[1] > 0 );
        ArrayVector3 yVec( 0, sign, 0 );

        //zVec = z > 0 ? Vector3::UNIT_Z : Vector3::NEGATIVE_UNIT_Z;
        sign = MathlibC::Cmov4( 1.0f, -1.0f, mChunkBase[2] > 0 );
        ArrayVector3 zVec( 0, 0, sign );

        //xVec = absx > absz ? xVec : zVec
        ArrayMaskR mask = absx > absz;
        xVec.mChunkBase[0] = MathlibC::Cmov4( xVec.mChunkBase[0], zVec.mChunkBase[0], mask );
        xVec.mChunkBase[2] = MathlibC::Cmov4( xVec.mChunkBase[2], zVec.mChunkBase[2], mask );

        //yVec = absy > absz ? yVec : zVec
        mask = absy > absz;
        yVec.mChunkBase[1] = MathlibC::Cmov4( yVec.mChunkBase[1], zVec.mChunkBase[1], mask );
        yVec.mChunkBase[2] = MathlibC::Cmov4( yVec.mChunkBase[2], zVec.mChunkBase[2], mask );

        yVec.Cmov4( absx > absy, xVec );
        return yVec;
    }
    //-----------------------------------------------------------------------------------
    inline Vector3 ArrayVector3::collapseMin( void ) const
    {
        return Vector3( mChunkBase[0], mChunkBase[1], mChunkBase[2] );
    }
    //-----------------------------------------------------------------------------------
    inline Vector3 ArrayVector3::collapseMax( void ) const
    {
        return Vector3( mChunkBase[0], mChunkBase[1], mChunkBase[2] );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayVector3::Cmov4( ArrayMaskR mask, const ArrayVector3 &replacement )
    {
        ArrayReal * RESTRICT_ALIAS aChunkBase = &mChunkBase[0];
        const ArrayReal * RESTRICT_ALIAS bChunkBase = &replacement.mChunkBase[0];
        aChunkBase[0] = MathlibC::Cmov4( aChunkBase[0], bChunkBase[0], mask );
        aChunkBase[1] = MathlibC::Cmov4( aChunkBase[1], bChunkBase[1], mask );
        aChunkBase[2] = MathlibC::Cmov4( aChunkBase[2], bChunkBase[2], mask );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayVector3::CmovRobust( ArrayMaskR mask, const ArrayVector3 &replacement )
    {
        ArrayReal * RESTRICT_ALIAS aChunkBase = &mChunkBase[0];
        const ArrayReal * RESTRICT_ALIAS bChunkBase = &replacement.mChunkBase[0];
        aChunkBase[0] = MathlibC::CmovRobust( aChunkBase[0], bChunkBase[0], mask );
        aChunkBase[1] = MathlibC::CmovRobust( aChunkBase[1], bChunkBase[1], mask );
        aChunkBase[2] = MathlibC::CmovRobust( aChunkBase[2], bChunkBase[2], mask );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayVector3::loadFromAoS( const Real * RESTRICT_ALIAS src )
    {
        mChunkBase[0] = src[0];
        mChunkBase[1] = src[1];
        mChunkBase[2] = src[2];
    }
    //-----------------------------------------------------------------------------------
    
#undef DEFINE_OPERATION
#undef DEFINE_L_OPERATION
#undef DEFINE_R_OPERATION
#undef DEFINE_L_SCALAR_DIVISION
#undef DEFINE_R_SCALAR_DIVISION

#undef DEFINE_UPDATE_OPERATION
#undef DEFINE_UPDATE_R_SCALAR_OPERATION
#undef DEFINE_UPDATE_DIVISION
#undef DEFINE_UPDATE_R_SCALAR_DIVISION

    //-----------------------------------------------------------------------------------
    // END CODE TO BE COPIED TO OgreArrayVector3.inl
    //-----------------------------------------------------------------------------------
}
