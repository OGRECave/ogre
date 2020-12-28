/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-present Torus Knot Software Ltd

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

#ifndef _OgreVulkanRenderSystem_H_
#define _OgreVulkanRenderSystem_H_

#include "OgreVulkanPrerequisites.h"

#include "OgreRenderSystem.h"
#include "OgreVulkanGlobalBindingTable.h"
#include "OgreVulkanPixelFormatToShaderType.h"
#include "OgreVulkanProgram.h"

#include "OgreVulkanRenderPassDescriptor.h"
#include "Vao/OgreVulkanConstBufferPacked.h"

namespace Ogre
{
    namespace v1
    {
        class HardwareBufferManager;
    }

    struct VulkanHlmsPso;
    class VulkanSupport;

    /**
       Implementation of Vulkan as a rendering system.
    */
    class _OgreVulkanExport VulkanRenderSystem : public RenderSystem
    {
        bool mInitialized;
        v1::HardwareBufferManager *mHardwareBufferManager;

        VulkanPixelFormatToShaderType mPixelFormatToShaderType;

        VkBuffer mIndirectBuffer;
        unsigned char *mSwIndirectBufferPtr;

        VulkanGpuProgramManager *mShaderManager;
        VulkanProgramFactory *mVulkanProgramFactory0;
        VulkanProgramFactory *mVulkanProgramFactory1;
        VulkanProgramFactory *mVulkanProgramFactory2;
        VulkanProgramFactory *mVulkanProgramFactory3;

        VkInstance mVkInstance;
        VulkanSupport *mVulkanSupport;

        // TODO: AutoParamsBuffer probably belongs to MetalDevice (because it's per device?)
        typedef vector<ConstBufferPacked *>::type ConstBufferPackedVec;
        ConstBufferPackedVec mAutoParamsBuffer;
        size_t mAutoParamsBufferIdx;
        uint8 *mCurrentAutoParamsBufferPtr;
        size_t mCurrentAutoParamsBufferSpaceLeft;
        size_t mHistoricalAutoParamsSize[60];

        // For v1 rendering.
        v1::IndexData *mCurrentIndexBuffer;
        v1::VertexData *mCurrentVertexBuffer;
        VkPrimitiveTopology mCurrentPrimType;

        VulkanDevice *mActiveDevice;

        VulkanDevice *mDevice;

        VulkanCache *mCache;

        HlmsPso const *mPso;
        HlmsComputePso const *mComputePso;

        uint32_t mStencilRefValue;
        bool mStencilEnabled;

        bool mTableDirty;
        bool mComputeTableDirty;
        VulkanGlobalBindingTable mGlobalTable;
        VulkanGlobalBindingTable mComputeTable;
        // Vulkan requires a valid handle when updating descriptors unless nullDescriptor is present
        // So we just use a dummy. The dummy texture we get it from TextureGpuManager which needs
        // to create some anyway for different reasons
        ConstBufferPacked *mDummyBuffer;
        TexBufferPacked *mDummyTexBuffer;
        VkImageView mDummyTextureView;
        VkSampler mDummySampler;

        // clang-format off
        VulkanFrameBufferDescMap    mFrameBufferDescMap;
        VulkanFlushOnlyDescMap      mFlushOnlyDescMap;
        uint32                      mEntriesToFlush;
        bool                        mVpChanged;
        bool                        mInterruptedRenderCommandEncoder;
        // clang-format on

        bool mValidationError;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        bool mHasWin32Support;
#elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        bool mHasAndroidSupport;
#else
        bool mHasXcbSupport;
#endif
#if OGRE_DEBUG_MODE >= OGRE_DEBUG_HIGH
        bool mHasValidationLayers;
#endif

        PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
        PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback;
        VkDebugReportCallbackEXT mDebugReportCallback;

        /// Declared here to avoid constant reallocations
        FastArray<VkImageMemoryBarrier> mImageBarriers;

        void addInstanceDebugCallback( void );

        /// Creates a dummy VkRenderPass for use in PSO creation
        VkRenderPass getVkRenderPass( HlmsPassPso passPso, uint8 &outMrtCount );

        void bindDescriptorSet() const;

        void flushRootLayout( void );
        void flushRootLayoutCS( void );

    public:
        VulkanRenderSystem();
        ~VulkanRenderSystem();

        virtual void shutdown( void );

        virtual const String &getName( void ) const;
        virtual const String &getFriendlyName( void ) const;
        void refreshConfig();
        void initConfigOptions();
        virtual ConfigOptionMap &getConfigOptions( void );
        virtual void setConfigOption( const String &name, const String &value );

        virtual HardwareOcclusionQuery *createHardwareOcclusionQuery( void );

        virtual String validateConfigOptions( void );

        virtual RenderSystemCapabilities *createRenderSystemCapabilities( void ) const;

        void resetAllBindings( void );

        virtual void reinitialise( void );

        void initializeVkInstance( void );

