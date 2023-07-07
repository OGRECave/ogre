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
#include <memory>

#include "OgreStableHeaders.h"

#include "OgreGpuProgramUsage.h"
#include "OgreTextureUnitState.h"

namespace Ogre {

    /** Default pass hash function.

        Tries to minimise the number of texture changes.
    */
    struct MinTextureStateChangeHashFunc : public Pass::HashFunc
    {
        uint32 operator()(const Pass* p) const override
        {
            OGRE_LOCK_MUTEX(p->mTexUnitChangeMutex);
            uint32 hash = 0;
            ushort c = p->getNumTextureUnitStates();

            for (ushort i = 0; i < c; ++i)
            {
                const TextureUnitState* tus = 0;
                tus = p->getTextureUnitState(i);
                hash = FastHash(tus->getTextureName().c_str(), tus->getTextureName().size(), hash);
            }

            return hash;
        }
    };
    MinTextureStateChangeHashFunc sMinTextureStateChangeHashFunc;
    /** Alternate pass hash function.

        Tries to minimise the number of GPU program changes.
    */
    struct MinGpuProgramChangeHashFunc : public Pass::HashFunc
    {
        uint32 operator()(const Pass* p) const override
        {
            OGRE_LOCK_MUTEX(p->mGpuProgramChangeMutex);
            uint32 hash = 0;

            for(int i = 0; i < GPT_COUNT; i++)
            {
                const String& name = p->getGpuProgramName(GpuProgramType(i));
                if(!name.empty()) {
                    hash = FastHash(name.c_str(), name.size(), hash);
                }
            }

            return hash;
        }
    };
    MinGpuProgramChangeHashFunc sMinGpuProgramChangeHashFunc;
    //-----------------------------------------------------------------------------
    Pass::PassSet Pass::msDirtyHashList;
    Pass::PassSet Pass::msPassGraveyard;
    OGRE_STATIC_MUTEX_INSTANCE(Pass::msDirtyHashListMutex);
    OGRE_STATIC_MUTEX_INSTANCE(Pass::msPassGraveyardMutex);

    Pass::HashFunc* Pass::msHashFunc = &sMinGpuProgramChangeHashFunc;
    //-----------------------------------------------------------------------------
    Pass::HashFunc* Pass::getBuiltinHashFunction(BuiltinHashFunction builtin)
    {
        Pass::HashFunc* hashFunc = NULL;

        switch(builtin)
        {
        case MIN_TEXTURE_CHANGE:
            hashFunc = &sMinTextureStateChangeHashFunc;
            break;
        case MIN_GPU_PROGRAM_CHANGE:
            hashFunc = &sMinGpuProgramChangeHashFunc;
            break;
        }

        return hashFunc;
    }
    //-----------------------------------------------------------------------------
    void Pass::setHashFunction(BuiltinHashFunction builtin)
    {
        switch(builtin)
        {
        case MIN_TEXTURE_CHANGE:
            msHashFunc = &sMinTextureStateChangeHashFunc;
            break;
        case MIN_GPU_PROGRAM_CHANGE:
            msHashFunc = &sMinGpuProgramChangeHashFunc;
            break;
        }
    }
    //-----------------------------------------------------------------------------
    Pass::Pass(Technique* parent, unsigned short index)
        : mParent(parent)
        , mHash(0)
        , mAmbient(ColourValue::White)
        , mDiffuse(ColourValue::White)
        , mSpecular(ColourValue::Black)
        , mEmissive(ColourValue::Black)
        , mShininess(0)
        , mTracking(TVC_NONE)
        , mHashDirtyQueued(false)
        , mDepthCheck(true)
        , mDepthWrite(true)
        , mAlphaToCoverageEnabled(false)
        , mTransparentSorting(true)
        , mTransparentSortingForced(false)
        , mLightingEnabled(true)
        , mIteratePerLight(false)
        , mRunOnlyForOneLightType(false)
        , mPolygonModeOverrideable(true)
        , mFogOverride(false)
        , mQueuedForDeletion(false)
        , mLightScissoring(false)
        , mLightClipPlanes(false)
        , mPointSpritesEnabled(false)
        , mPointAttenuationEnabled(false)
        , mContentTypeLookupBuilt(false)
        , mAlphaRejectVal(0)
        , mDepthBiasConstant(0.0f)
        , mDepthBiasSlopeScale(0.0f)
        , mDepthBiasPerIteration(0.0f)
        , mDepthFunc(CMPF_LESS_EQUAL)
        , mAlphaRejectFunc(CMPF_ALWAYS_PASS)
        , mCullMode(CULL_CLOCKWISE)
        , mManualCullMode(MANUAL_CULL_BACK)
        , mMaxSimultaneousLights(OGRE_MAX_SIMULTANEOUS_LIGHTS)
        , mStartLight(0)
        , mLightsPerIteration(1)
        , mIndex(index)
        , mLightMask(0xFFFFFFFF)
        , mFogColour(ColourValue::White)
        , mFogStart(0.0)
        , mFogEnd(1.0)
        , mFogDensity(0.001)
        , mLineWidth(1.0f)
        , mPassIterationCount(1)
        , mPointMinSize(0.0f)
        , mPointMaxSize(0.0f)
        , mPointAttenution(1.0f, 1.0f, 0.0f, 0.0f)
        , mShadeOptions(SO_GOURAUD)
        , mPolygonMode(PM_SOLID)
        , mIlluminationStage(IS_UNKNOWN)
        , mOnlyLightType(Light::LT_POINT)
        , mFogMode(FOG_NONE)
    {
        // init the hash inline
        _recalculateHash();
   }

