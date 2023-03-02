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
#ifndef __D3D11RENDERSYSTEM_H__
#define __D3D11RENDERSYSTEM_H__

#include "OgreD3D11Prerequisites.h"
#include "OgreRenderSystem.h"
#include "OgreD3D11Device.h"
#include "OgreD3D11DeviceResource.h"
#include "OgreD3D11Driver.h"
#include "OgreD3D11Mappings.h"

namespace Ogre 
{
    /** \addtogroup RenderSystems RenderSystems
    *  @{
    */
    /** \defgroup Direct3D11 Direct3D11
    * Implementation of DirectX11 as a rendering system.
    *  @{
    */
	/// Enable recognizing SM2.0 HLSL shaders.
	/// (the same shader code could be used by many RenderSystems, directly or via Cg)
	#define SUPPORT_SM2_0_HLSL_SHADERS  1

    class D3D11DriverList;
    class D3D11Driver;
	class D3D11StereoDriverBridge;
	
    /**
    Implementation of DirectX11 as a rendering system.
    */
    class _OgreD3D11Export D3D11RenderSystem
        : public RenderSystem
        , protected D3D11DeviceResourceManager
    {
    private:
	    friend class D3D11Sampler;
        Ogre::String mDriverName;    // it`s hint rather than hard requirement, could be ignored if empty or device removed
        D3D_DRIVER_TYPE mDriverType; // should be XXX_HARDWARE, XXX_SOFTWARE or XXX_WARP, never XXX_UNKNOWN or XXX_NULL
        D3D_FEATURE_LEVEL mFeatureLevel;
        D3D_FEATURE_LEVEL mMinRequestedFeatureLevel;
        D3D_FEATURE_LEVEL mMaxRequestedFeatureLevel;

        /// Direct3D rendering device
        D3D11Device     mDevice;

        /// List of D3D drivers installed (video cards)
        D3D11DriverList* mDriverList;
        /// Currently active driver
        D3D11Driver mActiveD3DDriver;
        /// NVPerfHUD allowed?
        bool mUseNVPerfHUD;
		int mSwitchingFullscreenCounter;	// Are we switching from windowed to fullscreen 

        ID3D11DeviceN* createD3D11Device(D3D11Driver* d3dDriver, D3D_DRIVER_TYPE driverType,
                         D3D_FEATURE_LEVEL minFL, D3D_FEATURE_LEVEL maxFL, D3D_FEATURE_LEVEL* pFeatureLevel);

        D3D11DriverList* getDirect3DDrivers(bool refreshList = false);
        void refreshD3DSettings(void);
        void refreshFSAAOptions(void);

        void freeDevice(void);
        void createDevice();
        
        D3D11HardwareBufferManager* mHardwareBufferManager;
        D3D11HLSLProgramFactory* mHLSLProgramFactory;

        size_t mLastVertexSourceCount;

        /// Internal method for populating the capabilities structure
        RenderSystemCapabilities* createRenderSystemCapabilities() const;
        /** See RenderSystem definition */
        void initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary);

        void convertVertexShaderCaps(RenderSystemCapabilities* rsc) const;
        void convertPixelShaderCaps(RenderSystemCapabilities* rsc) const;
        void convertGeometryShaderCaps(RenderSystemCapabilities* rsc) const;
        void convertHullShaderCaps(RenderSystemCapabilities* rsc) const;
        void convertDomainShaderCaps(RenderSystemCapabilities* rsc) const;
        void convertComputeShaderCaps(RenderSystemCapabilities* rsc) const;

        bool checkVertexTextureFormats(void);
        void detachRenderTargetImpl(const String& name);

        D3D11_BLEND_DESC    mBlendDesc;
        bool                mBlendDescChanged;

        D3D11_RASTERIZER_DESC   mRasterizerDesc;
        bool                    mRasterizerDescChanged;

        UINT mStencilRef;
        D3D11_DEPTH_STENCIL_DESC    mDepthStencilDesc; 
        bool                        mDepthStencilDescChanged;

        PolygonMode mPolygonMode;

        D3D11_RECT mScissorRect;
        
        bool mReadBackAsTexture;

        D3D11HLSLProgram* mBoundVertexProgram;
        D3D11HLSLProgram* mBoundFragmentProgram;
        D3D11HLSLProgram* mBoundGeometryProgram;
        D3D11HLSLProgram* mBoundTessellationHullProgram;
        D3D11HLSLProgram* mBoundTessellationDomainProgram;
        D3D11HLSLProgram* mBoundComputeProgram;

