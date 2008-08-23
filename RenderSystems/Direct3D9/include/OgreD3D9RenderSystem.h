/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include <dxdiag.h>

namespace Ogre 
{
#define MAX_LIGHTS 8

	class D3D9DriverList;
	class D3D9Driver;

	/**
	Implementation of DirectX9 as a rendering system.
	*/
	class D3D9RenderSystem : public RenderSystem
	{
	private:
		/// Direct3D
		LPDIRECT3D9			mpD3D;
		/// Direct3D rendering device
		LPDIRECT3DDEVICE9	mpD3DDevice;
		
		// Stored options
		ConfigOptionMap mOptions;
		/// full-screen multisampling antialiasing type
		D3DMULTISAMPLE_TYPE mFSAAType;
		/// full-screen multisampling antialiasing level
		DWORD mFSAAQuality;

		/// instance
		HINSTANCE mhInstance;

		/// List of D3D drivers installed (video cards)
		D3D9DriverList* mDriverList;
		/// Currently active driver
		D3D9Driver* mActiveD3DDriver;
		/// Device caps.
		D3DCAPS9 mCaps;
		/// NVPerfHUD allowed?
		bool mUseNVPerfHUD;
		/// Per-stage constant support? (not in main caps since D3D specific & minor)
		bool mPerStageConstantSupport;

		/// structure holding texture unit settings for every stage
		struct sD3DTextureStageDesc
		{
			/// the type of the texture
			D3D9Mappings::eD3DTexType texType;
			/// wich texCoordIndex to use
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
		// Note that a null value indeicates a free slot
		Light* mLights[MAX_LIGHTS];

		D3D9DriverList* getDirect3DDrivers(void);
		void refreshD3DSettings(void);
        void refreshFSAAOptions(void);
		void freeDevice(void);

		inline bool compareDecls( D3DVERTEXELEMENT9* pDecl1, D3DVERTEXELEMENT9* pDecl2, size_t size );


		void initInputDevices(void);
		void processInputDevices(void);
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
		/// set FSAA
		void _setFSAA(D3DMULTISAMPLE_TYPE type, DWORD qualityLevel);
		
		D3D9HardwareBufferManager* mHardwareBufferManager;
		D3D9GpuProgramManager* mGpuProgramManager;
        D3D9HLSLProgramFactory* mHLSLProgramFactory;

		size_t mLastVertexSourceCount;


        /// Internal method for populating the capabilities structure
		RenderSystemCapabilities* createRenderSystemCapabilities() const;

		/** See RenderSystem definition */
		virtual void initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary);


        void convertVertexShaderCaps(RenderSystemCapabilities* rsc) const;
        void convertPixelShaderCaps(RenderSystemCapabilities* rsc) const;
		bool checkVertexTextureFormats(void) const;

		DriverVersion getDxDiagDriverVersion(int deviceIndex);

        unsigned short mCurrentLights;
        /// Saved last view matrix
        Matrix4 mViewMatrix;

		D3DXMATRIX mDxViewMat, mDxProjMat, mDxWorldMat;

		// What follows is a set of duplicated lists just to make it
		// easier to deal with lost devices
		
		/// Primary window, the one used to create the device
		D3D9RenderWindow* mPrimaryWindow;

		typedef std::vector<D3D9RenderWindow*> SecondaryWindowList;
		// List of additional windows after the first (swap chains)
		SecondaryWindowList mSecondaryWindows;

		bool mDeviceLost;
		bool mBasicStatesInitialised;

		/** Mapping of texture format -> DepthStencil. Used as cache by _getDepthStencilFormatFor
		*/
		typedef HashMap<unsigned int, D3DFORMAT> DepthStencilHash;
		DepthStencilHash mDepthStencilHash;

		/** Mapping of depthstencil format -> depthstencil buffer
			Keep one depthstencil buffer around for every format that is used, it must be large
			enough to hold the largest rendering target.
			This is used as cache by _getDepthStencilFor.
		*/
		typedef std::pair<D3DFORMAT, D3DMULTISAMPLE_TYPE> ZBufferFormat;
		struct ZBufferRef
		{
			IDirect3DSurface9 *surface;
			size_t width, height;
		};
		typedef std::map<ZBufferFormat, ZBufferRef> ZBufferHash;
		ZBufferHash mZBufferHash;
	protected:
		void setClipPlanesImpl(const PlaneList& clipPlanes);
	public:
		// constructor
		D3D9RenderSystem( HINSTANCE hInstance );
		// destructor
		~D3D9RenderSystem();

		virtual void initConfigOptions(void);

		// Overridden RenderSystem functions
		ConfigOptionMap& getConfigOptions(void);
		String validateConfigOptions(void);
		RenderWindow* _initialise( bool autoCreateWindow, const String& windowTitle = "OGRE Render Window"  );
		/// @copydoc RenderSystem::_createRenderWindow
		RenderWindow* _createRenderWindow(const String &name, unsigned int width, unsigned int height, 
			bool fullScreen, const NameValuePairList *miscParams = 0);

		/// @copydoc RenderSystem::createMultiRenderTarget
		virtual MultiRenderTarget * createMultiRenderTarget(const String & name);

		String getErrorDescription( long errorNumber ) const;
		const String& getName(void) const;
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
            uint32 refValue = 0, uint32 mask = 0xFFFFFFFF, 
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
		void _setSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor );
		void _setSeparateSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendFactor sourceFactorAlpha, SceneBlendFactor destFactorAlpha );
		void _setAlphaRejectSettings( CompareFunction func, unsigned char value, bool alphaToCoverage );
		void _setViewport( Viewport *vp );
		void _beginFrame(void);
		void _endFrame(void);
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
		void _setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy);
		void setVertexDeclaration(VertexDeclaration* decl);
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
        void bindGpuProgramParameters(GpuProgramType gptype, GpuProgramParametersSharedPtr params);
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

		/** D3D specific method to restore a lost device. */
		void restoreLostDevice(void);
		/** D3D specific method to return whether the device has been lost. */
		bool isDeviceLost(void);
		/** Notify that a device has been lost */
		void _notifyDeviceLost(void);
		/// Get the current device
		LPDIRECT3DDEVICE9 getDevice() const { return mpD3DDevice; }

		/** Check which depthStencil formats can be used with a certain pixel format,
			and return the best suited.
		*/
		D3DFORMAT _getDepthStencilFormatFor(D3DFORMAT fmt);

		/** Get a depth stencil surface that is compatible with an internal pixel format and
			multisample type.
			@returns A directx surface, or 0 if there is no compatible depthstencil possible.
		*/
		IDirect3DSurface9* _getDepthStencilFor(D3DFORMAT fmt, D3DMULTISAMPLE_TYPE multisample, size_t width, size_t height);

		/** Clear all cached depth stencil surfaces
		*/
		void _cleanupDepthStencils();

        /** Check whether or not filtering is supported for the precise texture format requested
        with the given usage options.
        */
        bool _checkTextureFilteringSupported(TextureType ttype, PixelFormat format, int usage);

	};
}
#endif
