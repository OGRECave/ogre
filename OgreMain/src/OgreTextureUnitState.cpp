/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreTextureUnitState.h"
#include "OgreControllerManager.h"
#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"

namespace Ogre {
    // allow operation without hardware support
    static SamplerPtr DUMMY_SAMPLER = std::make_shared<Sampler>();

    Sampler::Sampler()
        : mBorderColour(ColourValue::Black)
        , mMaxAniso(1)
        , mMipmapBias(0)
        , mMinFilter(FO_LINEAR)
        , mMagFilter(FO_LINEAR)
        , mMipFilter(FO_POINT)
        , mCompareFunc(CMPF_GREATER_EQUAL)
        , mCompareEnabled(false)
        , mDirty(true)
    {
        setAddressingMode(TAM_WRAP);
    }
    Sampler::~Sampler() {}
    //-----------------------------------------------------------------------
    void Sampler::setAddressingMode(const UVWAddressingMode& uvw)
    {
        mAddressMode = uvw;
        mDirty = true;
    }

    //-----------------------------------------------------------------------
    void Sampler::setFiltering(TextureFilterOptions filterType)
    {
        switch (filterType)
        {
        case TFO_NONE:
            setFiltering(FO_POINT, FO_POINT, FO_NONE);
            break;
        case TFO_BILINEAR:
            setFiltering(FO_LINEAR, FO_LINEAR, FO_POINT);
            break;
        case TFO_TRILINEAR:
            setFiltering(FO_LINEAR, FO_LINEAR, FO_LINEAR);
            break;
        case TFO_ANISOTROPIC:
            setFiltering(FO_ANISOTROPIC, FO_ANISOTROPIC, FO_LINEAR);
            break;
        }
    }
    //-----------------------------------------------------------------------
    void Sampler::setFiltering(FilterType ft, FilterOptions fo)
    {
        switch (ft)
        {
        case FT_MIN:
            mMinFilter = fo;
            break;
        case FT_MAG:
            mMagFilter = fo;
            break;
        case FT_MIP:
            mMipFilter = fo;
            break;
        }
        mDirty = true;
    }
    //-----------------------------------------------------------------------
    void Sampler::setFiltering(FilterOptions minFilter, FilterOptions magFilter, FilterOptions mipFilter)
    {
        mMinFilter = minFilter;
        mMagFilter = magFilter;
        mMipFilter = mipFilter;
        mDirty = true;
    }
    //-----------------------------------------------------------------------
    FilterOptions Sampler::getFiltering(FilterType ft) const
    {
        switch (ft)
        {
        case FT_MIN:
            return mMinFilter;
        case FT_MAG:
            return mMagFilter;
        case FT_MIP:
            return mMipFilter;
        }
        // to keep compiler happy
        return mMinFilter;
    }
    //-----------------------------------------------------------------------
    TextureUnitState::TextureUnitState(Pass* parent)
        : mCurrentFrame(0)
        , mAnimDuration(0)
        , mTextureCoordSetIndex(0)
        , mUnorderedAccessMipLevel(-1)
        , mGamma(1)
        , mUMod(0)
        , mVMod(0)
        , mUScale(1)
        , mVScale(1)
        , mRotate(0)
        , mTexModMatrix(Matrix4::IDENTITY)
        , mContentType(CONTENT_NAMED)
        , mTextureLoadFailed(false)
        , mRecalcTexMatrix(false)
        , mFramePtrs(1)
        , mSampler(TextureManager::getSingletonPtr() ? TextureManager::getSingleton().getDefaultSampler() : DUMMY_SAMPLER)
        , mParent(parent)
        , mAnimController(0)
    {
        mColourBlendMode.blendType = LBT_COLOUR;
        mAlphaBlendMode.operation = LBX_MODULATE;
        mAlphaBlendMode.blendType = LBT_ALPHA;
        mAlphaBlendMode.source1 = LBS_TEXTURE;
        mAlphaBlendMode.source2 = LBS_CURRENT;
        setColourOperation(LBO_MODULATE);

        if( Pass::getHashFunction() == Pass::getBuiltinHashFunction( Pass::MIN_TEXTURE_CHANGE ) )
        {
            mParent->_dirtyHash();
        }

    }

    //-----------------------------------------------------------------------
    TextureUnitState::TextureUnitState(Pass* parent, const TextureUnitState& oth )
    {
        mParent = parent;
        mAnimController = 0;
        *this = oth;
    }