        VkInstance getVkInstance( void ) const { return mVkInstance; }

        virtual Window *_initialise( bool autoCreateWindow,
                                     const String &windowTitle = "OGRE Render Window" );

        virtual Window *_createRenderWindow( const String &name, uint32 width, uint32 height,
                                             bool fullScreen, const NameValuePairList *miscParams = 0 );

        virtual String getErrorDescription( long errorNumber ) const;

        virtual void _useLights( const LightList &lights, unsigned short limit );
        virtual void _setWorldMatrix( const Matrix4 &m );
        virtual void _setViewMatrix( const Matrix4 &m );
        virtual void _setProjectionMatrix( const Matrix4 &m );

        virtual void _setSurfaceParams( const ColourValue &ambient, const ColourValue &diffuse,
                                        const ColourValue &specular, const ColourValue &emissive,
                                        Real shininess, TrackVertexColourType tracking = TVC_NONE );
        virtual void _setPointSpritesEnabled( bool enabled );
        virtual void _setPointParameters( Real size, bool attenuationEnabled, Real constant, Real linear,
                                          Real quadratic, Real minSize, Real maxSize );

        void flushUAVs( void );

        void _setParamBuffer( GpuProgramType shaderStage, const VkDescriptorBufferInfo &bufferInfo );
        void _setConstBuffer( size_t slot, const VkDescriptorBufferInfo &bufferInfo );
        void _setConstBufferCS( size_t slot, const VkDescriptorBufferInfo &bufferInfo );
        void _setTexBuffer( size_t slot, VkBufferView bufferView );
        void _setTexBufferCS( size_t slot, VkBufferView bufferView );
        void _setReadOnlyBuffer( size_t slot, const VkDescriptorBufferInfo &bufferInfo );

        virtual void _setCurrentDeviceFromTexture( TextureGpu *texture );
        virtual void _setTexture( size_t unit, TextureGpu *texPtr, bool bDepthReadOnly );
        virtual void _setTextures( uint32 slotStart, const DescriptorSetTexture *set,
                                   uint32 hazardousTexIdx );
        virtual void _setTextures( uint32 slotStart, const DescriptorSetTexture2 *set );
        virtual void _setSamplers( uint32 slotStart, const DescriptorSetSampler *set );
        virtual void _setTexturesCS( uint32 slotStart, const DescriptorSetTexture *set );
        virtual void _setTexturesCS( uint32 slotStart, const DescriptorSetTexture2 *set );
        virtual void _setSamplersCS( uint32 slotStart, const DescriptorSetSampler *set );
        virtual void _setUavCS( uint32 slotStart, const DescriptorSetUav *set );

        virtual void _setTextureCoordCalculation( size_t unit, TexCoordCalcMethod m,
                                                  const Frustum *frustum = 0 );
        virtual void _setTextureBlendMode( size_t unit, const LayerBlendModeEx &bm );
        virtual void _setTextureMatrix( size_t unit, const Matrix4 &xform );

        virtual void _setIndirectBuffer( IndirectBufferPacked *indirectBuffer );

        virtual VulkanFrameBufferDescMap &_getFrameBufferDescMap( void ) { return mFrameBufferDescMap; }
        virtual VulkanFlushOnlyDescMap &_getFlushOnlyDescMap( void ) { return mFlushOnlyDescMap; }
        virtual RenderPassDescriptor *createRenderPassDescriptor( void );

        virtual void _hlmsComputePipelineStateObjectCreated( HlmsComputePso *newPso );
        virtual void _hlmsComputePipelineStateObjectDestroyed( HlmsComputePso *newPso );

        virtual void setStencilBufferParams( uint32 refValue, const StencilParams &stencilParams );

        virtual void _beginFrame( void );
        virtual void _endFrame( void );
        virtual void _endFrameOnce( void );

        virtual void _setHlmsSamplerblock( uint8 texUnit, const HlmsSamplerblock *Samplerblock );
        virtual void _setPipelineStateObject( const HlmsPso *pso );
        virtual void _setComputePso( const HlmsComputePso *pso );

        virtual VertexElementType getColourVertexElementType( void ) const;

        virtual void _dispatch( const HlmsComputePso &pso );

        virtual void _setVertexArrayObject( const VertexArrayObject *vao );
        void flushDescriptorState(
            VkPipelineBindPoint pipeline_bind_point, const VulkanConstBufferPacked &constBuffer,
            const size_t bindOffset, const size_t bytesToWrite,
            const unordered_map<unsigned, VulkanConstantDefinitionBindingParam>::type &shaderBindings );

        virtual void _render( const CbDrawCallIndexed *cmd );
        virtual void _render( const CbDrawCallStrip *cmd );
        void bindDescriptorSet( VulkanVaoManager *&vaoManager );
        virtual void _renderEmulated( const CbDrawCallIndexed *cmd );
        virtual void _renderEmulated( const CbDrawCallStrip *cmd );

