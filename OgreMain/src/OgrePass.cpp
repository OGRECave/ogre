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

#include "OgreGpuProgramUsage.h"
#include "OgreTextureUnitState.h"

namespace Ogre {

    /** Default pass hash function.
    @remarks
        Tries to minimise the number of texture changes.
    */
    struct MinTextureStateChangeHashFunc : public Pass::HashFunc
    {
        uint32 operator()(const Pass* p) const
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
    @remarks
        Tries to minimise the number of GPU program changes.
    */
    struct MinGpuProgramChangeHashFunc : public Pass::HashFunc
    {
        uint32 operator()(const Pass* p) const
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
        , mIndex(index)
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
        , mNormaliseNormals(false)
        , mPolygonModeOverrideable(true)
        , mFogOverride(false)
        , mQueuedForDeletion(false)
        , mLightScissoring(false)
        , mLightClipPlanes(false)
        , mPointSpritesEnabled(false)
        , mPointAttenuationEnabled(false)
        , mContentTypeLookupBuilt(false)
        , mAlphaRejectVal(0)
        , mDepthFunc(CMPF_LESS_EQUAL)
        , mDepthBiasConstant(0.0f)
        , mDepthBiasSlopeScale(0.0f)
        , mDepthBiasPerIteration(0.0f)
        , mAlphaRejectFunc(CMPF_ALWAYS_PASS)
        , mCullMode(CULL_CLOCKWISE)
        , mManualCullMode(MANUAL_CULL_BACK)
        , mMaxSimultaneousLights(OGRE_MAX_SIMULTANEOUS_LIGHTS)
        , mStartLight(0)
        , mLightsPerIteration(1)
        , mOnlyLightType(Light::LT_POINT)
        , mLightMask(0xFFFFFFFF)
        , mShadeOptions(SO_GOURAUD)
        , mPolygonMode(PM_SOLID)
        , mFogMode(FOG_NONE)
        , mFogColour(ColourValue::White)
        , mFogStart(0.0)
        , mFogEnd(1.0)
        , mFogDensity(0.001)
        , mPassIterationCount(1)
        , mLineWidth(1.0f)
        , mPointMinSize(0.0f)
        , mPointMaxSize(0.0f)
        , mPointAttenution(1.0f, 1.0f, 0.0f, 0.0f)
        , mIlluminationStage(IS_UNKNOWN)
    {
        // init the hash inline
        _recalculateHash();
   }

    //-----------------------------------------------------------------------------
    Pass::Pass(Technique *parent, unsigned short index, const Pass& oth)
        : mParent(parent), mIndex(index), mQueuedForDeletion(false), mPassIterationCount(1)
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
        mNormaliseNormals = oth.mNormaliseNormals;
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
            othUsage ? programUsage.reset(new GpuProgramUsage(*othUsage, this)) : programUsage.reset();
        }

        mShadowCasterVertexProgramUsage.reset();
        if (oth.mShadowCasterVertexProgramUsage)
        {
            mShadowCasterVertexProgramUsage.reset(new GpuProgramUsage(*(oth.mShadowCasterVertexProgramUsage), this));
        }

        mShadowCasterFragmentProgramUsage.reset();
        if (oth.mShadowCasterFragmentProgramUsage)
        {
            mShadowCasterFragmentProgramUsage.reset(new GpuProgramUsage(*(oth.mShadowCasterFragmentProgramUsage), this));
        }

        mShadowReceiverVertexProgramUsage.reset();
        if (oth.mShadowReceiverVertexProgramUsage)
        {
            mShadowReceiverVertexProgramUsage.reset(new GpuProgramUsage(*(oth.mShadowReceiverVertexProgramUsage), this));
        }

        mShadowReceiverFragmentProgramUsage.reset();
        if (oth.mShadowReceiverFragmentProgramUsage)
        {
            mShadowReceiverFragmentProgramUsage.reset(new GpuProgramUsage(*(oth.mShadowReceiverFragmentProgramUsage), this));
        }

