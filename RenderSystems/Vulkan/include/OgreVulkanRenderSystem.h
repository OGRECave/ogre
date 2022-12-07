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

        void shutdown( void ) override;

        const String &getName( void ) const override;
        void refreshConfig();
        void initConfigOptions() override;
        void setConfigOption( const String &name, const String &value ) override;

        HardwareOcclusionQuery *createHardwareOcclusionQuery( void ) override;

        RenderSystemCapabilities *createRenderSystemCapabilities( void ) const override;

        void resetAllBindings( void );

        VkInstance getVkInstance( void ) const { return mVkInstance; }

        RenderWindow *_createRenderWindow( const String &name, uint32 width, uint32 height,
                                             bool fullScreen, const NameValuePairList *miscParams = 0 ) override;

        void flushUAVs( void );

        void _setTexture( size_t unit, bool enabled, const TexturePtr& texPtr ) override;

        virtual VulkanFrameBufferDescMap &_getFrameBufferDescMap( void ) { return mFrameBufferDescMap; }

        void _beginFrame( void ) override;
        void _endFrame( void ) override;

        void _render( const RenderOperation &op ) override;

        void bindGpuProgram(GpuProgram* prg) override;
        void bindGpuProgramParameters( GpuProgramType gptype,
                                               const GpuProgramParametersPtr& params,
                                               uint16 variabilityMask ) override;

        Real getHorizontalTexelOffset( void ) override;
        Real getVerticalTexelOffset( void ) override;
        Real getMinimumDepthInputValue( void ) override;
        Real getMaximumDepthInputValue( void ) override;

        void beginProfileEvent( const String &eventName ) override;
        void endProfileEvent( void ) override;
        void markProfileEvent( const String &event ) override;

        virtual void initGPUProfiling( void );
        virtual void deinitGPUProfiling( void );
        virtual void beginGPUSampleProfile( const String &name, uint32 *hashCache );
        virtual void endGPUSampleProfile( const String &name );

        void initialiseFromRenderSystemCapabilities( RenderSystemCapabilities *caps,
                                                             RenderTarget *primary ) override;

        void executeRenderPassDescriptorDelayedActions( bool officialCall = true );
        void endRenderPassDescriptor();

        DepthBuffer *_createDepthBufferFor( RenderTarget* renderTarget) override;

        void notifySwapchainDestroyed();

        VulkanDevice *getVulkanDevice() const { return mDevice; }
        void _notifyDeviceStalled();

        void _notifyActiveEncoderEnded();
        void _notifyActiveComputeEnded( void );

        void _setViewport(Viewport *vp) override;
        void _setRenderTarget(RenderTarget *target) override;
        void clearFrameBuffer(unsigned int buffers, const ColourValue& colour = ColourValue::Black,
                              float depth = 1.0f, unsigned short stencil = 0) override;
        void setScissorTest(bool enabled, const Rect& rect = Rect()) override;
        void setStencilState(const StencilState& state) override;
        void _setPolygonMode(PolygonMode level) override;
        void _convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest, bool) override;
        void _setDepthBias(float constantBias, float slopeScaleBias = 0.0f) override;
        void _setDepthBufferParams(bool depthTest, bool depthWrite, CompareFunction depthFunction) override;
        void _setDepthClamp(bool enable) override;
        void _setCullingMode(CullingMode mode) override;
        void _setAlphaRejectSettings(CompareFunction func, unsigned char value, bool alphaToCoverage) override;
        void setColourBlendState(const ColourBlendState& state) override;
        void _setSampler(size_t texUnit, Sampler& s) override;
        MultiRenderTarget * createMultiRenderTarget(const String & name) override { return NULL; }
    };
    /** @} */
    /** @} */
}  // namespace Ogre

#endif
