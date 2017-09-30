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

#include "OgreNULLRenderSystem.h"
#include "OgreNULLRenderWindow.h"
#include "OgreNULLTextureManager.h"
#include "Vao/OgreNULLVaoManager.h"

#include "OgreDefaultHardwareBufferManager.h"

namespace Ogre
{
    //-------------------------------------------------------------------------
    NULLRenderSystem::NULLRenderSystem() :
        RenderSystem(),
        mInitialized( false ),
        mHardwareBufferManager( 0 )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::shutdown(void)
    {
        OGRE_DELETE mHardwareBufferManager;
        mHardwareBufferManager = 0;

        OGRE_DELETE mTextureManager;
        mTextureManager = 0;

        {
            vector<RenderTarget*>::type::const_iterator itor = mRenderTargets.begin();
            vector<RenderTarget*>::type::const_iterator end  = mRenderTargets.end();

            while( itor != end )
            {
                OGRE_DELETE *itor;
                ++itor;
            }

            mRenderTargets.clear();
        }
    }
    //-------------------------------------------------------------------------
    const String& NULLRenderSystem::getName(void) const
    {
        static String strName("NULL Rendering Subsystem");
        return strName;
    }
    //-------------------------------------------------------------------------
    const String& NULLRenderSystem::getFriendlyName(void) const
    {
        static String strName("NULL_RS");
        return strName;
    }
    //-------------------------------------------------------------------------
    HardwareOcclusionQuery* NULLRenderSystem::createHardwareOcclusionQuery(void)
    {
        return 0; //TODO
    }
    //-------------------------------------------------------------------------
    RenderSystemCapabilities* NULLRenderSystem::createRenderSystemCapabilities(void) const
    {
        RenderSystemCapabilities* rsc = new RenderSystemCapabilities();
        rsc->setRenderSystemName(getName());

        rsc->setCapability(RSC_HWSTENCIL);
        rsc->setStencilBufferBitDepth(8);
        rsc->setNumTextureUnits(16);
        rsc->setCapability(RSC_ANISOTROPY);
        rsc->setCapability(RSC_AUTOMIPMAP);
        rsc->setCapability(RSC_BLENDING);
        rsc->setCapability(RSC_DOT3);
        rsc->setCapability(RSC_CUBEMAPPING);
        rsc->setCapability(RSC_TEXTURE_COMPRESSION);
        rsc->setCapability(RSC_TEXTURE_COMPRESSION_DXT);
        rsc->setCapability(RSC_VBO);
        rsc->setCapability(RSC_TWO_SIDED_STENCIL);
        rsc->setCapability(RSC_STENCIL_WRAP);
        rsc->setCapability(RSC_USER_CLIP_PLANES);
        rsc->setCapability(RSC_VERTEX_FORMAT_UBYTE4);
        rsc->setCapability(RSC_INFINITE_FAR_PLANE);
        rsc->setCapability(RSC_TEXTURE_3D);
        rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);
        rsc->setNonPOW2TexturesLimited(false);
        rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);
        rsc->setCapability(RSC_TEXTURE_FLOAT);
        rsc->setCapability(RSC_POINT_SPRITES);
        rsc->setCapability(RSC_POINT_EXTENDED_PARAMETERS);
        rsc->setCapability(RSC_TEXTURE_2D_ARRAY);
        rsc->setCapability(RSC_CONST_BUFFER_SLOTS_IN_SHADER);
        rsc->setMaxPointSize(256);

        rsc->setMaximumResolutions( 16384, 4096, 16384 );

        return rsc;
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::reinitialise(void)
    {
        this->shutdown();
        this->_initialise(true);
    }
    //-------------------------------------------------------------------------
    RenderWindow* NULLRenderSystem::_initialise( bool autoCreateWindow, const String& windowTitle )
    {
        RenderWindow *autoWindow = 0;
        if( autoCreateWindow )
            autoWindow = _createRenderWindow( windowTitle, 1, 1, false );
        RenderSystem::_initialise(autoCreateWindow, windowTitle);

        return autoWindow;
    }
    //-------------------------------------------------------------------------
    RenderWindow* NULLRenderSystem::_createRenderWindow( const String &name,
                                                         unsigned int width, unsigned int height,
                                                         bool fullScreen,
                                                         const NameValuePairList *miscParams )
    {
        RenderWindow *win = OGRE_NEW NULLRenderWindow();

        if( !mInitialized )
        {
            mRealCapabilities = createRenderSystemCapabilities();
            mCurrentCapabilities = mRealCapabilities;

            mHardwareBufferManager = new v1::DefaultHardwareBufferManager();
            mTextureManager = new NULLTextureManager();
            mVaoManager = OGRE_NEW NULLVaoManager();

            mInitialized = true;
        }

        return win;
    }
    //-------------------------------------------------------------------------
    MultiRenderTarget* NULLRenderSystem::createMultiRenderTarget(const String & name)
    {
        return 0;
    }
    //-------------------------------------------------------------------------
    String NULLRenderSystem::getErrorDescription(long errorNumber) const
    {
        return BLANKSTRING;
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_useLights(const LightList& lights, unsigned short limit)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setWorldMatrix(const Matrix4 &m)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setViewMatrix(const Matrix4 &m)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setProjectionMatrix(const Matrix4 &m)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setSurfaceParams( const ColourValue &ambient,
                            const ColourValue &diffuse, const ColourValue &specular,
                            const ColourValue &emissive, Real shininess,
                            TrackVertexColourType tracking )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setPointSpritesEnabled(bool enabled)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setPointParameters(Real size, bool attenuationEnabled,
        Real constant, Real linear, Real quadratic, Real minSize, Real maxSize)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::queueBindUAV( uint32 slot, TexturePtr texture,
                                         ResourceAccess::ResourceAccess access,
                                         int32 mipmapLevel, int32 textureArrayIndex,
                                         PixelFormat pixelFormat )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::queueBindUAV( uint32 slot, UavBufferPacked *buffer,
                                         ResourceAccess::ResourceAccess access,
                                         size_t offset, size_t sizeBytes )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::clearUAVs(void)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::flushUAVs(void)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_bindTextureUavCS( uint32 slot, Texture *texture,
                                              ResourceAccess::ResourceAccess access,
                                              int32 mipmapLevel, int32 textureArrayIndex,
                                              PixelFormat pixelFormat )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setTextureCS( uint32 slot, bool enabled, Texture *texPtr )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setHlmsSamplerblockCS( uint8 texUnit, const HlmsSamplerblock *samplerblock )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setTexture(size_t unit, bool enabled,  Texture *texPtr)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setTextureCoordSet(size_t unit, size_t index)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setTextureCoordCalculation( size_t unit, TexCoordCalcMethod m,
                                                        const Frustum* frustum )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setTextureBlendMode(size_t unit, const LayerBlendModeEx& bm)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setTextureMatrix(size_t unit, const Matrix4& xform)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setIndirectBuffer( IndirectBufferPacked *indirectBuffer )
    {
    }
    //-------------------------------------------------------------------------
    DepthBuffer* NULLRenderSystem::_createDepthBufferFor( RenderTarget *renderTarget,
                                                          bool exactMatchFormat )
    {
        return 0;
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_beginFrame(void)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_endFrame(void)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setViewport(Viewport *vp)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setHlmsSamplerblock( uint8 texUnit, const HlmsSamplerblock *Samplerblock )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setPipelineStateObject( const HlmsPso *pso )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setComputePso( const HlmsComputePso *pso )
    {
    }
    //-------------------------------------------------------------------------
    VertexElementType NULLRenderSystem::getColourVertexElementType(void) const
    {
        return VET_COLOUR_ARGB;
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_dispatch( const HlmsComputePso &pso )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setVertexArrayObject( const VertexArrayObject *vao )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_render( const CbDrawCallIndexed *cmd )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_render( const CbDrawCallStrip *cmd )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_renderEmulated( const CbDrawCallIndexed *cmd )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_renderEmulated( const CbDrawCallStrip *cmd )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setRenderOperation( const v1::CbRenderOp *cmd )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_render( const v1::CbDrawCallIndexed *cmd )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_render( const v1::CbDrawCallStrip *cmd )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::bindGpuProgramParameters(GpuProgramType gptype,
        GpuProgramParametersSharedPtr params, uint16 variabilityMask)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::bindGpuProgramPassIterationParameters(GpuProgramType gptype)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::clearFrameBuffer( unsigned int buffers, const ColourValue& colour,
                                             Real depth, unsigned short stencil )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::discardFrameBuffer( unsigned int buffers )
    {
    }
    //-------------------------------------------------------------------------
    Real NULLRenderSystem::getHorizontalTexelOffset(void)
    {
        return 0.0f;
    }
    //-------------------------------------------------------------------------
    Real NULLRenderSystem::getVerticalTexelOffset(void)
    {
        return 0.0f;
    }
    //-------------------------------------------------------------------------
    Real NULLRenderSystem::getMinimumDepthInputValue(void)
    {
        return 0.0f;
    }
    //-------------------------------------------------------------------------
    Real NULLRenderSystem::getMaximumDepthInputValue(void)
    {
        return 1.0f;
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::_setRenderTarget(RenderTarget *target, uint8 viewportRenderTargetFlags)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::preExtraThreadsStarted()
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::postExtraThreadsStarted()
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::registerThread()
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::unregisterThread()
    {
    }
    //-------------------------------------------------------------------------
    const PixelFormatToShaderType* NULLRenderSystem::getPixelFormatToShaderType(void) const
    {
        return &mPixelFormatToShaderType;
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::beginProfileEvent( const String &eventName )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::endProfileEvent( void )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::markProfileEvent( const String &event )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::initGPUProfiling(void)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::deinitGPUProfiling(void)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::beginGPUSampleProfile( const String &name, uint32 *hashCache )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::endGPUSampleProfile( const String &name )
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::setClipPlanesImpl(const PlaneList& clipPlanes)
    {
    }
    //-------------------------------------------------------------------------
    void NULLRenderSystem::initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary)
    {
    }
}
