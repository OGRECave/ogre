/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "TextureUnitController.h"

#include "OgreTextureUnitState.h"

#include "PassController.h"

TextureUnitController::TextureUnitController()
: mParentController(NULL), mTextureUnit(NULL)
{
	registerEvents();
}

TextureUnitController::TextureUnitController(TextureUnitState* tus)
: mParentController(NULL), mTextureUnit(tus)
{
	registerEvents();
}

TextureUnitController::TextureUnitController(PassController* parent, TextureUnitState* tus)
: mParentController(parent), mTextureUnit(tus)
{
	registerEvents();
}

TextureUnitController::~TextureUnitController()
{
}

void TextureUnitController::registerEvents()
{
	registerEvent(TextureNameChanged);
	registerEvent(CubicTextureNameChanged);
	registerEvent(AnimatedTextureNameChanged);
	registerEvent(TextureDimensionsChanged);
	registerEvent(CurrentFrameChanged);
	registerEvent(FrameTextureNameChanged);
	registerEvent(FrameTextureNameAdded);
	registerEvent(FrameTextureNameRemoved);
	registerEvent(BindingTypeChanged);
	registerEvent(ContentTypeChanged);
	registerEvent(PixelFormatChanged);
	registerEvent(NumMipmapsChanged);
	registerEvent(AlphaChanged);
	registerEvent(TextureCoordSetChanged);
	registerEvent(TextureTransformChanged);
	registerEvent(TextureScrollChanged);
	registerEvent(TextureUScrollChanged);
	registerEvent(TextureVScrollChanged);
	registerEvent(TextureUScaleChanged);
	registerEvent(TextureVScaleChanged);
	registerEvent(TextureRotateChanged);
	registerEvent(TextureAddressingModeChanged);
	registerEvent(TextureBorderColourChanged);
	registerEvent(ColourOperationChanged);
	registerEvent(ColourOperationExChanged);
	registerEvent(ColourOpMultipassFallbackChanged);
	registerEvent(AlphaOperationChanged);
	registerEvent(EffectAdded);
	registerEvent(EnvironmentMapChanged);
	registerEvent(ScrollAnimationChanged);
	registerEvent(RotateAnimationChanged);
	registerEvent(TransformAnimationChanged);
	registerEvent(ProjectiveTexturingChanged);
	registerEvent(EffectRemoved);
	registerEvent(Blanked);
	registerEvent(TextureFilteringChanged);
	registerEvent(TextureAnisotropyChanged);
	registerEvent(TextureMipMapBiasChanged);
	registerEvent(NameChanged);
	registerEvent(TextureNameAliasChanged);
}


PassController* TextureUnitController::getParentController() const
{
	return mParentController;
}

TextureUnitState* TextureUnitController::getTextureUnit() const
{
	return mTextureUnit;
}

