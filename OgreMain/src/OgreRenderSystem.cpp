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
#include "OgreStableHeaders.h"
// RenderSystem implementation
// Note that most of this class is abstract since
//  we cannot know how to implement the behaviour without
//  being aware of the 3D API. However there are a few
//  simple functions which can have a base implementation

#include "OgreRenderTarget.h"
#include "OgreDepthBuffer.h"
#include "OgreHardwareOcclusionQuery.h"
#include "OgreComponents.h"

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
#include "OgreRTShaderConfig.h"
#endif

namespace Ogre {

    RenderSystem::Listener* RenderSystem::msSharedEventListener = 0;

    static const TexturePtr sNullTexPtr;

    //-----------------------------------------------------------------------
    RenderSystem::RenderSystem()
        : mActiveRenderTarget(0)
        , mTextureManager(0)
        , mActiveViewport(0)
        // This means CULL clockwise vertices, i.e. front of poly is counter-clockwise
        // This makes it the same as OpenGL and other right-handed systems
        , mCullingMode(CULL_CLOCKWISE)
        , mBatchCount(0)
        , mFaceCount(0)
        , mVertexCount(0)
        , mInvertVertexWinding(false)
        , mIsReverseDepthBufferEnabled(false)
        , mDisabledTexUnitsFrom(0)
        , mCurrentPassIterationCount(0)
        , mCurrentPassIterationNum(0)
        , mDerivedDepthBias(false)
        , mDerivedDepthBiasBase(0.0f)
        , mDerivedDepthBiasMultiplier(0.0f)
        , mDerivedDepthBiasSlopeScale(0.0f)
        , mGlobalInstanceVertexBufferVertexDeclaration(NULL)
        , mGlobalNumberOfInstances(1)
        , mClipPlanesDirty(true)
        , mRealCapabilities(0)
        , mCurrentCapabilities(0)
        , mUseCustomCapabilities(false)
        , mNativeShadingLanguageVersion(0)
        , mTexProjRelative(false)
        , mTexProjRelativeOrigin(Vector3::ZERO)
    {
        mEventNames.push_back("RenderSystemCapabilitiesCreated");
    }

    void RenderSystem::initFixedFunctionParams()
    {
        if(mFixedFunctionParams)
            return;

        GpuLogicalBufferStructPtr logicalBufferStruct(new GpuLogicalBufferStruct());
        mFixedFunctionParams.reset(new GpuProgramParameters);
        mFixedFunctionParams->_setLogicalIndexes(logicalBufferStruct);
        mFixedFunctionParams->setAutoConstant(0, GpuProgramParameters::ACT_WORLD_MATRIX);
        mFixedFunctionParams->setAutoConstant(4, GpuProgramParameters::ACT_VIEW_MATRIX);
        mFixedFunctionParams->setAutoConstant(8, GpuProgramParameters::ACT_PROJECTION_MATRIX);
        mFixedFunctionParams->setAutoConstant(12, GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR);
        mFixedFunctionParams->setAutoConstant(13, GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR);
        mFixedFunctionParams->setAutoConstant(14, GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR);
        mFixedFunctionParams->setAutoConstant(15, GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR);
        mFixedFunctionParams->setAutoConstant(16, GpuProgramParameters::ACT_SURFACE_SHININESS);
        mFixedFunctionParams->setAutoConstant(17, GpuProgramParameters::ACT_POINT_PARAMS);
        mFixedFunctionParams->setConstant(18, Vector4::ZERO); // ACT_FOG_PARAMS
        mFixedFunctionParams->setConstant(19, Vector4::ZERO); // ACT_FOG_COLOUR
        mFixedFunctionParams->setAutoConstant(20, GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR);

        // allocate per light parameters. slots 21..69
        for(int i = 0; i < OGRE_MAX_SIMULTANEOUS_LIGHTS; i++)
        {
            size_t light_offset = 21 + i * 6;
            mFixedFunctionParams->setConstant(light_offset + 0, Vector4::ZERO); // position
            mFixedFunctionParams->setConstant(light_offset + 1, Vector4::ZERO); // direction
            mFixedFunctionParams->setConstant(light_offset + 2, Vector4::ZERO); // diffuse
            mFixedFunctionParams->setConstant(light_offset + 3, Vector4::ZERO); // specular
            mFixedFunctionParams->setConstant(light_offset + 4, Vector4::ZERO); // attenuation
            mFixedFunctionParams->setConstant(light_offset + 5, Vector4::ZERO); // spotlight
        }
    }

