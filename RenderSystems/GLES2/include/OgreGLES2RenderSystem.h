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

#ifndef __GLES2RenderSystem_H__
#define __GLES2RenderSystem_H__

#include "OgreGLES2Prerequisites.h"

#include "OgreMaterialManager.h"
#include "OgreRenderSystem.h"
#include "OgreGLES2GpuProgram.h"
#include "OgreGLRenderSystemCommon.h"

namespace Ogre {
    /** \addtogroup RenderSystems RenderSystems
    *  @{
    */
    /** \defgroup GLES2 GLES2
    * Implementation of GL ES 2.x as a rendering system.
    *  @{
    */
    class GLES2Support;
    class GLES2RTTManager;
    class GLES2GpuProgramManager;
    class GLSLESProgramFactory;
    class GLES2StateCacheManager;
#if !OGRE_NO_GLES2_CG_SUPPORT
    class GLSLESCgProgramFactory;
#endif
    class GLSLESGpuProgram;
    class HardwareBufferManager;
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
    class GLES2ManagedResourceManager;
#endif
    
    /**
      Implementation of GL ES 2.x as a rendering system.
     */
    class _OgreGLES2Export GLES2RenderSystem : public GLRenderSystemCommon
    {
        private:
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

            GLfloat mAutoTextureMatrix[16];

            bool mUseAutoTextureMatrix;

            /// GL support class, used for creating windows etc.
            GLES2Support *mGLSupport;

            /// State cache manager which responsible to reduce redundant state changes
            GLES2StateCacheManager* mStateCacheManager;
            
            /* The main GL context - main thread only */
            GLES2Context *mMainContext;

            /* The current GL context  - main thread only */
            GLES2Context *mCurrentContext;

            typedef list<GLES2Context*>::type GLES2ContextList;
            /// List of background thread contexts
            GLES2ContextList mBackgroundContextList;

            GLES2GpuProgramManager *mGpuProgramManager;
            GLSLESProgramFactory* mGLSLESProgramFactory;
#if !OGRE_NO_GLES2_CG_SUPPORT
            GLSLESCgProgramFactory* mGLSLESCgProgramFactory;
#endif
            HardwareBufferManager* mHardwareBufferManager;

            /** Manager object for creating render textures.
                Direct render to texture via GL_OES_framebuffer_object is preferable 
                to pbuffers, which depend on the GL support used and are generally 
                unwieldy and slow. However, FBO support for stencil buffers is poor.
              */
            GLES2RTTManager *mRTTManager;

            /// Check if the GL system has already been initialised
            bool mGLInitialised;

            // check if GLES 3.0 is supported
            bool mHasGLES30;

            // local data member of _render that were moved here to improve performance
            // (save allocations)
            vector<GLuint>::type mRenderAttribsBound;
            vector<GLuint>::type mRenderInstanceAttribsBound;

            GLint getCombinedMinMipFilter(void) const;

            GLES2GpuProgram* mCurrentVertexProgram;
            GLES2GpuProgram* mCurrentFragmentProgram;

            GLint getTextureAddressingMode(TextureUnitState::TextureAddressingMode tam) const;
            GLenum getBlendMode(SceneBlendFactor ogreBlend) const;
            void bindVertexElementToGpu( const VertexElement &elem, HardwareVertexBufferSharedPtr vertexBuffer,
                                        const size_t vertexStart,
                                        vector<GLuint>::type &attribsBound,
                                        vector<GLuint>::type &instanceAttribsBound,
                                        bool updateVAO);

            // Mipmap count of the actual bounded texture
            size_t mCurTexMipCount;
            GLint mViewport[4];
            GLint mScissor[4];

        public:
            // Default constructor / destructor
            GLES2RenderSystem();
            virtual ~GLES2RenderSystem();
        
            friend class ShaderGeneratorTechniqueResolverListener;

            // ----------------------------------
            // Overridden RenderSystem functions
            // ----------------------------------

            const String& getName(void) const;

            ConfigOptionMap& getConfigOptions(void);

            void setConfigOption(const String &name, const String &value);

            String validateConfigOptions(void);

            RenderWindow* _initialise(bool autoCreateWindow, const String& windowTitle = "OGRE Render NativeWindowType");

            virtual RenderSystemCapabilities* createRenderSystemCapabilities() const;

            void initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary);

            void reinitialise(void); // Used if settings changed mid-rendering

            void shutdown(void);

            void setAmbientLight(float r, float g, float b) { };   // Not supported

            void setShadingType(ShadeOptions so) { };   // Not supported

            void setLightingEnabled(bool enabled) { };   // Not supported

