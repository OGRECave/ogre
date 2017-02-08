#pragma once

#include "OgreController.h"
#include "OgreMatrix4.h"

namespace Ogre {

	//-----------------------------------------------------------------------
	/** Predefined controller value for getting / setting a texture coordinate modifications (scales and translates).
	@remarks
	Effects can be applied to the scale or the offset of the u or v coordinates, or both. If separate
	modifications are required to u and v then 2 instances are required to control both independently, or 4
	if you want separate u and v scales as well as separate u and v offsets.
	@par
	Because of the nature of this value, it can accept values outside the 0..1 parametric range.
	*/
	class _OgreExport TexCoordModifierControllerValue : public Ogre::ControllerValue<Ogre::Real>
	{
	protected:
		bool mTransU, mTransV;
		bool mScaleU, mScaleV;
		bool mRotate;
		
		Ogre::HlmsDatablock *mdatablock;
		Ogre::uint8 mtextureUnit;

		Ogre::Real mUMod, mVMod;
		Ogre::Real mUScale, mVScale;
		Ogre::Radian mRotation;
		mutable Ogre::Matrix4 mTexModMatrix;

		//---- Tiled Texture
		int mNumFramesHorizontal, mNumFramesVertical;
		Ogre::uint16 mLastFrame, mCurrentVerticalFrame;

	public:
		/** Constructor.
		@param
		datablock HlmsDatablock to apply the modification to.
		@param
		textureUnit  to apply the modification to.
		*/
		TexCoordModifierControllerValue(Ogre::HlmsDatablock *datablock, Ogre::uint8 textureUnit);

		Ogre::Real getValue(void) const;

		void setValue(Ogre::Real value);

		/** scaleAnimation.
		@param
		scaleU If true, the u coordinates will be scaled by the modification.
		@param
		scaleV If true, the v coordinates will be scaled by the modification.
		*/
		void scaleAnimation(bool scaleU, bool scaleV);

		/** scrollAnimation.
		@param
		textureUnit  to apply the modification to.
		@param
		translateV If true, the v coordinates will be translated by the modification.
		*/
		void scrollAnimation(bool translateU, bool translateV);
		
		/** rotationAnimation.
		@param
		rotate If true, the texture will be rotated by the modification.
		*/
		void rotationAnimation(bool rotate);

		/** tiledAnimation.
		@param
		numFramesHorizontal number of horizontal tiles.
		@param
		numFramesVertical number of vertical tiles.
			*/
		void tiledAnimation(Ogre::uint16 numFramesHorizontal, Ogre::uint16 numFramesVertical);


	private:
		void recalcTextureMatrix() const;

	};

}