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
#include "OgreD3D11Mappings.h"

namespace Ogre 
{
	// Enable recognizing SM2.0 HLSL shaders.
	// (the same shader code could be used by many RenderSystems, directly or via Cg)
	#define SUPPORT_SM2_0_HLSL_SHADERS

	class D3D11DriverList;
	class D3D11Driver;

	/**
	Implementation of DirectX11 as a rendering system.
	*/
	class D3D11RenderSystem : public RenderSystem
	{
	private:

		// an enum to define the driver type of d3d11
		enum OGRE_D3D11_DRIVER_TYPE
		{
			DT_HARDWARE, // GPU based
			DT_SOFTWARE, // microsoft original (slow) software driver
			DT_WARP // microsoft new (faster) software driver - (Windows Advanced Rasterization Platform) - http://msdn.microsoft.com/en-us/library/dd285359.aspx
		};

		OGRE_D3D11_DRIVER_TYPE mDriverType; // d3d11 driver type
		D3D_FEATURE_LEVEL mFeatureLevel;
        D3D_FEATURE_LEVEL mMinRequestedFeatureLevel;
        D3D_FEATURE_LEVEL mMaxRequestedFeatureLevel;

		/// Direct3D rendering device
		D3D11Device 	mDevice;
		
		// Stored options
		ConfigOptionMap mOptions;

		/// List of D3D drivers installed (video cards)
		D3D11DriverList* mDriverList;
		/// Currently active driver
		D3D11Driver* mActiveD3DDriver;
		/// NVPerfHUD allowed?
		bool mUseNVPerfHUD;


		D3D11DriverList* getDirect3DDrivers(void);
		void refreshD3DSettings(void);
        void refreshFSAAOptions(void);
		void freeDevice(void);

		/// return anisotropy level
		DWORD _getCurrentAnisotropy(size_t unit);
		/// check if a FSAA is supported
		bool _checkMultiSampleQuality(UINT SampleCount, UINT *outQuality, DXGI_FORMAT format);
		
		D3D11HardwareBufferManager* mHardwareBufferManager;
		D3D11GpuProgramManager* mGpuProgramManager;
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

		CompareFunction mSceneAlphaRejectFunc; // should be merged with - mBlendDesc
		unsigned char mSceneAlphaRejectValue; // should be merged with - mBlendDesc
		bool mSceneAlphaToCoverage;

		D3D11_BLEND_DESC mBlendDesc;

		D3D11_RASTERIZER_DESC mRasterizerDesc;

		UINT mStencilRef;
		D3D11_DEPTH_STENCIL_DESC mDepthStencilDesc; 

		PolygonMode mPolygonMode;

		FilterOptions FilterMinification[OGRE_MAX_TEXTURE_LAYERS];
		FilterOptions FilterMagnification[OGRE_MAX_TEXTURE_LAYERS];
		FilterOptions FilterMips[OGRE_MAX_TEXTURE_LAYERS];
		bool		  CompareEnabled;

		D3D11_RECT mScissorRect;


		D3D11HLSLProgram* mBoundVertexProgram;
		D3D11HLSLProgram* mBoundFragmentProgram;
		D3D11HLSLProgram* mBoundGeometryProgram;
		D3D11HLSLProgram* mBoundTesselationHullProgram;
		D3D11HLSLProgram* mBoundTesselationDomainProgram;
		D3D11HLSLProgram* mBoundComputeProgram;

		ID3D11BlendState * mBoundBlendState;
		ID3D11RasterizerState * mBoundRasterizer;
		ID3D11DepthStencilState * mBoundDepthStencilState;
		ID3D11SamplerState * mBoundSamplerStates[OGRE_MAX_TEXTURE_LAYERS];
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
			/// the type of the texture
			TextureType type;
			/// which texCoordIndex to use
			size_t coordIndex;
			/// type of auto tex. calc. used
			TexCoordCalcMethod autoTexCoordType;
			/// Frustum, used if the above is projection
			const Frustum *frustum; 

			LayerBlendModeEx layerBlendMode;

			/// texture 
			ID3D11ShaderResourceView  *pTex;
			D3D11_SAMPLER_DESC	samplerDesc;
			D3D11_SAMPLER_DESC currentSamplerDesc;
			//ID3D11SamplerState * pSampler;
			bool used;
		} mTexStageDesc[OGRE_MAX_TEXTURE_LAYERS];



		/// Primary window, the one used to create the device
		D3D11RenderWindowBase* mPrimaryWindow;

		typedef vector<D3D11RenderWindowBase*>::type SecondaryWindowList;
		// List of additional windows after the first (swap chains)
		SecondaryWindowList mSecondaryWindows;

