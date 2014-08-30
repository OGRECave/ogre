/*
-------------------------------------------------------------------------
This source file is a part of OGRE
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
THE SOFTWARE
-------------------------------------------------------------------------
*/
#ifndef _TEXTUREUNITCONTROLLER_H_
#define _TEXTUREUNITCONTROLLER_H_

#include <list>

#include <boost/signal.hpp>

#include "OgreTexture.h"
#include "OgreTextureUnitState.h"

#include "EventArgs.h"
#include "EventContainer.h"

class PassController;

using namespace Ogre;

class TextureUnitController;

class TextureUnitEventArgs : public EventArgs
{
public:
    TextureUnitEventArgs(TextureUnitController* tuc) : mController(tuc) {}
    ~TextureUnitEventArgs() {}

    TextureUnitController* getController() { return mController; }

protected:
    TextureUnitController* mController;
};

class TextureUnitController : public EventContainer
{
public:
    enum TextureUnitEvent
    {
        TextureNameChanged,
        CubicTextureNameChanged,
        AnimatedTextureNameChanged,
        TextureDimensionsChanged,
        CurrentFrameChanged,
        FrameTextureNameChanged,
        FrameTextureNameAdded,
        FrameTextureNameRemoved,
        BindingTypeChanged,
        ContentTypeChanged,
        PixelFormatChanged,
        NumMipmapsChanged,
        AlphaChanged,
        TextureCoordSetChanged,
        TextureTransformChanged,
        TextureScrollChanged,
        TextureUScrollChanged,
        TextureVScrollChanged,
        TextureUScaleChanged,
        TextureVScaleChanged,
        TextureRotateChanged,
        TextureAddressingModeChanged,
        TextureBorderColourChanged,
        ColourOperationChanged,
        ColourOperationExChanged,
        ColourOpMultipassFallbackChanged,
        AlphaOperationChanged,
        EffectAdded,
        EnvironmentMapChanged,
        ScrollAnimationChanged,
        RotateAnimationChanged,
        TransformAnimationChanged,
        ProjectiveTexturingChanged,
        EffectRemoved,
        Blanked,
        TextureFilteringChanged,
        TextureAnisotropyChanged,
        TextureMipMapBiasChanged,
        NameChanged,
        TextureNameAliasChanged
    };

    TextureUnitController();
    TextureUnitController(TextureUnitState* textureUnit);
    TextureUnitController(PassController* parent, TextureUnitState* textureUnit);
    virtual ~TextureUnitController();

    TextureUnitState* getTextureUnit() const;
    void getTextureUnit(TextureUnitState* tus);

    PassController* getParentController() const;

    void setTextureName(const String &name, TextureType ttype);
    void setCubicTextureName(const String &name, bool forUVW = false);
    void setCubicTextureName(const String *const names, bool forUVW = false);
    void setAnimatedTextureName(const String &name, unsigned int numFrames, Real duration = 0); 
    void setAnimatedTextureName(const String *const names, unsigned int numFrames, Real duration = 0);
    void setCurrentFrame(unsigned int frameNumber);
    void setFrameTextureName(const String &name, unsigned int frameNumber); 
    void addFrameTextureName(const String &name);
    void deleteFrameTextureName(const size_t frameNumber); 
    void setBindingType(TextureUnitState::BindingType bt);
    void setContentType(TextureUnitState::ContentType ct);
    void setDesiredFormat(PixelFormat desiredFormat); 
    void setNumMipmaps(int numMipmaps);
    void setTextureCoordSet(unsigned int set); 
    void setTextureTransform(const Matrix4 &xform); 
    void setTextureScroll(Real u, Real v);
    void setTextureUScroll(Real value);
    void setTextureUScale(Real value);
    void setTextureVScale(Real value);
    void setTextureScale(Real uScale, Real vScale); 
    void setTextureRotate(const Radian &angle);
    void setTextureAddressingMode(TextureUnitState::TextureAddressingMode tam); 
    void setTextureAddressingMode(TextureUnitState::TextureAddressingMode u, TextureUnitState::TextureAddressingMode v, TextureUnitState::TextureAddressingMode w);
    void setTextureAddressingMode(const TextureUnitState::UVWAddressingMode &uvw);
    void setTextureBorderColour(const ColourValue &colour);
    void setColourOperationEx(LayerBlendOperationEx op, LayerBlendSource source1 = LBS_TEXTURE, LayerBlendSource source2 = LBS_CURRENT, const ColourValue &arg1 = ColourValue::White, const ColourValue &arg2 = ColourValue::White, Real manualBlend = 0.0);
    void setColourOperation(const LayerBlendOperation op); 
    void setColourOpMultipassFallback(const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor);
    void setAlphaOperation(LayerBlendOperationEx op, LayerBlendSource source1 = LBS_TEXTURE, LayerBlendSource source2 = LBS_CURRENT, Real arg1 = 1.0, Real arg2 = 1.0, Real manualBlend = 0.0);
    void addEffect(TextureUnitState::TextureEffect& effect);
    void setEnvironmentMap(bool enable, TextureUnitState::EnvMapType envMapType); 
    void setScrollAnimation(Real uSpeed, Real vSpeed);
    void setRotateAnimation(Real speed);
    void setTransformAnimation(const TextureUnitState::TextureTransformType ttype, const WaveformType waveType, Real base = 0, Real frequency = 1, Real phase = 0, Real amplitude = 1);
    void setProjectiveTexturing(bool enabled, const Frustum* projectionSettings = 0);
    void removeAllEffects(void); 
    void removeEffect(const TextureUnitState::TextureEffectType type); 
    void setBlank(void);
    void setTextureFiltering(TextureFilterOptions filterType); 
    void setTextureFiltering(FilterOptions minFilter, FilterOptions magFilter, FilterOptions mipFilter); 
    void setTextureAnisotropy(unsigned int maxAniso);
    void setTextureMipmapBias(float bias);
    void setName(const String &name);
    void setTextureNameAlias(const String &name); 
    bool applyTextureAliases(const AliasTextureNamePairList &aliasList, const bool apply = true);
        
protected:
    void registerEvents();

    PassController* mParentController;
    TextureUnitState* mTextureUnit;
};

#endif // _TEXTUREUNITCONTROLLER_H_