void TextureUnitController::setTextureName(const String &name, TextureType ttype /* = TEX_TYPE_2D */)
{
	mTextureUnit->setTextureName(name, ttype);
	
	fireEvent(TextureNameChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setCubicTextureName(const String &name, bool forUVW /* = false */)
{
	mTextureUnit->setCubicTextureName(name, forUVW);
	
	fireEvent(CubicTextureNameChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setCubicTextureName(const String *const names, bool forUVW /* = false */)
{
	mTextureUnit->setCubicTextureName(names, forUVW);
	
	fireEvent(CubicTextureNameChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setAnimatedTextureName(const String &name, unsigned int numFrames, Real duration /* = 0 */)
{
	mTextureUnit->setAnimatedTextureName(&name, numFrames, duration);
	
	fireEvent(AnimatedTextureNameChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setAnimatedTextureName(const String *const names, unsigned int numFrames, Real duration /* = 0 */)
{
	mTextureUnit->setAnimatedTextureName(names, numFrames, duration);
	
	fireEvent(AnimatedTextureNameChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setCurrentFrame(unsigned int frameNumber)
{
	mTextureUnit->setCurrentFrame(frameNumber);
	
	fireEvent(CurrentFrameChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setFrameTextureName(const String &name, unsigned int frameNumber)
{
	mTextureUnit->setFrameTextureName(name, frameNumber);
	
	fireEvent(FrameTextureNameChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::addFrameTextureName(const String &name)
{
	mTextureUnit->addFrameTextureName(name);
	
	fireEvent(FrameTextureNameAdded, TextureUnitEventArgs(this));
}

void TextureUnitController::deleteFrameTextureName(const size_t frameNumber)
{
	mTextureUnit->deleteFrameTextureName(frameNumber);
	
	fireEvent(FrameTextureNameRemoved, TextureUnitEventArgs(this));
}

void TextureUnitController::setBindingType(TextureUnitState::BindingType bt)
{
	mTextureUnit->setBindingType(bt);
	
	fireEvent(BindingTypeChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setContentType(TextureUnitState::ContentType ct)
{
	mTextureUnit->setContentType(ct);
	
	fireEvent(ContentTypeChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setDesiredFormat(PixelFormat desiredFormat)
{
	mTextureUnit->setDesiredFormat(desiredFormat);
	
	fireEvent(PixelFormatChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setNumMipmaps(int numMipmaps)
{
	mTextureUnit->setNumMipmaps(numMipmaps);
	
	fireEvent(NumMipmapsChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setTextureCoordSet(unsigned int set)
{
	mTextureUnit->setTextureCoordSet(set);
	
	fireEvent(TextureCoordSetChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setTextureTransform(const Matrix4 &xform)
{
	mTextureUnit->setTextureTransform(xform);
	
	fireEvent(TextureTransformChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setTextureScroll(Real u, Real v)
{
	mTextureUnit->setTextureScroll(u, v);
	
	fireEvent(TextureScrollChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setTextureUScroll(Real value)
{
	mTextureUnit->setTextureUScroll(value);
	
	fireEvent(TextureUScrollChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setTextureUScale(Real value)
{
	mTextureUnit->setTextureUScale(value);
	
	fireEvent(TextureUScaleChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setTextureVScale(Real value)
{
	mTextureUnit->setTextureVScale(value);
	
	fireEvent(TextureVScaleChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setTextureScale(Real uScale, Real vScale)
{
	mTextureUnit->setTextureScale(uScale, vScale);
	
	fireEvent(TextureUScaleChanged, TextureUnitEventArgs(this));
	fireEvent(TextureVScaleChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setTextureRotate(const Radian &angle)
{
	mTextureUnit->setTextureRotate(angle);
	
	fireEvent(TextureRotateChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setTextureAddressingMode(TextureUnitState::TextureAddressingMode tam)
{
	mTextureUnit->setTextureAddressingMode(tam);
	
	fireEvent(TextureAddressingModeChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setTextureAddressingMode(TextureUnitState::TextureAddressingMode u, TextureUnitState::TextureAddressingMode v, TextureUnitState::TextureAddressingMode w)
{
	mTextureUnit->setTextureAddressingMode(u, v, w);
	
	fireEvent(TextureAddressingModeChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setTextureAddressingMode(const TextureUnitState::UVWAddressingMode &uvw)
{
	mTextureUnit->setTextureAddressingMode(uvw);
	
	fireEvent(TextureAddressingModeChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setTextureBorderColour(const ColourValue &colour)
{
	mTextureUnit->setTextureBorderColour(colour);
	
	fireEvent(TextureBorderColourChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setColourOperationEx(LayerBlendOperationEx op, LayerBlendSource source1 /* = LBS_TEXTURE */, LayerBlendSource source2 /* = LBS_CURRENT */, const ColourValue &arg1 /* = ColourValue::White */, const ColourValue &arg2 /* = ColourValue::White */, Real manualBlend /* = 0.0 */)
{
	mTextureUnit->setColourOperationEx(op, source1, source2, arg1, arg2, manualBlend);
	
	fireEvent(ColourOperationExChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setColourOperation(const LayerBlendOperation op)
{
	mTextureUnit->setColourOperation(op);
	
	fireEvent(ColourOperationChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setColourOpMultipassFallback(const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor)
{
	mTextureUnit->setColourOpMultipassFallback(sourceFactor, destFactor);
	
	fireEvent(ColourOpMultipassFallbackChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setAlphaOperation(LayerBlendOperationEx op, LayerBlendSource source1, LayerBlendSource source2, Real arg1, Real arg2, Real manualBlend)
{
	mTextureUnit->setAlphaOperation(op, source1, source2, arg1, arg2, manualBlend);
	
	fireEvent(AlphaOperationChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::addEffect(TextureUnitState::TextureEffect& effect)
{
	mTextureUnit->addEffect(effect);
	
	fireEvent(EffectAdded, TextureUnitEventArgs(this));
}

void TextureUnitController::setEnvironmentMap(bool enable, TextureUnitState::EnvMapType envMapType)
{
	mTextureUnit->setEnvironmentMap(enable, envMapType);
	
	fireEvent(EnvironmentMapChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setScrollAnimation(Real uSpeed, Real vSpeed)
{
	mTextureUnit->setScrollAnimation(uSpeed, vSpeed);
	
	fireEvent(ScrollAnimationChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setRotateAnimation(Real speed)
{
	mTextureUnit->setRotateAnimation(speed);
	
	fireEvent(AlphaOperationChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setTransformAnimation(const TextureUnitState::TextureTransformType ttype, const WaveformType waveType, Real base, Real frequency, Real phase, Real amplitude)
{
	mTextureUnit->setTransformAnimation(ttype, waveType, base, frequency, phase, amplitude);
	
	fireEvent(TransformAnimationChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setProjectiveTexturing(bool enabled, const Frustum* projectionSettings)
{
	mTextureUnit->setProjectiveTexturing(enabled, projectionSettings);
	
	fireEvent(ProjectiveTexturingChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::removeAllEffects(void)
{
	mTextureUnit->removeAllEffects();
	
	fireEvent(EffectRemoved, TextureUnitEventArgs(this));
}

void TextureUnitController::removeEffect(const TextureUnitState::TextureEffectType type)
{
	mTextureUnit->removeEffect(type);
	
	fireEvent(EffectRemoved, TextureUnitEventArgs(this));
}
 
void TextureUnitController::setBlank(void)
{
	mTextureUnit->setBlank();
	
	fireEvent(Blanked, TextureUnitEventArgs(this));
}

void TextureUnitController::setTextureFiltering(TextureFilterOptions filterType)
{
	mTextureUnit->setTextureFiltering(filterType);
	
	fireEvent(TextureFilteringChanged, TextureUnitEventArgs(this));
}
 
void TextureUnitController::setTextureFiltering(FilterOptions minFilter, FilterOptions magFilter, FilterOptions mipFilter)
{
	mTextureUnit->setTextureFiltering(minFilter, magFilter, mipFilter);
	
	fireEvent(TextureFilteringChanged, TextureUnitEventArgs(this));
}
 
void TextureUnitController::setTextureAnisotropy(unsigned int maxAniso)
{
	mTextureUnit->setTextureAnisotropy(maxAniso);
	
	fireEvent(TextureAnisotropyChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setTextureMipmapBias(float bias)
{
	mTextureUnit->setTextureMipmapBias(bias);
	
	fireEvent(TextureMipMapBiasChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setName(const String &name)
{
	mTextureUnit->setName(name);
	
	fireEvent(NameChanged, TextureUnitEventArgs(this));
}

void TextureUnitController::setTextureNameAlias(const String &name)
{
	mTextureUnit->setTextureNameAlias(name);
	
	fireEvent(TextureNameAliasChanged, TextureUnitEventArgs(this));
}
 
bool TextureUnitController::applyTextureAliases(const AliasTextureNamePairList &aliasList, const bool apply)
{
	bool result = mTextureUnit->applyTextureAliases(aliasList, apply);
	
	fireEvent(TextureNameAliasChanged, TextureUnitEventArgs(this));

	return result;
}