		bool mRenderSystemWasInited;

		IDXGIFactoryN*	mpDXGIFactory;
	protected:
		void setClipPlanesImpl(const PlaneList& clipPlanes);

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

		void initRenderSystem();

		virtual void initConfigOptions(void);

		// Overridden RenderSystem functions
		ConfigOptionMap& getConfigOptions(void);
		String validateConfigOptions(void);
		RenderWindow* _initialise( bool autoCreateWindow, const String& windowTitle = "OGRE Render Window"  );
		/// @copydoc RenderSystem::_createRenderWindow
		RenderWindow* _createRenderWindow(const String &name, unsigned int width, unsigned int height, 
			bool fullScreen, const NameValuePairList *miscParams = 0);

		/// @copydoc RenderSystem::createRenderTexture
		RenderTexture * createRenderTexture( const String & name, unsigned int width, unsigned int height,
		 	TextureType texType = TEX_TYPE_2D, PixelFormat internalFormat = PF_X8R8G8B8, 
			const NameValuePairList *miscParams = 0 ); 

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
		void setConfigOption( const String &name, const String &value );
		void reinitialise();
		void shutdown();
		void setAmbientLight( float r, float g, float b );
		void setShadingType( ShadeOptions so );
		void setLightingEnabled( bool enabled );
		void destroyRenderTarget(const String& name);
		VertexElementType getColourVertexElementType(void) const;
		void setStencilCheckEnabled(bool enabled);
        void setStencilBufferParams(CompareFunction func = CMPF_ALWAYS_PASS, 
            uint32 refValue = 0, uint32 compareMask = 0xFFFFFFFF, uint32 writeMask = 0xFFFFFFFF,
            StencilOperation stencilFailOp = SOP_KEEP, 
            StencilOperation depthFailOp = SOP_KEEP,
            StencilOperation passOp = SOP_KEEP, 
            bool twoSidedOperation = false);
        void setNormaliseNormals(bool normalise);

		virtual String getErrorDescription(long errorNumber) const;

