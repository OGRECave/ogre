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

#include "OgrePass.h"
#include "OgreRoot.h"
#include "OgreTechnique.h"
#include "OgreException.h"
#include "OgreGpuProgramUsage.h"
#include "OgreTextureUnitState.h"
#include "OgreStringConverter.h"
#include "OgreHlmsLowLevelDatablock.h"
#include "OgreHlmsManager.h"
#include "OgreHlms.h"
#include "OgreMaterialManager.h"

namespace Ogre {
    AtomicScalar<uint32> Pass::gId = 0;

    //-----------------------------------------------------------------------------
    Pass::Pass(Technique* parent, unsigned short index)
        : mId(gId++)
        , mParent(parent)
        , mIndex(index)
        , mAmbient(ColourValue::White)
        , mDiffuse(ColourValue::White)
        , mSpecular(ColourValue::Black)
        , mEmissive(ColourValue::Black)
        , mShininess(0)
        , mTracking(TVC_NONE)
        , mDatablock(0)
        , mAlphaRejectFunc(CMPF_ALWAYS_PASS)
        , mAlphaRejectVal(0)
        , mMaxSimultaneousLights(OGRE_MAX_SIMULTANEOUS_LIGHTS)
        , mStartLight(0)
        , mIteratePerLight(false)
        , mLightsPerIteration(1)
        , mRunOnlyForOneLightType(false)
        , mOnlyLightType(Light::LT_POINT)
        , mLightMask(0xFFFFFFFF)
        , mShadeOptions(SO_GOURAUD)
        , mPolygonModeOverrideable(true)
        , mFogOverride(false)
        , mFogMode(FOG_NONE)
        , mFogColour(ColourValue::White)
        , mFogStart(0.0)
        , mFogEnd(1.0)
        , mFogDensity(0.001)
        , mVertexProgramUsage(0)
        , mShadowCasterVertexProgramUsage(0)
        , mShadowCasterFragmentProgramUsage(0)
        , mFragmentProgramUsage(0)
        , mGeometryProgramUsage(0)
        , mTessellationHullProgramUsage(0)
        , mTessellationDomainProgramUsage(0)
        , mComputeProgramUsage(0)
        , mPassIterationCount(1)
        , mPointSize(1.0f)
        , mPointMinSize(0.0f)
        , mPointMaxSize(0.0f)
        , mPointSpritesEnabled(false)
        , mPointAttenuationEnabled(false)
        , mLightScissoring(false)
        , mLightClipPlanes(false)
    {
        mPointAttenuationCoeffs[0] = 1.0f;
        mPointAttenuationCoeffs[1] = mPointAttenuationCoeffs[2] = 0.0f;

        // default name to index
        mName = StringConverter::toString(mIndex);

        HlmsManager *hlmsManager = Root::getSingleton().getHlmsManager();
        Hlms *hlms = hlmsManager->getHlms( HLMS_LOW_LEVEL );
        HlmsDatablock *datablock = hlms->createDatablock( IdString( mId ), "", HlmsMacroblock(),
                                                          HlmsBlendblock(), HlmsParamVec(), false );

        Material *parentMaterial = parent->getParent();

        assert( dynamic_cast<HlmsLowLevelDatablock*>( datablock ) );
        mDatablock = static_cast<HlmsLowLevelDatablock*>( datablock );
        mDatablock->mProxyMaterial = parentMaterial;
   }

