/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef __GLRenderSystem_H__
#define __GLRenderSystem_H__

#include "OgreGLPrerequisites.h"
#include "OgrePlatform.h"
#include "OgreRenderSystem.h"
#include "OgreGLHardwareBufferManager.h"
#include "OgreGLGpuProgramManager.h"
#include "OgreVector4.h"

#include "OgreGLRenderSystemCommon.h"
#include "OgreGLStateCacheManager.h"

namespace Ogre {
    /** \addtogroup RenderSystems RenderSystems
    *  @{
    */
    /** \defgroup GL GL
    * Implementation of GL as a rendering system.
    *  @{
    */

    namespace GLSL {
        class GLSLProgramFactory;
    }

    /**
      Implementation of GL as a rendering system.
     */
    class _OgreGLExport GLRenderSystem : public GLRenderSystemCommon
    {
    private:
        /// Rendering loop control
        bool mStopRendering;

        /** Array of up to 8 lights, indexed as per API
            Note that a null value indicates a free slot
          */ 
        #define MAX_LIGHTS 8
        Light* mLights[MAX_LIGHTS];

        /// View matrix to set world against
        Matrix4 mViewMatrix;
        Matrix4 mWorldMatrix;
        Matrix4 mTextureMatrix;

        /// Last min & mip filtering options, so we can combine them
        FilterOptions mMinFilter;
        FilterOptions mMipFilter;

        /// What texture coord set each texture unit is using
        size_t mTextureCoordIndex[OGRE_MAX_TEXTURE_LAYERS];

        /// Holds texture type settings for every stage
        GLenum mTextureTypes[OGRE_MAX_TEXTURE_LAYERS];

        /// Number of fixed-function texture units
        unsigned short mFixedFunctionTextureUnits;

        void initConfigOptions(void);

        void setGLLight(size_t index, Light* lt);
        void makeGLMatrix(GLfloat gl_matrix[16], const Matrix4& m);
 
        GLint getBlendMode(SceneBlendFactor ogreBlend) const;
        GLint getTextureAddressingMode(TextureUnitState::TextureAddressingMode tam) const;
                void initialiseContext(RenderWindow* primary);

        void setLights();

        /// Store last colour write state
        bool mColourWrite[4];
        /// Store last stencil mask state
        uint32 mStencilWriteMask;
        /// Store last depth write state
        bool mDepthWrite;
        /// Store last scissor enable state
        bool mScissorsEnabled;

        /// Store scissor box
        int mScissorBox[4];

        GLint convertCompareFunction(CompareFunction func) const;
        GLint convertStencilOp(StencilOperation op, bool invert = false) const;
        
        /// GL support class, used for creating windows etc.
        GLSupport* mGLSupport;
        
        /// Internal method to set pos / direction of a light
        void setGLLightPositionDirection(Light* lt, GLenum lightindex);

        bool mUseAutoTextureMatrix;
        GLfloat mAutoTextureMatrix[16];

        /// Check if the GL system has already been initialised
        bool mGLInitialised;

        HardwareBufferManager* mHardwareBufferManager;
        GLGpuProgramManager* mGpuProgramManager;
        GLSL::GLSLProgramFactory* mGLSLProgramFactory;

        unsigned short mCurrentLights;

        GLuint getCombinedMinMipFilter(void) const;

        GLGpuProgram* mCurrentVertexProgram;
        GLGpuProgram* mCurrentFragmentProgram;
        GLGpuProgram* mCurrentGeometryProgram;

        typedef std::list<GLContext*> GLContextList;
        /// List of background thread contexts
        GLContextList mBackgroundContextList;

        // statecaches are per context
        GLStateCacheManager* mStateCacheManager;

        /** Manager object for creating render textures.
            Direct render to texture via GL_EXT_framebuffer_object is preferable 
            to pbuffers, which depend on the GL support used and are generally 
            unwieldy and slow. However, FBO support for stencil buffers is poor.
        */
        GLRTTManager *mRTTManager;

        ushort mActiveTextureUnit;
        ushort mMaxBuiltInTextureAttribIndex;