    void RenderSystem::setFFPLightParams(uint32 index, bool enabled)
    {
        if(!mFixedFunctionParams)
            return;

        size_t light_offset = 21 + 6 * index;
        if (!enabled)
        {
            mFixedFunctionParams->clearAutoConstant(light_offset + 0);
            mFixedFunctionParams->clearAutoConstant(light_offset + 1);
            mFixedFunctionParams->clearAutoConstant(light_offset + 2);
            mFixedFunctionParams->clearAutoConstant(light_offset + 3);
            mFixedFunctionParams->clearAutoConstant(light_offset + 4);
            mFixedFunctionParams->clearAutoConstant(light_offset + 5);
            return;
        }
        mFixedFunctionParams->setAutoConstant(light_offset + 0, GpuProgramParameters::ACT_LIGHT_POSITION, index);
        mFixedFunctionParams->setAutoConstant(light_offset + 1, GpuProgramParameters::ACT_LIGHT_DIRECTION, index);
        mFixedFunctionParams->setAutoConstant(light_offset + 2, GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, index);
        mFixedFunctionParams->setAutoConstant(light_offset + 3, GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR, index);
        mFixedFunctionParams->setAutoConstant(light_offset + 4, GpuProgramParameters::ACT_LIGHT_ATTENUATION, index);
        mFixedFunctionParams->setAutoConstant(light_offset + 5, GpuProgramParameters::ACT_SPOTLIGHT_PARAMS, index);
    }

    //-----------------------------------------------------------------------
    RenderSystem::~RenderSystem()
    {
        shutdown();
        OGRE_DELETE mRealCapabilities;
        mRealCapabilities = 0;
        // Current capabilities managed externally
        mCurrentCapabilities = 0;
    }

    RenderWindowDescription RenderSystem::getRenderWindowDescription() const
    {
        RenderWindowDescription ret;
        auto& miscParams = ret.miscParams;

        auto end = mOptions.end();

        auto opt = mOptions.find("Full Screen");
        if (opt == end)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Can't find 'Full Screen' option");

        ret.useFullScreen = StringConverter::parseBool(opt->second.currentValue);

        opt = mOptions.find("Video Mode");
        if (opt == end)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Can't find 'Video Mode' option");

        StringStream mode(opt->second.currentValue);
        String token;

        mode >> ret.width;
        mode >> token; // 'x' as seperator between width and height
        mode >> ret.height;

        // backend specific options. Presence determined by getConfigOptions
        mode >> token; // '@' as seperator between bpp on D3D
        if(!mode.eof())
        {
            uint32 bpp;
            mode >> bpp;
            miscParams.emplace("colourDepth", std::to_string(bpp));
        }

        if((opt = mOptions.find("FSAA")) != end)
        {
            StringStream fsaaMode(opt->second.currentValue);
            uint32_t fsaa;
            fsaaMode >> fsaa;
            miscParams.emplace("FSAA", std::to_string(fsaa));

            // D3D specific
            if(!fsaaMode.eof())
            {
                String hint;
                fsaaMode >> hint;
                miscParams.emplace("FSAAHint", hint);
            }
        }

        if((opt = mOptions.find("VSync")) != end)
            miscParams.emplace("vsync", opt->second.currentValue);

        if((opt = mOptions.find("sRGB Gamma Conversion")) != end)
            miscParams.emplace("gamma", opt->second.currentValue);

        if((opt = mOptions.find("Colour Depth")) != end)
            miscParams.emplace("colourDepth", opt->second.currentValue);

        if((opt = mOptions.find("VSync Interval")) != end)
            miscParams.emplace("vsyncInterval", opt->second.currentValue);

        if((opt = mOptions.find("Display Frequency")) != end)
            miscParams.emplace("displayFrequency", opt->second.currentValue);

        if((opt = mOptions.find("Content Scaling Factor")) != end)
            miscParams["contentScalingFactor"] = opt->second.currentValue;

        if((opt = mOptions.find("Rendering Device")) != end)
        {
            // try to parse "Monitor-NN-"
            auto start = opt->second.currentValue.find('-') + 1;
            auto len = opt->second.currentValue.find('-', start) - start;
            if(start != String::npos)
                miscParams["monitorIndex"] = opt->second.currentValue.substr(start, len);
        }

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
        if((opt = mOptions.find("Frame Sequential Stereo")) != end)
            miscParams["stereoMode"] = opt->second.currentValue;
#endif
        return ret;
    }

