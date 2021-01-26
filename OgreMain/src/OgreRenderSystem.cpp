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
        , mVertexProgramBound(false)
        , mGeometryProgramBound(false)
        , mFragmentProgramBound(false)
        , mTessellationHullProgramBound(false)
        , mTessellationDomainProgramBound(false)
        , mComputeProgramBound(false)
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

        GpuLogicalBufferStructPtr nullPtr;
        GpuLogicalBufferStructPtr logicalBufferStruct(new GpuLogicalBufferStruct());
        mFixedFunctionParams.reset(new GpuProgramParameters);
        mFixedFunctionParams->_setLogicalIndexes(logicalBufferStruct, nullPtr, nullPtr);
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

    void RenderSystem::setFFPLightParams(size_t index, bool enabled)
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
        if((opt = mOptions.find("Stereo Mode")) != end)
            miscParams["stereoMode"] = opt->second.currentValue;
#endif
        return ret;
    }

    //-----------------------------------------------------------------------
    void RenderSystem::_initRenderTargets(void)
    {

        // Init stats
        for(
            RenderTargetMap::iterator it = mRenderTargets.begin();
            it != mRenderTargets.end();
            ++it )
        {
            it->second->resetStatistics();
        }

    }
    //-----------------------------------------------------------------------
    void RenderSystem::_updateAllRenderTargets(bool swapBuffers)
    {
        // Update all in order of priority
        // This ensures render-to-texture targets get updated before render windows
        RenderTargetPriorityMap::iterator itarg, itargend;
        itargend = mPrioritisedRenderTargets.end();
        for( itarg = mPrioritisedRenderTargets.begin(); itarg != itargend; ++itarg )
        {
            if( itarg->second->isActive() && itarg->second->isAutoUpdated())
                itarg->second->update(swapBuffers);
        }
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_swapAllRenderTargetBuffers()
    {
        OgreProfile("_swapAllRenderTargetBuffers");
        // Update all in order of priority
        // This ensures render-to-texture targets get updated before render windows
        RenderTargetPriorityMap::iterator itarg, itargend;
        itargend = mPrioritisedRenderTargets.end();
        for( itarg = mPrioritisedRenderTargets.begin(); itarg != itargend; ++itarg )
        {
            if( itarg->second->isActive() && itarg->second->isAutoUpdated())
                itarg->second->swapBuffers();
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
        
        mVertexProgramBound = false;
        mGeometryProgramBound = false;
        mFragmentProgramBound = false;
        mTessellationHullProgramBound = false;
        mTessellationDomainProgramBound = false;
        mComputeProgramBound = false;
    }

    //---------------------------------------------------------------------------------------------
    void RenderSystem::useCustomRenderSystemCapabilities(RenderSystemCapabilities* capabilities)
    {
    if (mRealCapabilities != 0)
    {
      OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
          "Custom render capabilities must be set before the RenderSystem is initialised.",
          "RenderSystem::useCustomRenderSystemCapabilities");
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

    bool RenderSystem::_createRenderWindows(const RenderWindowDescriptionList& renderWindowDescriptions, 
        RenderWindowList& createdWindows)
    {
        unsigned int fullscreenWindowsCount = 0;

        // Grab some information and avoid duplicate render windows.
        for (unsigned int nWindow=0; nWindow < renderWindowDescriptions.size(); ++nWindow)
        {
            const RenderWindowDescription* curDesc = &renderWindowDescriptions[nWindow];

            // Count full screen windows.
            if (curDesc->useFullScreen)         
                fullscreenWindowsCount++;   

            bool renderWindowFound = false;

            if (mRenderTargets.find(curDesc->name) != mRenderTargets.end())
                renderWindowFound = true;
            else
            {
                for (unsigned int nSecWindow = nWindow + 1 ; nSecWindow < renderWindowDescriptions.size(); ++nSecWindow)
                {
                    if (curDesc->name == renderWindowDescriptions[nSecWindow].name)
                    {
                        renderWindowFound = true;
                        break;
                    }                   
                }
            }

            // Make sure we don't already have a render target of the 
            // same name as the one supplied
            if(renderWindowFound)
            {
                String msg;

                msg = "A render target of the same name '" + String(curDesc->name) + "' already "
                    "exists.  You cannot create a new window with this name.";
                OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, msg, "RenderSystem::createRenderWindow" );
            }
        }
        
        // Case we have to create some full screen rendering windows.
        if (fullscreenWindowsCount > 0)
        {
            // Can not mix full screen and windowed rendering windows.
            if (fullscreenWindowsCount != renderWindowDescriptions.size())
            {
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                    "Can not create mix of full screen and windowed rendering windows",
                    "RenderSystem::createRenderWindows");
            }                   
        }

        // Simply call _createRenderWindow in a loop.
        for (const auto& curRenderWindowDescription : renderWindowDescriptions)
        {
            RenderWindow* curWindow = NULL;
            curWindow = _createRenderWindow(curRenderWindowDescription.name,
                                            curRenderWindowDescription.width,
                                            curRenderWindowDescription.height,
                                            curRenderWindowDescription.useFullScreen,
                                            &curRenderWindowDescription.miscParams);

            createdWindows.push_back(curWindow);
        }

        return true;
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
        // This method is only ever called to set a texture unit to valid details
        // The method _disableTextureUnit is called to turn a unit off
        TexturePtr tex = tl._getTexturePtr();
        if(!tex || tl.isTextureLoadFailing())
            tex = mTextureManager->_getWarningTexture();

        // Vertex texture binding (D3D9 only)
        if (mCurrentCapabilities->hasCapability(RSC_VERTEX_TEXTURE_FETCH) &&
            !mCurrentCapabilities->getVertexTextureUnitsShared())
        {
            if (tl.getBindingType() == TextureUnitState::BT_VERTEX)
            {
                // Bind vertex texture
                _setVertexTexture(texUnit, tex);
                // bind nothing to fragment unit (hardware isn't shared but fragment
                // unit can't be using the same index
                _setTexture(texUnit, true, sNullTexPtr);
            }
            else
            {
                // vice versa
                _setVertexTexture(texUnit, sNullTexPtr);
                _setTexture(texUnit, true, tex);
            }
        }
        else
        {
            // Shared vertex / fragment textures or no vertex texture support
            // Bind texture (may be blank)
            _setTexture(texUnit, true, tex);
        }

        // Set texture coordinate set
        _setTextureCoordSet(texUnit, tl.getTextureCoordSet());

        _setSampler(texUnit, *tl.getSampler());

        // Set blend modes
        // Note, colour before alpha is important
        _setTextureBlendMode(texUnit, tl.getColourBlendMode());
        _setTextureBlendMode(texUnit, tl.getAlphaBlendMode());

        // Set texture effects
        TextureUnitState::EffectMap::iterator effi;
        // Iterate over new effects
        bool anyCalcs = false;
        for (effi = tl.mEffects.begin(); effi != tl.mEffects.end(); ++effi)
        {
            switch (effi->second.type)
            {
            case TextureUnitState::ET_ENVIRONMENT_MAP:
                if (effi->second.subtype == TextureUnitState::ENV_CURVED)
                {
                    _setTextureCoordCalculation(texUnit, TEXCALC_ENVIRONMENT_MAP);
                    anyCalcs = true;
                }
                else if (effi->second.subtype == TextureUnitState::ENV_PLANAR)
                {
                    _setTextureCoordCalculation(texUnit, TEXCALC_ENVIRONMENT_MAP_PLANAR);
                    anyCalcs = true;
                }
                else if (effi->second.subtype == TextureUnitState::ENV_REFLECTION)
                {
                    _setTextureCoordCalculation(texUnit, TEXCALC_ENVIRONMENT_MAP_REFLECTION);
                    anyCalcs = true;
                }
                else if (effi->second.subtype == TextureUnitState::ENV_NORMAL)
                {
                    _setTextureCoordCalculation(texUnit, TEXCALC_ENVIRONMENT_MAP_NORMAL);
                    anyCalcs = true;
                }
                break;
            case TextureUnitState::ET_UVSCROLL:
            case TextureUnitState::ET_USCROLL:
            case TextureUnitState::ET_VSCROLL:
            case TextureUnitState::ET_ROTATE:
            case TextureUnitState::ET_TRANSFORM:
                break;
            case TextureUnitState::ET_PROJECTIVE_TEXTURE:
                _setTextureCoordCalculation(texUnit, TEXCALC_PROJECTIVE_TEXTURE, 
                    effi->second.frustum);
                anyCalcs = true;
                break;
            }
        }
        // Ensure any previous texcoord calc settings are reset if there are now none
        if (!anyCalcs)
        {
            _setTextureCoordCalculation(texUnit, TEXCALC_NONE);
        }

        // Change tetxure matrix 
        _setTextureMatrix(texUnit, tl.getTextureTransform());


    }
    //-----------------------------------------------------------------------
    void RenderSystem::_setVertexTexture(size_t unit, const TexturePtr& tex)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
            "This rendersystem does not support separate vertex texture samplers, "
            "you should use the regular texture samplers which are shared between "
            "the vertex and fragment units.", 
            "RenderSystem::_setVertexTexture");
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
    //-----------------------------------------------------------------------
    void RenderSystem::_setTextureUnitFiltering(size_t unit, FilterOptions minFilter,
            FilterOptions magFilter, FilterOptions mipFilter)
    {
        OGRE_IGNORE_DEPRECATED_BEGIN
        _setTextureUnitFiltering(unit, FT_MIN, minFilter);
        _setTextureUnitFiltering(unit, FT_MAG, magFilter);
        _setTextureUnitFiltering(unit, FT_MIP, mipFilter);
        OGRE_IGNORE_DEPRECATED_END
    }
    //---------------------------------------------------------------------
    void RenderSystem::_cleanupDepthBuffers( bool bCleanManualBuffers )
    {
        DepthBufferMap::iterator itMap = mDepthBufferPool.begin();
        DepthBufferMap::iterator enMap = mDepthBufferPool.end();

        while( itMap != enMap )
        {
            DepthBufferVec::const_iterator itor = itMap->second.begin();
            DepthBufferVec::const_iterator end  = itMap->second.end();

            while( itor != end )
            {
                if( bCleanManualBuffers || !(*itor)->isManual() )
                    delete *itor;
                ++itor;
            }

            itMap->second.clear();

            ++itMap;
        }

        mDepthBufferPool.clear();
    }
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
        DepthBufferVec::const_iterator itor = mDepthBufferPool[poolId].begin();
        DepthBufferVec::const_iterator end  = mDepthBufferPool[poolId].end();

        bool bAttached = false;
        while( itor != end && !bAttached )
            bAttached = renderTarget->attachDepthBuffer( *itor++ );

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
        // Remove occlusion queries
        for (HardwareOcclusionQueryList::iterator i = mHwOcclusionQueries.begin();
            i != mHwOcclusionQueries.end(); ++i)
        {
            OGRE_DELETE *i;
        }
        mHwOcclusionQueries.clear();

        _cleanupDepthBuffers();

        // Remove all the render targets. Destroy primary target last since others may depend on it.
        // Keep mRenderTargets valid all the time, so that render targets could receive
        // appropriate notifications, for example FBO based about GL context destruction.
        RenderTarget* primary = 0;
        for (RenderTargetMap::iterator it = mRenderTargets.begin(); it != mRenderTargets.end(); /* note - no increment */)
        {
            RenderTarget* current = it->second;
            if (!primary && current->isPrimary())
            {
                ++it;
                primary = current;
            }
            else
            {
                it = mRenderTargets.erase(it);
                OGRE_DELETE current;
            }
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
    void RenderSystem::convertColourValue(const ColourValue& colour, uint32* pDest)
    {
        OGRE_IGNORE_DEPRECATED_BEGIN
        *pDest = VertexElement::convertColourValue(colour, getColourVertexElementType());
        OGRE_IGNORE_DEPRECATED_END
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
        RenderTargetMap::iterator i, iend;
        iend = mRenderTargets.end();
        for (i = mRenderTargets.begin(); i != iend; ++i)
        {
            RenderTarget* target = i->second;
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

        if (mActiveVertexGpuProgramParameters)
        {
            mActiveVertexGpuProgramParameters->incPassIterationNumber();
            bindGpuProgramParameters(GPT_VERTEX_PROGRAM, mActiveVertexGpuProgramParameters, mask);
        }
        if (mActiveGeometryGpuProgramParameters)
        {
            mActiveGeometryGpuProgramParameters->incPassIterationNumber();
            bindGpuProgramParameters(GPT_GEOMETRY_PROGRAM, mActiveGeometryGpuProgramParameters, mask);
        }
        if (mActiveFragmentGpuProgramParameters)
        {
            mActiveFragmentGpuProgramParameters->incPassIterationNumber();
            bindGpuProgramParameters(GPT_FRAGMENT_PROGRAM, mActiveFragmentGpuProgramParameters, mask);
        }
        if (mActiveTessellationHullGpuProgramParameters)
        {
            mActiveTessellationHullGpuProgramParameters->incPassIterationNumber();
            bindGpuProgramParameters(GPT_HULL_PROGRAM, mActiveTessellationHullGpuProgramParameters, mask);
        }
        if (mActiveTessellationDomainGpuProgramParameters)
        {
            mActiveTessellationDomainGpuProgramParameters->incPassIterationNumber();
            bindGpuProgramParameters(GPT_DOMAIN_PROGRAM, mActiveTessellationDomainGpuProgramParameters, mask);
        }
        if (mActiveComputeGpuProgramParameters)
        {
            mActiveComputeGpuProgramParameters->incPassIterationNumber();
            bindGpuProgramParameters(GPT_COMPUTE_PROGRAM, mActiveComputeGpuProgramParameters, mask);
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
        for(ListenerList::iterator i = mEventListeners.begin(); 
            i != mEventListeners.end(); ++i)
        {
            (*i)->eventOccurred(name, params);
        }

        if(msSharedEventListener)
            msSharedEventListener->eventOccurred(name, params);
    }
    //-----------------------------------------------------------------------
    void RenderSystem::destroyHardwareOcclusionQuery( HardwareOcclusionQuery *hq)
    {
        HardwareOcclusionQueryList::iterator i =
            std::find(mHwOcclusionQueries.begin(), mHwOcclusionQueries.end(), hq);
        if (i != mHwOcclusionQueries.end())
        {
            mHwOcclusionQueries.erase(i);
            OGRE_DELETE hq;
        }
    }
    //-----------------------------------------------------------------------
    void RenderSystem::bindGpuProgram(GpuProgram* prg)
    {
        switch(prg->getType())
        {
        case GPT_VERTEX_PROGRAM:
            // mark clip planes dirty if changed (programmable can change space)
            if (!mVertexProgramBound && !mClipPlanes.empty())
                mClipPlanesDirty = true;

            mVertexProgramBound = true;
            break;
        case GPT_GEOMETRY_PROGRAM:
            mGeometryProgramBound = true;
            break;
        case GPT_FRAGMENT_PROGRAM:
            mFragmentProgramBound = true;
            break;
        case GPT_HULL_PROGRAM:
            mTessellationHullProgramBound = true;
            break;
        case GPT_DOMAIN_PROGRAM:
            mTessellationDomainProgramBound = true;
            break;
        case GPT_COMPUTE_PROGRAM:
            mComputeProgramBound = true;
            break;
        }
    }
    //-----------------------------------------------------------------------
    void RenderSystem::unbindGpuProgram(GpuProgramType gptype)
    {
        switch(gptype)
        {
        case GPT_VERTEX_PROGRAM:
            // mark clip planes dirty if changed (programmable can change space)
            if (mVertexProgramBound && !mClipPlanes.empty())
                mClipPlanesDirty = true;
            mVertexProgramBound = false;
            break;
        case GPT_GEOMETRY_PROGRAM:
            mGeometryProgramBound = false;
            break;
        case GPT_FRAGMENT_PROGRAM:
            mFragmentProgramBound = false;
            break;
        case GPT_HULL_PROGRAM:
            mTessellationHullProgramBound = false;
            break;
        case GPT_DOMAIN_PROGRAM:
            mTessellationDomainProgramBound = false;
            break;
        case GPT_COMPUTE_PROGRAM:
            mComputeProgramBound = false;
            break;
        }
    }
    //-----------------------------------------------------------------------
    bool RenderSystem::isGpuProgramBound(GpuProgramType gptype)
    {
        switch(gptype)
        {
        case GPT_VERTEX_PROGRAM:
            return mVertexProgramBound;
        case GPT_GEOMETRY_PROGRAM:
            return mGeometryProgramBound;
        case GPT_FRAGMENT_PROGRAM:
            return mFragmentProgramBound;
        case GPT_HULL_PROGRAM:
            return mTessellationHullProgramBound;
        case GPT_DOMAIN_PROGRAM:
            return mTessellationDomainProgramBound;
        case GPT_COMPUTE_PROGRAM:
            return mComputeProgramBound;
        }
        // Make compiler happy
        return false;
    }
    //---------------------------------------------------------------------
    void RenderSystem::_setTextureProjectionRelativeTo(bool enabled, const Vector3& pos)
    {
        mTexProjRelative = enabled;
        mTexProjRelativeOrigin = pos;

    }
    //---------------------------------------------------------------------
    RenderSystem::RenderSystemContext* RenderSystem::_pauseFrame(void)
    {
        _endFrame();
        return new RenderSystem::RenderSystemContext;
    }
    //---------------------------------------------------------------------
    void RenderSystem::_resumeFrame(RenderSystemContext* context)
    {
        _beginFrame();
        delete context;
    }
    //---------------------------------------------------------------------
    const String& RenderSystem::_getDefaultViewportMaterialScheme( void ) const
    {
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS   
        if ( !(getCapabilities()->hasCapability(Ogre::RSC_FIXED_FUNCTION)) )
        {
            // I am returning the exact value for now - I don't want to add dependency for the RTSS just for one string  
            static const String ShaderGeneratorDefaultScheme = "ShaderGeneratorDefaultScheme";
            return ShaderGeneratorDefaultScheme;
        }
        else
#endif
        {
            return MaterialManager::DEFAULT_SCHEME_NAME;
        }
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
        optStereoMode.name = "Stereo Mode";
        optStereoMode.possibleValues.push_back(StringConverter::toString(SMT_NONE));
        optStereoMode.possibleValues.push_back(StringConverter::toString(SMT_FRAME_SEQUENTIAL));
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

}

