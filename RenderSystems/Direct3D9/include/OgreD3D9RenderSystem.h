/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#ifndef __D3D9RENDERSYSTEM_H__
#define __D3D9RENDERSYSTEM_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreD3D9Mappings.h"

namespace Ogre 
{
#define MAX_LIGHTS 8

	class D3D9DriverList;
	class D3D9Driver;
	class D3D9Device;
	class D3D9DeviceManager;
	class D3D9ResourceManager;

	/**
	Implementation of DirectX9 as a rendering system.
	*/
	class _OgreD3D9Export D3D9RenderSystem : public RenderSystem
	{
	public:
		enum MultiheadUseType
		{
			mutAuto,
			mutYes,
			mutNo
		};
		
	private:
		/// Direct3D
		IDirect3D9*	 mD3D;		
		// Stored options
		ConfigOptionMap mOptions;
		size_t mFSAASamples;
		String mFSAAHint;

		/// instance
		HINSTANCE mhInstance;

		/// List of D3D drivers installed (video cards)
		D3D9DriverList* mDriverList;
		/// Currently active driver
		D3D9Driver* mActiveD3DDriver;
		/// NVPerfHUD allowed?
		bool mUseNVPerfHUD;
		/// Per-stage constant support? (not in main caps since D3D specific & minor)
		bool mPerStageConstantSupport;
		/// Fast singleton access.
		static D3D9RenderSystem* msD3D9RenderSystem;
		/// Tells whether to attempt to initialize the system with DirectX 9Ex driver
		/// Read more in http://msdn.microsoft.com/en-us/library/windows/desktop/ee890072(v=vs.85).aspx
		bool mAllowDirectX9Ex;
		/// Tells whether the system is initialized with DirectX 9Ex driver
		/// Read more in http://msdn.microsoft.com/en-us/library/windows/desktop/ee890072(v=vs.85).aspx
		bool mIsDirectX9Ex;

		/// structure holding texture unit settings for every stage
		struct sD3DTextureStageDesc
		{
			/// the type of the texture
			D3D9Mappings::eD3DTexType texType;
			/// which texCoordIndex to use
			size_t coordIndex;
			/// type of auto tex. calc. used
			TexCoordCalcMethod autoTexCoordType;
            /// Frustum, used if the above is projection
            const Frustum *frustum;
			/// texture 
			IDirect3DBaseTexture9 *pTex;
			/// vertex texture 
			IDirect3DBaseTexture9 *pVertexTex;
		} mTexStageDesc[OGRE_MAX_TEXTURE_LAYERS];

		// Array of up to 8 lights, indexed as per API
		// Note that a null value indicates a free slot
		Light* mLights[MAX_LIGHTS];		
		D3D9DriverList* getDirect3DDrivers();
		void refreshD3DSettings();
        void refreshFSAAOptions();
		
		void setD3D9Light( size_t index, Light* light );
		