        // local data members of _render that were moved here to improve performance
        // (save allocations)
        std::vector<GLuint> mRenderAttribsBound;
        std::vector<GLuint> mRenderInstanceAttribsBound;

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		/// @copydoc RenderSystem::setDrawBuffer
		virtual bool setDrawBuffer(ColourBufferType colourBuffer);
#endif

    protected:
        void setClipPlanesImpl(const PlaneList& clipPlanes);
        void bindVertexElementToGpu(const VertexElement& elem,
                                    const HardwareVertexBufferSharedPtr& vertexBuffer,
                                    const size_t vertexStart);
    public:
        // Default constructor / destructor
        GLRenderSystem();
        ~GLRenderSystem();

        // ----------------------------------
        // Overridden RenderSystem functions
        // ----------------------------------

        const String& getName(void) const;


        ConfigOptionMap& getConfigOptions(void);

        void setConfigOption(const String &name, const String &value);

        String validateConfigOptions(void);

        RenderWindow* _initialise(bool autoCreateWindow, const String& windowTitle = "OGRE Render Window");

        virtual RenderSystemCapabilities* createRenderSystemCapabilities() const;

        void initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary);

        void shutdown(void);

        void setAmbientLight(float r, float g, float b);

        void setShadingType(ShadeOptions so);

        void setLightingEnabled(bool enabled);
        
        /// @copydoc RenderSystem::_createRenderWindow
        RenderWindow* _createRenderWindow(const String &name, unsigned int width, unsigned int height, 
                                          bool fullScreen, const NameValuePairList *miscParams = 0);

        /// @copydoc RenderSystem::_createRenderWindows
        bool _createRenderWindows(const RenderWindowDescriptionList& renderWindowDescriptions, 
                                  RenderWindowList& createdWindows);

        /// @copydoc RenderSystem::_createDepthBufferFor
        DepthBuffer* _createDepthBufferFor( RenderTarget *renderTarget );

        /// Mimics D3D9RenderSystem::_getDepthStencilFormatFor, if no FBO RTT manager, outputs GL_NONE
        void _getDepthStencilFormatFor( PixelFormat internalColourFormat, GLenum *depthFormat,
                                        GLenum *stencilFormat );
        
        /// @copydoc RenderSystem::createMultiRenderTarget
        virtual MultiRenderTarget * createMultiRenderTarget(const String & name); 
        

        void destroyRenderWindow(const String& name);

        void setNormaliseNormals(bool normalise);

        // -----------------------------
        // Low-level overridden members
        // -----------------------------

        void _useLights(const LightList& lights, unsigned short limit);

        bool areFixedFunctionLightsInViewSpace() const { return true; }

        void _setWorldMatrix(const Matrix4 &m);

        void _setViewMatrix(const Matrix4 &m);

        void _setProjectionMatrix(const Matrix4 &m);

        void _setSurfaceParams(const ColourValue &ambient,
            const ColourValue &diffuse, const ColourValue &specular,
            const ColourValue &emissive, Real shininess,
            TrackVertexColourType tracking);

        void _setPointParameters(Real size, bool attenuationEnabled, 
            Real constant, Real linear, Real quadratic, Real minSize, Real maxSize);

        void _setLineWidth(float width);

        void _setPointSpritesEnabled(bool enabled);

        void _setTexture(size_t unit, bool enabled, const TexturePtr &tex);

        void _setTextureCoordSet(size_t stage, size_t index);

        void _setTextureCoordCalculation(size_t stage, TexCoordCalcMethod m, 
            const Frustum* frustum = 0);

        void _setTextureBlendMode(size_t stage, const LayerBlendModeEx& bm);

        void _setTextureAddressingMode(size_t stage, const TextureUnitState::UVWAddressingMode& uvw);

        void _setTextureBorderColour(size_t stage, const ColourValue& colour);

        void _setTextureMipmapBias(size_t unit, float bias);

        void _setTextureMatrix(size_t stage, const Matrix4& xform);