        ComPtr<ID3D11ShaderResourceView> mDSTResView;
        ComPtr<ID3D11BlendState> mBoundBlendState;
        ComPtr<ID3D11RasterizerState> mBoundRasterizer;
        ComPtr<ID3D11DepthStencilState> mBoundDepthStencilState;
        ComPtr<ID3D11SamplerState> mBoundSamplerStates[OGRE_MAX_TEXTURE_LAYERS];
        size_t mBoundSamplerStatesCount;

        ID3D11ShaderResourceView * mBoundTextures[OGRE_MAX_TEXTURE_LAYERS];
        size_t mBoundTexturesCount;

        // List of class instances per shader stage
        ID3D11ClassInstance* mClassInstances[6][8];

        // Number of class instances per shader stage
        UINT mNumClassInstances[6];
        
        // Store created shader subroutines, to prevent creation and destruction every frame
        typedef std::map<String, ID3D11ClassInstance*> ClassInstanceMap;
        typedef std::map<String, ID3D11ClassInstance*>::iterator ClassInstanceIterator;
        ClassInstanceMap mInstanceMap;

        /// structure holding texture unit settings for every stage
        struct sD3DTextureStageDesc
        {
            /// texture 
            D3D11Texture  *pTex;
            ID3D11SamplerState    *pSampler;
            bool used;
        } mTexStageDesc[OGRE_MAX_TEXTURE_LAYERS];

        size_t     mLastTextureUnitState;
		bool       mSamplerStatesChanged;



        /// Primary window, the one used to create the device
        D3D11RenderWindowBase* mPrimaryWindow;

        typedef std::vector<D3D11RenderWindowBase*> SecondaryWindowList;
        // List of additional windows after the first (swap chains)
        SecondaryWindowList mSecondaryWindows;

        bool mRenderSystemWasInited;

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		D3D11StereoDriverBridge* mStereoDriver;
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
		Windows::Foundation::EventRegistrationToken suspendingToken, surfaceContentLostToken;
#endif

    protected:
        /**
         * With DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL flag render target views are unbound
         * from us each Present(), and we need the way to reestablish connection.
         */
        void _setRenderTargetViews();

    public:
        // constructor
        D3D11RenderSystem( );

        // destructor
        ~D3D11RenderSystem();
		
		
		int getSwitchingFullscreenCounter() const					{ return mSwitchingFullscreenCounter; }
		void addToSwitchingFullscreenCounter()					{ mSwitchingFullscreenCounter++; }
		
        void initRenderSystem();

        virtual void initConfigOptions(void);

        // Overridden RenderSystem functions
        String validateConfigOptions(void);
        void _initialise() override;
        /// @copydoc RenderSystem::_createRenderWindow
        RenderWindow* _createRenderWindow(const String &name, unsigned int width, unsigned int height, 
            bool fullScreen, const NameValuePairList *miscParams = 0);
        /// @copydoc RenderSystem::_updateAllRenderTargets
        virtual void _updateAllRenderTargets(bool swapBuffers = true);
        /// @copydoc RenderSystem::_swapAllRenderTargetBuffers
        virtual void _swapAllRenderTargetBuffers();

        void fireDeviceEvent( D3D11Device* device, const String & name, D3D11RenderWindowBase* sendingWindow = NULL);

        /// @copydoc RenderSystem::createMultiRenderTarget
        virtual MultiRenderTarget * createMultiRenderTarget(const String & name);

        virtual DepthBuffer* _createDepthBufferFor( RenderTarget *renderTarget );

        /**
         * This function is meant to add Depth Buffers to the pool that aren't released when the DepthBuffer
         * is deleted. This is specially useful to put the Depth Buffer created along with the window's
         * back buffer into the pool. All depth buffers introduced with this method go to POOL_DEFAULT
         */
        DepthBuffer* _addManualDepthBuffer( ID3D11DepthStencilView *depthSurface,
                                            uint32 width, uint32 height, uint32 fsaa, uint32 fsaaQuality );

        /// Reverts _addManualDepthBuffer actions
        void _removeManualDepthBuffer(DepthBuffer *depthBuffer);
        /// @copydoc RenderSystem::detachRenderTarget
        virtual RenderTarget * detachRenderTarget(const String &name);