    //-----------------------------------------------------------------------------
    Pass::Pass(Technique *parent, unsigned short index, const Pass& oth)
        : mParent(parent), mQueuedForDeletion(false), mIndex(index), mPassIterationCount(1)
    {
        *this = oth;
        mParent = parent;
        mIndex = index;
        mQueuedForDeletion = false;

        // init the hash inline
        _recalculateHash();
    }
    Pass::~Pass() = default; // ensure unique_ptr destructors are in cpp
    //-----------------------------------------------------------------------------
    Pass& Pass::operator=(const Pass& oth)
    {
        mName = oth.mName;
        mHash = oth.mHash;
        mAmbient = oth.mAmbient;
        mDiffuse = oth.mDiffuse;
        mSpecular = oth.mSpecular;
        mEmissive = oth.mEmissive;
        mShininess = oth.mShininess;
        mTracking = oth.mTracking;

        // Copy fog parameters
        mFogOverride = oth.mFogOverride;
        mFogMode = oth.mFogMode;
        mFogColour = oth.mFogColour;
        mFogStart = oth.mFogStart;
        mFogEnd = oth.mFogEnd;
        mFogDensity = oth.mFogDensity;

        // Default blending (overwrite)
        mBlendState = oth.mBlendState;

        mDepthCheck = oth.mDepthCheck;
        mDepthWrite = oth.mDepthWrite;
        mAlphaRejectFunc = oth.mAlphaRejectFunc;
        mAlphaRejectVal = oth.mAlphaRejectVal;
        mAlphaToCoverageEnabled = oth.mAlphaToCoverageEnabled;
        mTransparentSorting = oth.mTransparentSorting;
        mTransparentSortingForced = oth.mTransparentSortingForced;
        mDepthFunc = oth.mDepthFunc;
        mDepthBiasConstant = oth.mDepthBiasConstant;
        mDepthBiasSlopeScale = oth.mDepthBiasSlopeScale;
        mDepthBiasPerIteration = oth.mDepthBiasPerIteration;
        mCullMode = oth.mCullMode;
        mManualCullMode = oth.mManualCullMode;
        mLightingEnabled = oth.mLightingEnabled;
        mMaxSimultaneousLights = oth.mMaxSimultaneousLights;
        mStartLight = oth.mStartLight;
        mIteratePerLight = oth.mIteratePerLight;
        mLightsPerIteration = oth.mLightsPerIteration;
        mRunOnlyForOneLightType = oth.mRunOnlyForOneLightType;
        mOnlyLightType = oth.mOnlyLightType;
        mShadeOptions = oth.mShadeOptions;
        mPolygonMode = oth.mPolygonMode;
        mPolygonModeOverrideable = oth.mPolygonModeOverrideable;
        mPassIterationCount = oth.mPassIterationCount;
        mLineWidth = oth.mLineWidth;
        mPointAttenution = oth.mPointAttenution;
        mPointMinSize = oth.mPointMinSize;
        mPointMaxSize = oth.mPointMaxSize;
        mPointSpritesEnabled = oth.mPointSpritesEnabled;
        mPointAttenuationEnabled = oth.mPointAttenuationEnabled;
        mShadowContentTypeLookup = oth.mShadowContentTypeLookup;
        mContentTypeLookupBuilt = oth.mContentTypeLookupBuilt;
        mLightScissoring = oth.mLightScissoring;
        mLightClipPlanes = oth.mLightClipPlanes;
        mIlluminationStage = oth.mIlluminationStage;
        mLightMask = oth.mLightMask;

        for(int i = 0; i < GPT_COUNT; i++)
        {
            auto& programUsage = mProgramUsage[i];
            auto& othUsage = oth.mProgramUsage[i];
            programUsage = othUsage ? std::make_unique<GpuProgramUsage>(*othUsage, this) : nullptr;
        }

        // Clear texture units but doesn't notify need recompilation in the case
        // we are cloning, The parent material will take care of this.
        for (auto *t : mTextureUnitStates)
        {
            OGRE_DELETE t;
        }

        mTextureUnitStates.clear();

        // Copy texture units
        for (auto *s : oth.mTextureUnitStates)
        {
            TextureUnitState* t = OGRE_NEW TextureUnitState(this, *s);
            mTextureUnitStates.push_back(t);
        }

        _dirtyHash();

        return *this;
    }
    //-----------------------------------------------------------------------------
    size_t Pass::calculateSize(void) const
    {
        size_t memSize = 0;

        // Tally up TU states
        for (auto *t : mTextureUnitStates)
        {
            memSize += t->calculateSize();
        }
        for(const auto& u : mProgramUsage)
            memSize += u ? u->calculateSize() : 0;

        return memSize;
    }
    //-----------------------------------------------------------------------
    void Pass::setPointAttenuation(bool enabled, float constant, float linear, float quadratic)
    {
        mPointAttenuationEnabled = enabled;
        mPointAttenution[1] = enabled ? constant : 1.0f;
        mPointAttenution[2] = enabled ? linear : 0.0f;
        mPointAttenution[3] = enabled ? quadratic : 0.0f;
    }
    //-----------------------------------------------------------------------
    void Pass::setPointMinSize(Real min)
    {
        mPointMinSize = min;
    }
    //-----------------------------------------------------------------------
    Real Pass::getPointMinSize(void) const
    {
        return mPointMinSize;
    }
    //-----------------------------------------------------------------------
    void Pass::setPointMaxSize(Real max)
    {
        mPointMaxSize = max;
    }
    //-----------------------------------------------------------------------
    Real Pass::getPointMaxSize(void) const
    {
        return mPointMaxSize;
    }
    //-----------------------------------------------------------------------
    void Pass::setAmbient(float red, float green, float blue)
    {
        mAmbient.r = red;
        mAmbient.g = green;
        mAmbient.b = blue;

    }
    //-----------------------------------------------------------------------
    void Pass::setDiffuse(float red, float green, float blue, float alpha)
    {
        mDiffuse.r = red;
        mDiffuse.g = green;
        mDiffuse.b = blue;
        mDiffuse.a = alpha;
    }
    //-----------------------------------------------------------------------
    void Pass::setSpecular(float red, float green, float blue, float alpha)
    {
        mSpecular.r = red;
        mSpecular.g = green;
        mSpecular.b = blue;
        mSpecular.a = alpha;
    }
    //-----------------------------------------------------------------------
    void Pass::setSelfIllumination(float red, float green, float blue)
    {
        mEmissive.r = red;
        mEmissive.g = green;
        mEmissive.b = blue;
    }
    //-----------------------------------------------------------------------
    TextureUnitState* Pass::createTextureUnitState(void)
    {
        TextureUnitState *t = OGRE_NEW TextureUnitState(this);
        addTextureUnitState(t);
        mContentTypeLookupBuilt = false;
        return t;
    }
    //-----------------------------------------------------------------------
    TextureUnitState* Pass::createTextureUnitState(
        const String& textureName, unsigned short texCoordSet)
    {
        TextureUnitState *t = OGRE_NEW TextureUnitState(this);
        t->setTextureName(textureName);
        t->setTextureCoordSet(texCoordSet);
        addTextureUnitState(t);
        mContentTypeLookupBuilt = false;
        return t;
    }
    //-----------------------------------------------------------------------
    void Pass::addTextureUnitState(TextureUnitState* state)
    {
        OGRE_LOCK_MUTEX(mTexUnitChangeMutex);

        OgreAssert(state , "TextureUnitState is NULL");

        // only attach TUS to pass if TUS does not belong to another pass
        OgreAssert(!state->getParent() || (state->getParent() == this), "TextureUnitState already attached to another pass");

        mTextureUnitStates.push_back(state);
        // Notify state
        state->_notifyParent(this);
        // if texture unit state name is empty then give it a default name based on its index
        if (state->getName().empty())
        {
            // its the last entry in the container so its index is size - 1
            size_t idx = mTextureUnitStates.size() - 1;

            // allow 8 digit hex number. there should never be that many texture units.
            // This sprintf replaced a call to StringConverter::toString for performance reasons
            state->setName( StringUtil::format("%lx", static_cast<long>(idx)));
        }
        _notifyNeedsRecompile();
        _dirtyHash();

        mContentTypeLookupBuilt = false;
    }
    //-----------------------------------------------------------------------------
    TextureUnitState* Pass::getTextureUnitState(const String& name) const
    {
        OGRE_LOCK_MUTEX(mTexUnitChangeMutex);
        TextureUnitState* foundTUS = 0;

        // iterate through TUS Container to find a match
        for (auto *t : mTextureUnitStates)
        {
            if (t->getName() == name)
            {
                foundTUS = t;
                break;
            }
        }

        return foundTUS;
    }

