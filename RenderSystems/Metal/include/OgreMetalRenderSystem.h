/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2016 Torus Knot Software Ltd

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

#ifndef _OgreMetalRenderSystem_H_
#define _OgreMetalRenderSystem_H_

#include "OgreMetalPrerequisites.h"

#include "OgreRenderSystem.h"
#include "OgreMetalDevice.h"

#import <Metal/MTLRenderCommandEncoder.h>
#import <dispatch/dispatch.h>

namespace Ogre
{
        class HardwareBufferManager;

    /** \addtogroup RenderSystems RenderSystems
    *  @{
    */
    /** \defgroup Metal Metal
    * Implementation of Metal as a rendering system.
    *  @{
    */
    class _OgreMetalExport MetalRenderSystem : public RenderSystem
    {
        MTLRenderPipelineDescriptor *psd;
        MTLDepthStencilDescriptor*    mDepthStencilDesc;
        id<MTLDepthStencilState>    mDepthStencilState;
        bool                        mDepthStencilDescChanged;

        bool mInitialized;
        HardwareBufferManager   *mHardwareBufferManager;
        MetalProgramFactory         *mMetalProgramFactory;

        bool mStencilEnabled;
        uint32_t mStencilRefValue;

        //For v1 rendering.
        IndexData       *mCurrentIndexBuffer;
        VertexData      *mCurrentVertexBuffer;
        MTLPrimitiveType    mCurrentPrimType;

        uint8           mNumMRTs;
        MetalRenderTargetCommon     *mCurrentColourRTs[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
        MetalDepthBuffer            *mCurrentDepthBuffer;
        MetalDevice                 *mActiveDevice;
        __unsafe_unretained id<MTLRenderCommandEncoder> mActiveRenderEncoder;

        MetalDevice             mDevice;
        dispatch_semaphore_t    mMainGpuSyncSemaphore;
        bool                    mMainSemaphoreAlreadyWaited;
        bool                    mBeginFrameOnceStarted;

        void setActiveDevice( MetalDevice *device );
        void createRenderEncoder(void);

        std::map<NSUInteger, id<MTLRenderPipelineState>> mPSOCache;
        id<MTLRenderPipelineState> getPipelineState();

        GpuProgram* mDefaultVP;
        GpuProgram* mDefaultFP;

        std::unique_ptr<MetalHardwareBufferCommon> mAutoParamsBuffer;
    public:
        const GpuProgramParametersPtr& getFixedFunctionParams(TrackVertexColourType tracking,
                                                                      FogMode fog) override;
        void applyFixedFunctionParams(const GpuProgramParametersPtr& params, uint16 mask) override;

        MetalRenderSystem();
        virtual ~MetalRenderSystem();

        void shutdown(void) override;

        const String& getName(void) const override;
        void setConfigOption(const String &name, const String &value) override {}

        HardwareOcclusionQuery* createHardwareOcclusionQuery(void) override;

        RenderSystemCapabilities* createRenderSystemCapabilities(void) const override;

        RenderWindow* _createRenderWindow( const String &name,
                                                   unsigned int width, unsigned int height,
                                                   bool fullScreen,
                                                   const NameValuePairList *miscParams = 0) override;

        MultiRenderTarget* createMultiRenderTarget(const String & name) override;

        void _setTexture(size_t unit, bool enabled,  const TexturePtr& texPtr) override;

        DepthBuffer* _createDepthBufferFor( RenderTarget *renderTarget) override;

        void setStencilState(const StencilState& state) override;

        /// See VaoManager::waitForTailFrameToFinish
        virtual void _waitForTailFrameToFinish(void);
        virtual bool _willTailFrameStall(void);

        void _beginFrame(void) override;
        void _endFrame(void) override;

        void setScissorTest(bool enabled, const Rect& rect) override;

        void _setViewport(Viewport *vp) override;

        void setColourBlendState(const ColourBlendState& state) override;

        void _setSampler( size_t texUnit, Sampler& s) override;
        void _setDepthClamp(bool enable) override;
        void _setDepthBufferParams(bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL) override;

        void _setCullingMode(CullingMode mode) override;
        void _setDepthBias(float constantBias, float slopeScaleBias = 0.0f) override;

        void _convertProjectionMatrix( const Matrix4& matrix, Matrix4& dest,
                                               bool forGpuProgram = false) override;
        void _setPolygonMode(PolygonMode level) override;
        void _setAlphaRejectSettings(CompareFunction func, unsigned char value, bool alphaToCoverage) override;
        void _render( const RenderOperation &op ) override;

        void bindGpuProgram(GpuProgram* prg) override;
        void bindGpuProgramParameters(GpuProgramType gptype,
            const GpuProgramParametersPtr& params, uint16 variabilityMask) override;
        void clearFrameBuffer(unsigned int buffers,
            const ColourValue& colour = ColourValue::Black,
            float depth = 1.0f, unsigned short stencil = 0) override;

        Real getMinimumDepthInputValue(void) override;
        Real getMaximumDepthInputValue(void) override;

        void _setRenderTarget(RenderTarget *target) override;

        void beginProfileEvent( const String &eventName ) override;
        void endProfileEvent( void ) override;
        void markProfileEvent( const String &event ) override;

        virtual void initGPUProfiling(void);
        virtual void deinitGPUProfiling(void);
        virtual void beginGPUSampleProfile( const String &name, uint32 *hashCache );
        virtual void endGPUSampleProfile( const String &name );

        void initialiseFromRenderSystemCapabilities( RenderSystemCapabilities* caps,
                                                             RenderTarget* primary ) override;

        MetalDevice* getActiveDevice(void)                      { return mActiveDevice; }

        void _notifyActiveEncoderEnded(void);
        void _notifyActiveComputeEnded(void);
        void _notifyDeviceStalled(void);
    };
    /** @} */
    /** @} */
}

#endif