        const String& getName(void) const;

        void getCustomAttribute(const String& name, void* pData);
        // Low-level overridden members
        /**
         Specific options:

        | Key |  Default | Description |
        |-----|---------------|---------|
        | Min Requested Feature Levels | 9.1 | Min D3D_FEATURE_LEVEL |
        | Max Requested Feature Levels | 11.0 | Min D3D_FEATURE_LEVEL |
        | Information Queue Exceptions Bottom Level | No information queue exceptions | Throw exception on message from validation layer |
        | Driver type | Hardware | D3D_DRIVER_TYPE |
        | Rendering Device | (default) |  |
        */
        void setConfigOption( const String &name, const String &value );
        void shutdown();
        void validateDevice(bool forceDeviceElection = false);
        void handleDeviceLost();
        void destroyRenderTarget(const String& name);
        void setStencilState(const StencilState& state) override;

        // Low-level overridden members, mainly for internal use
        D3D11HLSLProgram* _getBoundVertexProgram() const;
        D3D11HLSLProgram* _getBoundFragmentProgram() const;
        D3D11HLSLProgram* _getBoundGeometryProgram() const;
        D3D11HLSLProgram* _getBoundTessellationHullProgram() const;
        D3D11HLSLProgram* _getBoundTessellationDomainProgram() const;
        D3D11HLSLProgram* _getBoundComputeProgram() const;
        void _setTexture(size_t unit, bool enabled, const TexturePtr &texPtr);
        void _setSampler(size_t unit, Sampler& sampler);
        void _setAlphaRejectSettings( CompareFunction func, unsigned char value, bool alphaToCoverage );
        void _setViewport( Viewport *vp );
        void _endFrame(void);
        void _setCullingMode( CullingMode mode );
        void _setDepthClamp(bool enable);
        void _setDepthBufferParams( bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL );
        void setColourBlendState(const ColourBlendState& state);
        void _setDepthBias(float constantBias, float slopeScaleBias);
		void _convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest, bool forGpuProgram = false);
        void _setPolygonMode(PolygonMode level);
        void setVertexDeclaration(VertexDeclaration* decl);
        void setVertexDeclaration(VertexDeclaration* decl, VertexBufferBinding* binding);
        void setVertexBufferBinding(VertexBufferBinding* binding);
        void _dispatchCompute(const Vector3i& workgroupDim);
        void _render(const RenderOperation& op);

        void bindGpuProgram(GpuProgram* prg);

        void unbindGpuProgram(GpuProgramType gptype);

        void bindGpuProgramParameters(GpuProgramType gptype, const GpuProgramParametersPtr& params, uint16 mask);

        void setScissorTest(bool enabled, const Rect& rect = Rect());
        void clearFrameBuffer(unsigned int buffers, 
            const ColourValue& colour = ColourValue::Black, 
            float depth = 1.0f, unsigned short stencil = 0);
        HardwareOcclusionQuery* createHardwareOcclusionQuery(void);
        Real getMinimumDepthInputValue(void);
        Real getMaximumDepthInputValue(void);

        /**
         * Set current render target to target, enabling its GL context if needed
         */
        void _setRenderTarget(RenderTarget *target);

        void determineFSAASettings(uint fsaa, const String& fsaaHint, DXGI_FORMAT format, DXGI_SAMPLE_DESC* outFSAASettings);

        D3D11Device &_getDevice() { return mDevice; }
        
        
        D3D_FEATURE_LEVEL _getFeatureLevel() const { return mFeatureLevel; }

        void setSubroutine(GpuProgramType gptype, unsigned int slotIndex, const String& subroutineName);
        
        void setSubroutine(GpuProgramType gptype, const String& slotName, const String& subroutineName);

        /// @copydoc RenderSystem::beginProfileEvent
        virtual void beginProfileEvent( const String &eventName );

        /// @copydoc RenderSystem::endProfileEvent
        virtual void endProfileEvent( void );

        /// @copydoc RenderSystem::markProfileEvent
        virtual void markProfileEvent( const String &eventName );
		
		/// @copydoc RenderSystem::setDrawBuffer
		virtual bool setDrawBuffer(ColourBufferType colourBuffer);
    };
    /** @} */
    /** @} */
}
#endif