            /// @copydoc RenderSystem::_createRenderWindow
            RenderWindow* _createRenderWindow(const String &name, unsigned int width, unsigned int height, 
                bool fullScreen, const NameValuePairList *miscParams = 0);

            /// @copydoc RenderSystem::_createDepthBufferFor
            DepthBuffer* _createDepthBufferFor( RenderTarget *renderTarget );

            /// Mimics D3D9RenderSystem::_getDepthStencilFormatFor, if no FBO RTT manager, outputs GL_NONE
            void _getDepthStencilFormatFor( GLenum internalColourFormat, GLenum *depthFormat,
                                            GLenum *stencilFormat );

            /// @copydoc RenderSystem::createMultiRenderTarget
            virtual MultiRenderTarget * createMultiRenderTarget(const String & name);


            void destroyRenderWindow(const String& name);

            String getErrorDescription(long errorNumber) const;

            VertexElementType getColourVertexElementType(void) const;

            void setNormaliseNormals(bool normalise) { };   // Not supported

            // -----------------------------
            // Low-level overridden members
            // -----------------------------

            void _useLights(const LightList& lights, unsigned short limit) { };   // Not supported

            bool areFixedFunctionLightsInViewSpace() const { return true; }

            void _setWorldMatrix(const Matrix4 &m);

            void _setViewMatrix(const Matrix4 &m);

            void _setProjectionMatrix(const Matrix4 &m);

            void _setSurfaceParams(const ColourValue &ambient,
                                   const ColourValue &diffuse, const ColourValue &specular,
                                   const ColourValue &emissive, Real shininess,
                                   TrackVertexColourType tracking) {}

            void _setPointParameters(Real size, bool attenuationEnabled,
                                     Real constant, Real linear, Real quadratic, Real minSize, Real maxSize) {}

            void _setPointSpritesEnabled(bool enabled) {}

            void _setTexture(size_t unit, bool enabled, const TexturePtr &tex);

            void _setTextureCoordSet(size_t stage, size_t index);

            void _setTextureCoordCalculation(size_t stage, TexCoordCalcMethod m,
                    const Frustum* frustum = 0) { };   // Not supported

            void _setTextureBlendMode(size_t stage, const LayerBlendModeEx& bm) { };   // Not supported

            void _setTextureAddressingMode(size_t stage, const TextureUnitState::UVWAddressingMode& uvw);

            void _setTextureBorderColour(size_t stage, const ColourValue& colour) { };   // Not supported

            void _setTextureMipmapBias(size_t unit, float bias) { };   // Not supported

            void _setTextureMatrix(size_t stage, const Matrix4& xform) { };   // Not supported

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

            void _convertProjectionMatrix(const Matrix4& matrix,
                    Matrix4& dest, bool forGpuProgram = false);

            void _makeProjectionMatrix(const Radian& fovy, Real aspect, Real nearPlane, Real farPlane,
                    Matrix4& dest, bool forGpuProgram = false);

            void _makeProjectionMatrix(Real left, Real right, Real bottom, Real top, 
                    Real nearPlane, Real farPlane, Matrix4& dest, bool forGpuProgram = false);

            void _makeOrthoMatrix(const Radian& fovy, Real aspect, Real nearPlane, Real farPlane,
                    Matrix4& dest, bool forGpuProgram = false);

            void _applyObliqueDepthProjection(Matrix4& matrix, const Plane& plane, 
                    bool forGpuProgram);

            void setClipPlane (ushort index, Real A, Real B, Real C, Real D);

            void enableClipPlane (ushort index, bool enable);

            void _setPolygonMode(PolygonMode level);

            void setStencilCheckEnabled(bool enabled);

            void setStencilBufferParams(CompareFunction func = CMPF_ALWAYS_PASS, 
                    uint32 refValue = 0, uint32 compareMask = 0xFFFFFFFF, uint32 writeMask = 0xFFFFFFFF,
                    StencilOperation stencilFailOp = SOP_KEEP,
                    StencilOperation depthFailOp = SOP_KEEP,
                    StencilOperation passOp = SOP_KEEP,
                    bool twoSidedOperation = false,
                    bool readBackAsTexture = false);

            void _setTextureUnitCompareFunction(size_t unit, CompareFunction function);

            void _setTextureUnitCompareEnabled(size_t unit, bool compare);          

            virtual void _setTextureUnitFiltering(size_t unit, FilterOptions minFilter,
                FilterOptions magFilter, FilterOptions mipFilter);              

            void _setTextureUnitFiltering(size_t unit, FilterType ftype, FilterOptions filter);