        TextureUnitStates::const_iterator i, iend;

        // Clear texture units but doesn't notify need recompilation in the case
        // we are cloning, The parent material will take care of this.
        iend = mTextureUnitStates.end();
        for (i = mTextureUnitStates.begin(); i != iend; ++i)
        {
            OGRE_DELETE *i;
        }

        mTextureUnitStates.clear();

        // Copy texture units
        iend = oth.mTextureUnitStates.end();
        for (i = oth.mTextureUnitStates.begin(); i != iend; ++i)
        {
            TextureUnitState* t = OGRE_NEW TextureUnitState(this, *(*i));
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
        TextureUnitStates::const_iterator i, iend;
        iend = mTextureUnitStates.end();
        for (i = mTextureUnitStates.begin(); i != iend; ++i)
        {
            memSize += (*i)->calculateSize();
        }
        for(const auto& u : mProgramUsage)
            memSize += u ? u->calculateSize() : 0;

        if(mShadowCasterVertexProgramUsage)
            memSize += mShadowCasterVertexProgramUsage->calculateSize();
        if(mShadowCasterFragmentProgramUsage)
            memSize += mShadowCasterFragmentProgramUsage->calculateSize();
        if(mShadowReceiverVertexProgramUsage)
            memSize += mShadowReceiverVertexProgramUsage->calculateSize();
        if(mShadowReceiverFragmentProgramUsage)
            memSize += mShadowReceiverFragmentProgramUsage->calculateSize();
        return memSize;
    }
    //-----------------------------------------------------------------------
    void Pass::setName(const String& name)
    {
        mName = name;
    }
    //-----------------------------------------------------------------------
    void Pass::setPointSpritesEnabled(bool enabled)
    {
        mPointSpritesEnabled = enabled;
    }
    //-----------------------------------------------------------------------
    bool Pass::getPointSpritesEnabled(void) const
    {
        return mPointSpritesEnabled;
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
    bool Pass::isPointAttenuationEnabled(void) const
    {
        return mPointAttenuationEnabled;
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
    void Pass::setAmbient(const ColourValue& ambient)
    {
        mAmbient = ambient;
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
    void Pass::setDiffuse(const ColourValue& diffuse)
    {
        mDiffuse = diffuse;
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
    void Pass::setSpecular(const ColourValue& specular)
    {
        mSpecular = specular;
    }
    //-----------------------------------------------------------------------
    void Pass::setShininess(Real val)
    {
        mShininess = val;
    }
    //-----------------------------------------------------------------------
    void Pass::setSelfIllumination(float red, float green, float blue)
    {
        mEmissive.r = red;
        mEmissive.g = green;
        mEmissive.b = blue;

    }
    //-----------------------------------------------------------------------
    void Pass::setSelfIllumination(const ColourValue& selfIllum)
    {
        mEmissive = selfIllum;
    }
    //-----------------------------------------------------------------------
    void Pass::setVertexColourTracking(TrackVertexColourType tracking)
    {
        mTracking = tracking;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Pass::getAmbient(void) const
    {
        return mAmbient;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Pass::getDiffuse(void) const
    {
        return mDiffuse;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Pass::getSpecular(void) const
    {
        return mSpecular;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Pass::getSelfIllumination(void) const
    {
        return mEmissive;
    }
    //-----------------------------------------------------------------------
    Real Pass::getShininess(void) const
    {
        return mShininess;
    }
    //-----------------------------------------------------------------------
    TrackVertexColourType Pass::getVertexColourTracking(void) const
    {
        return mTracking;
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

        assert(state && "state is 0 in Pass::addTextureUnitState()");
        if (state)
        {
            // only attach TUS to pass if TUS does not belong to another pass
            if ((state->getParent() == 0) || (state->getParent() == this))
            {
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
                    
                    OGRE_IGNORE_DEPRECATED_BEGIN
                    /** since the name was never set and a default one has been made, clear the alias name
                     so that when the texture unit name is set by the user, the alias name will be set to
                     that name
                    */
                    state->setTextureNameAlias(BLANKSTRING);
                    OGRE_IGNORE_DEPRECATED_END
                }
                _notifyNeedsRecompile();
                _dirtyHash();
            }
            else
            {
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "TextureUnitState already attached to another pass",
                    "Pass:addTextureUnitState");

            }
            mContentTypeLookupBuilt = false;
        }
    }
    //-----------------------------------------------------------------------
    TextureUnitState* Pass::getTextureUnitState(unsigned short index) const
    {
            OGRE_LOCK_MUTEX(mTexUnitChangeMutex);
        assert (index < mTextureUnitStates.size() && "Index out of bounds");
        return mTextureUnitStates[index];
    }
    //-----------------------------------------------------------------------------
    TextureUnitState* Pass::getTextureUnitState(const String& name) const
    {
            OGRE_LOCK_MUTEX(mTexUnitChangeMutex);
        TextureUnitStates::const_iterator i    = mTextureUnitStates.begin();
        TextureUnitStates::const_iterator iend = mTextureUnitStates.end();
        TextureUnitState* foundTUS = 0;

        // iterate through TUS Container to find a match
        while (i != iend)
        {
            if ( (*i)->getName() == name )
            {
                foundTUS = (*i);
                break;
            }

            ++i;
        }

        return foundTUS;
    }

    //-----------------------------------------------------------------------
    unsigned short Pass::getTextureUnitStateIndex(const TextureUnitState* state) const
    {
        OGRE_LOCK_MUTEX(mTexUnitChangeMutex);
        assert(state && "state is 0 in Pass::getTextureUnitStateIndex()");

        // only find index for state attached to this pass
        if (state->getParent() == this)
        {
            TextureUnitStates::const_iterator i =
                std::find(mTextureUnitStates.begin(), mTextureUnitStates.end(), state);
            assert(i != mTextureUnitStates.end() && "state is supposed to attached to this pass");
            return static_cast<unsigned short>(std::distance(mTextureUnitStates.begin(), i));
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "TextureUnitState is not attached to this pass",
                "Pass:getTextureUnitStateIndex");
        }
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
    void Pass::_getBlendFlags(SceneBlendType type, SceneBlendFactor& source, SceneBlendFactor& dest)
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
        case SBT_REPLACE:
            source = SBF_ONE;
            dest = SBF_ZERO;
            return;
        }

        // Default to SBT_REPLACE

        source = SBF_ONE;
        dest = SBF_ZERO;
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
    SceneBlendFactor Pass::getSourceBlendFactor(void) const
    {
        return mBlendState.sourceFactor;
    }
    //-----------------------------------------------------------------------
    SceneBlendFactor Pass::getDestBlendFactor(void) const
    {
        return mBlendState.destFactor;
    }
    //-----------------------------------------------------------------------
    SceneBlendFactor Pass::getSourceBlendFactorAlpha(void) const
    {
        return mBlendState.sourceFactorAlpha ;
    }
    //-----------------------------------------------------------------------
    SceneBlendFactor Pass::getDestBlendFactorAlpha(void) const
    {
        return mBlendState.destFactorAlpha;
    }
    //-----------------------------------------------------------------------
    bool Pass::hasSeparateSceneBlending() const
    {
        return true;
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
    SceneBlendOperation Pass::getSceneBlendingOperation() const
    {
        return mBlendState.operation;
    }
    //-----------------------------------------------------------------------
    SceneBlendOperation Pass::getSceneBlendingOperationAlpha() const
    {
        return mBlendState.alphaOperation;
    }
    //-----------------------------------------------------------------------
    bool Pass::hasSeparateSceneBlendingOperations() const
    {
        return true;
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
    void Pass::setDepthCheckEnabled(bool enabled)
    {
        mDepthCheck = enabled;
    }
    //-----------------------------------------------------------------------
    bool Pass::getDepthCheckEnabled(void) const
    {
        return mDepthCheck;
    }
    //-----------------------------------------------------------------------
    void Pass::setDepthWriteEnabled(bool enabled)
    {
        mDepthWrite = enabled;
    }
    //-----------------------------------------------------------------------
    bool Pass::getDepthWriteEnabled(void) const
    {
        return mDepthWrite;
    }
    //-----------------------------------------------------------------------
    void Pass::setDepthFunction( CompareFunction func)
    {
        mDepthFunc = func;
    }
    //-----------------------------------------------------------------------
    CompareFunction Pass::getDepthFunction(void) const
    {
        return mDepthFunc;
    }
    //-----------------------------------------------------------------------
    void Pass::setAlphaRejectSettings(CompareFunction func, unsigned char value, bool alphaToCoverage)
    {
        mAlphaRejectFunc = func;
        mAlphaRejectVal = value;
        mAlphaToCoverageEnabled = alphaToCoverage;
    }
    //-----------------------------------------------------------------------
    void Pass::setAlphaRejectFunction(CompareFunction func)
    {
        mAlphaRejectFunc = func;
    }
    //-----------------------------------------------------------------------
    void Pass::setAlphaRejectValue(unsigned char val)
    {
        mAlphaRejectVal = val;
    }
    //---------------------------------------------------------------------
    void Pass::setAlphaToCoverageEnabled(bool enabled)
    {
        mAlphaToCoverageEnabled = enabled;
    }
    //-----------------------------------------------------------------------
    void Pass::setTransparentSortingEnabled(bool enabled)
    {
        mTransparentSorting = enabled;
    }
    //-----------------------------------------------------------------------
    bool Pass::getTransparentSortingEnabled(void) const
    {
        return mTransparentSorting;
    }
    //-----------------------------------------------------------------------
    void Pass::setTransparentSortingForced(bool enabled)
    {
        mTransparentSortingForced = enabled;
    }
    //-----------------------------------------------------------------------
    bool Pass::getTransparentSortingForced(void) const
    {
        return mTransparentSortingForced;
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
    void Pass::setCullingMode( CullingMode mode)
    {
        mCullMode = mode;
    }
    //-----------------------------------------------------------------------
    CullingMode Pass::getCullingMode(void) const
    {
        return mCullMode;
    }
    //-----------------------------------------------------------------------
    void Pass::setLightingEnabled(bool enabled)
    {
        mLightingEnabled = enabled;
    }
    //-----------------------------------------------------------------------
    bool Pass::getLightingEnabled(void) const
    {
        return mLightingEnabled;
    }
    //-----------------------------------------------------------------------
    void Pass::setMaxSimultaneousLights(unsigned short maxLights)
    {
        mMaxSimultaneousLights = maxLights;
    }
    //-----------------------------------------------------------------------
    unsigned short Pass::getMaxSimultaneousLights(void) const
    {
        return mMaxSimultaneousLights;
    }
    //-----------------------------------------------------------------------
    void Pass::setStartLight(unsigned short startLight)
    {
        mStartLight = startLight;
    }
    //-----------------------------------------------------------------------
    unsigned short Pass::getStartLight(void) const
    {
        return mStartLight;
    }
    //-----------------------------------------------------------------------
    void Pass::setLightMask(uint32 mask)
    {
        mLightMask = mask;
    }
    //-----------------------------------------------------------------------
    uint32 Pass::getLightMask() const
    {
        return mLightMask;
    }
    //-----------------------------------------------------------------------
    void Pass::setLightCountPerIteration(unsigned short c)
    {
        mLightsPerIteration = c;
    }
    //-----------------------------------------------------------------------
    unsigned short Pass::getLightCountPerIteration(void) const
    {
        return mLightsPerIteration;
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
    void Pass::setShadingMode(ShadeOptions mode)
    {
        mShadeOptions = mode;
    }
    //-----------------------------------------------------------------------
    ShadeOptions Pass::getShadingMode(void) const
    {
        return mShadeOptions;
    }
    //-----------------------------------------------------------------------
    void Pass::setPolygonMode(PolygonMode mode)
    {
        mPolygonMode = mode;
    }
    //-----------------------------------------------------------------------
    PolygonMode Pass::getPolygonMode(void) const
    {
        return mPolygonMode;
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
    void Pass::setFog(bool overrideScene, FogMode mode, const ColourValue& colour, Real density, Real start, Real end)
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
    bool Pass::getFogOverride(void) const
    {
        return mFogOverride;
    }
    //-----------------------------------------------------------------------
    FogMode Pass::getFogMode(void) const
    {
        return mFogMode;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Pass::getFogColour(void) const
    {
        return mFogColour;
    }
    //-----------------------------------------------------------------------
    Real Pass::getFogStart(void) const
    {
        return mFogStart;
    }
    //-----------------------------------------------------------------------
    Real Pass::getFogEnd(void) const
    {
        return mFogEnd;
    }
    //-----------------------------------------------------------------------
    Real Pass::getFogDensity(void) const
    {
        return mFogDensity;
    }
    //-----------------------------------------------------------------------
    void Pass::setDepthBias(float constantBias, float slopeScaleBias)
    {
       mDepthBiasConstant = constantBias;
       mDepthBiasSlopeScale = slopeScaleBias;
    }
    //-----------------------------------------------------------------------
    float Pass::getDepthBiasConstant(void) const
    {
        return mDepthBiasConstant;
    }
    //-----------------------------------------------------------------------
    float Pass::getDepthBiasSlopeScale(void) const
    {
        return mDepthBiasSlopeScale;
    }
    //---------------------------------------------------------------------
    void Pass::setIterationDepthBias(float biasPerIteration)
    {
        mDepthBiasPerIteration = biasPerIteration;
    }
    //---------------------------------------------------------------------
    float Pass::getIterationDepthBias() const
    {
        return mDepthBiasPerIteration;
    }
    //-----------------------------------------------------------------------
    Pass* Pass::_split(unsigned short numUnits)
    {
        if (isProgrammable())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Programmable passes cannot be "
                "automatically split, define a fallback technique instead.",
                "Pass:_split");
        }

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
        TextureUnitStates::iterator i, iend;
        iend = mTextureUnitStates.end();
        for (i = mTextureUnitStates.begin(); i != iend; ++i)
        {
            (*i)->_prepare();
        }

    }
    //-----------------------------------------------------------------------
    void Pass::_unprepare(void)
    {
        // unprepare each TextureUnitState
        TextureUnitStates::iterator i, iend;
        iend = mTextureUnitStates.end();
        for (i = mTextureUnitStates.begin(); i != iend; ++i)
        {
            (*i)->_unprepare();
        }

    }
    //-----------------------------------------------------------------------
    void Pass::_load(void)
    {
        // We assume the Technique only calls this when the material is being
        // loaded

        // Load each TextureUnitState
        TextureUnitStates::iterator i, iend;
        iend = mTextureUnitStates.end();
        for (i = mTextureUnitStates.begin(); i != iend; ++i)
        {
            (*i)->_load();
        }

        // Load programs
        for (const auto& u : mProgramUsage)
            if(u) u->_load();

        if (mShadowCasterVertexProgramUsage)
        {
            // Load vertex program
            mShadowCasterVertexProgramUsage->_load();
        }
        if (mShadowCasterFragmentProgramUsage)
        {
            // Load fragment program
            mShadowCasterFragmentProgramUsage->_load();
        }
        if (mShadowReceiverVertexProgramUsage)
        {
            // Load vertex program
            mShadowReceiverVertexProgramUsage->_load();
        }

        if (mShadowReceiverFragmentProgramUsage)
        {
            // Load Fragment program
            mShadowReceiverFragmentProgramUsage->_load();
        }

        if (mHashDirtyQueued)
        {
            _dirtyHash();
        }

    }
    //-----------------------------------------------------------------------
    void Pass::_unload(void)
    {
        // Unload each TextureUnitState
        TextureUnitStates::iterator i, iend;
        iend = mTextureUnitStates.end();
        for (i = mTextureUnitStates.begin(); i != iend; ++i)
        {
            (*i)->_unload();
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
                programUsage.reset(new GpuProgramUsage(type, this));
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
            OGRE_EXCEPT (Exception::ERR_INVALIDPARAMS,
                "This pass does not have this program type assigned!");
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
    const GpuProgramPtr& Pass::getVertexProgram(void) const
    {
        return getGpuProgram(GPT_VERTEX_PROGRAM);
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
    const GpuProgramPtr& Pass::getFragmentProgram(void) const
    {
        return getGpuProgram(GPT_FRAGMENT_PROGRAM);
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getGeometryProgramParameters(void) const
    {
        return getGpuProgramParameters(GPT_GEOMETRY_PROGRAM);
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getGeometryProgram(void) const
    {
        return getGpuProgram(GPT_GEOMETRY_PROGRAM);
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getTessellationHullProgramParameters(void) const
    {
        return getGpuProgramParameters(GPT_HULL_PROGRAM);
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getTessellationHullProgram(void) const
    {
        return getGpuProgram(GPT_HULL_PROGRAM);
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getTessellationDomainProgramParameters(void) const
    {
        return getGpuProgramParameters(GPT_DOMAIN_PROGRAM);
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getTessellationDomainProgram(void) const
    {
        return getGpuProgram(GPT_DOMAIN_PROGRAM);
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getComputeProgramParameters(void) const
    {
        return getGpuProgramParameters(GPT_COMPUTE_PROGRAM);
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getComputeProgram(void) const
    {
        return getGpuProgram(GPT_COMPUTE_PROGRAM);
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

        TextureUnitStates::iterator i, iend;
        iend = mTextureUnitStates.end();
        for (i = mTextureUnitStates.begin(); i != iend; ++i)
        {
            (*i)->setTextureFiltering(filterType);
        }
    }
    // --------------------------------------------------------------------
    void Pass::setTextureAnisotropy(unsigned int maxAniso)
    {
        OGRE_LOCK_MUTEX(mTexUnitChangeMutex);
        TextureUnitStates::iterator i, iend;
        iend = mTextureUnitStates.end();
        for (i = mTextureUnitStates.begin(); i != iend; ++i)
        {
            (*i)->setTextureAnisotropy(maxAniso);
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
            PassSet::iterator i, iend;
            iend = msPassGraveyard.end();
            for (i = msPassGraveyard.begin(); i != iend; ++i)
            {
                OGRE_DELETE *i;
            }
            msPassGraveyard.clear();
        }
        PassSet tempDirtyHashList;
        {
                    OGRE_LOCK_MUTEX(msDirtyHashListMutex);
            // The dirty ones will have been removed from the groups above using the old hash now
            tempDirtyHashList.swap(msDirtyHashList);
        }
        PassSet::iterator i, iend;
        iend = tempDirtyHashList.end();
        for (i = tempDirtyHashList.begin(); i != iend; ++i)
        {
            Pass* p = *i;
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

        mShadowCasterVertexProgramUsage.reset();
        mShadowCasterFragmentProgramUsage.reset();
        mShadowReceiverVertexProgramUsage.reset();
        mShadowReceiverFragmentProgramUsage.reset();

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
    void Pass::setShadowCasterVertexProgram(const String& name)
    {
        // Turn off vertex program if name blank
        if (name.empty())
        {
            mShadowCasterVertexProgramUsage.reset();
        }
        else
        {
            if (!mShadowCasterVertexProgramUsage)
            {
                mShadowCasterVertexProgramUsage.reset(new GpuProgramUsage(GPT_VERTEX_PROGRAM, this));
            }
            mShadowCasterVertexProgramUsage->setProgramName(name);
        }
        _notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    void Pass::setShadowCasterVertexProgramParameters(GpuProgramParametersSharedPtr params)
    {
        if (!mShadowCasterVertexProgramUsage)
        {
            OGRE_EXCEPT (Exception::ERR_INVALIDPARAMS,
                "This pass does not have a shadow caster vertex program assigned!",
                "Pass::setShadowCasterVertexProgramParameters");
        }
        mShadowCasterVertexProgramUsage->setParameters(params);
    }
    //-----------------------------------------------------------------------
    const String& Pass::getShadowCasterVertexProgramName(void) const
    {
        if (!mShadowCasterVertexProgramUsage)
            return BLANKSTRING;
        else
            return mShadowCasterVertexProgramUsage->getProgramName();
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getShadowCasterVertexProgramParameters(void) const
    {
        if (!mShadowCasterVertexProgramUsage)
        {
            OGRE_EXCEPT (Exception::ERR_INVALIDPARAMS,
                "This pass does not have a shadow caster vertex program assigned!",
                "Pass::getShadowCasterVertexProgramParameters");
        }
        return mShadowCasterVertexProgramUsage->getParameters();
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getShadowCasterVertexProgram(void) const
    {
        return mShadowCasterVertexProgramUsage->getProgram();
    }
    //-----------------------------------------------------------------------
    void Pass::setShadowCasterFragmentProgram(const String& name)
    {
        // Turn off fragment program if name blank
        if (name.empty())
        {
            mShadowCasterFragmentProgramUsage.reset();
        }
        else
        {
            if (!mShadowCasterFragmentProgramUsage)
            {
                mShadowCasterFragmentProgramUsage.reset(new GpuProgramUsage(GPT_FRAGMENT_PROGRAM, this));
            }
            mShadowCasterFragmentProgramUsage->setProgramName(name);
        }
        _notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    void Pass::setShadowCasterFragmentProgramParameters(GpuProgramParametersSharedPtr params)
    {
        if (!mShadowCasterFragmentProgramUsage &&
            !Root::getSingletonPtr()->getRenderSystem()->getCapabilities()->hasCapability(
                RSC_FIXED_FUNCTION))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "This pass does not have a shadow caster fragment program assigned!",
                        "Pass::setShadowCasterFragmentProgramParameters");
        }
        mShadowCasterFragmentProgramUsage->setParameters(params);
    }
    //-----------------------------------------------------------------------
    const String& Pass::getShadowCasterFragmentProgramName(void) const
    {
        if (!mShadowCasterFragmentProgramUsage)
            return BLANKSTRING;
        else
            return mShadowCasterFragmentProgramUsage->getProgramName();
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getShadowCasterFragmentProgramParameters(void) const
    {

        if (!mShadowCasterFragmentProgramUsage &&
            !Root::getSingletonPtr()->getRenderSystem()->getCapabilities()->hasCapability(
                RSC_FIXED_FUNCTION))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "This pass does not have a shadow caster fragment program assigned!",
                        "Pass::getShadowCasterFragmentProgramParameters");
        }

        return mShadowCasterFragmentProgramUsage->getParameters();
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getShadowCasterFragmentProgram(void) const
    {
        return mShadowCasterFragmentProgramUsage->getProgram();
    }
    //-----------------------------------------------------------------------
    void Pass::setShadowReceiverVertexProgram(const String& name)
    {
        // Turn off vertex program if name blank
        if (name.empty())
        {
            mShadowReceiverVertexProgramUsage.reset();
        }
        else
        {
            if (!mShadowReceiverVertexProgramUsage)
            {
                mShadowReceiverVertexProgramUsage.reset(new GpuProgramUsage(GPT_VERTEX_PROGRAM, this));
            }
            mShadowReceiverVertexProgramUsage->setProgramName(name);
        }
        _notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    void Pass::setShadowReceiverVertexProgramParameters(GpuProgramParametersSharedPtr params)
    {
        if (!mShadowReceiverVertexProgramUsage)
        {
            OGRE_EXCEPT (Exception::ERR_INVALIDPARAMS,
                "This pass does not have a shadow receiver vertex program assigned!",
                "Pass::setShadowReceiverVertexProgramParameters");
        }
        mShadowReceiverVertexProgramUsage->setParameters(params);
    }
    //-----------------------------------------------------------------------
    const String& Pass::getShadowReceiverVertexProgramName(void) const
    {
        if (!mShadowReceiverVertexProgramUsage)
            return BLANKSTRING;
        else
            return mShadowReceiverVertexProgramUsage->getProgramName();
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getShadowReceiverVertexProgramParameters(void) const
    {
        if (!mShadowReceiverVertexProgramUsage)
        {
            OGRE_EXCEPT (Exception::ERR_INVALIDPARAMS,
                "This pass does not have a shadow receiver vertex program assigned!",
                "Pass::getShadowReceiverVertexProgramParameters");
        }
        return mShadowReceiverVertexProgramUsage->getParameters();
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getShadowReceiverVertexProgram(void) const
    {
        return mShadowReceiverVertexProgramUsage->getProgram();
    }
    //-----------------------------------------------------------------------
    void Pass::setShadowReceiverFragmentProgram(const String& name)
    {
        // Turn off Fragment program if name blank
        if (name.empty())
        {
            mShadowReceiverFragmentProgramUsage.reset();
        }
        else
        {
            if (!mShadowReceiverFragmentProgramUsage)
            {
                mShadowReceiverFragmentProgramUsage.reset(new GpuProgramUsage(GPT_FRAGMENT_PROGRAM, this));
            }
            mShadowReceiverFragmentProgramUsage->setProgramName(name);
        }
        _notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    void Pass::setShadowReceiverFragmentProgramParameters(GpuProgramParametersSharedPtr params)
    {
        if (!mShadowReceiverFragmentProgramUsage)
        {
            OGRE_EXCEPT (Exception::ERR_INVALIDPARAMS,
                "This pass does not have a shadow receiver fragment program assigned!",
                "Pass::setShadowReceiverFragmentProgramParameters");
        }
        mShadowReceiverFragmentProgramUsage->setParameters(params);
    }
    //-----------------------------------------------------------------------
    const String& Pass::getShadowReceiverFragmentProgramName(void) const
    {
        if (!mShadowReceiverFragmentProgramUsage)
            return BLANKSTRING;
        else
            return mShadowReceiverFragmentProgramUsage->getProgramName();
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getShadowReceiverFragmentProgramParameters(void) const
    {
        if (!mShadowReceiverFragmentProgramUsage)
        {
            OGRE_EXCEPT (Exception::ERR_INVALIDPARAMS,
                "This pass does not have a shadow receiver fragment program assigned!",
                "Pass::getShadowReceiverFragmentProgramParameters");
        }
        return mShadowReceiverFragmentProgramUsage->getParameters();
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getShadowReceiverFragmentProgram(void) const
    {
        return mShadowReceiverFragmentProgramUsage->getProgram();
    }
    //-----------------------------------------------------------------------
    const String& Pass::getResourceGroup(void) const
    {
        return mParent->getResourceGroup();
    }

    //-----------------------------------------------------------------------
    bool Pass::applyTextureAliases(const AliasTextureNamePairList& aliasList, const bool apply) const
    {
        // iterate through each texture unit state and apply the texture alias if it applies
        TextureUnitStates::const_iterator i, iend;
        iend = mTextureUnitStates.end();
        bool testResult = false;

        for (i = mTextureUnitStates.begin(); i != iend; ++i)
        {
            OGRE_IGNORE_DEPRECATED_BEGIN
            if ((*i)->applyTextureAliases(aliasList, apply))
                testResult = true;
            OGRE_IGNORE_DEPRECATED_END
        }

        return testResult;

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
