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

#ifndef __NULLRenderSystem_H__
#define __NULLRenderSystem_H__

#include "OgreNULLPrerequisites.h"

#include "OgreRenderSystem.h"

namespace Ogre
{
    namespace v1
    {
        class HardwareBufferManager;
    }

    class _OgreNULLExport NULLPixelFormatToShaderType : public PixelFormatToShaderType
    {
    public:
        virtual const char* getPixelFormatType( PixelFormat pixelFormat ) const { return 0; }
    };

    /**
       Implementation of NULL as a rendering system.
    */
    class _OgreNULLExport NULLRenderSystem : public RenderSystem
    {
        bool mInitialized;
        v1::HardwareBufferManager *mHardwareBufferManager;

        ConfigOptionMap mOptions;

        vector<RenderTarget*>::type mRenderTargets;

        NULLPixelFormatToShaderType mPixelFormatToShaderType;

    public:
        NULLRenderSystem();

        virtual void shutdown(void);

        virtual const String& getName(void) const;
        virtual const String& getFriendlyName(void) const;
        virtual ConfigOptionMap& getConfigOptions(void) { return mOptions; }
        virtual void setConfigOption(const String &name, const String &value) {}

        virtual HardwareOcclusionQuery* createHardwareOcclusionQuery(void);

        virtual String validateConfigOptions(void)  { return BLANKSTRING; }

        virtual RenderSystemCapabilities* createRenderSystemCapabilities(void) const;

        virtual void reinitialise(void);

        virtual RenderWindow* _initialise(bool autoCreateWindow, const String& windowTitle = "OGRE Render Window");

        virtual RenderWindow* _createRenderWindow( const String &name,
                                                   unsigned int width, unsigned int height,
                                                   bool fullScreen,
                                                   const NameValuePairList *miscParams = 0);

        virtual MultiRenderTarget* createMultiRenderTarget(const String & name);

        virtual String getErrorDescription(long errorNumber) const;

        virtual void _useLights(const LightList& lights, unsigned short limit);
        virtual void _setWorldMatrix(const Matrix4 &m);
        virtual void _setViewMatrix(const Matrix4 &m);
        virtual void _setProjectionMatrix(const Matrix4 &m);

        virtual void _setSurfaceParams( const ColourValue &ambient,
                                const ColourValue &diffuse, const ColourValue &specular,
                                const ColourValue &emissive, Real shininess,
                                TrackVertexColourType tracking = TVC_NONE );
        virtual void _setPointSpritesEnabled(bool enabled);
        virtual void _setPointParameters(Real size, bool attenuationEnabled,
            Real constant, Real linear, Real quadratic, Real minSize, Real maxSize);

        virtual void queueBindUAV( uint32 slot, TexturePtr texture,
                                           ResourceAccess::ResourceAccess access = ResourceAccess::ReadWrite,
                                           int32 mipmapLevel = 0, int32 textureArrayIndex = 0,
                                           PixelFormat pixelFormat = PF_UNKNOWN );
        virtual void queueBindUAV( uint32 slot, UavBufferPacked *buffer,
                                   ResourceAccess::ResourceAccess access = ResourceAccess::ReadWrite,
                                   size_t offset = 0, size_t sizeBytes = 0 );
        virtual void clearUAVs(void);
        virtual void flushUAVs(void);

        virtual void _bindTextureUavCS( uint32 slot, Texture *texture,
                                        ResourceAccess::ResourceAccess access,
                                        int32 mipmapLevel, int32 textureArrayIndex,
                                        PixelFormat pixelFormat );
        virtual void _setTextureCS( uint32 slot, bool enabled, Texture *texPtr );
        virtual void _setHlmsSamplerblockCS( uint8 texUnit, const HlmsSamplerblock *samplerblock );

        virtual void _setTexture(size_t unit, bool enabled,  Texture *texPtr);

