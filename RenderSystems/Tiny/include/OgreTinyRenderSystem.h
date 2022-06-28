// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#ifndef __TinyRenderSystem_H__
#define __TinyRenderSystem_H__

#include "OgreRenderWindow.h"
#include "OgreRenderSystem.h"

namespace Ogre {
    /** \addtogroup RenderSystems RenderSystems
    *  @{
    */
    /** \defgroup Tiny Tiny
    * Software rasterizer Implementation as a rendering system.
    *  @{
    */
    class HardwareBufferManager;

    struct IShader {
        // typedefs to make Ogre types more GLSLy
        typedef Vector<2, float> vec2;
        typedef Vector<3, float> vec3;
        typedef Vector<4, float> vec4;
        typedef Vector<3, uchar> vec3b;
        typedef Vector<4, uchar> vec4b;

        typedef Matrix3 mat3;
        typedef Matrix4 mat4;

        static int mod(int a, int b) {
            return (b + (a % b)) % b;
        }

        static const vec4b& sample2D(const Image& img, const vec2& uv)
        {
            Vector2i uvi(uv.x * (img.getWidth() - 1), uv.y * (img.getHeight() - 1));
            return *img.getData<const vec4b>(mod(uvi[0], img.getWidth()), mod(uvi[1], img.getHeight()));
        }

        virtual bool fragment(const vec3& bar, ColourValue& gl_FragColor) = 0;
    };

    /**
       Software rasterizer Implementation as a rendering system.
    */
    class TinyRenderSystem : public RenderSystem
    {
        Matrix4 mVP; // viewport transform

        Image* mActiveColourBuffer;
        Image* mActiveDepthBuffer;

        struct DefaultShader : public IShader
        {
            mat4 uniform_MVP;
            mat4 uniform_Tex;
            mat4 uniform_MVIT;
            vec3 uniform_lightDir;
            ColourValue uniform_ambientCol;

            bool uniform_doLighting;

            const Image* image;

            vec2 var_uv[3];
            vec3 var_normal[3];

            void vertex(const vec4& vertex, const vec2* uv, const vec3* normal, int gl_VertexID,
                        vec4& gl_Position);
            bool fragment(const vec3& bar, ColourValue& gl_FragColor) override;
        } mDefaultShader;

        bool mDepthTest;
        bool mDepthWrite;
        bool mBlendAdd;

        HardwareBufferManager* mHardwareBufferManager;

        /// Check if the GL system has already been initialised
        bool mGLInitialised;
    public:
        // Default constructor / destructor
        TinyRenderSystem();
        ~TinyRenderSystem();

        void initConfigOptions();

        Real getMinimumDepthInputValue(void) { return -1.0f; }            // Range [-1.0f, 1.0f]
        Real getMaximumDepthInputValue(void) { return 1.0f; }             // Range [-1.0f, 1.0f]
        void _convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest, bool) { dest = matrix; }

        void setConfigOption(const String &name, const String &value) {}

        // ----------------------------------
        // Overridden RenderSystem functions
        // ----------------------------------

        const String& getName(void) const;

        virtual RenderSystemCapabilities* createRenderSystemCapabilities() const;

        void initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary);

        void shutdown(void);

        /// @copydoc RenderSystem::_createRenderWindow
        RenderWindow* _createRenderWindow(const String &name, unsigned int width, unsigned int height,
                                          bool fullScreen, const NameValuePairList *miscParams = 0);

        /// @copydoc RenderSystem::_createDepthBufferFor
        DepthBuffer* _createDepthBufferFor( RenderTarget *renderTarget );

        /// @copydoc RenderSystem::createMultiRenderTarget
        virtual MultiRenderTarget * createMultiRenderTarget(const String & name);

        // -----------------------------
        // Low-level overridden members
        // -----------------------------
        void _setTexture(size_t unit, bool enabled, const TexturePtr &tex);

        void _setSampler(size_t unit, Sampler& sampler);


        void setLightingEnabled(bool enabled) { mDefaultShader.uniform_doLighting = enabled; }

        void _setViewport(Viewport *vp);

        void _endFrame(void);

        void _setCullingMode(CullingMode mode);

        void _setDepthBufferParams(bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL);

        void _setDepthBias(float constantBias, float slopeScaleBias);

        void setColourBlendState(const ColourBlendState& state);

        void _setPolygonMode(PolygonMode level);

        void setStencilState(const StencilState& state) override {}

        void applyFixedFunctionParams(const GpuProgramParametersPtr& params, uint16 variabilityMask);

        void _render(const RenderOperation& op);

        void setScissorTest(bool enabled, const Rect& rect = Rect());

        void clearFrameBuffer(unsigned int buffers,
                              const ColourValue& colour = ColourValue::Black,
                              float depth = 1.0f, unsigned short stencil = 0);
        HardwareOcclusionQuery* createHardwareOcclusionQuery(void);

        /**
         * Set current render target to target, enabling its GL context if needed
         */
        void _setRenderTarget(RenderTarget *target);

        void bindGpuProgramParameters(GpuProgramType gptype,
            const GpuProgramParametersPtr& params, uint16 variabilityMask) {}

        /// @copydoc RenderSystem::_setAlphaRejectSettings
        void _setAlphaRejectSettings( CompareFunction func, unsigned char value, bool alphaToCoverage );

        virtual void beginProfileEvent( const String &eventName ) {}
        virtual void endProfileEvent( void ) {}
        virtual void markProfileEvent( const String &eventName ) {}
    };
    /** @} */
    /** @} */
}

#endif