    //-----------------------------------------------------------------------------
    Pass::Pass(Technique *parent, unsigned short index, const Pass& oth)
        :mId(gId++), mParent(parent), mIndex(index), mVertexProgramUsage(0),
        mShadowCasterVertexProgramUsage(0),
        mShadowCasterFragmentProgramUsage(0), mFragmentProgramUsage(0), 
        mGeometryProgramUsage(0), mTessellationHullProgramUsage(0)
        , mTessellationDomainProgramUsage(0), mComputeProgramUsage(0), mPassIterationCount(1)
    {
        HlmsManager *hlmsManager = Root::getSingleton().getHlmsManager();
        Hlms *hlms = hlmsManager->getHlms( HLMS_LOW_LEVEL );
        HlmsDatablock *datablock = hlms->createDatablock( IdString( mId ), "", HlmsMacroblock(),
                                                          HlmsBlendblock(), HlmsParamVec(), false );

        Material *parentMaterial = parent->getParent();

        assert( dynamic_cast<HlmsLowLevelDatablock*>( datablock ) );
        mDatablock = static_cast<HlmsLowLevelDatablock*>( datablock );
        mDatablock->mProxyMaterial = parentMaterial;

        *this = oth;
        mParent = parent;
        mIndex = index;
    }
    //-----------------------------------------------------------------------------
    Pass::~Pass()
    {
        OGRE_DELETE mVertexProgramUsage;
        OGRE_DELETE mFragmentProgramUsage;
        OGRE_DELETE mTessellationHullProgramUsage;
        OGRE_DELETE mTessellationDomainProgramUsage;
        OGRE_DELETE mGeometryProgramUsage;
        OGRE_DELETE mComputeProgramUsage;
        OGRE_DELETE mShadowCasterVertexProgramUsage;
        OGRE_DELETE mShadowCasterFragmentProgramUsage;

        removeAllTextureUnitStates();

        HlmsManager *hlmsManager = Root::getSingleton().getHlmsManager();
        Hlms *hlms = hlmsManager->getHlms( HLMS_LOW_LEVEL );
        hlms->destroyDatablock( mDatablock->getName() );
        mDatablock = 0;
    }
    //-----------------------------------------------------------------------------
    Pass& Pass::operator=(const Pass& oth)
    {
        //Do not copy mId
        mName = oth.mName;
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

        // The datablock belongs to us, so we don't copy everything (overwrites critical data)
        mDatablock->setMacroblock( oth.mDatablock->getMacroblock() );
        mDatablock->setBlendblock( oth.mDatablock->getBlendblock() );

        mAlphaRejectFunc = oth.mAlphaRejectFunc;
        mAlphaRejectVal = oth.mAlphaRejectVal;
        mMaxSimultaneousLights = oth.mMaxSimultaneousLights;
        mStartLight = oth.mStartLight;
        mIteratePerLight = oth.mIteratePerLight;
        mLightsPerIteration = oth.mLightsPerIteration;
        mRunOnlyForOneLightType = oth.mRunOnlyForOneLightType;
        mOnlyLightType = oth.mOnlyLightType;
        mShadeOptions = oth.mShadeOptions;
        mPolygonModeOverrideable = oth.mPolygonModeOverrideable;
        mPassIterationCount = oth.mPassIterationCount;
        mPointSize = oth.mPointSize;
        mPointMinSize = oth.mPointMinSize;
        mPointMaxSize = oth.mPointMaxSize;
        mPointSpritesEnabled = oth.mPointSpritesEnabled;
        mPointAttenuationEnabled = oth.mPointAttenuationEnabled;
        memcpy(mPointAttenuationCoeffs, oth.mPointAttenuationCoeffs, sizeof(Real)*3);
        mShadowContentTypeLookup = oth.mShadowContentTypeLookup;
        mLightScissoring = oth.mLightScissoring;
        mLightClipPlanes = oth.mLightClipPlanes;
        mLightMask = oth.mLightMask;

        OGRE_DELETE mVertexProgramUsage;
        if (oth.mVertexProgramUsage)
        {
            mVertexProgramUsage = OGRE_NEW GpuProgramUsage(*(oth.mVertexProgramUsage), this);
        }
        else
        {
            mVertexProgramUsage = NULL;
        }

        OGRE_DELETE mShadowCasterVertexProgramUsage;
        if (oth.mShadowCasterVertexProgramUsage)
        {
            mShadowCasterVertexProgramUsage = OGRE_NEW GpuProgramUsage(*(oth.mShadowCasterVertexProgramUsage), this);
        }
        else
        {
            mShadowCasterVertexProgramUsage = NULL;
        }

        OGRE_DELETE mShadowCasterFragmentProgramUsage;
        if (oth.mShadowCasterFragmentProgramUsage)
        {
            mShadowCasterFragmentProgramUsage = OGRE_NEW GpuProgramUsage(*(oth.mShadowCasterFragmentProgramUsage), this);
        }
        else
        {
            mShadowCasterFragmentProgramUsage = NULL;
        }

        OGRE_DELETE mFragmentProgramUsage;
        if (oth.mFragmentProgramUsage)
        {
            mFragmentProgramUsage = OGRE_NEW GpuProgramUsage(*(oth.mFragmentProgramUsage), this);
        }
        else
        {
            mFragmentProgramUsage = NULL;
        }

        OGRE_DELETE mGeometryProgramUsage;
        if (oth.mGeometryProgramUsage)
        {
            mGeometryProgramUsage = OGRE_NEW GpuProgramUsage(*(oth.mGeometryProgramUsage), this);
        }
        else
        {
            mGeometryProgramUsage = NULL;
        }

        OGRE_DELETE mTessellationHullProgramUsage;
        if (oth.mTessellationHullProgramUsage)
        {
            mTessellationHullProgramUsage = OGRE_NEW GpuProgramUsage(*(oth.mTessellationHullProgramUsage), this);
        }
        else
        {
            mTessellationHullProgramUsage = NULL;
        }

        OGRE_DELETE mTessellationDomainProgramUsage;
        if (oth.mTessellationDomainProgramUsage)
        {
            mTessellationDomainProgramUsage = OGRE_NEW GpuProgramUsage(*(oth.mTessellationDomainProgramUsage), this);
        }
        else
        {
            mTessellationDomainProgramUsage = NULL;
        }

        OGRE_DELETE mComputeProgramUsage;
        if (oth.mComputeProgramUsage)
        {
            mComputeProgramUsage = OGRE_NEW GpuProgramUsage(*(oth.mComputeProgramUsage), this);
        }
        else
        {
            mComputeProgramUsage = NULL;
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
        if(mVertexProgramUsage)
            memSize += mVertexProgramUsage->calculateSize();
        if(mShadowCasterVertexProgramUsage)
            memSize += mShadowCasterVertexProgramUsage->calculateSize();
        if(mShadowCasterFragmentProgramUsage)
            memSize += mShadowCasterFragmentProgramUsage->calculateSize();
        if(mFragmentProgramUsage)
            memSize += mFragmentProgramUsage->calculateSize();
        if(mGeometryProgramUsage)
            memSize += mGeometryProgramUsage->calculateSize();
        if(mTessellationHullProgramUsage)
            memSize += mTessellationHullProgramUsage->calculateSize();
        if(mTessellationDomainProgramUsage)
            memSize += mTessellationDomainProgramUsage->calculateSize();
        if(mComputeProgramUsage)
            memSize += mComputeProgramUsage->calculateSize();
        return memSize;
    }
    //-----------------------------------------------------------------------
    void Pass::setName(const String& name)
    {
        mName = name;
    }
    //-----------------------------------------------------------------------
    void Pass::setPointSize(Real ps)
    {
        mPointSize = ps;
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
    void Pass::setPointAttenuation(bool enabled,
        Real constant, Real linear, Real quadratic)
    {
        mPointAttenuationEnabled = enabled;
        mPointAttenuationCoeffs[0] = constant;
        mPointAttenuationCoeffs[1] = linear;
        mPointAttenuationCoeffs[2] = quadratic;
    }
    //-----------------------------------------------------------------------
    bool Pass::isPointAttenuationEnabled(void) const
    {
        return mPointAttenuationEnabled;
    }
    //-----------------------------------------------------------------------
    Real Pass::getPointAttenuationConstant(void) const
    {
        return mPointAttenuationCoeffs[0];
    }
    //-----------------------------------------------------------------------
    Real Pass::getPointAttenuationLinear(void) const
    {
        return mPointAttenuationCoeffs[1];
    }
    //-----------------------------------------------------------------------
    Real Pass::getPointAttenuationQuadratic(void) const
    {
        return mPointAttenuationCoeffs[2];
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
    void Pass::setAmbient(Real red, Real green, Real blue)
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
    void Pass::setDiffuse(Real red, Real green, Real blue, Real alpha)
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
    void Pass::setSpecular(Real red, Real green, Real blue, Real alpha)
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
    void Pass::setSelfIllumination(Real red, Real green, Real blue)
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
    Real Pass::getPointSize(void) const
    {
        return mPointSize;
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
                    char buff[9];
                    memset(buff, 0, 9);
                    sprintf(buff, "%lx", static_cast<long>(idx));
                    state->setName( buff );
                    
                    /** since the name was never set and a default one has been made, clear the alias name
                     so that when the texture unit name is set by the user, the alias name will be set to
                     that name
                    */
                    state->setTextureNameAlias(BLANKSTRING);
                }

                if( state->getContentType() == TextureUnitState::CONTENT_SHADOW )
                    mShadowContentTypeLookup.push_back( mTextureUnitStates.size()-1 );
            }
            else
            {
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "TextureUnitState already attached to another pass",
                    "Pass:addTextureUnitState");

            }
        }
    }
    //-----------------------------------------------------------------------
    TextureUnitState* Pass::getTextureUnitState( size_t index )
    {
        OGRE_LOCK_MUTEX(mTexUnitChangeMutex);
        assert (index < mTextureUnitStates.size() && "Index out of bounds");
        return mTextureUnitStates[index];
    }
    //-----------------------------------------------------------------------------
    TextureUnitState* Pass::getTextureUnitState(const String& name)
    {
        OGRE_LOCK_MUTEX(mTexUnitChangeMutex);
        TextureUnitStates::iterator i    = mTextureUnitStates.begin();
        TextureUnitStates::iterator iend = mTextureUnitStates.end();
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
    const TextureUnitState* Pass::getTextureUnitState( size_t index ) const
    {
            OGRE_LOCK_MUTEX(mTexUnitChangeMutex);
        assert (index < mTextureUnitStates.size() && "Index out of bounds");
        return mTextureUnitStates[index];
    }
    //-----------------------------------------------------------------------------
    const TextureUnitState* Pass::getTextureUnitState(const String& name) const
    {
            OGRE_LOCK_MUTEX(mTexUnitChangeMutex);
        TextureUnitStates::const_iterator i    = mTextureUnitStates.begin();
        TextureUnitStates::const_iterator iend = mTextureUnitStates.end();
        const TextureUnitState* foundTUS = 0;

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

        removeShadowContentTypeLookup( index );

        TextureUnitStates::iterator i = mTextureUnitStates.begin() + index;
        OGRE_DELETE *i;
        mTextureUnitStates.erase(i);
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
        mShadowContentTypeLookup.clear();
    }
    //-----------------------------------------------------------------------
    void Pass::recreateShadowContentTypeLookup(void)
    {
        mShadowContentTypeLookup.clear();
        for (unsigned short i = 0; i < mTextureUnitStates.size(); ++i)
        {
            if (mTextureUnitStates[i]->getContentType() == TextureUnitState::CONTENT_SHADOW)
                mShadowContentTypeLookup.push_back(i);
        }
    }
    //-----------------------------------------------------------------------
    void Pass::insertShadowContentTypeLookup( size_t textureUnitIndex )
    {
        //Shadow content lookup indexes may have been shifted (and a new index must be inserted)
        ContentTypeLookup::iterator itor = std::lower_bound( mShadowContentTypeLookup.begin(),
                                                             mShadowContentTypeLookup.end(),
                                                             textureUnitIndex );
        itor = mShadowContentTypeLookup.insert( itor, textureUnitIndex ) + 1;
        ContentTypeLookup::iterator end  = mShadowContentTypeLookup.end();

        while( itor != end )
        {
            *itor += 1;
            ++itor;
        }
    }
    //-----------------------------------------------------------------------
    void Pass::removeShadowContentTypeLookup( size_t textureUnitIndex )
    {
        //Shadow content lookup indexes may have been shifted (or removed)
        ContentTypeLookup::iterator itor = std::lower_bound( mShadowContentTypeLookup.begin(),
                                                             mShadowContentTypeLookup.end(),
                                                             textureUnitIndex );
        if( itor != mShadowContentTypeLookup.end() && *itor == textureUnitIndex )
        {
            const size_t idx = itor - mShadowContentTypeLookup.begin();
            mShadowContentTypeLookup.erase( itor );
            itor = mShadowContentTypeLookup.begin() + idx;
        }
        ContentTypeLookup::iterator end  = mShadowContentTypeLookup.end();

        while( itor != end )
        {
            *itor -= 1;
            ++itor;
        }
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
    HlmsDatablock* Pass::_getDatablock(void) const
    {
        return mDatablock;
    }
    //-----------------------------------------------------------------------
    void Pass::setMacroblock( const HlmsMacroblock &macroblock )
    {
        mDatablock->setMacroblock( macroblock );
    }
    //-----------------------------------------------------------------------
    const HlmsMacroblock* Pass::getMacroblock(void) const
    {
        return mDatablock->getMacroblock();
    }
    //-----------------------------------------------------------------------
    void Pass::setBlendblock( const HlmsBlendblock &blendblock )
    {
        mDatablock->setBlendblock( blendblock );
    }
    //-----------------------------------------------------------------------
    const HlmsBlendblock* Pass::getBlendblock(void) const
    {
        return mDatablock->getBlendblock();
    }
    //-----------------------------------------------------------------------
    bool Pass::isTransparent(void) const
    {
        return mDatablock->getBlendblock()->mIsTransparent;
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
    //-----------------------------------------------------------------------
    bool Pass::getColourWriteEnabled(void) const
    {
        return mDatablock->getBlendblock()->mBlendChannelMask != 0;
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
    void Pass::_notifyIndex(unsigned short index)
    {
        if (mIndex != index)
        {
            mIndex = index;
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
        if (mVertexProgramUsage)
        {
            // Load vertex program
            mVertexProgramUsage->_load();
        }
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

        if (mTessellationHullProgramUsage)
        {
            // Load tessellation control program
            mTessellationHullProgramUsage->_load();
        }

        if (mTessellationDomainProgramUsage)
        {
            // Load tessellation evaluation program
            mTessellationDomainProgramUsage->_load();
        }

        if (mGeometryProgramUsage)
        {
            // Load geometry program
            mGeometryProgramUsage->_load();
        }

        if (mFragmentProgramUsage)
        {
            // Load fragment program
            mFragmentProgramUsage->_load();
        }

        if (mComputeProgramUsage)
        {
            // Load compute program
            mComputeProgramUsage->_load();
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

        // Unload programs
        if (mVertexProgramUsage)
        {
            // TODO
        }
        if (mGeometryProgramUsage)
        {
            // TODO
        }
        if (mFragmentProgramUsage)
        {
            // TODO
        }
        if (mTessellationHullProgramUsage)
        {
            // TODO
        }
        if (mTessellationDomainProgramUsage)
        {
            // TODO
        }
        if (mComputeProgramUsage)
        {
            // TODO
        }
        if (mGeometryProgramUsage)
        {
            // TODO
        }
    }
    //-----------------------------------------------------------------------
    void Pass::setVertexProgram(const String& name, bool resetParams)
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);

        if (getVertexProgramName() != name)
        {
            // Turn off vertex program if name blank
            if (name.empty())
            {
                OGRE_DELETE mVertexProgramUsage;
                mVertexProgramUsage = NULL;
            }
            else
            {
                if (!mVertexProgramUsage)
                {
                    mVertexProgramUsage = OGRE_NEW GpuProgramUsage(GPT_VERTEX_PROGRAM, this);
                }
                mVertexProgramUsage->setProgramName(name, resetParams);
            }
            // Needs recompilation
            mParent->_notifyNeedsRecompile();
        }
    }
    //-----------------------------------------------------------------------
    void Pass::setVertexProgramParameters(GpuProgramParametersSharedPtr params)
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        if (!mVertexProgramUsage)
        {
            OGRE_EXCEPT (Exception::ERR_INVALIDPARAMS,
                "This pass does not have a vertex program assigned!",
                "Pass::setVertexProgramParameters");
        }
        mVertexProgramUsage->setParameters(params);
    }
    //-----------------------------------------------------------------------
    void Pass::setFragmentProgram(const String& name, bool resetParams)
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);

        if (getFragmentProgramName() != name)
        {
            // Turn off fragment program if name blank
            if (name.empty())
            {
                OGRE_DELETE mFragmentProgramUsage;
                mFragmentProgramUsage = NULL;
            }
            else
            {
                if (!mFragmentProgramUsage)
                {
                    mFragmentProgramUsage = OGRE_NEW GpuProgramUsage(GPT_FRAGMENT_PROGRAM, this);
                }
                mFragmentProgramUsage->setProgramName(name, resetParams);
            }
            // Needs recompilation
            mParent->_notifyNeedsRecompile();
        }
    }
    //-----------------------------------------------------------------------
    void Pass::setFragmentProgramParameters(GpuProgramParametersSharedPtr params)
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        if (!mFragmentProgramUsage)
        {
            OGRE_EXCEPT (Exception::ERR_INVALIDPARAMS,
                "This pass does not have a fragment program assigned!",
                "Pass::setFragmentProgramParameters");
        }
        mFragmentProgramUsage->setParameters(params);
    }
    //-----------------------------------------------------------------------
    void Pass::setGeometryProgram(const String& name, bool resetParams)
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);

        if (getGeometryProgramName() != name)
        {
            // Turn off geometry program if name blank
            if (name.empty())
            {
                OGRE_DELETE mGeometryProgramUsage;
                mGeometryProgramUsage = NULL;
            }
            else
            {
                if (!mGeometryProgramUsage)
                {
                    mGeometryProgramUsage = OGRE_NEW GpuProgramUsage(GPT_GEOMETRY_PROGRAM, this);
                }
                mGeometryProgramUsage->setProgramName(name, resetParams);
            }
            // Needs recompilation
            mParent->_notifyNeedsRecompile();
        }
    }
    //-----------------------------------------------------------------------
    void Pass::setGeometryProgramParameters(GpuProgramParametersSharedPtr params)
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        if (!mGeometryProgramUsage)
        {
            OGRE_EXCEPT (Exception::ERR_INVALIDPARAMS,
                "This pass does not have a geometry program assigned!",
                "Pass::setGeometryProgramParameters");
        }
        mGeometryProgramUsage->setParameters(params);
    }
    //-----------------------------------------------------------------------
    void Pass::setTessellationHullProgram(const String& name, bool resetParams)
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);

        if (getTessellationHullProgramName() != name)
        {
            // Turn off tessellation Hull program if name blank
            if (name.empty())
            {
                OGRE_DELETE mTessellationHullProgramUsage;
                mTessellationHullProgramUsage = NULL;
            }
            else
            {
                if (!mTessellationHullProgramUsage)
                {
                    mTessellationHullProgramUsage = OGRE_NEW GpuProgramUsage(GPT_HULL_PROGRAM, this);
                }
                mTessellationHullProgramUsage->setProgramName(name, resetParams);
            }
            // Needs recompilation
            mParent->_notifyNeedsRecompile();
        }
    }
    //-----------------------------------------------------------------------
    void Pass::setTessellationHullProgramParameters(GpuProgramParametersSharedPtr params)
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        if (!mTessellationHullProgramUsage)
        {
            OGRE_EXCEPT (Exception::ERR_INVALIDPARAMS,
                "This pass does not have a tessellation Hull program assigned!",
                "Pass::setTessellationHullProgramParameters");
        }
        mTessellationHullProgramUsage->setParameters(params);
    }
    //-----------------------------------------------------------------------
    void Pass::setTessellationDomainProgram(const String& name, bool resetParams)
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);

        if (getTessellationDomainProgramName() != name)
        {
            // Turn off tessellation Domain program if name blank
            if (name.empty())
            {
                OGRE_DELETE mTessellationDomainProgramUsage;
                mTessellationDomainProgramUsage = NULL;
            }
            else
            {
                if (!mTessellationDomainProgramUsage)
                {
                    mTessellationDomainProgramUsage = OGRE_NEW GpuProgramUsage(GPT_DOMAIN_PROGRAM, this);
                }
                mTessellationDomainProgramUsage->setProgramName(name, resetParams);
            }
            // Needs recompilation
            mParent->_notifyNeedsRecompile();
        }
    }
    //-----------------------------------------------------------------------
    void Pass::setTessellationDomainProgramParameters(GpuProgramParametersSharedPtr params)
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        if (!mTessellationDomainProgramUsage)
        {
            OGRE_EXCEPT (Exception::ERR_INVALIDPARAMS,
                "This pass does not have a tessellation Domain program assigned!",
                "Pass::setTessellationDomainProgramParameters");
        }
        mTessellationDomainProgramUsage->setParameters(params);
    }
    //-----------------------------------------------------------------------
    void Pass::setComputeProgram(const String& name, bool resetParams)
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);

        if (getComputeProgramName() != name)
        {
            // Turn off compute program if name blank
            if (name.empty())
            {
                OGRE_DELETE mComputeProgramUsage;
                mComputeProgramUsage = NULL;
            }
            else
            {
                if (!mComputeProgramUsage)
                {
                    mComputeProgramUsage = OGRE_NEW GpuProgramUsage(GPT_COMPUTE_PROGRAM, this);
                }
                mComputeProgramUsage->setProgramName(name, resetParams);
            }
            // Needs recompilation
            mParent->_notifyNeedsRecompile();
        }
    }
    //-----------------------------------------------------------------------
    void Pass::setComputeProgramParameters(GpuProgramParametersSharedPtr params)
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        if (!mComputeProgramUsage)
        {
            OGRE_EXCEPT (Exception::ERR_INVALIDPARAMS,
                "This pass does not have a compute program assigned!",
                "Pass::setComputeProgramParameters");
        }
        mComputeProgramUsage->setParameters(params);
    }
    //-----------------------------------------------------------------------
    const String& Pass::getVertexProgramName(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        if (!mVertexProgramUsage)
            return BLANKSTRING;
        else
            return mVertexProgramUsage->getProgramName();
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getVertexProgramParameters(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        if (!mVertexProgramUsage)
        {
            OGRE_EXCEPT (Exception::ERR_INVALIDPARAMS,
                "This pass does not have a vertex program assigned!",
                "Pass::getVertexProgramParameters");
        }
        return mVertexProgramUsage->getParameters();
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getVertexProgram(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        return mVertexProgramUsage->getProgram();
    }
    //-----------------------------------------------------------------------
    const String& Pass::getFragmentProgramName(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        if (!mFragmentProgramUsage)
            return BLANKSTRING;
        else
            return mFragmentProgramUsage->getProgramName();
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getFragmentProgramParameters(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        return mFragmentProgramUsage->getParameters();
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getFragmentProgram(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        return mFragmentProgramUsage->getProgram();
    }
    //-----------------------------------------------------------------------
    const String& Pass::getGeometryProgramName(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        if (!mGeometryProgramUsage)
            return BLANKSTRING;
        else
            return mGeometryProgramUsage->getProgramName();
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getGeometryProgramParameters(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        return mGeometryProgramUsage->getParameters();
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getGeometryProgram(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        return mGeometryProgramUsage->getProgram();
    }
    //-----------------------------------------------------------------------
    const String& Pass::getTessellationHullProgramName(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        if (!mTessellationHullProgramUsage)
            return BLANKSTRING;
        else
            return mTessellationHullProgramUsage->getProgramName();
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getTessellationHullProgramParameters(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        return mTessellationHullProgramUsage->getParameters();
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getTessellationHullProgram(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        return mTessellationHullProgramUsage->getProgram();
    }
    //-----------------------------------------------------------------------
    const String& Pass::getTessellationDomainProgramName(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        if (!mTessellationDomainProgramUsage)
            return BLANKSTRING;
        else
            return mTessellationDomainProgramUsage->getProgramName();
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getTessellationDomainProgramParameters(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        return mTessellationDomainProgramUsage->getParameters();
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getTessellationDomainProgram(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        return mTessellationDomainProgramUsage->getProgram();
    }
    //-----------------------------------------------------------------------
    const String& Pass::getComputeProgramName(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        if (!mComputeProgramUsage)
            return BLANKSTRING;
        else
            return mComputeProgramUsage->getProgramName();
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getComputeProgramParameters(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        return mComputeProgramUsage->getParameters();
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getComputeProgram(void) const
    {
            OGRE_LOCK_MUTEX(mGpuProgramChangeMutex);
        return mComputeProgramUsage->getProgram();
    }
    //-----------------------------------------------------------------------
    bool Pass::isLoaded(void) const
    {
        return mParent->isLoaded();
    }
    //-----------------------------------------------------------------------
    void Pass::setSamplerblock( const HlmsSamplerblock &samplerblock )
    {
        OGRE_LOCK_MUTEX(mTexUnitChangeMutex);

        TextureUnitStates::iterator i, iend;
        iend = mTextureUnitStates.end();
        for (i = mTextureUnitStates.begin(); i != iend; ++i)
        {
            (*i)->setSamplerblock( samplerblock );
        }
    }
    //-----------------------------------------------------------------------
    void Pass::_updateAutoParams(const AutoParamDataSource* source, uint16 mask) const
    {
        if (hasVertexProgram())
        {
            // Update vertex program auto params
            mVertexProgramUsage->getParameters()->_updateAutoParams(source, mask);
        }

        if (hasGeometryProgram())
        {
            // Update geometry program auto params
            mGeometryProgramUsage->getParameters()->_updateAutoParams(source, mask);
        }

        if (hasFragmentProgram())
        {
            // Update fragment program auto params
            mFragmentProgramUsage->getParameters()->_updateAutoParams(source, mask);
        }

        if (hasTessellationHullProgram())
        {
            // Update fragment program auto params
            mTessellationHullProgramUsage->getParameters()->_updateAutoParams(source, mask);
        }

        if (hasTessellationDomainProgram())
        {
            // Update fragment program auto params
            mTessellationDomainProgramUsage->getParameters()->_updateAutoParams(source, mask);
        }

        if (hasComputeProgram())
        {
            // Update fragment program auto params
            mComputeProgramUsage->getParameters()->_updateAutoParams(source, mask);
        }
    }
    //-----------------------------------------------------------------------
    bool Pass::isAmbientOnly(void) const
    {
        // treat as ambient if colour write is off,
        // or all non-ambient (& emissive) colours are black
        // NB a vertex program could override this, but passes using vertex
        // programs are expected to indicate they are ambient only by
        // setting the state so it matches one of the conditions above, even
        // though this state is not used in rendering.
        return (!getColourWriteEnabled() ||
            (mDiffuse == ColourValue::Black &&
             mSpecular == ColourValue::Black));
    }
    //-----------------------------------------------------------------------
    void Pass::setShadowCasterVertexProgram(const String& name)
    {
        // Turn off vertex program if name blank
        if (name.empty())
        {
            OGRE_DELETE mShadowCasterVertexProgramUsage;
            mShadowCasterVertexProgramUsage = NULL;
        }
        else
        {
            if (!mShadowCasterVertexProgramUsage)
            {
                mShadowCasterVertexProgramUsage = OGRE_NEW GpuProgramUsage(GPT_VERTEX_PROGRAM, this);
            }
            mShadowCasterVertexProgramUsage->setProgramName(name);
        }
        // Needs recompilation
        mParent->_notifyNeedsRecompile();
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
            OGRE_DELETE mShadowCasterFragmentProgramUsage;
            mShadowCasterFragmentProgramUsage = NULL;
        }
        else
        {
            if (!mShadowCasterFragmentProgramUsage)
            {
                mShadowCasterFragmentProgramUsage = OGRE_NEW GpuProgramUsage(GPT_FRAGMENT_PROGRAM, this);
            }
            mShadowCasterFragmentProgramUsage->setProgramName(name);
        }
        // Needs recompilation
        mParent->_notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    void Pass::setShadowCasterFragmentProgramParameters(GpuProgramParametersSharedPtr params)
    {
        if (Ogre::Root::getSingletonPtr()->getRenderSystem()->getName().find("OpenGL ES 2") != String::npos)
        {
            if (!mShadowCasterFragmentProgramUsage)
            {
                OGRE_EXCEPT (Exception::ERR_INVALIDPARAMS,
                    "This pass does not have a shadow caster fragment program assigned!",
                    "Pass::setShadowCasterFragmentProgramParameters");
            }
            mShadowCasterFragmentProgramUsage->setParameters(params);
        }
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
        if (Ogre::Root::getSingletonPtr()->getRenderSystem()->getName().find("OpenGL ES 2") != String::npos)
        {
            if (!mShadowCasterFragmentProgramUsage)
            {
                OGRE_EXCEPT (Exception::ERR_INVALIDPARAMS,
                    "This pass does not have a shadow caster fragment program assigned!",
                    "Pass::getShadowCasterFragmentProgramParameters");
            }
        }
        return mShadowCasterFragmentProgramUsage->getParameters();
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getShadowCasterFragmentProgram(void) const
    {
        return mShadowCasterFragmentProgramUsage->getProgram();
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
            if ((*i)->applyTextureAliases(aliasList, apply))
                testResult = true;
        }

        return testResult;

    }
    //-----------------------------------------------------------------------
    size_t Pass::_getTextureUnitWithContentTypeIndex(
        TextureUnitState::ContentType contentType, size_t index) const
    {
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