            void _setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy);

            virtual bool hasAnisotropicMipMapFilter() const { return false; }   

            void setVertexDeclaration(VertexDeclaration* decl);

            void setVertexDeclaration(VertexDeclaration* decl, VertexBufferBinding* binding);

            void setVertexBufferBinding(VertexBufferBinding* binding) {}

            void _render(const RenderOperation& op);

            void setScissorTest(bool enabled, size_t left = 0, size_t top = 0, size_t right = 800, size_t bottom = 600);

            void clearFrameBuffer(unsigned int buffers,
                const ColourValue& colour = ColourValue::Black,
                Real depth = 1.0f, unsigned short stencil = 0);
            HardwareOcclusionQuery* createHardwareOcclusionQuery(void);
            Real getHorizontalTexelOffset(void) { return 0.0; }               // No offset in GL
            Real getVerticalTexelOffset(void) { return 0.0; }                 // No offset in GL
            Real getMinimumDepthInputValue(void) { return -1.0f; }            // Range [-1.0f, 1.0f]
            Real getMaximumDepthInputValue(void) { return 1.0f; }             // Range [-1.0f, 1.0f]
            OGRE_MUTEX(mThreadInitMutex);
            void registerThread();
            void unregisterThread();
            void preExtraThreadsStarted();
            void postExtraThreadsStarted();
            void setClipPlanesImpl(const Ogre::PlaneList& planeList) {}
            GLES2Support* getGLSupportRef() { return mGLSupport; }

            // ----------------------------------
            // GLES2RenderSystem specific members
            // ----------------------------------
            /** Returns the main context */
            GLES2Context* _getMainContext() { return mMainContext; }
            /** Unregister a render target->context mapping. If the context of target 
             is the current context, change the context to the main context so it
             can be destroyed safely. 
             
             @note This is automatically called by the destructor of 
             GLES2Context.
             */
            void _unregisterContext(GLES2Context *context);
            /** Switch GL context, dealing with involved internal cached states too
             */
            void _switchContext(GLES2Context *context);
            /** One time initialization for the RenderState of a context. Things that
             only need to be set once, like the LightingModel can be defined here.
             */
            void _oneTimeContextInitialization();
            void initialiseContext(RenderWindow* primary);
            /**
             * Set current render target to target, enabling its GL context if needed
             */
            void _setRenderTarget(RenderTarget *target);

            GLES2Support* getGLES2Support() { return mGLSupport; }
            GLint convertCompareFunction(CompareFunction func) const;
            GLint convertStencilOp(StencilOperation op, bool invert = false) const;

            void bindGpuProgram(GpuProgram* prg);
            void unbindGpuProgram(GpuProgramType gptype);
            void bindGpuProgramParameters(GpuProgramType gptype, GpuProgramParametersSharedPtr params, uint16 mask);
            void bindGpuProgramPassIterationParameters(GpuProgramType gptype);

            /// @copydoc RenderSystem::_setSceneBlending
            void _setSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendOperation op );
            /// @copydoc RenderSystem::_setSeparateSceneBlending
            void _setSeparateSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendFactor sourceFactorAlpha, SceneBlendFactor destFactorAlpha, SceneBlendOperation op, SceneBlendOperation alphaOp );
            /// @copydoc RenderSystem::_setAlphaRejectSettings
            void _setAlphaRejectSettings( CompareFunction func, unsigned char value, bool alphaToCoverage );
            /// @copydoc RenderSystem::getDisplayMonitorCount
            unsigned int getDisplayMonitorCount() const;

            /// Internal method for anisotropy validation
            GLfloat _getCurrentAnisotropy(size_t unit);

            void _setSceneBlendingOperation(SceneBlendOperation op);
            void _setSeparateSceneBlendingOperation(SceneBlendOperation op, SceneBlendOperation alphaOp);

            unsigned int getDiscardBuffers(void);

            void _destroyDepthBuffer(RenderTarget* pRenderWnd);
        
            /// @copydoc RenderSystem::beginProfileEvent
            virtual void beginProfileEvent( const String &eventName );
            
            /// @copydoc RenderSystem::endProfileEvent
            virtual void endProfileEvent( void );
            
            /// @copydoc RenderSystem::markProfileEvent
            virtual void markProfileEvent( const String &eventName );

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
            void resetRenderer(RenderWindow* pRenderWnd);
        
            void notifyOnContextLost();

            static GLES2ManagedResourceManager* getResourceManager();
    private:
            static GLES2ManagedResourceManager* mResourceManager;
#endif
            void _copyContentsToMemory(Viewport* vp, const Box& src, const PixelBox& dst, RenderWindow::FrameBuffer buffer);
    };
}

#endif
