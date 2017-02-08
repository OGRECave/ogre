#include "OgreTextureAnimationController.h"
#include "OgreMath.h"
#include "OgreHlmsUnlitDatablock.h"

namespace Ogre {

	void TexCoordModifierControllerValue::recalcTextureMatrix() const
	{
		// Assumption: 2D texture coords
		Ogre::Matrix4 xform;

		xform = Ogre::Matrix4::IDENTITY;
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
			Ogre::Matrix4 xlate = Ogre::Matrix4::IDENTITY;

			xlate[0][3] = mUMod;
			xlate[1][3] = mVMod;

			xform = xlate * xform;
		}

		if (mRotation != Ogre::Radian(0))
		{
			Ogre::Matrix4 rot = Ogre::Matrix4::IDENTITY;
			Ogre::Radian theta(mRotation);
			Ogre::Real cosTheta = Ogre::Math::Cos(theta);
			Ogre::Real sinTheta = Ogre::Math::Sin(theta);

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
		
		Ogre::HlmsUnlitDatablock *datablockUnlit = (Ogre::HlmsUnlitDatablock*)mdatablock;
		datablockUnlit->setAnimationMatrix(mtextureUnit, mTexModMatrix);
	}

	//-----------------------------------------------------------------------
	// TexCoordModifierControllerValue
	//-----------------------------------------------------------------------
	TexCoordModifierControllerValue::TexCoordModifierControllerValue(Ogre::HlmsDatablock *datablock, Ogre::uint8 textureUnit)
		: mdatablock(datablock)
		, mtextureUnit(textureUnit)
		, mTransU(false)
		, mTransV(false)
		, mScaleU(false)
		, mScaleV(false)
		, mRotate(false)
		, mUMod(0)
		, mVMod(0)
		, mUScale(1)
		, mVScale(1)
		, mRotation(0)
		, mTexModMatrix(Ogre::Matrix4::IDENTITY)
		, mLastFrame(0)
		, mNumFramesHorizontal(0)
		, mNumFramesVertical(0)
		, mCurrentVerticalFrame(0)
	{

	}

	//-----------------------------------------------------------------------
	Ogre::Real TexCoordModifierControllerValue::getValue() const
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
	//-----------------------------------------------------------------------
	void TexCoordModifierControllerValue::setValue(Ogre::Real value)
	{
		if (mNumFramesHorizontal > 0) // tex tile animation
		{
			unsigned int currentFrame = ((int)(value * mNumFramesHorizontal) % mNumFramesHorizontal); // The current animation frame.

			if (mLastFrame == 0 && currentFrame == (mNumFramesHorizontal - 1))
			{
				if (mCurrentVerticalFrame == (mNumFramesVertical - 1))
					mCurrentVerticalFrame = 0;
				else
					mCurrentVerticalFrame++;
			}

			mUMod = (1 / (Ogre::Real)mNumFramesHorizontal) * (Ogre::Real)currentFrame;
			mVMod = (1 / (Ogre::Real)mNumFramesVertical) * (Ogre::Real)mCurrentVerticalFrame;

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
				mRotation = Ogre::Radian(value * Ogre::Math::TWO_PI);
		}

		recalcTextureMatrix();
	}


	void TexCoordModifierControllerValue::scaleAnimation(bool scaleU, bool scaleV)
	{
		 mScaleU = scaleU;
		 mScaleV = scaleV;
	}

	void TexCoordModifierControllerValue::scrollAnimation(bool translateU, bool translateV)
	{
		mTransU = translateU;
		mTransV = translateV;
	}

	void TexCoordModifierControllerValue::rotationAnimation(bool rotate)
	{
		mRotate = rotate;
	}

	void TexCoordModifierControllerValue::tiledAnimation(Ogre::uint16 numFramesHorizontal, Ogre::uint16 numFramesVertical)
	{
		mNumFramesHorizontal = numFramesHorizontal;
		mNumFramesVertical = numFramesVertical;
	}
}