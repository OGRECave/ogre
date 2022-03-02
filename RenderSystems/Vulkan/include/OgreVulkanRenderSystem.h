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
#include "OgreVulkanProgram.h"

#include "OgreVulkanRenderPassDescriptor.h"

namespace Ogre
{
    class VulkanHardwareBufferManager;

    /** \addtogroup RenderSystems RenderSystems
    *  @{
    */
    /** \defgroup Vulkan Vulkan
    * Implementation of Vulkan as a rendering system.
    *  @{
    */
    class _OgreVulkanExport VulkanRenderSystem : public RenderSystem
    {
        friend class VulkanSampler;
        bool mInitialized;
        VulkanHardwareBufferManager *mHardwareBufferManager;

        VkBuffer mIndirectBuffer;
        unsigned char *mSwIndirectBufferPtr;

        VulkanProgramFactory *mSPIRVProgramFactory;

        VkInstance mVkInstance;

        std::vector<String> mDevices;

        HardwareBufferPtr mAutoParamsBuffer;
        uint32 mAutoParamsBufferPos;
        std::vector<uint32> mAutoParamsBufferUsage;

        void resizeAutoParamsBuffer(size_t size);

        VulkanDevice *mActiveDevice;

        VulkanDevice *mDevice;

        VulkanRenderPassDescriptor    *mCurrentRenderPassDescriptor;

        VulkanFrameBufferDescMap    mFrameBufferDescMap;

        bool mHasValidationLayers;

        PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
        PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback;
        VkDebugReportCallbackEXT mDebugReportCallback;

        /// Declared here to avoid constant reallocations
        FastArray<VkImageMemoryBarrier> mImageBarriers;

        void addInstanceDebugCallback( void );

        void flushRootLayout( void );

        // Pipeline State Infos
        VkGraphicsPipelineCreateInfo pipelineCi;
        VkPipelineLayoutCreateInfo pipelineLayoutCi;
        VkPipelineVertexInputStateCreateInfo vertexFormatCi;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCi;
        VkPipelineMultisampleStateCreateInfo mssCi;
        VkPipelineRasterizationStateCreateInfo rasterState;
        VkPipelineColorBlendStateCreateInfo blendStateCi;
        std::array<VkPipelineShaderStageCreateInfo, GPT_COUNT> shaderStages;

        // descriptor set layout
        std::vector<VkDescriptorSetLayoutBinding> mDescriptorSetBindings;
        std::vector<VkDescriptorPoolSize> mDescriptorPoolSizes;
        std::vector<VkWriteDescriptorSet> mDescriptorWrites;
        std::array<VkDescriptorBufferInfo, 2> mUBOInfo;
        std::array<uint32, 2> mUBODynOffsets;
        std::array<VkDescriptorImageInfo, OGRE_MAX_TEXTURE_LAYERS> mImageInfos;
        VkDescriptorSetLayout mDescriptorSetLayout;

        VkPipelineLayout mLayout;

        std::array<VkPipelineColorBlendAttachmentState, OGRE_MAX_MULTIPLE_RENDER_TARGETS> blendStates;

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCi;
        VkViewport mVkViewport;
        VkRect2D   mScissorRect;
        VkPipelineViewportStateCreateInfo viewportStateCi;

        std::unordered_map<uint32, VkDescriptorSet> mDescriptorSetCache;
        std::unordered_map<uint32, VkRenderPass> mRenderPassCache;
        std::unordered_map<uint32, VkPipeline> mPipelineCache;

        std::shared_ptr<VulkanDescriptorPool> mDescriptorPool;

        // clears the pipeline cache
        void clearPipelineCache();

        void initializeVkInstance( void );
        void enumerateDevices();
        uint32 getSelectedDeviceIdx() const;

        VkDescriptorSet getDescriptorSet();
        VkPipeline getPipeline();
    public:
        VulkanRenderSystem();
        ~VulkanRenderSystem();

        virtual void shutdown( void );

        virtual const String &getName( void ) const;
        void refreshConfig();
        void initConfigOptions();
        virtual void setConfigOption( const String &name, const String &value );

        virtual HardwareOcclusionQuery *createHardwareOcclusionQuery( void );

        virtual RenderSystemCapabilities *createRenderSystemCapabilities( void ) const;

        void resetAllBindings( void );

        VkInstance getVkInstance( void ) const { return mVkInstance; }

        virtual RenderWindow *_createRenderWindow( const String &name, uint32 width, uint32 height,
                                             bool fullScreen, const NameValuePairList *miscParams = 0 );

        void flushUAVs( void );

        void _setTexture( size_t unit, bool enabled, const TexturePtr& texPtr ) override;

        virtual VulkanFrameBufferDescMap &_getFrameBufferDescMap( void ) { return mFrameBufferDescMap; }

        virtual void _beginFrame( void );
        virtual void _endFrame( void );

        virtual void _render( const RenderOperation &op );

        void bindGpuProgram(GpuProgram* prg);
        virtual void bindGpuProgramParameters( GpuProgramType gptype,
                                               const GpuProgramParametersPtr& params,
                                               uint16 variabilityMask );

        virtual Real getHorizontalTexelOffset( void );
        virtual Real getVerticalTexelOffset( void );
        virtual Real getMinimumDepthInputValue( void );
        virtual Real getMaximumDepthInputValue( void );

        virtual void beginProfileEvent( const String &eventName );
        virtual void endProfileEvent( void );
        virtual void markProfileEvent( const String &event );

        virtual void initGPUProfiling( void );
        virtual void deinitGPUProfiling( void );
        virtual void beginGPUSampleProfile( const String &name, uint32 *hashCache );
        virtual void endGPUSampleProfile( const String &name );

        virtual void initialiseFromRenderSystemCapabilities( RenderSystemCapabilities *caps,
                                                             RenderTarget *primary );

        void executeRenderPassDescriptorDelayedActions( bool officialCall = true );
        void endRenderPassDescriptor();

        DepthBuffer *_createDepthBufferFor( RenderTarget* renderTarget);

        void notifySwapchainDestroyed();

        VulkanDevice *getVulkanDevice() const { return mDevice; }
        void _notifyDeviceStalled();

        void _notifyActiveEncoderEnded();
        void _notifyActiveComputeEnded( void );

        void _setViewport(Viewport *vp);
        void _setRenderTarget(RenderTarget *target);
        void clearFrameBuffer(unsigned int buffers, const ColourValue& colour = ColourValue::Black,
                              Real depth = 1.0f, unsigned short stencil = 0);
        void setScissorTest(bool enabled, const Rect& rect = Rect()) override;
        void setStencilState(const StencilState& state) override;
        void _setPolygonMode(PolygonMode level) override;
        void _convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest, bool) override;
        void _setDepthBias(float constantBias, float slopeScaleBias = 0.0f) override;
        void _setDepthBufferParams(bool depthTest, bool depthWrite, CompareFunction depthFunction) override;
        void _setCullingMode(CullingMode mode) override;
        void _setAlphaRejectSettings(CompareFunction func, unsigned char value, bool alphaToCoverage) override;
        void setColourBlendState(const ColourBlendState& state) override;
        void _setSampler(size_t texUnit, Sampler& s) override;
        MultiRenderTarget * createMultiRenderTarget(const String & name) { return NULL; }
    };
    /** @} */
    /** @} */
}  // namespace Ogre

#endif