        void _setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendOperation op );

        void _setSeparateSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendFactor sourceFactorAlpha, SceneBlendFactor destFactorAlpha, SceneBlendOperation op, SceneBlendOperation alphaOp );

        void _setSceneBlendingOperation(SceneBlendOperation op);

        void _setSeparateSceneBlendingOperation(SceneBlendOperation op, SceneBlendOperation alphaOp);

        void _setAlphaRejectSettings(CompareFunction func, unsigned char value, bool alphaToCoverage);

        void _setViewport(Viewport *vp);

        void _beginFrame(void);

        void _endFrame(void);

        void _setCullingMode(CullingMode mode);

        void _setDepthBufferParams(bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL);

        void _setDepthBufferCheckEnabled(bool enabled = true);

        void _setDepthBufferWriteEnabled(bool enabled = true);

        void _setDepthBufferFunction(CompareFunction func = CMPF_LESS_EQUAL);

        void _setDepthBias(float constantBias, float slopeScaleBias);

        void _setColourBufferWriteEnabled(bool red, bool green, bool blue, bool alpha);

        void _setFog(FogMode mode, const ColourValue& colour, Real density, Real start, Real end);

        void setClipPlane (ushort index, Real A, Real B, Real C, Real D);

        void enableClipPlane (ushort index, bool enable);

        void _setPolygonMode(PolygonMode level);

        void setStencilCheckEnabled(bool enabled);
        /** See
          RenderSystem.
         */
        void setStencilBufferParams(CompareFunction func = CMPF_ALWAYS_PASS, 
            uint32 refValue = 0, uint32 compareMask = 0xFFFFFFFF, uint32 writeMask = 0xFFFFFFFF,
            StencilOperation stencilFailOp = SOP_KEEP, 
            StencilOperation depthFailOp = SOP_KEEP,
            StencilOperation passOp = SOP_KEEP, 
            bool twoSidedOperation = false,
            bool readBackAsTexture = false);

        void _setTextureUnitFiltering(size_t unit, FilterType ftype, FilterOptions filter);

        void _setTextureUnitCompareFunction(size_t unit, CompareFunction function);

        void _setTextureUnitCompareEnabled(size_t unit, bool compare);

        void _setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy);

        void _render(const RenderOperation& op);

        void bindGpuProgram(GpuProgram* prg);

        void unbindGpuProgram(GpuProgramType gptype);

        void bindGpuProgramParameters(GpuProgramType gptype, 
                                      GpuProgramParametersSharedPtr params, uint16 variabilityMask);
        /** See
            RenderSystem
        */
        void bindGpuProgramPassIterationParameters(GpuProgramType gptype);

        void setScissorTest(bool enabled, size_t left = 0, size_t top = 0, size_t right = 800, size_t bottom = 600) ;
        void clearFrameBuffer(unsigned int buffers, 
                              const ColourValue& colour = ColourValue::Black, 
                              Real depth = 1.0f, unsigned short stencil = 0);
        HardwareOcclusionQuery* createHardwareOcclusionQuery(void);
        OGRE_MUTEX(mThreadInitMutex);
        void registerThread();
        void unregisterThread();
        void preExtraThreadsStarted();
        void postExtraThreadsStarted();
        GLSupport* getGLSupportRef() { return mGLSupport; }

        // ----------------------------------
        // GLRenderSystem specific members
        // ----------------------------------
        /** One time initialization for the RenderState of a context. Things that
            only need to be set once, like the LightingModel can be defined here.
         */
        void _oneTimeContextInitialization();
        /** Switch GL context, dealing with involved internal cached states too
        */
        void _switchContext(GLContext *context);
        /**
         * Set current render target to target, enabling its GL context if needed
         */
        void _setRenderTarget(RenderTarget *target);
        /** Unregister a render target->context mapping. If the context of target 
            is the current context, change the context to the main context so it
            can be destroyed safely. 
            
            @note This is automatically called by the destructor of 
            GLContext.
         */
        void _unregisterContext(GLContext *context);

        GLStateCacheManager * _getStateCacheManager() { return mStateCacheManager; }

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

        /** @copydoc RenderTarget::copyContentsToMemory */
        void _copyContentsToMemory(Viewport* vp, const Box& src, const PixelBox &dst, RenderWindow::FrameBuffer buffer);
    };
    /** @} */
    /** @} */
}
#endif