        virtual void _setRenderOperation( const v1::CbRenderOp *cmd );
        virtual void _render( const v1::CbDrawCallIndexed *cmd );
        virtual void _render( const v1::CbDrawCallStrip *cmd );

        virtual void _render( const v1::RenderOperation &op );

        virtual void bindGpuProgramParameters( GpuProgramType gptype,
                                               GpuProgramParametersSharedPtr params,
                                               uint16 variabilityMask );
        virtual void bindGpuProgramPassIterationParameters( GpuProgramType gptype );

        virtual void clearFrameBuffer( RenderPassDescriptor *renderPassDesc, TextureGpu *anyTarget,
                                       uint8 mipLevel );

        virtual Real getHorizontalTexelOffset( void );
        virtual Real getVerticalTexelOffset( void );
        virtual Real getMinimumDepthInputValue( void );
        virtual Real getMaximumDepthInputValue( void );

        virtual void preExtraThreadsStarted();
        virtual void postExtraThreadsStarted();
        virtual void registerThread();
        virtual void unregisterThread();
        virtual unsigned int getDisplayMonitorCount() const { return 1; }

        virtual const PixelFormatToShaderType *getPixelFormatToShaderType( void ) const;

        virtual void flushCommands( void );

        virtual void beginProfileEvent( const String &eventName );
        virtual void endProfileEvent( void );
        virtual void markProfileEvent( const String &event );

        virtual void initGPUProfiling( void );
        virtual void deinitGPUProfiling( void );
        virtual void beginGPUSampleProfile( const String &name, uint32 *hashCache );
        virtual void endGPUSampleProfile( const String &name );

        virtual bool hasAnisotropicMipMapFilter() const { return true; }

        virtual void setClipPlanesImpl( const PlaneList &clipPlanes );
        virtual void initialiseFromRenderSystemCapabilities( RenderSystemCapabilities *caps,
                                                             Window *primary );

        virtual void beginRenderPassDescriptor( RenderPassDescriptor *desc, TextureGpu *anyTarget,
                                                uint8 mipLevel, const Vector4 *viewportSizes,
                                                const Vector4 *scissors, uint32 numViewports,
                                                bool overlaysEnabled, bool warnIfRtvWasFlushed );
        void executeRenderPassDescriptorDelayedActions( bool officialCall );
        virtual void executeRenderPassDescriptorDelayedActions( void );
        inline void endRenderPassDescriptor( bool isInterruptingRender );
        virtual void endRenderPassDescriptor( void );

        TextureGpu *createDepthBufferFor( TextureGpu *colourTexture, bool preferDepthTexture,
                                          PixelFormatGpu depthBufferFormat, uint16 poolId );

        void notifySwapchainCreated( VulkanWindow *window );
        void notifySwapchainDestroyed( VulkanWindow *window );

        virtual void flushPendingAutoResourceLayouts( void );
        virtual void executeResourceTransition( const ResourceTransitionArray &rstCollection );

        virtual void _hlmsPipelineStateObjectCreated( HlmsPso *newPso );
        virtual void _hlmsPipelineStateObjectDestroyed( HlmsPso *pos );
        virtual void _hlmsMacroblockCreated( HlmsMacroblock *newBlock );
        virtual void _hlmsMacroblockDestroyed( HlmsMacroblock *block );
        virtual void _hlmsBlendblockCreated( HlmsBlendblock *newBlock );
        virtual void _hlmsBlendblockDestroyed( HlmsBlendblock *block );
        virtual void _hlmsSamplerblockCreated( HlmsSamplerblock *newBlock );
        virtual void _hlmsSamplerblockDestroyed( HlmsSamplerblock *block );
        virtual void _descriptorSetTextureCreated( DescriptorSetTexture *newSet );
        virtual void _descriptorSetTextureDestroyed( DescriptorSetTexture *set );
        virtual void _descriptorSetTexture2Created( DescriptorSetTexture2 *newSet );
        virtual void _descriptorSetTexture2Destroyed( DescriptorSetTexture2 *set );
        virtual void _descriptorSetSamplerCreated( DescriptorSetSampler *newSet );
        virtual void _descriptorSetSamplerDestroyed( DescriptorSetSampler *set );
        virtual void _descriptorSetUavCreated( DescriptorSetUav *newSet );
        virtual void _descriptorSetUavDestroyed( DescriptorSetUav *set );

        SampleDescription validateSampleDescription( const SampleDescription &sampleDesc,
                                                     PixelFormatGpu format );
        VulkanDevice *getVulkanDevice() const { return mDevice; }
        void _notifyDeviceStalled();

        void _notifyActiveEncoderEnded( bool callEndRenderPassDesc );
        void _notifyActiveComputeEnded( void );

        void debugCallback( void );

        virtual bool isSameLayout( ResourceLayout::Layout a, ResourceLayout::Layout b,
                                   const TextureGpu *texture, bool bIsDebugCheck ) const;
    };
}  // namespace Ogre

#endif