    //-----------------------------------------------------------------------
    void RenderSystem::_initRenderTargets(void)
    {

        // Init stats
        for (auto& rt : mRenderTargets)
        {
            rt.second->resetStatistics();
        }
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_updateAllRenderTargets(bool swapBuffers)
    {
        // Update all in order of priority
        // This ensures render-to-texture targets get updated before render windows
        for (auto& rt : mPrioritisedRenderTargets)
        {
            if (rt.second->isActive() && rt.second->isAutoUpdated())
                rt.second->update(swapBuffers);
        }
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_swapAllRenderTargetBuffers()
    {
        OgreProfile("_swapAllRenderTargetBuffers");
        // Update all in order of priority
        // This ensures render-to-texture targets get updated before render windows
        for (auto& rt : mPrioritisedRenderTargets)
        {
            if (rt.second->isActive() && rt.second->isAutoUpdated())
                rt.second->swapBuffers();
        }
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_initialise()
    {
        // Have I been registered by call to Root::setRenderSystem?
        /** Don't do this anymore, just allow via Root
        RenderSystem* regPtr = Root::getSingleton().getRenderSystem();
        if (!regPtr || regPtr != this)
            // Register self - library user has come to me direct
            Root::getSingleton().setRenderSystem(this);
        */


        // Subclasses should take it from here
        // They should ALL call this superclass method from
        //   their own initialise() implementations.
        
        mProgramBound.fill(false);
    }

    //---------------------------------------------------------------------------------------------
    void RenderSystem::useCustomRenderSystemCapabilities(RenderSystemCapabilities* capabilities)
    {
        if (mRealCapabilities)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Custom render capabilities must be set before the RenderSystem is initialised");
        }

        if (capabilities->getRenderSystemName() != getName())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Trying to use RenderSystemCapabilities that were created for a different RenderSystem");
        }

        mCurrentCapabilities = capabilities;
        mUseCustomCapabilities = true;
    }

    //---------------------------------------------------------------------------------------------
    RenderWindow* RenderSystem::_createRenderWindow(const String& name, unsigned int width,
                                                    unsigned int height, bool fullScreen,
                                                    const NameValuePairList* miscParams)
    {
        if (mRenderTargets.find(name) != mRenderTargets.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Window with name '" + name + "' already exists");
        }

        // Log a message
        StringStream ss;
        ss << "RenderSystem::_createRenderWindow \"" << name << "\", " <<
            width << "x" << height << " ";
        if (fullScreen)
            ss << "fullscreen ";
        else
            ss << "windowed ";

        if (miscParams)
        {
            ss << " miscParams: ";
            NameValuePairList::const_iterator it;
            for (const auto& p : *miscParams)
            {
                ss << p.first << "=" << p.second << " ";
            }
        }
        LogManager::getSingleton().logMessage(ss.str());

        return NULL;
    }
    //---------------------------------------------------------------------------------------------
    void RenderSystem::destroyRenderWindow(const String& name)
    {
        destroyRenderTarget(name);
    }
    //---------------------------------------------------------------------------------------------
    void RenderSystem::destroyRenderTexture(const String& name)
    {
        destroyRenderTarget(name);
    }
    //---------------------------------------------------------------------------------------------
    void RenderSystem::destroyRenderTarget(const String& name)
    {
        RenderTarget* rt = detachRenderTarget(name);
        OGRE_DELETE rt;
    }
    //---------------------------------------------------------------------------------------------
    void RenderSystem::attachRenderTarget( RenderTarget &target )
    {
        assert( target.getPriority() < OGRE_NUM_RENDERTARGET_GROUPS );

        mRenderTargets.emplace(target.getName(), &target);
        mPrioritisedRenderTargets.emplace(target.getPriority(), &target);
    }

    //---------------------------------------------------------------------------------------------
    RenderTarget * RenderSystem::getRenderTarget( const String &name )
    {
        RenderTargetMap::iterator it = mRenderTargets.find( name );
        RenderTarget *ret = NULL;

        if( it != mRenderTargets.end() )
        {
            ret = it->second;
        }

        return ret;
    }

    //---------------------------------------------------------------------------------------------
    RenderTarget * RenderSystem::detachRenderTarget( const String &name )
    {
        RenderTargetMap::iterator it = mRenderTargets.find( name );
        RenderTarget *ret = NULL;

        if( it != mRenderTargets.end() )
        {
            ret = it->second;
            
            /* Remove the render target from the priority groups. */
            RenderTargetPriorityMap::iterator itarg, itargend;
            itargend = mPrioritisedRenderTargets.end();
            for( itarg = mPrioritisedRenderTargets.begin(); itarg != itargend; ++itarg )
            {
                if( itarg->second == ret ) {
                    mPrioritisedRenderTargets.erase( itarg );
                    break;
                }
            }

            mRenderTargets.erase( it );
        }
        /// If detached render target is the active render target, reset active render target
        if(ret == mActiveRenderTarget)
            mActiveRenderTarget = 0;

        return ret;
    }
    //-----------------------------------------------------------------------
    Viewport* RenderSystem::_getViewport(void)
    {
        return mActiveViewport;
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_setTextureUnitSettings(size_t texUnit, TextureUnitState& tl)
    {
        if(texUnit >= getCapabilities()->getNumTextureUnits())
            return;

        // This method is only ever called to set a texture unit to valid details
        // The method _disableTextureUnit is called to turn a unit off
        TexturePtr tex = tl._getTexturePtr();
        if(!tex || tl.isTextureLoadFailing())
            tex = mTextureManager->_getWarningTexture();

        if(tl.getUnorderedAccessMipLevel() > -1)
        {
            tex->createShaderAccessPoint(texUnit, TA_READ_WRITE, tl.getUnorderedAccessMipLevel());
            return;
        }

        // Bind texture (may be blank)
        _setTexture(texUnit, true, tex);

        _setSampler(texUnit, *tl.getSampler());

        if(!getCapabilities()->hasCapability(RSC_FIXED_FUNCTION))
            return;

        // Set texture coordinate set
        _setTextureCoordSet(texUnit, tl.getTextureCoordSet());

        // Set blend modes
        // Note, colour before alpha is important
        _setTextureBlendMode(texUnit, tl.getColourBlendMode());
        _setTextureBlendMode(texUnit, tl.getAlphaBlendMode());

        auto calcMode = tl._deriveTexCoordCalcMethod();
        if(calcMode == TEXCALC_PROJECTIVE_TEXTURE)
        {
            auto frustum = tl.getEffects().find(TextureUnitState::ET_PROJECTIVE_TEXTURE)->second.frustum;
            _setTextureCoordCalculation(texUnit, calcMode, frustum);
        }
        else
        {
            _setTextureCoordCalculation(texUnit, calcMode);
        }

        // Change tetxure matrix 
        _setTextureMatrix(texUnit, tl.getTextureTransform());
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_disableTextureUnit(size_t texUnit)
    {
        _setTexture(texUnit, false, sNullTexPtr);
    }
    //---------------------------------------------------------------------
    void RenderSystem::_disableTextureUnitsFrom(size_t texUnit)
    {
        size_t disableTo = OGRE_MAX_TEXTURE_LAYERS;
        if (disableTo > mDisabledTexUnitsFrom)
            disableTo = mDisabledTexUnitsFrom;
        mDisabledTexUnitsFrom = texUnit;
        for (size_t i = texUnit; i < disableTo; ++i)
        {
            _disableTextureUnit(i);
        }
    }
    //---------------------------------------------------------------------
    void RenderSystem::_cleanupDepthBuffers( bool bCleanManualBuffers )
    {
        for (auto& m : mDepthBufferPool)
        {
            for (auto *b : m.second)
            {
                if (bCleanManualBuffers || !b->isManual())
                    delete b;
            }
            m.second.clear();
        }
        mDepthBufferPool.clear();
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_beginFrame(void)
    {
        if (!mActiveViewport)
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "Cannot begin frame - no viewport selected.");
    }
    //-----------------------------------------------------------------------
    CullingMode RenderSystem::_getCullingMode(void) const
    {
        return mCullingMode;
    }
    //-----------------------------------------------------------------------
    void RenderSystem::setDepthBufferFor( RenderTarget *renderTarget )
    {
        uint16 poolId = renderTarget->getDepthBufferPool();
        if( poolId == DepthBuffer::POOL_NO_DEPTH )
            return; //RenderTarget explicitly requested no depth buffer

        //Find a depth buffer in the pool
        bool bAttached = false;
        for (auto& d : mDepthBufferPool[poolId]) {
            bAttached = renderTarget->attachDepthBuffer(d);
            if (bAttached) break;
        }

        //Not found yet? Create a new one!
        if( !bAttached )
        {
            DepthBuffer *newDepthBuffer = _createDepthBufferFor( renderTarget );

            if( newDepthBuffer )
            {
                newDepthBuffer->_setPoolId( poolId );
                mDepthBufferPool[poolId].push_back( newDepthBuffer );

                bAttached = renderTarget->attachDepthBuffer( newDepthBuffer );

                OgreAssert( bAttached ,"A new DepthBuffer for a RenderTarget was created, but after creation"
                                     " it says it's incompatible with that RT" );
            }
            else
                LogManager::getSingleton().logWarning( "Couldn't create a suited DepthBuffer"
                                                       "for RT: " + renderTarget->getName());
        }
    }
    //-----------------------------------------------------------------------
    bool RenderSystem::isReverseDepthBufferEnabled() const
    {
        return mIsReverseDepthBufferEnabled;
    }
    //-----------------------------------------------------------------------
    void RenderSystem::reinitialise()
    {
        shutdown();
        _initialise();
    }

    void RenderSystem::shutdown(void)
    {
        for (auto& q : mHwOcclusionQueries)
        {
            OGRE_DELETE q;
        }
        mHwOcclusionQueries.clear();

        _cleanupDepthBuffers();

        // Remove all the render targets. Destroy primary target last since others may depend on it.
        // Keep mRenderTargets valid all the time, so that render targets could receive
        // appropriate notifications, for example FBO based about GL context destruction.
        RenderTarget* primary {nullptr};
        for (auto &&a : mRenderTargets) {
            if (!primary && a.second->isPrimary()) {
                primary = a.second;
                continue;
            }
            OGRE_DELETE a.second;
            mRenderTargets.erase(a.first);
        }
        OGRE_DELETE primary;
        mRenderTargets.clear();

        mPrioritisedRenderTargets.clear();
    }

    void RenderSystem::_setProjectionMatrix(Matrix4 m)
    {
        if (!mFixedFunctionParams) return;

        if (mActiveRenderTarget->requiresTextureFlipping())
        {
            // Invert transformed y
            m[1][0] = -m[1][0];
            m[1][1] = -m[1][1];
            m[1][2] = -m[1][2];
            m[1][3] = -m[1][3];
        }

        mFixedFunctionParams->setConstant(8, m);
        applyFixedFunctionParams(mFixedFunctionParams, GPV_GLOBAL);
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_beginGeometryCount(void)
    {
        mBatchCount = mFaceCount = mVertexCount = 0;

    }
    //-----------------------------------------------------------------------
    unsigned int RenderSystem::_getFaceCount(void) const
    {
        return static_cast< unsigned int >( mFaceCount );
    }
    //-----------------------------------------------------------------------
    unsigned int RenderSystem::_getBatchCount(void) const
    {
        return static_cast< unsigned int >( mBatchCount );
    }
    //-----------------------------------------------------------------------
    unsigned int RenderSystem::_getVertexCount(void) const
    {
        return static_cast< unsigned int >( mVertexCount );
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_render(const RenderOperation& op)
    {
        // Update stats
        size_t val;

        if (op.useIndexes)
            val = op.indexData->indexCount;
        else
            val = op.vertexData->vertexCount;

        size_t trueInstanceNum = std::max<size_t>(op.numberOfInstances,1);
        val *= trueInstanceNum;

        // account for a pass having multiple iterations
        if (mCurrentPassIterationCount > 1)
            val *= mCurrentPassIterationCount;
        mCurrentPassIterationNum = 0;

        switch(op.operationType)
        {
        case RenderOperation::OT_TRIANGLE_LIST:
            mFaceCount += (val / 3);
            break;
        case RenderOperation::OT_TRIANGLE_LIST_ADJ:
            mFaceCount += (val / 6);
            break;
        case RenderOperation::OT_TRIANGLE_STRIP_ADJ:
            mFaceCount += (val / 2 - 2);
            break;
        case RenderOperation::OT_TRIANGLE_STRIP:
        case RenderOperation::OT_TRIANGLE_FAN:
            mFaceCount += (val - 2);
            break;
        default:
            break;
        }

        mVertexCount += op.vertexData->vertexCount * trueInstanceNum;
        mBatchCount += mCurrentPassIterationCount;

        // sort out clip planes
        // have to do it here in case of matrix issues
        if (mClipPlanesDirty)
        {
            setClipPlanesImpl(mClipPlanes);
            mClipPlanesDirty = false;
        }
    }
    //-----------------------------------------------------------------------
    void RenderSystem::setInvertVertexWinding(bool invert)
    {
        mInvertVertexWinding = invert;
    }
    //-----------------------------------------------------------------------
    bool RenderSystem::getInvertVertexWinding(void) const
    {
        return mInvertVertexWinding;
    }
    //---------------------------------------------------------------------
    void RenderSystem::setClipPlanes(const PlaneList& clipPlanes)
    {
        if (clipPlanes != mClipPlanes)
        {
            mClipPlanes = clipPlanes;
            mClipPlanesDirty = true;
        }
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_notifyCameraRemoved(const Camera* cam)
    {
        for (auto& rt : mRenderTargets)
        {
            auto target = rt.second;
            target->_notifyCameraRemoved(cam);
        }
    }

    //---------------------------------------------------------------------
    bool RenderSystem::updatePassIterationRenderState(void)
    {
        if (mCurrentPassIterationCount <= 1)
            return false;

        // Update derived depth bias
        if (mDerivedDepthBias)
        {
            _setDepthBias(mDerivedDepthBiasBase + mDerivedDepthBiasMultiplier * mCurrentPassIterationNum,
                          mDerivedDepthBiasSlopeScale);
        }

        --mCurrentPassIterationCount;
        ++mCurrentPassIterationNum;

        const uint16 mask = GPV_PASS_ITERATION_NUMBER;

        for (int i = 0; i < GPT_COUNT; i++)
        {
            if (!mActiveParameters[i])
                continue;
            mActiveParameters[i]->incPassIterationNumber();
            bindGpuProgramParameters(GpuProgramType(i), mActiveParameters[i], mask);
        }

        return true;
    }

    //-----------------------------------------------------------------------
    void RenderSystem::setSharedListener(Listener* listener)
    {
        assert(msSharedEventListener == NULL || listener == NULL); // you can set or reset, but for safety not directly override
        msSharedEventListener = listener;
    }
    //-----------------------------------------------------------------------
    RenderSystem::Listener* RenderSystem::getSharedListener(void)
    {
        return msSharedEventListener;
    }
    //-----------------------------------------------------------------------
    void RenderSystem::addListener(Listener* l)
    {
        mEventListeners.push_back(l);
    }
    //-----------------------------------------------------------------------
    void RenderSystem::removeListener(Listener* l)
    {
        mEventListeners.remove(l);
    }
    //-----------------------------------------------------------------------
    void RenderSystem::fireEvent(const String& name, const NameValuePairList* params)
    {
        for(auto& el : mEventListeners)
        {
            el->eventOccurred(name, params);
        }

        if(msSharedEventListener)
            msSharedEventListener->eventOccurred(name, params);
    }
    //-----------------------------------------------------------------------
    void RenderSystem::destroyHardwareOcclusionQuery( HardwareOcclusionQuery *hq)
    {
        auto end = mHwOcclusionQueries.end();
        auto i = std::find(mHwOcclusionQueries.begin(), end, hq);
        if (i != end)
        {
            mHwOcclusionQueries.erase(i);
            OGRE_DELETE hq;
        }
    }
    //-----------------------------------------------------------------------
    void RenderSystem::bindGpuProgram(GpuProgram* prg)
    {
        auto gptype = prg->getType();
        // mark clip planes dirty if changed (programmable can change space)
        if(gptype == GPT_VERTEX_PROGRAM && !mClipPlanes.empty() && !mProgramBound[gptype])
            mClipPlanesDirty = true;

        mProgramBound[gptype] = true;
    }
    //-----------------------------------------------------------------------
    void RenderSystem::unbindGpuProgram(GpuProgramType gptype)
    {
        // mark clip planes dirty if changed (programmable can change space)
        if(gptype == GPT_VERTEX_PROGRAM && !mClipPlanes.empty() && mProgramBound[gptype])
            mClipPlanesDirty = true;

        mProgramBound[gptype] = false;
    }
    //-----------------------------------------------------------------------
    bool RenderSystem::isGpuProgramBound(GpuProgramType gptype)
    {
        return mProgramBound[gptype];
    }
    //---------------------------------------------------------------------
    void RenderSystem::_setTextureProjectionRelativeTo(bool enabled, const Vector3& pos)
    {
        mTexProjRelative = enabled;
        mTexProjRelativeOrigin = pos;

    }
    //---------------------------------------------------------------------
    const String& RenderSystem::_getDefaultViewportMaterialScheme( void ) const
    {
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
        if (!getCapabilities()->hasCapability(RSC_FIXED_FUNCTION))
        {
            return MSN_SHADERGEN;
        }
#endif
        return MSN_DEFAULT;
    }
    //---------------------------------------------------------------------
    Ogre::HardwareVertexBufferSharedPtr RenderSystem::getGlobalInstanceVertexBuffer() const
    {
        return mGlobalInstanceVertexBuffer;
    }
    //---------------------------------------------------------------------
    void RenderSystem::setGlobalInstanceVertexBuffer( const HardwareVertexBufferSharedPtr &val )
    {
        if ( val && !val->isInstanceData() )
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                        "A none instance data vertex buffer was set to be the global instance vertex buffer.",
                        "RenderSystem::setGlobalInstanceVertexBuffer");
        }
        mGlobalInstanceVertexBuffer = val;
    }
    //---------------------------------------------------------------------
    size_t RenderSystem::getGlobalNumberOfInstances() const
    {
        return mGlobalNumberOfInstances;
    }
    //---------------------------------------------------------------------
    void RenderSystem::setGlobalNumberOfInstances( const size_t val )
    {
        mGlobalNumberOfInstances = val;
    }

    VertexDeclaration* RenderSystem::getGlobalInstanceVertexBufferVertexDeclaration() const
    {
        return mGlobalInstanceVertexBufferVertexDeclaration;
    }
    //---------------------------------------------------------------------
    void RenderSystem::setGlobalInstanceVertexBufferVertexDeclaration( VertexDeclaration* val )
    {
        mGlobalInstanceVertexBufferVertexDeclaration = val;
    }
    //---------------------------------------------------------------------
    void RenderSystem::getCustomAttribute(const String& name, void* pData)
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Attribute not found.", "RenderSystem::getCustomAttribute");
    }

    void RenderSystem::initConfigOptions()
    {
        // FS setting possibilities
        ConfigOption optFullScreen;
        optFullScreen.name = "Full Screen";
        optFullScreen.possibleValues.push_back( "No" );
        optFullScreen.possibleValues.push_back( "Yes" );
        optFullScreen.currentValue = optFullScreen.possibleValues[0];
        optFullScreen.immutable = false;
        mOptions[optFullScreen.name] = optFullScreen;

        // Video mode possibilities, can be overwritten by actual values
        ConfigOption optVideoMode;
        optVideoMode.name = "Video Mode";
        optVideoMode.possibleValues.push_back("1920 x 1080");
        optVideoMode.possibleValues.push_back("1280 x 720");
        optVideoMode.possibleValues.push_back("800 x 600");
        optVideoMode.currentValue = optVideoMode.possibleValues.back();
        optVideoMode.immutable = false;
        mOptions[optVideoMode.name] = optVideoMode;

        ConfigOption optVSync;
        optVSync.name = "VSync";
        optVSync.immutable = false;
        optVSync.possibleValues.push_back("No");
        optVSync.possibleValues.push_back("Yes");
        optVSync.currentValue = optVSync.possibleValues[1];
        mOptions[optVSync.name] = optVSync;

        ConfigOption optVSyncInterval;
        optVSyncInterval.name = "VSync Interval";
        optVSyncInterval.immutable = false;
        optVSyncInterval.possibleValues.push_back("1");
        optVSyncInterval.possibleValues.push_back("2");
        optVSyncInterval.possibleValues.push_back("3");
        optVSyncInterval.possibleValues.push_back("4");
        optVSyncInterval.currentValue = optVSyncInterval.possibleValues[0];
        mOptions[optVSyncInterval.name] = optVSyncInterval;

        ConfigOption optSRGB;
        optSRGB.name = "sRGB Gamma Conversion";
        optSRGB.immutable = false;
        optSRGB.possibleValues.push_back("No");
        optSRGB.possibleValues.push_back("Yes");
        optSRGB.currentValue = optSRGB.possibleValues[0];
        mOptions[optSRGB.name] = optSRGB;

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
        ConfigOption optStereoMode;
        optStereoMode.name = "Frame Sequential Stereo";
        optStereoMode.possibleValues.push_back("Off");
        optStereoMode.possibleValues.push_back("On");
        optStereoMode.currentValue = optStereoMode.possibleValues[0];
        optStereoMode.immutable = false;

        mOptions[optStereoMode.name] = optStereoMode;
#endif
    }

    CompareFunction RenderSystem::reverseCompareFunction(CompareFunction func)
    {
        switch(func)
        {
        default:
            return func;
        case CMPF_LESS:
            return CMPF_GREATER;
        case CMPF_LESS_EQUAL:
            return CMPF_GREATER_EQUAL;
        case CMPF_GREATER_EQUAL:
            return CMPF_LESS_EQUAL;
        case CMPF_GREATER:
            return CMPF_LESS;
        }
    }

    bool RenderSystem::flipFrontFace() const
    {
        return mInvertVertexWinding != mActiveRenderTarget->requiresTextureFlipping();
    }

    void RenderSystem::setStencilCheckEnabled(bool enabled)
    {
        mStencilState.enabled = enabled;
        if (!enabled)
            setStencilState(mStencilState);
    }
    void RenderSystem::setStencilBufferParams(CompareFunction func, uint32 refValue, uint32 compareMask,
                                              uint32 writeMask, StencilOperation stencilFailOp,
                                              StencilOperation depthFailOp, StencilOperation passOp,
                                              bool twoSidedOperation)
    {
        mStencilState.compareOp = func;
        mStencilState.referenceValue = refValue;
        mStencilState.compareMask = compareMask;
        mStencilState.writeMask = writeMask;
        mStencilState.stencilFailOp = stencilFailOp;
        mStencilState.depthFailOp = depthFailOp;
        mStencilState.depthStencilPassOp = passOp;
        mStencilState.twoSidedOperation = twoSidedOperation;
        if(mStencilState.enabled)
            setStencilState(mStencilState);
    }
}

