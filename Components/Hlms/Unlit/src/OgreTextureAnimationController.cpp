
#include "OgreStableHeaders.h"

#include "OgreTextureAnimationController.h"
#include "OgreMath.h"
#include "OgreHlmsUnlitDatablock.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    // TexCoordModifierControllerValue
    //-----------------------------------------------------------------------
    TextureAnimationControllerValue::TextureAnimationControllerValue( HlmsDatablock *datablock,
                                                                      uint8 textureUnit )
        : mTransU(false)
        , mTransV(false)
        , mScaleU(false)
        , mScaleV(false)
        , mRotate(false)
        , mTextureUnit(textureUnit)
        , mDatablock(datablock)
        , mUMod(0)
        , mVMod(0)
        , mUScale(1)
        , mVScale(1)
        , mRotation(0)
        , mTexModMatrix(Matrix4::IDENTITY)
        , mNumFramesHorizontal(0)
        , mNumFramesVertical(0)
        , mLastFrame(0)
        , mCurrentVerticalFrame(0)
    {
    }
    //-----------------------------------------------------------------------------------
    Real TextureAnimationControllerValue::getValue() const
    {
        if (mTransU)
        {
            return mTexModMatrix[0][3];
        }
        else if (mTransV)
        {
            return mTexModMatrix[1][3];
        }
        else if (mScaleU)
        {
            return mTexModMatrix[0][0];
        }
        else if (mScaleV)
        {
            return mTexModMatrix[1][1];
        }
        // Shouldn't get here
        return 0;
    }
    //-----------------------------------------------------------------------------------
    void TextureAnimationControllerValue::setValue(Real value)
    {
        if (mNumFramesHorizontal > 0) // tex tile animation
        {
            // The current animation frame.
            unsigned int currentFrame = ((int)(value * mNumFramesHorizontal) % mNumFramesHorizontal);

            if (mLastFrame == 0 && currentFrame == (mNumFramesHorizontal - 1u))
            {
                if (mCurrentVerticalFrame == (mNumFramesVertical - 1u))
                    mCurrentVerticalFrame = 0;
                else
                    mCurrentVerticalFrame++;
            }

            mUMod = (1 / (Real)mNumFramesHorizontal) * (Real)currentFrame;
            mVMod = (1 / (Real)mNumFramesVertical) * (Real)mCurrentVerticalFrame;

            mLastFrame = currentFrame;

            mUScale = mNumFramesHorizontal;
            mVScale = mNumFramesVertical;
        }
        else
        {
            if (mTransU)
                mUMod = value;

            if (mTransV)
                mVMod = value;

            if (mScaleU)
                mUScale = value;

            if (mScaleV)
                mVScale = value;

            if (mRotate)
                mRotation = Radian(value * Math::TWO_PI);
        }

        recalcTextureMatrix();
    }
    //-----------------------------------------------------------------------------------
    void TextureAnimationControllerValue::scaleAnimation(bool scaleU, bool scaleV)
    {
         mScaleU = scaleU;
         mScaleV = scaleV;
    }
    //-----------------------------------------------------------------------------------
    void TextureAnimationControllerValue::scrollAnimation(bool translateU, bool translateV)
    {
        mTransU = translateU;
        mTransV = translateV;
    }
    //-----------------------------------------------------------------------------------
    void TextureAnimationControllerValue::rotationAnimation(bool rotate)
    {
        mRotate = rotate;
    }
    //-----------------------------------------------------------------------------------
    void TextureAnimationControllerValue::tiledAnimation( uint16 numFramesHorizontal,
                                                          uint16 numFramesVertical )
    {
        mNumFramesHorizontal = numFramesHorizontal;
        mNumFramesVertical = numFramesVertical;
    }
    //-----------------------------------------------------------------------------------
    void TextureAnimationControllerValue::recalcTextureMatrix() const
    {
        // Assumption: 2D texture coords
        Matrix4 xform;

        xform = Matrix4::IDENTITY;
        if (mUScale != 1 || mVScale != 1)
        {
            // Offset to center of texture
            xform[0][0] = 1 / mUScale;
            xform[1][1] = 1 / mVScale;

            if (mNumFramesHorizontal == 0) // no tex tile animation
            {
                //// Skip matrix concat since first matrix update
                xform[0][3] = (-0.5f * xform[0][0]) + 0.5f;
                xform[1][3] = (-0.5f * xform[1][1]) + 0.5f;
            }
        }

        if (mUMod || mVMod)
        {
            Matrix4 xlate = Matrix4::IDENTITY;

            xlate[0][3] = mUMod;
            xlate[1][3] = mVMod;

            xform = xlate * xform;
        }

        if (mRotation != Radian(0))
        {
            Matrix4 rot = Matrix4::IDENTITY;
            Radian theta(mRotation);
            Real cosTheta = Math::Cos(theta);
            Real sinTheta = Math::Sin(theta);

            rot[0][0] = cosTheta;
            rot[0][1] = -sinTheta;
            rot[1][0] = sinTheta;
            rot[1][1] = cosTheta;
            // Offset center of rotation to center of texture
            rot[0][3] = 0.5f + ((-0.5f * cosTheta) - (-0.5f * sinTheta));
            rot[1][3] = 0.5f + ((-0.5f * sinTheta) + (-0.5f * cosTheta));

            xform = rot * xform;
        }

        mTexModMatrix = xform;

        HlmsUnlitDatablock *datablockUnlit = (HlmsUnlitDatablock*)mDatablock;
        datablockUnlit->setAnimationMatrix(mTextureUnit, mTexModMatrix);
    }
}