        virtual void _setTextureCoordSet(size_t unit, size_t index);
        virtual void _setTextureCoordCalculation(size_t unit, TexCoordCalcMethod m,
                                                 const Frustum* frustum = 0);
        virtual void _setTextureBlendMode(size_t unit, const LayerBlendModeEx& bm);
        virtual void _setTextureMatrix(size_t unit, const Matrix4& xform);

        virtual void _setIndirectBuffer( IndirectBufferPacked *indirectBuffer );

        virtual DepthBuffer* _createDepthBufferFor( RenderTarget *renderTarget,
                                                    bool exactMatchFormat );

        virtual void _beginFrame(void);
        virtual void _endFrame(void);
        virtual void _setViewport(Viewport *vp);

        virtual void _setHlmsSamplerblock( uint8 texUnit, const HlmsSamplerblock *Samplerblock );
        virtual void _setPipelineStateObject( const HlmsPso *pso );
        virtual void _setComputePso( const HlmsComputePso *pso );

        virtual VertexElementType getColourVertexElementType(void) const;
        virtual void _convertProjectionMatrix(const Matrix4& matrix,
            Matrix4& dest, bool forGpuProgram = false) {}
        virtual void _makeProjectionMatrix(const Radian& fovy, Real aspect, Real nearPlane, Real farPlane,
            Matrix4& dest, bool forGpuProgram = false) {}

        virtual void _makeProjectionMatrix(Real left, Real right, Real bottom, Real top,
            Real nearPlane, Real farPlane, Matrix4& dest, bool forGpuProgram = false) {}
        virtual void _makeOrthoMatrix(const Radian& fovy, Real aspect, Real nearPlane, Real farPlane,
            Matrix4& dest, bool forGpuProgram = false) {}
        virtual void _applyObliqueDepthProjection(Matrix4& matrix, const Plane& plane,
            bool forGpuProgram) {}

        virtual void _dispatch( const HlmsComputePso &pso );

        virtual void _setVertexArrayObject( const VertexArrayObject *vao );

        virtual void _render( const CbDrawCallIndexed *cmd );
        virtual void _render( const CbDrawCallStrip *cmd );
        virtual void _renderEmulated( const CbDrawCallIndexed *cmd );
        virtual void _renderEmulated( const CbDrawCallStrip *cmd );

        virtual void _setRenderOperation( const v1::CbRenderOp *cmd );
        virtual void _render( const v1::CbDrawCallIndexed *cmd );
        virtual void _render( const v1::CbDrawCallStrip *cmd );

        virtual void bindGpuProgramParameters(GpuProgramType gptype,
            GpuProgramParametersSharedPtr params, uint16 variabilityMask);
        virtual void bindGpuProgramPassIterationParameters(GpuProgramType gptype);

        virtual void clearFrameBuffer(unsigned int buffers,
            const ColourValue& colour = ColourValue::Black,
            Real depth = 1.0f, unsigned short stencil = 0);
        virtual void discardFrameBuffer( unsigned int buffers );

        virtual Real getHorizontalTexelOffset(void);
        virtual Real getVerticalTexelOffset(void);
        virtual Real getMinimumDepthInputValue(void);
        virtual Real getMaximumDepthInputValue(void);

        virtual void _setRenderTarget( RenderTarget *target, uint8 viewportRenderTargetFlags );
        virtual void preExtraThreadsStarted();
        virtual void postExtraThreadsStarted();
        virtual void registerThread();
        virtual void unregisterThread();
        virtual unsigned int getDisplayMonitorCount() const     { return 1; }

        virtual const PixelFormatToShaderType* getPixelFormatToShaderType(void) const;

        virtual void beginProfileEvent( const String &eventName );
        virtual void endProfileEvent( void );
        virtual void markProfileEvent( const String &event );

        virtual void initGPUProfiling(void);
        virtual void deinitGPUProfiling(void);
        virtual void beginGPUSampleProfile( const String &name, uint32 *hashCache );
        virtual void endGPUSampleProfile( const String &name );

        virtual bool hasAnisotropicMipMapFilter() const         { return true; }

        virtual void setClipPlanesImpl(const PlaneList& clipPlanes);
        virtual void initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary);
    };
}

#endif