		// Low-level overridden members, mainly for internal use
		D3D11HLSLProgram* _getBoundVertexProgram() const;
		D3D11HLSLProgram* _getBoundFragmentProgram() const;
		D3D11HLSLProgram* _getBoundGeometryProgram() const;
		D3D11HLSLProgram* _getBoundTesselationHullProgram() const;
		D3D11HLSLProgram* _getBoundTesselationDomainProgram() const;
		D3D11HLSLProgram* _getBoundComputeProgram() const;
        void _useLights(const LightList& lights, unsigned short limit);
		void _setWorldMatrix( const Matrix4 &m );
		void _setViewMatrix( const Matrix4 &m );
		void _setProjectionMatrix( const Matrix4 &m );
		void _setSurfaceParams( const ColourValue &ambient, const ColourValue &diffuse, const ColourValue &specular, const ColourValue &emissive, Real shininess, TrackVertexColourType tracking );
		void _setPointSpritesEnabled(bool enabled);
		void _setPointParameters(Real size, bool attenuationEnabled, 
			Real constant, Real linear, Real quadratic, Real minSize, Real maxSize);
		void _setTexture(size_t unit, bool enabled, const TexturePtr &texPtr);
		void _setVertexTexture(size_t unit, const TexturePtr& tex);
		void _disableTextureUnit(size_t texUnit);
		void _setTextureCoordSet( size_t unit, size_t index );
        void _setTextureCoordCalculation(size_t unit, TexCoordCalcMethod m, const Frustum* frustum = 0);
		void _setTextureBlendMode( size_t unit, const LayerBlendModeEx& bm );
        void _setTextureAddressingMode(size_t stage, const TextureUnitState::UVWAddressingMode& uvw);
        void _setTextureBorderColour(size_t stage, const ColourValue& colour);
		void _setTextureMipmapBias(size_t unit, float bias);
		void _setTextureMatrix( size_t unit, const Matrix4 &xform );
		void _setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendOperation op = SBO_ADD);
		void _setSeparateSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendFactor sourceFactorAlpha, 
			SceneBlendFactor destFactorAlpha, SceneBlendOperation op = SBO_ADD, SceneBlendOperation alphaOp = SBO_ADD);
		void _setAlphaRejectSettings( CompareFunction func, unsigned char value, bool alphaToCoverage );
		void _setViewport( Viewport *vp );
		void _beginFrame(void);
		void _endFrame(void);
		void _setCullingMode( CullingMode mode );
		void _setDepthBufferParams( bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL );
		void _setDepthBufferCheckEnabled( bool enabled = true );
		bool _getDepthBufferCheckEnabled( void );
		void _setColourBufferWriteEnabled(bool red, bool green, bool blue, bool alpha);
		void _setDepthBufferWriteEnabled(bool enabled = true);
		void _setDepthBufferFunction( CompareFunction func = CMPF_LESS_EQUAL );
		void _setDepthBias(float constantBias, float slopeScaleBias);
		void _setFog( FogMode mode = FOG_NONE, const ColourValue& colour = ColourValue::White, Real expDensity = 1.0, Real linearStart = 0.0, Real linearEnd = 1.0 );
		void _convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest, bool forGpuProgram = false);
		void _makeProjectionMatrix(const Radian& fovy, Real aspect, Real nearPlane, Real farPlane, 
            Matrix4& dest, bool forGpuProgram = false);
		void _makeProjectionMatrix(Real left, Real right, Real bottom, Real top, Real nearPlane, 
            Real farPlane, Matrix4& dest, bool forGpuProgram = false);
		void _makeOrthoMatrix(const Radian& fovy, Real aspect, Real nearPlane, Real farPlane, 
            Matrix4& dest, bool forGpuProgram = false);
        void _applyObliqueDepthProjection(Matrix4& matrix, const Plane& plane, bool forGpuProgram);
		void _setPolygonMode(PolygonMode level);
        void _setTextureUnitFiltering(size_t unit, FilterType ftype, FilterOptions filter);
		void _setTextureUnitCompareFunction(size_t unit, CompareFunction function);
		void _setTextureUnitCompareEnabled(size_t unit, bool compare);
		void _setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy);
		void setVertexDeclaration(VertexDeclaration* decl);
		void setVertexDeclaration(VertexDeclaration* decl, VertexBufferBinding* binding);
		void setVertexBufferBinding(VertexBufferBinding* binding);
        void _render(const RenderOperation& op);
        /** See
          RenderSystem
         */
        void bindGpuProgram(GpuProgram* prg);
        /** See
          RenderSystem
         */
        void unbindGpuProgram(GpuProgramType gptype);
        /** See
          RenderSystem
         */
        void bindGpuProgramParameters(GpuProgramType gptype, GpuProgramParametersSharedPtr params, uint16 mask);
        /** See
          RenderSystem
         */
        void bindGpuProgramPassIterationParameters(GpuProgramType gptype);

        void setScissorTest(bool enabled, size_t left = 0, size_t top = 0, size_t right = 800, size_t bottom = 600);
        void clearFrameBuffer(unsigned int buffers, 
            const ColourValue& colour = ColourValue::Black, 
            Real depth = 1.0f, unsigned short stencil = 0);
		void setClipPlane (ushort index, Real A, Real B, Real C, Real D);
		void enableClipPlane (ushort index, bool enable);
        HardwareOcclusionQuery* createHardwareOcclusionQuery(void);
        Real getHorizontalTexelOffset(void);
        Real getVerticalTexelOffset(void);
        Real getMinimumDepthInputValue(void);
        Real getMaximumDepthInputValue(void);
		void registerThread();
		void unregisterThread();
		void preExtraThreadsStarted();
		void postExtraThreadsStarted();

		/**
         * Set current render target to target, enabling its GL context if needed
         */
		void _setRenderTarget(RenderTarget *target);

        /** Check whether or not filtering is supported for the precise texture format requested
        with the given usage options.
        */
        bool _checkTextureFilteringSupported(TextureType ttype, PixelFormat format, int usage);

		void determineFSAASettings(uint fsaa, const String& fsaaHint, DXGI_FORMAT format, DXGI_SAMPLE_DESC* outFSAASettings);

		/// @copydoc RenderSystem::getDisplayMonitorCount
		unsigned int getDisplayMonitorCount() const {return 1;} //todo

		/// @copydoc RenderSystem::hasAnisotropicMipMapFilter
		virtual bool hasAnisotropicMipMapFilter() const { return true; }  

		D3D11Device &_getDevice() { return mDevice; }
		
		
        D3D_FEATURE_LEVEL _getFeatureLevel() const { return mFeatureLevel; }

		/// @copydoc RenderSystem::setSubroutine
		void setSubroutine(GpuProgramType gptype, unsigned int slotIndex, const String& subroutineName);
		
		/// @copydoc RenderSystem::setSubroutineName
		void setSubroutine(GpuProgramType gptype, const String& slotName, const String& subroutineName);

        /// @copydoc RenderSystem::beginProfileEvent
        virtual void beginProfileEvent( const String &eventName );

        /// @copydoc RenderSystem::endProfileEvent
        virtual void endProfileEvent( void );

        /// @copydoc RenderSystem::markProfileEvent
        virtual void markProfileEvent( const String &eventName );
    };
}
#endif
