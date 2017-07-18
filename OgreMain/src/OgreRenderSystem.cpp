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

#include "OgreRenderSystem.h"

#include "OgreRoot.h"
#include "OgreViewport.h"
#include "OgreException.h"
#include "OgreRenderTarget.h"
#include "OgreRenderWindow.h"
#include "OgreDepthBuffer.h"
#include "OgreIteratorWrappers.h"
#include "OgreLogManager.h"
#include "OgreTextureManager.h"
#include "OgreMaterialManager.h"
#include "OgreHardwareOcclusionQuery.h"
#include "OgreHlmsPso.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Vao/OgreVaoManager.h"
#include "Vao/OgreVertexArrayObject.h"

namespace Ogre {

    static const TexturePtr sNullTexPtr;

    //-----------------------------------------------------------------------
    RenderSystem::RenderSystem()
        : mActiveRenderTarget(0)
        , mTextureManager(0)
        , mVaoManager(0)
        , mActiveViewport(0)
        , mWBuffer(false)
        , mBatchCount(0)
        , mFaceCount(0)
        , mVertexCount(0)
        , mInvertVertexWinding(false)
        , mDisabledTexUnitsFrom(0)
        , mCurrentPassIterationCount(0)
        , mCurrentPassIterationNum(0)
        , mDerivedDepthBias(false)
        , mDerivedDepthBiasBase(0.0f)
        , mDerivedDepthBiasMultiplier(0.0f)
        , mDerivedDepthBiasSlopeScale(0.0f)
        , mUavStartingSlot( 1 )
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
    //-----------------------------------------------------------------------
    RenderSystem::~RenderSystem()
    {
        shutdown();
        OGRE_DELETE mRealCapabilities;
        mRealCapabilities = 0;
        // Current capabilities managed externally
        mCurrentCapabilities = 0;
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
    RenderWindow* RenderSystem::_initialise(bool autoCreateWindow, const String& windowTitle)
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

        return 0;
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

        mRenderTargets.insert( RenderTargetMap::value_type( target.getName(), &target ) );
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
    void RenderSystem::_setPipelineStateObject( const HlmsPso *pso )
    {
        assert( (!pso || pso->rsData) &&
                "The PipelineStateObject must have been created via "
                "RenderSystem::_hlmsPipelineStateObjectCreated!" );

        //Disable previous state
        mActiveVertexGpuProgramParameters.setNull();
        mActiveGeometryGpuProgramParameters.setNull();
        mActiveTessellationHullGpuProgramParameters.setNull();
        mActiveTessellationDomainGpuProgramParameters.setNull();
        mActiveFragmentGpuProgramParameters.setNull();
        mActiveComputeGpuProgramParameters.setNull();

        if( mVertexProgramBound && !mClipPlanes.empty() )
            mClipPlanesDirty = true;

        mVertexProgramBound             = false;
        mGeometryProgramBound           = false;
        mFragmentProgramBound           = false;
        mTessellationHullProgramBound   = false;
        mTessellationDomainProgramBound = false;
        mComputeProgramBound            = false;

        //Derived class must set new state
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_setTextureUnitSettings(size_t texUnit, TextureUnitState& tl)
    {
        // This method is only ever called to set a texture unit to valid details
        // The method _disableTextureUnit is called to turn a unit off

        const TexturePtr& tex = tl._getTexturePtr();
        bool isValidBinding = false;
        
        if (mCurrentCapabilities->hasCapability(RSC_COMPLETE_TEXTURE_BINDING))
            _setBindingType(tl.getBindingType());

        // Vertex texture binding?
        if (mCurrentCapabilities->hasCapability(RSC_VERTEX_TEXTURE_FETCH) &&
            !mCurrentCapabilities->getVertexTextureUnitsShared())
        {
            isValidBinding = true;
            if (tl.getBindingType() == TextureUnitState::BT_VERTEX)
            {
                // Bind vertex texture
                _setVertexTexture(texUnit, tex);
                // bind nothing to fragment unit (hardware isn't shared but fragment
                // unit can't be using the same index
                _setTexture(texUnit, true, sNullTexPtr.get());
            }
            else
            {
                // vice versa
                _setVertexTexture(texUnit, sNullTexPtr);
                _setTexture(texUnit, true, tex.get());
            }
        }

        if (mCurrentCapabilities->hasCapability(RSC_GEOMETRY_PROGRAM))
        {
            isValidBinding = true;
            if (tl.getBindingType() == TextureUnitState::BT_GEOMETRY)
            {
                // Bind vertex texture
                _setGeometryTexture(texUnit, tex);
                // bind nothing to fragment unit (hardware isn't shared but fragment
                // unit can't be using the same index
                _setTexture(texUnit, true, sNullTexPtr.get());
            }
            else
            {
                // vice versa
                _setGeometryTexture(texUnit, sNullTexPtr);
                _setTexture(texUnit, true, tex.get());
            }
        }

        if (mCurrentCapabilities->hasCapability(RSC_TESSELLATION_DOMAIN_PROGRAM))
        {
            isValidBinding = true;
            if (tl.getBindingType() == TextureUnitState::BT_TESSELLATION_DOMAIN)
            {
                // Bind vertex texture
                _setTessellationDomainTexture(texUnit, tex);
                // bind nothing to fragment unit (hardware isn't shared but fragment
                // unit can't be using the same index
                _setTexture(texUnit, true, sNullTexPtr.get());
            }
            else
            {
                // vice versa
                _setTessellationDomainTexture(texUnit, sNullTexPtr);
                _setTexture(texUnit, true, tex.get());
            }
        }

        if (mCurrentCapabilities->hasCapability(RSC_TESSELLATION_HULL_PROGRAM))
        {
            isValidBinding = true;
            if (tl.getBindingType() == TextureUnitState::BT_TESSELLATION_HULL)
            {
                // Bind vertex texture
                _setTessellationHullTexture(texUnit, tex);
                // bind nothing to fragment unit (hardware isn't shared but fragment
                // unit can't be using the same index
                _setTexture(texUnit, true, sNullTexPtr.get());
            }
            else
            {
                // vice versa
                _setTessellationHullTexture(texUnit, sNullTexPtr);
                _setTexture(texUnit, true, tex.get());
            }
        }

        if (!isValidBinding)
        {
            // Shared vertex / fragment textures or no vertex texture support
            // Bind texture (may be blank)
            _setTexture(texUnit, true, tex.get());
        }

        _setHlmsSamplerblock( texUnit, tl.getSamplerblock() );

        // Set texture coordinate set
        _setTextureCoordSet(texUnit, tl.getTextureCoordSet());

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
    void RenderSystem::_setTexture(size_t unit, bool enabled, 
        const String &texname)
    {
        TexturePtr t = TextureManager::getSingleton().getByName(texname);
        _setTexture(unit, enabled, t.get());
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_setBindingType(TextureUnitState::BindingType bindingType)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
            "This rendersystem does not support binding texture to other shaders then fragment", 
            "RenderSystem::_setBindingType");
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
    void RenderSystem::_setGeometryTexture(size_t unit, const TexturePtr& tex)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
            "This rendersystem does not support separate geometry texture samplers, "
            "you should use the regular texture samplers which are shared between "
            "the vertex and fragment units.", 
            "RenderSystem::_setGeometryTexture");
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_setTessellationHullTexture(size_t unit, const TexturePtr& tex)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
            "This rendersystem does not support separate tessellation hull texture samplers, "
            "you should use the regular texture samplers which are shared between "
            "the vertex and fragment units.", 
            "RenderSystem::_setTessellationHullTexture");
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_setTessellationDomainTexture(size_t unit, const TexturePtr& tex)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
            "This rendersystem does not support separate tessellation domain texture samplers, "
            "you should use the regular texture samplers which are shared between "
            "the vertex and fragment units.", 
            "RenderSystem::_setTessellationDomainTexture");
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_disableTextureUnit(size_t texUnit)
    {
        _setTexture(texUnit, false, sNullTexPtr.get());
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
    void RenderSystem::setUavStartingSlot( uint32 startingSlot )
    {
        mUavStartingSlot = startingSlot;
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

        cleanReleasedDepthBuffers();
    }
    //-----------------------------------------------------------------------
    void RenderSystem::cleanReleasedDepthBuffers(void)
    {
        DepthBufferVec::const_iterator itor = mReleasedDepthBuffers.begin();
        DepthBufferVec::const_iterator end  = mReleasedDepthBuffers.end();

        while( itor != end )
        {
            delete *itor;
            ++itor;
        }

        mReleasedDepthBuffers.clear();
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_beginFrameOnce(void)
    {
        mVaoManager->_beginFrame();
    }
    //-----------------------------------------------------------------------
    void RenderSystem::setDepthBufferFor( RenderTarget *renderTarget, bool exactMatch )
    {
        uint16 poolId = renderTarget->getDepthBufferPool();
        if( poolId == DepthBuffer::POOL_NO_DEPTH )
            return; //RenderTarget explicitly requested no depth buffer

        if( poolId == DepthBuffer::POOL_NON_SHAREABLE )
        {
            createUniqueDepthBufferFor( renderTarget, exactMatch );
            return;
        }

        //Find a depth buffer in the pool
        DepthBufferVec::const_iterator itor = mDepthBufferPool[poolId].begin();
        DepthBufferVec::const_iterator end  = mDepthBufferPool[poolId].end();

        bool bAttached = false;
        while( itor != end && !bAttached )
            bAttached = renderTarget->attachDepthBuffer( *itor++, exactMatch );

        //Not found yet? Create a new one!
        if( !bAttached )
        {
            DepthBuffer *newDepthBuffer = _createDepthBufferFor( renderTarget, exactMatch );

            if( newDepthBuffer )
            {
                newDepthBuffer->_setPoolId( poolId );
                mDepthBufferPool[poolId].push_back( newDepthBuffer );

                bAttached = renderTarget->attachDepthBuffer( newDepthBuffer, exactMatch );

                assert( bAttached && "A new DepthBuffer for a RenderTarget was created, but after"
                                     " creation it says it's incompatible with that RT" );
            }
            else
            {
                if( exactMatch )
                {
                    //The GPU doesn't support this depth buffer format. Try a fallback.
                    setDepthBufferFor( renderTarget, false );
                }
                else
                {
                    LogManager::getSingleton().logMessage( "WARNING: Couldn't create a suited "
                                                           "DepthBuffer for RT: " +
                                                           renderTarget->getName(), LML_CRITICAL );
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    void RenderSystem::createUniqueDepthBufferFor( RenderTarget *renderTarget, bool exactMatch )
    {
        assert( renderTarget->getForceDisableColourWrites() );
        assert( renderTarget->getDepthBufferPool() == DepthBuffer::POOL_NON_SHAREABLE );
        assert( !renderTarget->getDepthBuffer() );

        const uint16 poolId = DepthBuffer::POOL_NON_SHAREABLE;

        DepthBuffer *newDepthBuffer = _createDepthBufferFor( renderTarget, exactMatch );

        if( newDepthBuffer )
        {
            newDepthBuffer->_setPoolId( poolId );
            mDepthBufferPool[poolId].push_back( newDepthBuffer );

            bool bAttached = renderTarget->attachDepthBuffer( newDepthBuffer, exactMatch );

            assert( bAttached && "A new DepthBuffer for a RenderTarget was created, but after"
                                 " creation it says it's incompatible with that RT" );
        }
        else
        {
            if( exactMatch )
            {
                //The GPU doesn't support this depth buffer format. Try a fallback.
                createUniqueDepthBufferFor( renderTarget, false );
            }
            else
            {
                LogManager::getSingleton().logMessage( "WARNING: Couldn't create a suited "
                                                       "unique DepthBuffer for RT: " +
                                                       renderTarget->getName(), LML_CRITICAL );
            }
        }
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_destroyDepthBuffer( DepthBuffer *depthBuffer )
    {
        DepthBufferVec &depthBuffersInPool = mDepthBufferPool[depthBuffer->getPoolId()];
        DepthBufferVec::iterator itor = depthBuffersInPool.begin();
        DepthBufferVec::iterator end  = depthBuffersInPool.end();

        while( itor != end && *itor != depthBuffer )
            ++itor;

        assert( itor != depthBuffersInPool.end() );

        efficientVectorRemove( depthBuffersInPool, itor );
        mReleasedDepthBuffers.push_back( depthBuffer );
    }
    //-----------------------------------------------------------------------
    bool RenderSystem::getWBufferEnabled(void) const
    {
        return mWBuffer;
    }
    //-----------------------------------------------------------------------
    void RenderSystem::setWBufferEnabled(bool enabled)
    {
        mWBuffer = enabled;
    }
    //-----------------------------------------------------------------------
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

        OGRE_DELETE mVaoManager;
        mVaoManager = 0;

        // Remove all the render targets.
        // (destroy primary target last since others may depend on it)
        RenderTarget* primary = 0;
        for (RenderTargetMap::iterator it = mRenderTargets.begin(); it != mRenderTargets.end(); ++it)
        {
            if (!primary && it->second->isPrimary())
                primary = it->second;
            else
                OGRE_DELETE it->second;
        }
        OGRE_DELETE primary;
        mRenderTargets.clear();
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
        *pDest = v1::VertexElement::convertColourValue(colour, getColourVertexElementType());
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_setWorldMatrices(const Matrix4* m, unsigned short count)
    {
        // Do nothing with these matrices here, it never used for now,
        // derived class should take care with them if required.

        // Set hardware matrix to nothing
        _setWorldMatrix(Matrix4::IDENTITY);
    }
    //-----------------------------------------------------------------------
    void RenderSystem::setStencilBufferParams( uint32 refValue, const StencilParams &stencilParams )
    {
        mStencilParams = stencilParams;

        // NB: We should always treat CCW as front face for consistent with default
        // culling mode.
        const bool mustFlip = ((mInvertVertexWinding && !mActiveRenderTarget->requiresTextureFlipping()) ||
                               (!mInvertVertexWinding && mActiveRenderTarget->requiresTextureFlipping()));

        if( mustFlip )
        {
            mStencilParams.stencilBack = stencilParams.stencilFront;
            mStencilParams.stencilFront = stencilParams.stencilBack;
        }
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_render(const v1::RenderOperation& op)
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
        case OT_TRIANGLE_LIST:
            mFaceCount += (val / 3);
            break;
        case OT_TRIANGLE_STRIP:
        case OT_TRIANGLE_FAN:
            mFaceCount += (val - 2);
            break;
        case OT_POINT_LIST:
        case OT_LINE_LIST:
        case OT_LINE_STRIP:
        case OT_PATCH_1_CONTROL_POINT:
        case OT_PATCH_2_CONTROL_POINT:
        case OT_PATCH_3_CONTROL_POINT:
        case OT_PATCH_4_CONTROL_POINT:
        case OT_PATCH_5_CONTROL_POINT:
        case OT_PATCH_6_CONTROL_POINT:
        case OT_PATCH_7_CONTROL_POINT:
        case OT_PATCH_8_CONTROL_POINT:
        case OT_PATCH_9_CONTROL_POINT:
        case OT_PATCH_10_CONTROL_POINT:
        case OT_PATCH_11_CONTROL_POINT:
        case OT_PATCH_12_CONTROL_POINT:
        case OT_PATCH_13_CONTROL_POINT:
        case OT_PATCH_14_CONTROL_POINT:
        case OT_PATCH_15_CONTROL_POINT:
        case OT_PATCH_16_CONTROL_POINT:
        case OT_PATCH_17_CONTROL_POINT:
        case OT_PATCH_18_CONTROL_POINT:
        case OT_PATCH_19_CONTROL_POINT:
        case OT_PATCH_20_CONTROL_POINT:
        case OT_PATCH_21_CONTROL_POINT:
        case OT_PATCH_22_CONTROL_POINT:
        case OT_PATCH_23_CONTROL_POINT:
        case OT_PATCH_24_CONTROL_POINT:
        case OT_PATCH_25_CONTROL_POINT:
        case OT_PATCH_26_CONTROL_POINT:
        case OT_PATCH_27_CONTROL_POINT:
        case OT_PATCH_28_CONTROL_POINT:
        case OT_PATCH_29_CONTROL_POINT:
        case OT_PATCH_30_CONTROL_POINT:
        case OT_PATCH_31_CONTROL_POINT:
        case OT_PATCH_32_CONTROL_POINT:
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
    /*void RenderSystem::_render( const VertexArrayObject *vao )
    {
        // Update stats
        mFaceCount      += vao->mFaceCount;
        mVertexCount    += vao->mVertexBuffers[0]->getNumElements();
        ++mBatchCount;
    }*/
    //-----------------------------------------------------------------------
    void RenderSystem::_renderUsingReadBackAsTexture(unsigned int secondPass,Ogre::String variableName,unsigned int StartSlot)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
            "This rendersystem does not support reading back the inactive depth/stencil \
            buffer as a texture. Only DirectX 11 Render System supports it.",
            "RenderSystem::_renderUsingReadBackAsTexture"); 
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
    void RenderSystem::addClipPlane (const Plane &p)
    {
        mClipPlanes.push_back(p);
        mClipPlanesDirty = true;
    }
    //---------------------------------------------------------------------
    void RenderSystem::addClipPlane (Real A, Real B, Real C, Real D)
    {
        addClipPlane(Plane(A, B, C, D));
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
    //---------------------------------------------------------------------
    void RenderSystem::resetClipPlanes()
    {
        if (!mClipPlanes.empty())
        {
            mClipPlanes.clear();
            mClipPlanesDirty = true;
        }
    }
    //---------------------------------------------------------------------
    bool RenderSystem::updatePassIterationRenderState(void)
    {
        if (mCurrentPassIterationCount <= 1)
            return false;

        --mCurrentPassIterationCount;
        ++mCurrentPassIterationNum;
        if (!mActiveVertexGpuProgramParameters.isNull())
        {
            mActiveVertexGpuProgramParameters->incPassIterationNumber();
            bindGpuProgramPassIterationParameters(GPT_VERTEX_PROGRAM);
        }
        if (!mActiveGeometryGpuProgramParameters.isNull())
        {
            mActiveGeometryGpuProgramParameters->incPassIterationNumber();
            bindGpuProgramPassIterationParameters(GPT_GEOMETRY_PROGRAM);
        }
        if (!mActiveFragmentGpuProgramParameters.isNull())
        {
            mActiveFragmentGpuProgramParameters->incPassIterationNumber();
            bindGpuProgramPassIterationParameters(GPT_FRAGMENT_PROGRAM);
        }
        if (!mActiveTessellationHullGpuProgramParameters.isNull())
        {
            mActiveTessellationHullGpuProgramParameters->incPassIterationNumber();
            bindGpuProgramPassIterationParameters(GPT_HULL_PROGRAM);
        }
        if (!mActiveTessellationDomainGpuProgramParameters.isNull())
        {
            mActiveTessellationDomainGpuProgramParameters->incPassIterationNumber();
            bindGpuProgramPassIterationParameters(GPT_DOMAIN_PROGRAM);
        }
        if (!mActiveComputeGpuProgramParameters.isNull())
        {
            mActiveComputeGpuProgramParameters->incPassIterationNumber();
            bindGpuProgramPassIterationParameters(GPT_COMPUTE_PROGRAM);
        }
        return true;
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
    void RenderSystem::_update(void)
    {
        mVaoManager->_update();
        cleanReleasedDepthBuffers();
    }
    //---------------------------------------------------------------------
    void RenderSystem::updateCompositorManager( CompositorManager2 *compositorManager,
                                                SceneManagerEnumerator &sceneManagers,
                                                HlmsManager *hlmsManager )
    {
        compositorManager->_updateImplementation( sceneManagers, hlmsManager );
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
    Ogre::v1::HardwareVertexBufferSharedPtr RenderSystem::getGlobalInstanceVertexBuffer() const
    {
        return mGlobalInstanceVertexBuffer;
    }
    //---------------------------------------------------------------------
    void RenderSystem::setGlobalInstanceVertexBuffer( const v1::HardwareVertexBufferSharedPtr &val )
    {
        if ( !val.isNull() && !val->getIsInstanceData() )
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

    v1::VertexDeclaration* RenderSystem::getGlobalInstanceVertexBufferVertexDeclaration() const
    {
        return mGlobalInstanceVertexBufferVertexDeclaration;
    }
    //---------------------------------------------------------------------
    void RenderSystem::setGlobalInstanceVertexBufferVertexDeclaration( v1::VertexDeclaration* val )
    {
        mGlobalInstanceVertexBufferVertexDeclaration = val;
    }
    //---------------------------------------------------------------------
    void RenderSystem::getCustomAttribute(const String& name, void* pData)
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Attribute not found.", "RenderSystem::getCustomAttribute");
    }
}