		// state management methods, very primitive !!!
		HRESULT __SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
		HRESULT __SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);
		HRESULT __SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);

		HRESULT __SetFloatRenderState(D3DRENDERSTATETYPE state, Real value)
		{
#if OGRE_DOUBLE_PRECISION == 1
			float temp = static_cast<float>(value);
			return __SetRenderState(state, *((LPDWORD)(&temp)));
#else
			return __SetRenderState(state, *((LPDWORD)(&value)));
#endif
		}

		/// return anisotropy level
		DWORD _getCurrentAnisotropy(size_t unit);
		/// check if a FSAA is supported
		bool _checkMultiSampleQuality(D3DMULTISAMPLE_TYPE type, DWORD *outQuality, D3DFORMAT format, UINT adapterNum, D3DDEVTYPE deviceType, BOOL fullScreen);
		
		D3D9HardwareBufferManager* mHardwareBufferManager;
		D3D9GpuProgramManager* mGpuProgramManager;
        D3D9HLSLProgramFactory* mHLSLProgramFactory;
		D3D9ResourceManager* mResourceManager;
		D3D9DeviceManager* mDeviceManager;

		size_t mLastVertexSourceCount;


        /// Internal method for populating the capabilities structure
		virtual RenderSystemCapabilities* createRenderSystemCapabilities() const;
		RenderSystemCapabilities* updateRenderSystemCapabilities(D3D9RenderWindow* renderWindow);

		/** See RenderSystem definition */
		virtual void initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary);


        void convertVertexShaderCaps(RenderSystemCapabilities* rsc) const;
        void convertPixelShaderCaps(RenderSystemCapabilities* rsc) const;
		bool checkVertexTextureFormats(D3D9RenderWindow* renderWindow) const;
		void detachRenderTargetImpl(const String& name);
		
        HashMap<IDirect3DDevice9*, unsigned short> mCurrentLights;
        /// Saved last view matrix
        Matrix4 mViewMatrix;

		D3DXMATRIX mDxViewMat, mDxProjMat, mDxWorldMat;
	
		typedef vector<D3D9RenderWindow*>::type D3D9RenderWindowList;
		// List of additional windows after the first (swap chains)
		D3D9RenderWindowList mRenderWindows;
		
		/** Mapping of texture format -> DepthStencil. Used as cache by _getDepthStencilFormatFor
		*/
		typedef HashMap<unsigned int, D3DFORMAT> DepthStencilHash;
		DepthStencilHash mDepthStencilHash;

		MultiheadUseType mMultiheadUse;

	protected:
		void setClipPlanesImpl(const PlaneList& clipPlanes);		
	public:
		// constructor
		D3D9RenderSystem( HINSTANCE hInstance );
		// destructor
		~D3D9RenderSystem();

		virtual void initConfigOptions();

		// Overridden RenderSystem functions
		ConfigOptionMap& getConfigOptions();
		String validateConfigOptions();
		RenderWindow* _initialise( bool autoCreateWindow, const String& windowTitle = "OGRE Render Window"  );
		/// @copydoc RenderSystem::_createRenderWindow
		RenderWindow* _createRenderWindow(const String &name, unsigned int width, unsigned int height, 
			bool fullScreen, const NameValuePairList *miscParams = 0);
		
		/// @copydoc RenderSystem::_createRenderWindows
		bool _createRenderWindows(const RenderWindowDescriptionList& renderWindowDescriptions, 
			RenderWindowList& createdWindows);

		/// @copydoc RenderSystem::_createDepthBufferFor
		DepthBuffer* _createDepthBufferFor( RenderTarget *renderTarget );

		/**
		 * This function is meant to add Depth Buffers to the pool that aren't released when the DepthBuffer
		 * is deleted. This is specially useful to put the Depth Buffer created along with the window's
		 * back buffer into the pool. All depth buffers introduced with this method go to POOL_DEFAULT
		 */
		DepthBuffer* _addManualDepthBuffer( IDirect3DDevice9* depthSurfaceDevice, IDirect3DSurface9 *surf );

		/**
		 * This function does NOT override RenderSystem::_cleanupDepthBuffers(bool) functionality.
		 * On multi monitor setups, when a device becomes "inactive" (it has no RenderWindows; like
		 * when the window was moved from one monitor to another); the Device will be destroyed,
		 * meaning all it's depth buffers (auto & manual) should be removed from the pool,
		 * but only selectively removing those created by that D3D9Device.
		 * @param:
		 *		Creator device to compare against. Shouldn't be null
		 */
		using RenderSystem::_cleanupDepthBuffers;
		void _cleanupDepthBuffers( IDirect3DDevice9 *creator );

		/**
		 * This function does NOT override RenderSystem::_cleanupDepthBuffers(bool) functionality.
		 * Manually created surfaces may be released arbitrarely without being pulled out from the pool
		 * (specially RenderWindows) this function takes care of that.
		 * @param manualSurface
		 *		Depth buffer surface to compare against. Shouldn't be null
		 */
		void _cleanupDepthBuffers( IDirect3DSurface9 *manualSurface );

		/**
         * Set current render target to target, enabling its GL context if needed
         */
		void _setRenderTarget(RenderTarget *target);
		
		/// @copydoc RenderSystem::createMultiRenderTarget
		virtual MultiRenderTarget * createMultiRenderTarget(const String & name);

		/// @copydoc RenderSystem::detachRenderTarget
		virtual RenderTarget * detachRenderTarget(const String &name);

		String getErrorDescription( long errorNumber ) const;
		const String& getName() const;
		// Low-level overridden members
		void setConfigOption( const String &name, const String &value );
		void reinitialise();
		void shutdown();
		void setAmbientLight( float r, float g, float b );
		void setShadingType( ShadeOptions so );
		void setLightingEnabled( bool enabled );
		void destroyRenderTarget(const String& name);
		VertexElementType getColourVertexElementType() const;
		void setStencilCheckEnabled(bool enabled);
        void setStencilBufferParams(CompareFunction func = CMPF_ALWAYS_PASS, 
            uint32 refValue = 0, uint32 compareMask = 0xFFFFFFFF, uint32 writeMask = 0xFFFFFFFF,
            StencilOperation stencilFailOp = SOP_KEEP, 
            StencilOperation depthFailOp = SOP_KEEP,
            StencilOperation passOp = SOP_KEEP, 
            bool twoSidedOperation = false);
        void setNormaliseNormals(bool normalise);

		// Low-level overridden members, mainly for internal use
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
        void _setTextureCoordCalculation(size_t unit, TexCoordCalcMethod m, 
            const Frustum* frustum = 0);
		void _setTextureBlendMode( size_t unit, const LayerBlendModeEx& bm );
        void _setTextureAddressingMode(size_t stage, const TextureUnitState::UVWAddressingMode& uvw);
        void _setTextureBorderColour(size_t stage, const ColourValue& colour);
		void _setTextureMipmapBias(size_t unit, float bias);
		void _setTextureMatrix( size_t unit, const Matrix4 &xform );
		void _setSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendOperation op );
		void _setSeparateSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendFactor sourceFactorAlpha, SceneBlendFactor destFactorAlpha, SceneBlendOperation op, SceneBlendOperation alphaOp );
		void _setAlphaRejectSettings( CompareFunction func, unsigned char value, bool alphaToCoverage );
		void _setViewport( Viewport *vp );		
		void _beginFrame();
		virtual RenderSystemContext* _pauseFrame(void);
		virtual void _resumeFrame(RenderSystemContext* context);
		void _endFrame();		
		void _setCullingMode( CullingMode mode );
		void _setDepthBufferParams( bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL );
		void _setDepthBufferCheckEnabled( bool enabled = true );
		void _setColourBufferWriteEnabled(bool red, bool green, bool blue, bool alpha);
		void _setDepthBufferWriteEnabled(bool enabled = true);
		void _setDepthBufferFunction( CompareFunction func = CMPF_LESS_EQUAL );
		void _setDepthBias(float constantBias, float slopeScaleBias);
		void _setFog( FogMode mode = FOG_NONE, const ColourValue& colour = ColourValue::White, Real expDensity = 1.0, Real linearStart = 0.0, Real linearEnd = 1.0 );
		void _convertProjectionMatrix(const Matrix4& matrix,
            Matrix4& dest, bool forGpuProgram = false);
		void _makeProjectionMatrix(const Radian& fovy, Real aspect, Real nearPlane, Real farPlane, 
            Matrix4& dest, bool forGpuProgram = false);
		void _makeProjectionMatrix(Real left, Real right, Real bottom, Real top, Real nearPlane, 
            Real farPlane, Matrix4& dest, bool forGpuProgram = false);
		void _makeOrthoMatrix(const Radian& fovy, Real aspect, Real nearPlane, Real farPlane, 
            Matrix4& dest, bool forGpuProgram = false);
        void _applyObliqueDepthProjection(Matrix4& matrix, const Plane& plane, 
            bool forGpuProgram);
		void _setPolygonMode(PolygonMode level);
        void _setTextureUnitFiltering(size_t unit, FilterType ftype, FilterOptions filter);
		void _setTextureUnitCompareFunction(size_t unit, CompareFunction function);
		void _setTextureUnitCompareEnabled(size_t unit, bool compare);
		void _setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy);
		void setVertexDeclaration(VertexDeclaration* decl);
		void setVertexDeclaration(VertexDeclaration* decl, bool useGlobalInstancingVertexBufferIsAvailable);
		void setVertexBufferBinding(VertexBufferBinding* binding);
		void setVertexBufferBinding(VertexBufferBinding* binding, size_t numberOfInstances, bool useGlobalInstancingVertexBufferIsAvailable, bool indexesUsed);
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
		void bindGpuProgramParameters(GpuProgramType gptype, 
			GpuProgramParametersSharedPtr params, uint16 variabilityMask);
        void bindGpuProgramPassIterationParameters(GpuProgramType gptype);

        void setScissorTest(bool enabled, size_t left = 0, size_t top = 0, size_t right = 800, size_t bottom = 600);
        void clearFrameBuffer(unsigned int buffers, 
            const ColourValue& colour = ColourValue::Black, 
            Real depth = 1.0f, unsigned short stencil = 0);
		void setClipPlane (ushort index, Real A, Real B, Real C, Real D);
		void enableClipPlane (ushort index, bool enable);
        HardwareOcclusionQuery* createHardwareOcclusionQuery();
        Real getHorizontalTexelOffset();
        Real getVerticalTexelOffset();
        Real getMinimumDepthInputValue();
        Real getMaximumDepthInputValue();
		void registerThread();
		void unregisterThread();
		void preExtraThreadsStarted();
		void postExtraThreadsStarted();		
				
		/*
		Returns whether under the current render system buffers marked as TU_STATIC can be locked for update
		*/
		virtual bool isStaticBufferLockable() const { return !mIsDirectX9Ex; }
		
		/// Tells whether the system is initialized with DirectX 9Ex driver
		/// Read more in http://msdn.microsoft.com/en-us/library/windows/desktop/ee890072(v=vs.85).aspx
		static bool isDirectX9Ex()  { return msD3D9RenderSystem->mIsDirectX9Ex; }

		static D3D9ResourceManager* getResourceManager();
		static D3D9DeviceManager* getDeviceManager();
		static IDirect3D9* getDirect3D9();
		static UINT	getResourceCreationDeviceCount();
		static IDirect3DDevice9* getResourceCreationDevice(UINT index);
		static IDirect3DDevice9* getActiveD3D9Device();

		/** Check which depthStencil formats can be used with a certain pixel format,
			and return the best suited.
		*/
		D3DFORMAT _getDepthStencilFormatFor(D3DFORMAT fmt);

        /** Check whether or not filtering is supported for the precise texture format requested
        with the given usage options.
        */
        bool _checkTextureFilteringSupported(TextureType ttype, PixelFormat format, int usage);

		/// Take in some requested FSAA settings and output supported D3D settings
		void determineFSAASettings(IDirect3DDevice9* d3d9Device, size_t fsaa, const String& fsaaHint, D3DFORMAT d3dPixelFormat, 
			bool fullScreen, D3DMULTISAMPLE_TYPE *outMultisampleType, DWORD *outMultisampleQuality);

		/// @copydoc RenderSystem::getDisplayMonitorCount
		unsigned int getDisplayMonitorCount() const;
		/// @copydoc RenderSystem::hasAnisotropicMipMapFilter
		virtual bool hasAnisotropicMipMapFilter() const { return false; }  	

		/// @copydoc RenderSystem::beginProfileEvent
        virtual void beginProfileEvent( const String &eventName );

		/// @copydoc RenderSystem::endProfileEvent
        virtual void endProfileEvent( void );

		/// @copydoc RenderSystem::markProfileEvent
        virtual void markProfileEvent( const String &eventName );
		 	
		/// Fires a device related event
		void fireDeviceEvent( D3D9Device* device, const String & name );

		/// Returns how multihead should be activated
		MultiheadUseType getMultiheadUse() const { return mMultiheadUse; }
	protected:	
		/// Returns the sampler id for a given unit texture number
		DWORD getSamplerId(size_t unit);

		/// Notify when a device has been lost.
		void notifyOnDeviceLost(D3D9Device* device);

		/// Notify when a device has been reset.
		void notifyOnDeviceReset(D3D9Device* device);

	private:
		friend class D3D9Device;
		friend class D3D9DeviceManager;		
	};
}
#endif