    //-----------------------------------------------------------------------
    unsigned short Pass::getTextureUnitStateIndex(const TextureUnitState* state) const
    {
        OGRE_LOCK_MUTEX(mTexUnitChangeMutex);
        assert(state && "state is 0 in Pass::getTextureUnitStateIndex()");

        // only find index for state attached to this pass
        OgreAssert(state->getParent() == this, "TextureUnitState is not attached to this pass");
        auto i = std::find(mTextureUnitStates.begin(), mTextureUnitStates.end(), state);
        assert(i != mTextureUnitStates.end() && "state is supposed to attached to this pass");
        return static_cast<unsigned short>(std::distance(mTextureUnitStates.begin(), i));
    }

    //-----------------------------------------------------------------------
    Pass::TextureUnitStateIterator
        Pass::getTextureUnitStateIterator(void)
    {
        return TextureUnitStateIterator(mTextureUnitStates.begin(), mTextureUnitStates.end());
    }
    //-----------------------------------------------------------------------
    Pass::ConstTextureUnitStateIterator
        Pass::getTextureUnitStateIterator(void) const
    {
        return ConstTextureUnitStateIterator(mTextureUnitStates.begin(), mTextureUnitStates.end());
    }
    //-----------------------------------------------------------------------
    void Pass::removeTextureUnitState(unsigned short index)
    {
        OGRE_LOCK_MUTEX(mTexUnitChangeMutex);
        assert (index < mTextureUnitStates.size() && "Index out of bounds");

        TextureUnitStates::iterator i = mTextureUnitStates.begin() + index;
        OGRE_DELETE *i;
        mTextureUnitStates.erase(i);
        _notifyNeedsRecompile();
        _dirtyHash();
        mContentTypeLookupBuilt = false;
    }
    //-----------------------------------------------------------------------
    void Pass::removeAllTextureUnitStates(void)
    {
        OGRE_LOCK_MUTEX(mTexUnitChangeMutex);
        TextureUnitStates::iterator i, iend;
        iend = mTextureUnitStates.end();
        for (i = mTextureUnitStates.begin(); i != iend; ++i)
        {
            OGRE_DELETE *i;
        }
        mTextureUnitStates.clear();
        _notifyNeedsRecompile();
        _dirtyHash();
        mContentTypeLookupBuilt = false;
    }
    //-----------------------------------------------------------------------
    static void _getBlendFlags(SceneBlendType type, SceneBlendFactor& source, SceneBlendFactor& dest)
    {
        switch ( type )
        {
        case SBT_TRANSPARENT_ALPHA:
            source = SBF_SOURCE_ALPHA;
            dest = SBF_ONE_MINUS_SOURCE_ALPHA;
            return;
        case SBT_TRANSPARENT_COLOUR:
            source = SBF_SOURCE_COLOUR;
            dest = SBF_ONE_MINUS_SOURCE_COLOUR;
            return;
        case SBT_MODULATE:
            source = SBF_DEST_COLOUR;
            dest = SBF_ZERO;
            return;
        case SBT_ADD:
            source = SBF_ONE;
            dest = SBF_ONE;
            return;
        default:
        case SBT_REPLACE:
            source = SBF_ONE;
            dest = SBF_ZERO;
            return;
        }
    }
    //-----------------------------------------------------------------------
    void Pass::setSceneBlending(SceneBlendType sbt)
    {
        // Convert type into blend factors

        SceneBlendFactor source;
        SceneBlendFactor dest;
        _getBlendFlags(sbt, source, dest);

        // Set blend factors

        setSceneBlending(source, dest);
    }
    //-----------------------------------------------------------------------
    void Pass::setSeparateSceneBlending( const SceneBlendType sbt, const SceneBlendType sbta )
    {
        // Convert types into blend factors

        SceneBlendFactor source;
        SceneBlendFactor dest;
        _getBlendFlags(sbt, source, dest);

        SceneBlendFactor sourceAlpha;
        SceneBlendFactor destAlpha;
        _getBlendFlags(sbta, sourceAlpha, destAlpha);

        // Set blend factors

        setSeparateSceneBlending(source, dest, sourceAlpha, destAlpha);
    }