    //-----------------------------------------------------------------------
    TextureUnitState::TextureUnitState( Pass* parent, const String& texName, unsigned int texCoordSet)
        : mCurrentFrame(0)
        , mAnimDuration(0)
        , mTextureCoordSetIndex(0)
        , mUnorderedAccessMipLevel(-1)
        , mGamma(1)
        , mUMod(0)
        , mVMod(0)
        , mUScale(1)
        , mVScale(1)
        , mRotate(0)
        , mTexModMatrix(Matrix4::IDENTITY)
        , mContentType(CONTENT_NAMED)
        , mTextureLoadFailed(false)
        , mRecalcTexMatrix(false)
        , mSampler(TextureManager::getSingletonPtr() ? TextureManager::getSingleton().getDefaultSampler() : DUMMY_SAMPLER)
        , mParent(parent)
        , mAnimController(0)
    {
        mColourBlendMode.blendType = LBT_COLOUR;
        mAlphaBlendMode.operation = LBX_MODULATE;
        mAlphaBlendMode.blendType = LBT_ALPHA;
        mAlphaBlendMode.source1 = LBS_TEXTURE;
        mAlphaBlendMode.source2 = LBS_CURRENT;
        setColourOperation(LBO_MODULATE);

        setTextureName(texName);
        setTextureCoordSet(texCoordSet);

        if( Pass::getHashFunction() == Pass::getBuiltinHashFunction( Pass::MIN_TEXTURE_CHANGE ) )
        {
            mParent->_dirtyHash();
        }

    }
    //-----------------------------------------------------------------------
    TextureUnitState::~TextureUnitState()
    {
        // Unload ensure all controllers destroyed
        _unload();
    }
    //-----------------------------------------------------------------------
    TextureUnitState & TextureUnitState::operator = ( 
        const TextureUnitState &oth )
    {
        if (this == &oth)
            return *this;

        assert(mAnimController == 0);
        removeAllEffects();

        // copy basic members (int's, real's)
        memcpy( (uchar*)this, &oth, (const uchar *)(&oth.mFramePtrs) - (const uchar *)(&oth) );
        // copy complex members
        mFramePtrs = oth.mFramePtrs;
        mSampler = oth.mSampler;
        mName    = oth.mName;
        mEffects = oth.mEffects;

        mCompositorRefName = oth.mCompositorRefName;
        mCompositorRefTexName = oth.mCompositorRefTexName;
        // Can't sharing controllers with other TUS, reset to null to avoid potential bug.
        for (auto & e : mEffects)
        {
            e.second.controller = 0;
        }

        // Load immediately if Material loaded
        if (isLoaded())
        {
            _load();
        }

        // Tell parent to recalculate hash
        if( Pass::getHashFunction() == Pass::getBuiltinHashFunction( Pass::MIN_TEXTURE_CHANGE ) )
        {
            mParent->_dirtyHash();
        }

        return *this;
    }
    //-----------------------------------------------------------------------
    const String& TextureUnitState::getTextureName(void) const
    {
        // Return name of current frame
        if (mCurrentFrame < mFramePtrs.size() && mFramePtrs[mCurrentFrame])
            return mFramePtrs[mCurrentFrame]->getName();
        else
            return BLANKSTRING;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureName( const String& name)
    {
        if(TexturePtr tex = retrieveTexture(name))
            setTexture(tex);
    }

    void TextureUnitState::setTextureName( const String& name, TextureType texType)
    {
        TexturePtr tex = retrieveTexture(name);

        if(!tex)
            return;

        tex->setTextureType(texType);
        setTexture(tex);
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTexture( const TexturePtr& texPtr)
    {
        if (!texPtr)
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                "Texture Pointer is empty.",
                "TextureUnitState::setTexture");
        }

        setContentType(CONTENT_NAMED);
        mTextureLoadFailed = false;
        
        if (texPtr->getTextureType() == TEX_TYPE_EXTERNAL_OES)
        {
            setTextureAddressingMode( TAM_CLAMP );
            setTextureFiltering(FT_MIP, FO_NONE);
        }

        mFramePtrs.resize(1);
        mFramePtrs[0] = texPtr;

        mCurrentFrame = 0;

        // Load immediately ?
        if (isLoaded())
        {
            _load(); // reload
        }
        // Tell parent to recalculate hash
        if( Pass::getHashFunction() == Pass::getBuiltinHashFunction( Pass::MIN_TEXTURE_CHANGE ) )
        {
            mParent->_dirtyHash();
        }
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setContentType(TextureUnitState::ContentType ct)
    {
        mContentType = ct;
    }
    //-----------------------------------------------------------------------
    TextureUnitState::ContentType TextureUnitState::getContentType(void) const
    {
        return mContentType;
    }
    //-----------------------------------------------------------------------
    TextureType TextureUnitState::getTextureType(void) const
    {
        return !mFramePtrs[0] ? TEX_TYPE_2D : mFramePtrs[0]->getTextureType();
    }

    //-----------------------------------------------------------------------
    void TextureUnitState::setFrameTextureName(const String& name, unsigned int frameNumber)
    {
        mTextureLoadFailed = false;
        OgreAssert(frameNumber < mFramePtrs.size(), "out of range");

        mFramePtrs[frameNumber] = retrieveTexture(name);

        if (isLoaded())
        {
            _load(); // reload
        }
        // Tell parent to recalculate hash
        if( Pass::getHashFunction() == Pass::getBuiltinHashFunction( Pass::MIN_TEXTURE_CHANGE ) )
        {
            mParent->_dirtyHash();
        }
    }

    //-----------------------------------------------------------------------
    void TextureUnitState::addFrameTextureName(const String& name)
    {
        setContentType(CONTENT_NAMED);
        mTextureLoadFailed = false;

        mFramePtrs.push_back(retrieveTexture(name));

        // Load immediately if Material loaded
        if (isLoaded())
        {
            _load();
        }
        // Tell parent to recalculate hash
        if( Pass::getHashFunction() == Pass::getBuiltinHashFunction( Pass::MIN_TEXTURE_CHANGE ) )
        {
            mParent->_dirtyHash();
        }
    }

    //-----------------------------------------------------------------------
    void TextureUnitState::deleteFrameTextureName(const size_t frameNumber)
    {
        mTextureLoadFailed = false;
        OgreAssert(frameNumber < mFramePtrs.size(), "out of range");
        mFramePtrs.erase(mFramePtrs.begin() + frameNumber);

        if (isLoaded())
        {
            _load();
        }
        // Tell parent to recalculate hash
        if( Pass::getHashFunction() == Pass::getBuiltinHashFunction( Pass::MIN_TEXTURE_CHANGE ) )
        {
            mParent->_dirtyHash();
        }
    }

    void TextureUnitState::setCubicTextureName(const String* const names, bool forUVW)
    {
        setLayerArrayNames(TEX_TYPE_CUBE_MAP, std::vector<String>(names, names + 6));
    }

    //-----------------------------------------------------------------------
    void TextureUnitState::setAnimatedTextureName( const String& name, size_t numFrames, Real duration)
    {
        String baseName, ext;
        StringUtil::splitBaseFilename(name, baseName, ext);

        std::vector<String> names(numFrames);
        for (uint32 i = 0; i < names.size(); ++i)
        {
            names[i] = StringUtil::format("%s_%u.%s", baseName.c_str(), i, ext.c_str());
        }

        setAnimatedTextureName(names, duration);
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setAnimatedTextureName(const String* const names, size_t numFrames, Real duration)
    {
        setContentType(CONTENT_NAMED);
        mTextureLoadFailed = false;

        // resize pointers, but don't populate until needed
        mFramePtrs.resize(numFrames);
        mAnimDuration = duration;
        mCurrentFrame = 0;

        for (unsigned int i = 0; i < mFramePtrs.size(); ++i)
        {
            mFramePtrs[i] = retrieveTexture(names[i]);
        }

        // Load immediately if Material loaded
        if (isLoaded())
        {
            _load();
        }
        // Tell parent to recalculate hash
        if( Pass::getHashFunction() == Pass::getBuiltinHashFunction( Pass::MIN_TEXTURE_CHANGE ) )
        {
            mParent->_dirtyHash();
        }
    }
    void TextureUnitState::setLayerArrayNames(TextureType type, const std::vector<String>& names)
    {
        OgreAssert(!names.empty(), "array layers empty");

        const char* typeName;
        switch(type)
        {
        case TEX_TYPE_CUBE_MAP:
            typeName = "Cube";
            break;
        case TEX_TYPE_2D_ARRAY:
            typeName = "Array";
            break;
        case TEX_TYPE_3D:
            typeName = "Volume";
            break;
        default:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "arrays not possible for this texture type");
            return;
        }

        // use hash to auto-name the texture
        uint32 hash = 0;
        for(const String& name : names)
            hash = FastHash(name.data(), name.size(), hash);

        auto tex = retrieveTexture(StringUtil::format("%sTex_%x", typeName, hash));
        tex->setTextureType(type);
        tex->setLayerNames(names);
        setTexture(tex);
    }

    //-----------------------------------------------------------------------
    std::pair<uint32, uint32> TextureUnitState::getTextureDimensions(unsigned int frame) const
    {
        
        TexturePtr tex = _getTexturePtr(frame);
        if (!tex)
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Could not find texture " + StringConverter::toString(frame),
            "TextureUnitState::getTextureDimensions" );

        return {tex->getWidth(), tex->getHeight()};
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setCurrentFrame(unsigned int frameNumber)
    {
        OgreAssert(frameNumber < mFramePtrs.size(), "out of range");
        mCurrentFrame = frameNumber;
        // this will affect the hash
        if( Pass::getHashFunction() == Pass::getBuiltinHashFunction( Pass::MIN_TEXTURE_CHANGE ) )
        {
            mParent->_dirtyHash();
        }
    }
    //-----------------------------------------------------------------------
    unsigned int TextureUnitState::getCurrentFrame(void) const
    {
        return mCurrentFrame;
    }
    //-----------------------------------------------------------------------
    unsigned int TextureUnitState::getNumFrames(void) const
    {
        return (unsigned int)mFramePtrs.size();
    }
    //-----------------------------------------------------------------------
    const String& TextureUnitState::getFrameTextureName(unsigned int frameNumber) const
    {
        OgreAssert(frameNumber < mFramePtrs.size(), "out of range");

        return mFramePtrs[0] ? mFramePtrs[frameNumber]->getName() : BLANKSTRING;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setDesiredFormat(PixelFormat desiredFormat)
    {
        OgreAssert(mFramePtrs[0], "frame must not be blank");
        for(auto& frame : mFramePtrs)
            frame->setFormat(desiredFormat);
    }
    //-----------------------------------------------------------------------
    PixelFormat TextureUnitState::getDesiredFormat(void) const
    {
        return !mFramePtrs[0] ? PF_UNKNOWN : mFramePtrs[0]->getDesiredFormat();
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setNumMipmaps(int numMipmaps)
    {
        OgreAssert(mFramePtrs[0], "frame must not be blank");
        for (auto& frame : mFramePtrs)
            frame->setNumMipmaps(numMipmaps == MIP_DEFAULT
                                     ? TextureManager::getSingleton().getDefaultNumMipmaps()
                                     : numMipmaps);
    }
    //-----------------------------------------------------------------------
    int TextureUnitState::getNumMipmaps(void) const
    {
        return !mFramePtrs[0] ? int(MIP_DEFAULT) : mFramePtrs[0]->getNumMipmaps();
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setIsAlpha(bool isAlpha)
    {
        OgreAssert(mFramePtrs[0], "frame must not be blank");
        OGRE_IGNORE_DEPRECATED_BEGIN
        for(auto& frame : mFramePtrs)
            frame->setTreatLuminanceAsAlpha(isAlpha);
        OGRE_IGNORE_DEPRECATED_END
    }
    float TextureUnitState::getGamma() const
    {
        return !mFramePtrs[0] ? 1.0f : mFramePtrs[0]->getGamma();
    }
    void TextureUnitState::setGamma(float gamma)
    {
        OgreAssert(mFramePtrs[0], "frame must not be blank");
        for(auto& frame : mFramePtrs)
            frame->setGamma(gamma);
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setHardwareGammaEnabled(bool g)
    {
        OgreAssert(mFramePtrs[0], "frame must not be blank");
        for(auto& frame : mFramePtrs)
            frame->setHardwareGammaEnabled(g);
    }
    //-----------------------------------------------------------------------
    bool TextureUnitState::isHardwareGammaEnabled() const
    {
        return mFramePtrs[0] && mFramePtrs[0]->isHardwareGammaEnabled();
    }
    //-----------------------------------------------------------------------
    unsigned int TextureUnitState::getTextureCoordSet(void) const
    {
        return mTextureCoordSetIndex;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureCoordSet(unsigned int set)
    {
        mTextureCoordSetIndex = set;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setColourOperationEx(LayerBlendOperationEx op,
        LayerBlendSource source1,
        LayerBlendSource source2,
        const ColourValue& arg1,
        const ColourValue& arg2,
        Real manualBlend)
    {
        mColourBlendMode.operation = op;
        mColourBlendMode.source1 = source1;
        mColourBlendMode.source2 = source2;
        mColourBlendMode.colourArg1 = arg1;
        mColourBlendMode.colourArg2 = arg2;
        mColourBlendMode.factor = manualBlend;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setColourOperation(LayerBlendOperation op)
    {
        // Set up the multitexture and multipass blending operations
        switch (op)
        {
        case LBO_REPLACE:
            setColourOperationEx(LBX_SOURCE1, LBS_TEXTURE, LBS_CURRENT);
            setColourOpMultipassFallback(SBF_ONE, SBF_ZERO);
            break;
        case LBO_ADD:
            setColourOperationEx(LBX_ADD, LBS_TEXTURE, LBS_CURRENT);
            setColourOpMultipassFallback(SBF_ONE, SBF_ONE);
            break;
        case LBO_MODULATE:
            setColourOperationEx(LBX_MODULATE, LBS_TEXTURE, LBS_CURRENT);
            setColourOpMultipassFallback(SBF_DEST_COLOUR, SBF_ZERO);
            break;
        case LBO_ALPHA_BLEND:
            setColourOperationEx(LBX_BLEND_TEXTURE_ALPHA, LBS_TEXTURE, LBS_CURRENT);
            setColourOpMultipassFallback(SBF_SOURCE_ALPHA, SBF_ONE_MINUS_SOURCE_ALPHA);
            break;
        }


    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setColourOpMultipassFallback(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor)
    {
        mColourBlendFallbackSrc = sourceFactor;
        mColourBlendFallbackDest = destFactor;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setAlphaOperation(LayerBlendOperationEx op,
        LayerBlendSource source1,
        LayerBlendSource source2,
        Real arg1,
        Real arg2,
        Real manualBlend)
    {
        mAlphaBlendMode.operation = op;
        mAlphaBlendMode.source1 = source1;
        mAlphaBlendMode.source2 = source2;
        mAlphaBlendMode.alphaArg1 = arg1;
        mAlphaBlendMode.alphaArg2 = arg2;
        mAlphaBlendMode.factor = manualBlend;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::addEffect(TextureEffect& effect)
    {
        // Ensure controller pointer is null
        effect.controller = 0;

        if (effect.type == ET_ENVIRONMENT_MAP 
            || effect.type == ET_UVSCROLL
            || effect.type == ET_USCROLL
            || effect.type == ET_VSCROLL
            || effect.type == ET_ROTATE
            || effect.type == ET_PROJECTIVE_TEXTURE)
        {
            // Replace - must be unique
            // Search for existing effect of this type
            EffectMap::iterator i = mEffects.find(effect.type);
            if (i != mEffects.end())
            {
                // Destroy old effect controller if exist
                if (i->second.controller)
                {
                    ControllerManager::getSingleton().destroyController(i->second.controller);
                }

                mEffects.erase(i);
            }
        }

        if (isLoaded())
        {
            // Create controller
            createEffectController(effect);
        }

        // Record new effect
        mEffects.emplace(effect.type, effect);

    }
    //-----------------------------------------------------------------------
    void TextureUnitState::removeAllEffects(void)
    {
        // Iterate over effects to remove controllers
        EffectMap::iterator i, iend;
        iend = mEffects.end();
        for (i = mEffects.begin(); i != iend; ++i)
        {
            if (i->second.controller)
            {
                ControllerManager::getSingleton().destroyController(i->second.controller);
            }
        }

        mEffects.clear();
    }

    //-----------------------------------------------------------------------
    bool TextureUnitState::isBlank(void) const
    {
        return !mFramePtrs[0] || mTextureLoadFailed;
    }

    //-----------------------------------------------------------------------
    SceneBlendFactor TextureUnitState::getColourBlendFallbackSrc(void) const
    {
        return mColourBlendFallbackSrc;
    }
    //-----------------------------------------------------------------------
    SceneBlendFactor TextureUnitState::getColourBlendFallbackDest(void) const
    {
        return mColourBlendFallbackDest;
    }
    //-----------------------------------------------------------------------
    const LayerBlendModeEx& TextureUnitState::getColourBlendMode(void) const
    {
        return mColourBlendMode;
    }
    //-----------------------------------------------------------------------
    const LayerBlendModeEx& TextureUnitState::getAlphaBlendMode(void) const
    {
        return mAlphaBlendMode;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setEnvironmentMap(bool enable, EnvMapType envMapType)
    {
        if (enable)
        {
            TextureEffect eff;
            eff.type = ET_ENVIRONMENT_MAP;

            eff.subtype = envMapType;
            addEffect(eff);
        }
        else
        {
            removeEffect(ET_ENVIRONMENT_MAP);
        }
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::removeEffect(TextureEffectType type)
    {
        // Get range of items matching this effect
        std::pair< EffectMap::iterator, EffectMap::iterator > remPair = 
            mEffects.equal_range( type );
        // Remove controllers
        for (EffectMap::iterator i = remPair.first; i != remPair.second; ++i)
        {
            if (i->second.controller)
            {
                ControllerManager::getSingleton().destroyController(i->second.controller);
            }
        }
        // Erase         
        mEffects.erase( remPair.first, remPair.second );
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setBlank(void)
    {
        mFramePtrs.clear();
        mFramePtrs.push_back(TexturePtr()); // insert nullptr to show warning tex
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureTransform(const Matrix4& xform)
    {
        mTexModMatrix = xform;
        mRecalcTexMatrix = false;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureScroll(Real u, Real v)
    {
        mUMod = u;
        mVMod = v;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureScale(Real uScale, Real vScale)
    {
        mUScale = uScale;
        mVScale = vScale;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureRotate(const Radian& angle)
    {
        mRotate = angle;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    const Matrix4& TextureUnitState::getTextureTransform() const
    {
        if (mRecalcTexMatrix)
            recalcTextureMatrix();
        return mTexModMatrix;

    }
    //-----------------------------------------------------------------------
    void TextureUnitState::recalcTextureMatrix() const
    {
        // Assumption: 2D texture coords
        // that would make this Affine2(Matrix3), but we lack such a class
        // Matrix3 is horribly unoptimized ATM
        Affine3 xform = Affine3::IDENTITY;

        if (mUScale != 1 || mVScale != 1)
        {
            // Offset to center of texture
            xform[0][0] = 1/mUScale;
            xform[1][1] = 1/mVScale;
            // Skip matrix concat since first matrix update
            xform[0][3] = (-0.5f * xform[0][0]) + 0.5f;
            xform[1][3] = (-0.5f * xform[1][1]) + 0.5f;
        }

        if (mUMod || mVMod)
        {
            xform = Affine3::getTrans(mUMod, mVMod, 0) * xform;
        }

        if (mRotate != Radian(0))
        {
            Affine3 rot = Affine3::IDENTITY;
            Radian theta ( mRotate );
            Real cosTheta = Math::Cos(theta);
            Real sinTheta = Math::Sin(theta);

            rot[0][0] = cosTheta;
            rot[0][1] = -sinTheta;
            rot[1][0] = sinTheta;
            rot[1][1] = cosTheta;
            // Offset center of rotation to center of texture
            rot[0][3] = 0.5f + ( (-0.5f * cosTheta) - (-0.5f * sinTheta) );
            rot[1][3] = 0.5f + ( (-0.5f * sinTheta) + (-0.5f * cosTheta) );

            xform = rot * xform;
        }

        mTexModMatrix = xform;
        mRecalcTexMatrix = false;

    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureUScroll(Real value)
    {
        mUMod = value;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureVScroll(Real value)
    {
        mVMod = value;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureUScale(Real value)
    {
        mUScale = value;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureVScale(Real value)
    {
        mVScale = value;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setScrollAnimation(Real uSpeed, Real vSpeed)
    {
        // Remove existing effects
        removeEffect(ET_UVSCROLL);
        removeEffect(ET_USCROLL);
        removeEffect(ET_VSCROLL);

        // don't create an effect if the speeds are both 0
        if(uSpeed == 0.0f && vSpeed == 0.0f) 
        {
          return;
        }

        // Create new effect
        TextureEffect eff;
    if(uSpeed == vSpeed) 
    {
        eff.type = ET_UVSCROLL;
        eff.arg1 = uSpeed;
        addEffect(eff);
    }
    else
    {
        if(uSpeed)
        {
            eff.type = ET_USCROLL;
        eff.arg1 = uSpeed;
        addEffect(eff);
    }
        if(vSpeed)
        {
            eff.type = ET_VSCROLL;
            eff.arg1 = vSpeed;
            addEffect(eff);
        }
    }
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setRotateAnimation(Real speed)
    {
        // Remove existing effect
        removeEffect(ET_ROTATE);
        // don't create an effect if the speed is 0
        if(speed == 0.0f) 
        {
          return;
        }
        // Create new effect
        TextureEffect eff;
        eff.type = ET_ROTATE;
        eff.arg1 = speed;
        addEffect(eff);
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTransformAnimation(TextureTransformType ttype,
        WaveformType waveType, Real base, Real frequency, Real phase, Real amplitude)
    {
        // Remove existing effect
        // note, only remove for subtype, not entire ET_TRANSFORM
        // otherwise we won't be able to combine subtypes
        // Get range of items matching this effect
        for (EffectMap::iterator i = mEffects.begin(); i != mEffects.end(); ++i)
        {
            if (i->second.type == ET_TRANSFORM && i->second.subtype == ttype)
            {
                if (i->second.controller)
                {
                    ControllerManager::getSingleton().destroyController(i->second.controller);
                }
                mEffects.erase(i);

                // should only be one, so jump out
                break;
            }
        }

    // don't create an effect if the given values are all 0
    if(base == 0.0f && phase == 0.0f && frequency == 0.0f && amplitude == 0.0f) 
    {
      return;
    }
        // Create new effect
        TextureEffect eff;
        eff.type = ET_TRANSFORM;
        eff.subtype = ttype;
        eff.waveType = waveType;
        eff.base = base;
        eff.frequency = frequency;
        eff.phase = phase;
        eff.amplitude = amplitude;
        addEffect(eff);
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::_prepare(void)
    {
        // Unload first
        //_unload();

        // Load textures
        for (unsigned int i = 0; i < mFramePtrs.size(); ++i)
        {
            ensurePrepared(i);
        }
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::_load(void)
    {

        // Load textures
        for (unsigned int i = 0; i < mFramePtrs.size(); ++i)
        {
            ensureLoaded(i);
        }
        // Animation controller
        if (mAnimDuration != 0)
        {
            createAnimController();
        }
        // Effect controllers
        for (auto & e : mEffects)
        {
            createEffectController(e.second);
        }

    }
    //-----------------------------------------------------------------------
    const TexturePtr& TextureUnitState::_getTexturePtr(void) const
    {
        return _getTexturePtr(mCurrentFrame);
    }
    //-----------------------------------------------------------------------
    const TexturePtr& TextureUnitState::_getTexturePtr(size_t frame) const
    {
        if (frame < mFramePtrs.size())
        {
            if (mContentType == CONTENT_NAMED)
            {
                ensureLoaded(frame);
            }

            return mFramePtrs[frame];
        }
        
        // Silent fail with empty texture for internal method
        static TexturePtr nullTexPtr;
        return nullTexPtr;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::_setTexturePtr(const TexturePtr& texptr)
    {
        _setTexturePtr(texptr, mCurrentFrame);
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::_setTexturePtr(const TexturePtr& texptr, size_t frame)
    {
        assert(frame < mFramePtrs.size());
        mFramePtrs[frame] = texptr;
    }
    //-----------------------------------------------------------------------
    TexturePtr TextureUnitState::retrieveTexture(const String& name) {
        TextureManager::ResourceCreateOrRetrieveResult res;
        res = TextureManager::getSingleton().createOrRetrieve(name, mParent->getResourceGroup());
        return static_pointer_cast<Texture>(res.first);
    }
    //-----------------------------------------------------------------------
    bool TextureUnitState::checkTexCalcSettings(const TexturePtr& tex) const
    {
        if(mContentType != TextureUnitState::CONTENT_NAMED)
            return true; // can only check normal textures

        String err;
        auto texCalc = _deriveTexCoordCalcMethod();
        if ((texCalc == TEXCALC_ENVIRONMENT_MAP_PLANAR || texCalc == TEXCALC_ENVIRONMENT_MAP) &&
            getTextureType() != TEX_TYPE_2D)
        {
            err = "env_map setting requires a 2d texture";
        }
        else if ((texCalc == TEXCALC_ENVIRONMENT_MAP_NORMAL || texCalc == TEXCALC_ENVIRONMENT_MAP_REFLECTION) &&
                 getTextureType() != TEX_TYPE_CUBE_MAP)
        {
            err = "env_map setting requires a cubic texture";
        }
        if(err.empty())
            return true;

        String msg = err+", but '"+tex->getName()+"' is not. Texture layer will be blank";
        LogManager::getSingleton().logError(msg);
        mTextureLoadFailed = true;
        return false;
    }
    void TextureUnitState::ensurePrepared(size_t frame) const
    {
        const TexturePtr& tex = mFramePtrs[frame];
        if (!tex || mTextureLoadFailed || !checkTexCalcSettings(tex))
            return;

        tex->setGamma(mGamma);

        try {
            tex->prepare();
        }
        catch (Exception& e)
        {
            String msg = "preparing texture '" + tex->getName() +
                         "'. Texture layer will be blank: " + e.getDescription();
            LogManager::getSingleton().logError(msg);
            mTextureLoadFailed = true;
        }
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::ensureLoaded(size_t frame) const
    {
        const TexturePtr& tex = mFramePtrs[frame];
        if (!tex || mTextureLoadFailed || !checkTexCalcSettings(tex))
            return;

        tex->setGamma(mGamma);

        if(mUnorderedAccessMipLevel > -1)
            tex->setUsage(HBU_GPU_ONLY | TU_UNORDERED_ACCESS);

        try {
            tex->load();
        }
        catch (Exception& e)
        {
            String msg = "loading texture '" + tex->getName() +
                         "'. Texture layer will be blank: " + e.getDescription();
            LogManager::getSingleton().logError(msg);
            mTextureLoadFailed = true;
        }
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::createAnimController(void)
    {
        if (mAnimController)
        {
            ControllerManager::getSingleton().destroyController(mAnimController);
            mAnimController = 0;
        }
        mAnimController = ControllerManager::getSingleton().createTextureAnimator(this, mAnimDuration);

    }
    //-----------------------------------------------------------------------
    void TextureUnitState::createEffectController(TextureEffect& effect)
    {
        if (effect.controller)
        {
            ControllerManager::getSingleton().destroyController(effect.controller);
            effect.controller = 0;
        }
        ControllerManager& cMgr = ControllerManager::getSingleton();
        switch (effect.type)
        {
        case ET_UVSCROLL:
            effect.controller = cMgr.createTextureUVScroller(this, effect.arg1);
            break;
        case ET_USCROLL:
            effect.controller = cMgr.createTextureUScroller(this, effect.arg1);
            break;
        case ET_VSCROLL:
            effect.controller = cMgr.createTextureVScroller(this, effect.arg1);
            break;
        case ET_ROTATE:
            effect.controller = cMgr.createTextureRotater(this, effect.arg1);
            break;
        case ET_TRANSFORM:
            effect.controller = cMgr.createTextureWaveTransformer(this, (TextureUnitState::TextureTransformType)effect.subtype, effect.waveType, effect.base,
                effect.frequency, effect.phase, effect.amplitude);
            break;
        case ET_ENVIRONMENT_MAP:
            break;
        default:
            break;
        }
    }
    //-----------------------------------------------------------------------
    Real TextureUnitState::getTextureUScroll(void) const
    {
        return mUMod;
    }

    //-----------------------------------------------------------------------
    Real TextureUnitState::getTextureVScroll(void) const
    {
        return mVMod;
    }

    //-----------------------------------------------------------------------
    Real TextureUnitState::getTextureUScale(void) const
    {
        return mUScale;
    }

    //-----------------------------------------------------------------------
    Real TextureUnitState::getTextureVScale(void) const
    {
        return mVScale;
    }

    //-----------------------------------------------------------------------
    const Radian& TextureUnitState::getTextureRotate(void) const
    {
        return mRotate;
    }
    
    //-----------------------------------------------------------------------
    Real TextureUnitState::getAnimationDuration(void) const
    {
        return mAnimDuration;
    }

    //-----------------------------------------------------------------------
    const TextureUnitState::EffectMap& TextureUnitState::getEffects(void) const
    {
        return mEffects;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::_unprepare(void)
    {
        // don't unload textures. may be used elsewhere
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::_unload(void)
    {
        // Destroy animation controller
        if (mAnimController)
        {
            ControllerManager::getSingleton().destroyController(mAnimController);
            mAnimController = 0;
        }

        // Destroy effect controllers
        for (auto & e : mEffects)
        {
            if (e.second.controller)
            {
                ControllerManager::getSingleton().destroyController(e.second.controller);
                e.second.controller = 0;
            }
        }

        // don't unload named textures. may be used elsewhere
        // drop references on managed textures, however
        if(mContentType != CONTENT_NAMED)
            mFramePtrs[0].reset();
    }
    //-----------------------------------------------------------------------------
    bool TextureUnitState::isLoaded(void) const
    {
        return mParent->isLoaded();
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::_notifyNeedsRecompile(void)
    {
        mParent->_notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setProjectiveTexturing(bool enable, 
        const Frustum* projectionSettings)
    {
        if (enable)
        {
            TextureEffect eff;
            eff.type = ET_PROJECTIVE_TEXTURE;
            eff.frustum = projectionSettings;
            addEffect(eff);
        }
        else
        {
            removeEffect(ET_PROJECTIVE_TEXTURE);
        }

    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setName(const String& name)
    {
        mName = name;
    }
    //-----------------------------------------------------------------------------
    void TextureUnitState::_notifyParent(Pass* parent)
    {
        mParent = parent;
    }
    //-----------------------------------------------------------------------------
    void TextureUnitState::setCompositorReference(const String& compositorName, const String& textureName, size_t mrtIndex)
    {  
        mCompositorRefName = compositorName; 
        mCompositorRefTexName = textureName; 
        mCompositorRefMrtIndex = mrtIndex; 
    }
    //-----------------------------------------------------------------------
    size_t TextureUnitState::calculateSize(void) const
    {
        size_t memSize = sizeof(*this);
        memSize += mFramePtrs.size() * sizeof(TexturePtr);
        memSize += mEffects.size() * sizeof(TextureEffect);
        return memSize;
    }

    bool TextureUnitState::isDefaultFiltering() const {
        return mSampler == TextureManager::getSingleton().getDefaultSampler();
    }

    const SamplerPtr& TextureUnitState::_getLocalSampler()
    {
        if(isDefaultFiltering())
            mSampler = TextureManager::getSingleton().createSampler();

        return mSampler;
    }

    TexCoordCalcMethod TextureUnitState::_deriveTexCoordCalcMethod() const
    {
        TexCoordCalcMethod texCoordCalcMethod = TEXCALC_NONE;
        for (const auto& effi : mEffects)
        {
            switch (effi.second.type)
            {
            case ET_ENVIRONMENT_MAP:
                if (effi.second.subtype == ENV_CURVED)
                {
                    texCoordCalcMethod = TEXCALC_ENVIRONMENT_MAP;
                }
                else if (effi.second.subtype == ENV_PLANAR)
                {
                    texCoordCalcMethod = TEXCALC_ENVIRONMENT_MAP_PLANAR;
                }
                else if (effi.second.subtype == ENV_REFLECTION)
                {
                    texCoordCalcMethod = TEXCALC_ENVIRONMENT_MAP_REFLECTION;
                }
                else if (effi.second.subtype == ENV_NORMAL)
                {
                    texCoordCalcMethod = TEXCALC_ENVIRONMENT_MAP_NORMAL;
                }
                break;
            case ET_UVSCROLL:
            case ET_USCROLL:
            case ET_VSCROLL:
            case ET_ROTATE:
            case ET_TRANSFORM:
                break;
            case ET_PROJECTIVE_TEXTURE:
                texCoordCalcMethod = TEXCALC_PROJECTIVE_TEXTURE;
                break;
            }
        }

        return texCoordCalcMethod;
    }
}