    //-----------------------------------------------------------------------
    void Pass::setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor)
    {
        mBlendState.sourceFactor = sourceFactor;
        mBlendState.sourceFactorAlpha = sourceFactor;
        mBlendState.destFactor = destFactor;
        mBlendState.destFactorAlpha = destFactor;
    }
    //-----------------------------------------------------------------------
    void Pass::setSeparateSceneBlending( const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor, const SceneBlendFactor sourceFactorAlpha, const SceneBlendFactor destFactorAlpha )
    {
        mBlendState.sourceFactor = sourceFactor;
        mBlendState.destFactor = destFactor;
        mBlendState.sourceFactorAlpha = sourceFactorAlpha;
        mBlendState.destFactorAlpha = destFactorAlpha;
    }
    //-----------------------------------------------------------------------
    void Pass::setSceneBlendingOperation(SceneBlendOperation op)
    {
        mBlendState.operation = op;
        mBlendState.alphaOperation = op;
    }
    //-----------------------------------------------------------------------
    void Pass::setSeparateSceneBlendingOperation(SceneBlendOperation op, SceneBlendOperation alphaOp)
    {
        mBlendState.operation = op;
        mBlendState.alphaOperation = alphaOp;
    }
    //-----------------------------------------------------------------------
    bool Pass::isTransparent(void) const
    {
        // Transparent if any of the destination colour is taken into account
        if (mBlendState.destFactor == SBF_ZERO &&
            mBlendState.sourceFactor != SBF_DEST_COLOUR &&
            mBlendState.sourceFactor != SBF_ONE_MINUS_DEST_COLOUR &&
            mBlendState.sourceFactor != SBF_DEST_ALPHA &&
            mBlendState.sourceFactor != SBF_ONE_MINUS_DEST_ALPHA)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    //-----------------------------------------------------------------------
    void Pass::setAlphaRejectSettings(CompareFunction func, unsigned char value, bool alphaToCoverage)
    {
        mAlphaRejectFunc = func;
        mAlphaRejectVal = value;
        mAlphaToCoverageEnabled = alphaToCoverage;
    }
    //-----------------------------------------------------------------------
    void Pass::setColourWriteEnabled(bool enabled)
    {
        mBlendState.writeR = enabled;
        mBlendState.writeG = enabled;
        mBlendState.writeB = enabled;
        mBlendState.writeA = enabled;
    }
    //-----------------------------------------------------------------------
    bool Pass::getColourWriteEnabled() const
    {
        return mBlendState.writeR || mBlendState.writeG || mBlendState.writeB ||
               mBlendState.writeA;
    }
    //-----------------------------------------------------------------------

    void Pass::setColourWriteEnabled(bool red, bool green, bool blue, bool alpha)
    {
        mBlendState.writeR = red;
        mBlendState.writeG = green;
        mBlendState.writeB = blue;
        mBlendState.writeA = alpha;
    }
    //-----------------------------------------------------------------------
    void Pass::getColourWriteEnabled(bool& red, bool& green, bool& blue, bool& alpha) const
    {
        red = mBlendState.writeR;
        green = mBlendState.writeG;
        blue = mBlendState.writeB;
        alpha = mBlendState.writeA;
    }
    //-----------------------------------------------------------------------
    void Pass::setIteratePerLight(bool enabled,
            bool onlyForOneLightType, Light::LightTypes lightType)
    {
        mIteratePerLight = enabled;
        mRunOnlyForOneLightType = onlyForOneLightType;
        mOnlyLightType = lightType;
    }
    //-----------------------------------------------------------------------
    void Pass::setManualCullingMode(ManualCullingMode mode)
    {
        mManualCullMode = mode;
    }
    //-----------------------------------------------------------------------
    ManualCullingMode Pass::getManualCullingMode(void) const
    {
        return mManualCullMode;
    }
    //-----------------------------------------------------------------------
    void Pass::setFog(bool overrideScene, FogMode mode, const ColourValue& colour, float density, float start, float end)
    {
        mFogOverride = overrideScene;
        if (overrideScene)
        {
            mFogMode = mode;
            mFogColour = colour;
            mFogStart = start;
            mFogEnd = end;
            mFogDensity = density;
        }
    }
    //-----------------------------------------------------------------------
    void Pass::setDepthBias(float constantBias, float slopeScaleBias)
    {
       mDepthBiasConstant = constantBias;
       mDepthBiasSlopeScale = slopeScaleBias;
    }
    //-----------------------------------------------------------------------
    Pass* Pass::_split(unsigned short numUnits)
    {
        OgreAssert(
            !isProgrammable(),
            "Programmable passes cannot be automatically split, define a fallback technique instead");

        if (mTextureUnitStates.size() > numUnits)
        {
            size_t start = mTextureUnitStates.size() - numUnits;

            Pass* newPass = mParent->createPass();

            TextureUnitStates::iterator istart, i, iend;
            iend = mTextureUnitStates.end();
            i = istart = mTextureUnitStates.begin() + start;
            // Set the new pass to fallback using scene blend
            newPass->setSceneBlending(
                (*i)->getColourBlendFallbackSrc(), (*i)->getColourBlendFallbackDest());
            // Fixup the texture unit 0   of new pass   blending method   to replace
            // all colour and alpha   with texture without adjustment, because we
            // assume it's detail texture.
            (*i)->setColourOperationEx(LBX_SOURCE1,   LBS_TEXTURE, LBS_CURRENT);
            (*i)->setAlphaOperation(LBX_SOURCE1, LBS_TEXTURE, LBS_CURRENT);

            // Add all the other texture unit states
            for (; i != iend; ++i)
            {
                // detach from parent first
                (*i)->_notifyParent(0);
                newPass->addTextureUnitState(*i);
            }
            // Now remove texture units from this Pass, we don't need to delete since they've
            // been transferred
            mTextureUnitStates.erase(istart, iend);
            _dirtyHash();
            mContentTypeLookupBuilt = false;
            return newPass;
        }
        return NULL;
    }
    //-----------------------------------------------------------------------------
    void Pass::_notifyIndex(unsigned short index)
    {
        if (mIndex != index)
        {
            mIndex = index;
            _dirtyHash();
        }
    }
    //-----------------------------------------------------------------------
    void Pass::_prepare(void)
    {
        // We assume the Technique only calls this when the material is being
        // prepared
        // prepare each TextureUnitState
        for (auto *t : mTextureUnitStates)
        {
            t->_prepare();
        }

    }
    //-----------------------------------------------------------------------
    void Pass::_unprepare(void)
    {
        // unprepare each TextureUnitState
        for (auto *t : mTextureUnitStates)
        {
            t->_unprepare();
        }

    }
    //-----------------------------------------------------------------------
    void Pass::_load(void)
    {
        // We assume the Technique only calls this when the material is being
        // loaded
        // Load each TextureUnitState
        for (auto *t : mTextureUnitStates)
        {
            t->_load();
        }

        // Load programs
        for (const auto& u : mProgramUsage)
            if(u) u->_load();

        if (mHashDirtyQueued)
        {
            _dirtyHash();
        }

    }
    //-----------------------------------------------------------------------
    void Pass::_unload(void)
    {
        // Unload each TextureUnitState
        for (auto *t : mTextureUnitStates)
        {
            t->_unload();
        }

        // TODO Unload programs
    }
    //-----------------------------------------------------------------------
    void Pass::setVertexProgram(const String& name, bool resetParams)
    {
        setGpuProgram(GPT_VERTEX_PROGRAM, name, resetParams);
    }
    //-----------------------------------------------------------------------
    void Pass::setGpuProgramParameters(GpuProgramType type, const GpuProgramParametersSharedPtr& params)
    {
        OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);

        const auto& programUsage = getProgramUsage(type);
        if (!programUsage)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "This pass does not have this program type assigned!");
        }
        programUsage->setParameters(params);
    }
    void Pass::setVertexProgramParameters(GpuProgramParametersSharedPtr params)
    {
        setGpuProgramParameters(GPT_VERTEX_PROGRAM, params);
    }
    //-----------------------------------------------------------------------
    void Pass::setGpuProgram(GpuProgramType type, const GpuProgramPtr& program, bool resetParams)
    {
        OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);

        std::unique_ptr<GpuProgramUsage>& programUsage = getProgramUsage(type);

        // Turn off fragment program if name blank
        if (!program)
        {
            programUsage.reset();
        }
        else
        {
            if (!programUsage)
            {
                programUsage = std::make_unique<GpuProgramUsage>(type, this);
            }
            programUsage->setProgram(program, resetParams);
        }
        // Needs recompilation
        _notifyNeedsRecompile();

        if( Pass::getHashFunction() == Pass::getBuiltinHashFunction( Pass::MIN_GPU_PROGRAM_CHANGE ) )
        {
            _dirtyHash();
        }
    }

    void Pass::setGpuProgram(GpuProgramType type, const String& name, bool resetParams)
    {
        if (getGpuProgramName(type) == name)
            return;

        GpuProgramPtr program;
        if (!name.empty())
            program = GpuProgramUsage::_getProgramByName(name, getResourceGroup(), type);

        setGpuProgram(type, program, resetParams);
    }

    void Pass::setFragmentProgram(const String& name, bool resetParams)
    {
        setGpuProgram(GPT_FRAGMENT_PROGRAM, name, resetParams);
    }
    //-----------------------------------------------------------------------
    void Pass::setFragmentProgramParameters(GpuProgramParametersSharedPtr params)
    {
        setGpuProgramParameters(GPT_FRAGMENT_PROGRAM, params);
    }
    //-----------------------------------------------------------------------
    void Pass::setGeometryProgram(const String& name, bool resetParams)
    {
        setGpuProgram(GPT_GEOMETRY_PROGRAM, name, resetParams);
    }
    //-----------------------------------------------------------------------
    void Pass::setGeometryProgramParameters(GpuProgramParametersSharedPtr params)
    {
        setGpuProgramParameters(GPT_GEOMETRY_PROGRAM, params);
    }
    //-----------------------------------------------------------------------
    void Pass::setTessellationHullProgram(const String& name, bool resetParams)
    {
        setGpuProgram(GPT_HULL_PROGRAM, name, resetParams);
    }
    //-----------------------------------------------------------------------
    void Pass::setTessellationHullProgramParameters(GpuProgramParametersSharedPtr params)
    {
        setGpuProgramParameters(GPT_HULL_PROGRAM, params);
    }
    //-----------------------------------------------------------------------
    void Pass::setTessellationDomainProgram(const String& name, bool resetParams)
    {
        setGpuProgram(GPT_DOMAIN_PROGRAM, name, resetParams);
    }
    //-----------------------------------------------------------------------
    void Pass::setTessellationDomainProgramParameters(GpuProgramParametersSharedPtr params)
    {
        setGpuProgramParameters(GPT_DOMAIN_PROGRAM, params);
    }
    //-----------------------------------------------------------------------
    void Pass::setComputeProgram(const String& name, bool resetParams)
    {
        setGpuProgram(GPT_COMPUTE_PROGRAM, name, resetParams);
    }
    //-----------------------------------------------------------------------
    void Pass::setComputeProgramParameters(GpuProgramParametersSharedPtr params)
    {
        setGpuProgramParameters(GPT_COMPUTE_PROGRAM, params);
    }
    //-----------------------------------------------------------------------
    const GpuProgramParametersSharedPtr& Pass::getGpuProgramParameters(GpuProgramType type) const
    {
        OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        const auto& programUsage = getProgramUsage(type);
        if (!programUsage)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "This pass has no " + to_string(type) + " program");
        }
        return programUsage->getParameters();
    }

    GpuProgramParametersSharedPtr Pass::getVertexProgramParameters(void) const
    {
        return getGpuProgramParameters(GPT_VERTEX_PROGRAM);
    }

    std::unique_ptr<GpuProgramUsage>& Pass::getProgramUsage(GpuProgramType programType) {
        return mProgramUsage[programType];
    }

    const std::unique_ptr<GpuProgramUsage>& Pass::getProgramUsage(GpuProgramType programType) const
    {
        return mProgramUsage[programType];
    }

    bool Pass::hasGpuProgram(GpuProgramType programType) const {
        return getProgramUsage(programType) != NULL;
    }
    const GpuProgramPtr& Pass::getGpuProgram(GpuProgramType programType) const
	{
        OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        OgreAssert(mProgramUsage[programType], "check whether program is available using hasGpuProgram()");
        return mProgramUsage[programType]->getProgram();
	}
    //-----------------------------------------------------------------------
    const String& Pass::getGpuProgramName(GpuProgramType type) const
    {
        OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);

        const std::unique_ptr<GpuProgramUsage>& programUsage = getProgramUsage(type);
        if (!programUsage)
            return BLANKSTRING;
        else
            return programUsage->getProgramName();
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getFragmentProgramParameters(void) const
    {
        return getGpuProgramParameters(GPT_FRAGMENT_PROGRAM);
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getGeometryProgramParameters(void) const
    {
        return getGpuProgramParameters(GPT_GEOMETRY_PROGRAM);
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getTessellationHullProgramParameters(void) const
    {
        return getGpuProgramParameters(GPT_HULL_PROGRAM);
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getTessellationDomainProgramParameters(void) const
    {
        return getGpuProgramParameters(GPT_DOMAIN_PROGRAM);
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getComputeProgramParameters(void) const
    {
        return getGpuProgramParameters(GPT_COMPUTE_PROGRAM);
    }
    //-----------------------------------------------------------------------
    bool Pass::isLoaded(void) const
    {
        return mParent->isLoaded();
    }
    //-----------------------------------------------------------------------
    void Pass::_recalculateHash(void)
    {
        /* Hash format is 32-bit, divided as follows (high to low bits)
           bits   purpose
            4     Pass index (i.e. max 16 passes!)
           28     Pass contents
       */
        mHash = (*msHashFunc)(this);

        // overwrite the 4 upper bits with pass index
        mHash = (uint32(mIndex) << 28) | (mHash >> 4);
    }
    //-----------------------------------------------------------------------
    void Pass::_dirtyHash(void)
    {
        if (mQueuedForDeletion)
            return;

        Material* mat = mParent->getParent();
        if (mat->isLoading() || mat->isLoaded())
        {
            OGRE_LOCK_MUTEX(msDirtyHashListMutex);
            // Mark this hash as for follow up
            msDirtyHashList.insert(this);
            mHashDirtyQueued = false;
        }
        else
        {
            mHashDirtyQueued = true;
        }
    }
    //---------------------------------------------------------------------
    void Pass::clearDirtyHashList(void) 
    { 
            OGRE_LOCK_MUTEX(msDirtyHashListMutex);
        msDirtyHashList.clear(); 
    }
    //-----------------------------------------------------------------------
    void Pass::_notifyNeedsRecompile(void)
    {
        if (!mQueuedForDeletion)
            mParent->_notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    void Pass::setTextureFiltering(TextureFilterOptions filterType)
    {
        OGRE_LOCK_MUTEX(mTexUnitChangeMutex);
        for (auto *t : mTextureUnitStates)
        {
            t->setTextureFiltering(filterType);
        }
    }
    // --------------------------------------------------------------------
    void Pass::setTextureAnisotropy(unsigned int maxAniso)
    {
        OGRE_LOCK_MUTEX(mTexUnitChangeMutex);
        for (auto *t : mTextureUnitStates)
        {
            t->setTextureAnisotropy(maxAniso);
        }
    }
    //-----------------------------------------------------------------------
    void Pass::_updateAutoParams(const AutoParamDataSource* source, uint16 mask) const
    {
        for(int i = 0; i < GPT_COUNT; i++)
        {
            const auto& programUsage = getProgramUsage(GpuProgramType(i));
            if (programUsage)
            {
                // Update program auto params
                programUsage->getParameters()->_updateAutoParams(source, mask);
            }
        }
    }
    //-----------------------------------------------------------------------
    void Pass::processPendingPassUpdates(void)
    {
        {
            OGRE_LOCK_MUTEX(msPassGraveyardMutex);
            // Delete items in the graveyard
            for (auto& i : msPassGraveyard)
            {
                OGRE_DELETE i;
            }
            msPassGraveyard.clear();
        }
        PassSet tempDirtyHashList;
        {
            OGRE_LOCK_MUTEX(msDirtyHashListMutex);
            // The dirty ones will have been removed from the groups above using the old hash now
            tempDirtyHashList.swap(msDirtyHashList);
        }
        for (auto *p : tempDirtyHashList)
        {
            p->_recalculateHash();
        }
    }
    //-----------------------------------------------------------------------
    void Pass::queueForDeletion(void)
    {
        mQueuedForDeletion = true;

        removeAllTextureUnitStates();
        for (auto& u : mProgramUsage)
            u.reset();

        // remove from dirty list, if there
        {
            OGRE_LOCK_MUTEX(msDirtyHashListMutex);
            msDirtyHashList.erase(this);
        }
        {
            OGRE_LOCK_MUTEX(msPassGraveyardMutex);
            msPassGraveyard.insert(this);
        }
    }
    //-----------------------------------------------------------------------
    bool Pass::isAmbientOnly(void) const
    {
        // treat as ambient if lighting is off, or colour write is off,
        // or all non-ambient (& emissive) colours are black
        // NB a vertex program could override this, but passes using vertex
        // programs are expected to indicate they are ambient only by
        // setting the state so it matches one of the conditions above, even
        // though this state is not used in rendering.
        return (!mLightingEnabled || !getColourWriteEnabled() ||
            (mDiffuse == ColourValue::Black &&
             mSpecular == ColourValue::Black));
    }
    //-----------------------------------------------------------------------
    const String& Pass::getResourceGroup(void) const
    {
        return mParent->getResourceGroup();
    }
    //-----------------------------------------------------------------------
    unsigned short Pass::_getTextureUnitWithContentTypeIndex(
        TextureUnitState::ContentType contentType, unsigned short index) const
    {
        if (!mContentTypeLookupBuilt)
        {
            mShadowContentTypeLookup.clear();
            for (unsigned short i = 0; i < mTextureUnitStates.size(); ++i)
            {
                if (mTextureUnitStates[i]->getContentType() == TextureUnitState::CONTENT_SHADOW)
                {
                    mShadowContentTypeLookup.push_back(i);
                }
            }
            mContentTypeLookupBuilt = true;
        }

        switch(contentType)
        {
        case TextureUnitState::CONTENT_SHADOW:
            if (index < mShadowContentTypeLookup.size())
            {
                return mShadowContentTypeLookup[index];
            }
            break;
        default:
            // Simple iteration
            for (unsigned short i = 0; i < mTextureUnitStates.size(); ++i)
            {
                if (mTextureUnitStates[i]->getContentType() == TextureUnitState::CONTENT_SHADOW)
                {
                    if (index == 0)
                    {
                        return i;
                    }
                    else
                    {
                        --index;
                    }
                }
            }
            break;
        }

        // not found - return out of range
        return static_cast<unsigned short>(mTextureUnitStates.size() + 1);

    }
}
