// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreTinyRenderSystem.h"

#include "OgreRenderSystem.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreTinyTextureManager.h"
#include "OgreHardwareBuffer.h"
#include "OgreGpuProgramManager.h"
#include "OgreException.h"
#include "OgreTinyDepthBuffer.h"
#include "OgreTinyHardwarePixelBuffer.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreRoot.h"
#include "OgreConfig.h"
#include "OgreViewport.h"
#include "OgreTinyWindow.h"
#include "OgreTinyTexture.h"

#include "tinyrenderer.h"

namespace Ogre {
    TinyRenderSystem::TinyRenderSystem()
        : mShaderManager(0),
          mHardwareBufferManager(0)
    {
        LogManager::getSingleton().logMessage(getName() + " created.");

        initConfigOptions();

        // create params
        GpuLogicalBufferStructPtr nullPtr;
        GpuLogicalBufferStructPtr logicalBufferStruct(new GpuLogicalBufferStruct());
        mFixedFunctionParams.reset(new GpuProgramParameters);
        mFixedFunctionParams->_setLogicalIndexes(logicalBufferStruct, nullPtr, nullPtr);
        mFixedFunctionParams->setAutoConstant(0, GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
        mFixedFunctionParams->setAutoConstant(4, GpuProgramParameters::ACT_TEXTURE_MATRIX);
        mFixedFunctionParams->setAutoConstant(8, GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR);
        mFixedFunctionParams->setAutoConstant(9, GpuProgramParameters::ACT_LIGHT_POSITION);
        mFixedFunctionParams->setAutoConstant(10, GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX);

        mActiveRenderTarget = 0;
        mGLInitialised = false;
    }


    void TinyRenderSystem::applyFixedFunctionParams(const GpuProgramParametersPtr& params, uint16 mask)
    {
        // Autoconstant index is not a physical index
        for (const auto& ac : params->getAutoConstants())
        {
            // Only update needed slots
            if (ac.variability & mask)
            {
                const float* ptr = params->getFloatPointer(ac.physicalIndex);
                switch(ac.paramType)
                {
                case GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX:
                    mDefaultShader.uniform_MVP = Matrix4(ptr);
                    break;
                case GpuProgramParameters::ACT_TEXTURE_MATRIX:
                    mDefaultShader.uniform_Tex = Matrix4(ptr);
                    break;
                case GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR:
                    memcpy(mDefaultShader.uniform_ambientCol.ptr(), ptr, sizeof(float)*4);
                    break;
                case GpuProgramParameters::ACT_LIGHT_POSITION:
                    // ptr[3] ? LIGHT_POINT : LIGHT_DIRECTIONAL;
                    mDefaultShader.uniform_lightDir = ptr[3] ? Vector3(ptr).normalisedCopy() : -Vector3(ptr);
                    break;
                case GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX:
                    mDefaultShader.uniform_MVIT = Matrix4(ptr);
                    break;
                default:
                    // ignore
                    break;
                }
            }
        }
    }

    TinyRenderSystem::~TinyRenderSystem()
    {
        shutdown();
    }

    const String& TinyRenderSystem::getName(void) const
    {
        static String strName("Tiny Rendering Subsystem");
        return strName;
    }
    void TinyRenderSystem::initConfigOptions()
    {
        RenderSystem::initConfigOptions();

        ConfigOption opt;
        opt.name = "Video Mode";
        opt.immutable = false;
        opt.possibleValues.push_back("800 x 600");
        opt.currentValue = opt.possibleValues[0];

        mOptions[opt.name] = opt;
    }
    RenderSystemCapabilities* TinyRenderSystem::createRenderSystemCapabilities() const
    {
        RenderSystemCapabilities* rsc = OGRE_NEW RenderSystemCapabilities();

        rsc->setDriverVersion(mDriverVersion);

        rsc->setRenderSystemName(getName());
        rsc->setNumTextureUnits(1);

        rsc->setCapability(RSC_FIXED_FUNCTION);

        // Vertex Buffer Objects are always supported
        rsc->setCapability(RSC_MAPBUFFER);

        // GL always shares vertex and fragment texture units (for now?)
        rsc->setVertexTextureUnitsShared(true);

        // Blending support
        rsc->setCapability(RSC_ADVANCED_BLEND_OPERATIONS);

        // Check for non-power-of-2 texture support
        rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);

        // Scissor test is standard
        rsc->setCapability(RSC_SCISSOR_TEST);

        // As are user clipping planes
        rsc->setCapability(RSC_USER_CLIP_PLANES);

        // So are 1D & 3D textures
        rsc->setCapability(RSC_TEXTURE_1D);
        rsc->setCapability(RSC_TEXTURE_3D);
        rsc->setCapability(RSC_TEXTURE_2D_ARRAY);

        // UBYTE4 always supported
        rsc->setCapability(RSC_VERTEX_FORMAT_UBYTE4);
        rsc->setCapability(RSC_32BIT_INDEX);

        // Infinite far plane always supported
        rsc->setCapability(RSC_INFINITE_FAR_PLANE);
        rsc->setCapability(RSC_DEPTH_CLAMP);

        rsc->setCapability(RSC_VERTEX_BUFFER_INSTANCE_DATA);

        // Check for Float textures
        rsc->setCapability(RSC_TEXTURE_FLOAT);

        rsc->setCapability(RSC_VERTEX_TEXTURE_FETCH);

        return rsc;
    }

    void TinyRenderSystem::initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary)
    {
        if (caps->getRenderSystemName() != getName())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Trying to initialize TinyRenderSystem from RenderSystemCapabilities that do not support Tiny");
        }

        mShaderManager = new GpuProgramManager();
        ResourceGroupManager::getSingleton()._registerResourceManager(mShaderManager->getResourceType(),
                                                                      mShaderManager);

        // Use VBO's by default
        mHardwareBufferManager = new DefaultHardwareBufferManager();

        // Create the texture manager
        mTextureManager = new TinyTextureManager();

        mGLInitialised = true;
    }

    void TinyRenderSystem::shutdown(void)
    {
        RenderSystem::shutdown();

        OGRE_DELETE mHardwareBufferManager;
        mHardwareBufferManager = 0;

        OGRE_DELETE mTextureManager;
        mTextureManager = 0;

        OGRE_DELETE mShaderManager;
        mShaderManager = 0;

        mGLInitialised = 0;
    }

    RenderWindow* TinyRenderSystem::_createRenderWindow(const String &name, unsigned int width, unsigned int height,
                                                           bool fullScreen, const NameValuePairList *miscParams)
    {
        RenderSystem::_createRenderWindow(name, width, height, fullScreen, miscParams);

        // Create the window
        RenderWindow* win = new TinyWindow();
        win->create(name, width, height, fullScreen, miscParams);
        attachRenderTarget(*win);

        if (!mGLInitialised)
        {
            LogManager::getSingleton().logMessage("**************************************");
            LogManager::getSingleton().logMessage("***     Tiny Renderer Started      ***");
            LogManager::getSingleton().logMessage("**************************************");

            // Initialise GL after the first window has been created
            // TODO: fire this from emulation options, and don't duplicate Real and Current capabilities
            mRealCapabilities = createRenderSystemCapabilities();

            // use real capabilities if custom capabilities are not available
            if (!mUseCustomCapabilities)
                mCurrentCapabilities = mRealCapabilities;

            fireEvent("RenderSystemCapabilitiesCreated");

            initialiseFromRenderSystemCapabilities(mCurrentCapabilities, (RenderTarget *) win);
        }

        if ( win->getDepthBufferPool() != DepthBuffer::POOL_NO_DEPTH )
        {
            // Unlike D3D9, OGL doesn't allow sharing the main depth buffer, so keep them separate.
            // Only Copy does, but Copy means only one depth buffer...
            auto *depthBuffer = new TinyDepthBuffer( DepthBuffer::POOL_DEFAULT,
                                                                      win->getWidth(), win->getHeight(),
                                                                      win->getFSAA(), true );

            mDepthBufferPool[depthBuffer->getPoolId()].push_back( depthBuffer );

            win->attachDepthBuffer( depthBuffer );
        }

        return win;
    }


    DepthBuffer* TinyRenderSystem::_createDepthBufferFor( RenderTarget *rt )
    {
        // No "custom-quality" multisample for now in GL
        return new TinyDepthBuffer(0, rt->getWidth(), rt->getHeight(), rt->getFSAA(), false);
    }

    MultiRenderTarget* TinyRenderSystem::createMultiRenderTarget(const String & name)
    {
        return NULL;
    }

    void TinyRenderSystem::_setTexture(size_t stage, bool enabled, const TexturePtr &texPtr)
    {
        if(stage > 0)
            return;

        if(!enabled || !texPtr)
        {
            mDefaultShader.image = NULL;
            return;
        }

        mDefaultShader.image = static_cast<TinyTexture*>(texPtr.get())->getImage();
    }

    void TinyRenderSystem::_setSampler(size_t unit, Sampler& sampler)
    {
    }

    void TinyRenderSystem::_setAlphaRejectSettings(CompareFunction func, unsigned char value, bool alphaToCoverage)
    {
    }

    void TinyRenderSystem::_setViewport(Viewport *vp)
    {
        // Check if viewport is different
        if (!vp)
        {
            mActiveViewport = NULL;
            _setRenderTarget(NULL);
        }

        else if (vp != mActiveViewport || vp->_isUpdated())
        {
            RenderTarget* target;

            target = vp->getTarget();
            _setRenderTarget(target);
            mActiveViewport = vp;

            // Calculate the "lower-left" corner of the viewport
            Rect vpRect = vp->getActualDimensions();
            if (!target->requiresTextureFlipping())
            {
                // Convert "upper-left" corner to "lower-left"
                std::swap(vpRect.top, vpRect.bottom);
                vpRect.top = target->getHeight() - vpRect.top;
                vpRect.bottom = target->getHeight() - vpRect.bottom;
            }

            mVP.makeTransform({vpRect.left + vpRect.width() / 2.f, vpRect.top + vpRect.height() / 2.f, 0.5},
                              {vpRect.width() / 2.f, vpRect.height() / 2.f, 0.5}, Ogre::Quaternion::IDENTITY);

            vp->_clearUpdatedFlag();
        }
    }

    void TinyRenderSystem::_endFrame(void)
    {
    }

    void TinyRenderSystem::_setCullingMode(CullingMode mode)
    {
        mCullingMode = mode;
    }

    void TinyRenderSystem::_setDepthBufferParams(bool depthTest, bool depthWrite, CompareFunction depthFunction)
    {
        mDepthTest = depthTest;
        mDepthWrite = depthWrite;
    }

    void TinyRenderSystem::_setDepthBias(float constantBias, float slopeScaleBias)
    {

    }

    void TinyRenderSystem::setColourBlendState(const ColourBlendState& state)
    {
        mBlendAdd = state.destFactor == SBF_ONE;
    }

    HardwareOcclusionQuery* TinyRenderSystem::createHardwareOcclusionQuery(void)
    {
        return NULL;
    }

    void TinyRenderSystem::_setPolygonMode(PolygonMode level)
    {

    }

    void TinyRenderSystem::DefaultShader::vertex(const vec4& vertex, const vec2* uv, const vec3* normal,
                                                 int gl_VertexID, vec4& gl_Position)
    {
        gl_Position = uniform_MVP * vertex;

        if(uv)
            var_uv[gl_VertexID] = (uniform_Tex*vec4(uv->x, uv->y, 0, 1)).xy();

        if(normal)
            var_normal[gl_VertexID] = uniform_MVIT.linear() * *normal;
    }
    bool TinyRenderSystem::DefaultShader::fragment(const vec3& bar, ColourValue& gl_FragColor)
    {
        if(image)
        {
            vec2 uv = var_uv[0]*bar.x + var_uv[1]*bar.y + var_uv[2]*bar.z;

            const vec4b& tex = sample2D(*image, uv);

            if(tex[3] < 1)
                return true;

            gl_FragColor = ColourValue(tex.ptr());
        }

        if(uniform_doLighting)
        {
            vec3 n = var_normal[0]*bar.x + var_normal[1]*bar.y + var_normal[2]*bar.z;
            float diffuse = std::max(0.f, n.dotProduct(uniform_lightDir));
            gl_FragColor *= diffuse;
            gl_FragColor += uniform_ambientCol;
        }

        return false;
    }

    static uchar* getData(const RenderOperation& op, VertexElementSemantic sem, size_t& step)
    {
        auto element = op.vertexData->vertexDeclaration->findElementBySemantic(sem);
        if(!element)
            return NULL;

        step = op.vertexData->vertexDeclaration->getVertexSize(element->getSource());

        auto buf = op.vertexData->vertexBufferBinding->getBuffer(element->getSource());
        uchar* ret = (uchar*)buf->lock(HardwareBuffer::HBL_NORMAL);
        buf->unlock(); // no real locking performed
        return ret + element->getOffset() + op.vertexData->vertexStart * step;
    }

    void TinyRenderSystem::_render(const RenderOperation& op)
    {
        // Call super class.
        RenderSystem::_render(op);

        bool isStrip = op.operationType == RenderOperation::OT_TRIANGLE_STRIP;

        if(!isStrip && op.operationType != RenderOperation::OT_TRIANGLE_LIST) // only triangle list/ strip supported
            return;

        size_t posStep;
        auto posData = getData(op, VES_POSITION, posStep);
        OgreAssert(posData, "VES_POSITION required");

        size_t uvStep = 0;
        auto uvData = getData(op, VES_TEXTURE_COORDINATES, uvStep);

        size_t normStep = 0;
        uchar* normData = getData(op, VES_NORMAL, normStep);;

        mDefaultShader.uniform_doLighting &= bool(normData);

        int16* idx16Data = NULL;
        int32* idx32Data = NULL;
        size_t drawCount = op.vertexData->vertexCount;
        if (op.useIndexes)
        {
            if(op.indexData->indexBuffer->getIndexSize() == 2)
            {
                idx16Data = (int16*)op.indexData->indexBuffer->lock(HardwareBuffer::HBL_NORMAL);
                idx16Data += op.indexData->indexStart;
            }
            else
            {
                idx32Data = (int32*)op.indexData->indexBuffer->lock(HardwareBuffer::HBL_NORMAL);
                idx32Data += op.indexData->indexStart;
            }
            op.indexData->indexBuffer->unlock();
            drawCount = op.indexData->indexCount;
        }

        Vector3f* v = NULL;
        Vector2* uv = NULL;
        Vector3f* n = NULL;
        vec4 clip_vert[3]; // triangle coordinates (clip coordinates), written by VS, read by FS
        do
        {
            for(size_t i = 0; i < drawCount; i += 3)
            {
                if (i && isStrip)
                    i -= 2;
                for(int j= 0; j < 3; j++)
                {
                    int idx = i + j;
                    idx = idx16Data ? idx16Data[idx] : (idx32Data ? idx32Data[idx] : idx);
                    v = (Vector3f*)(posData + posStep*idx);
                    uv = (Vector2*)(uvData + uvStep*idx);
                    n = (Vector3f*)(normData + normStep*idx);
                    mDefaultShader.vertex(vec4(*v), uv, n, j, clip_vert[j]);
                }
                triangle(mVP, clip_vert, mDefaultShader, *mActiveColourBuffer, *mActiveDepthBuffer,
                            mDepthTest, mDepthWrite, mBlendAdd, !isStrip);
            }

        } while (updatePassIterationRenderState());
    }

    void TinyRenderSystem::setScissorTest(bool enabled, const Rect& rect)
    {

    }

    void TinyRenderSystem::clearFrameBuffer(unsigned int buffers,
                                               const ColourValue& colour,
                                               Real depth, unsigned short stencil)
    {
        if (buffers & FBT_COLOUR)
        {
            mActiveColourBuffer->setTo(colour);
        }
        if (buffers & FBT_DEPTH)
        {
            mActiveDepthBuffer->setTo(ColourValue(depth));
        }
    }

    void TinyRenderSystem::_setRenderTarget(RenderTarget *target)
    {
        mActiveRenderTarget = target;

        if (!target)
            return;

        if(auto win = dynamic_cast<TinyWindow*>(target))
        {
            mActiveColourBuffer = win->getImage();
            mActiveDepthBuffer = dynamic_cast<TinyDepthBuffer*>(win->getDepthBuffer())->getImage();
        }

        // Check the depth buffer status
        auto *depthBuffer = target->getDepthBuffer();

        if ( target->getDepthBufferPool() != DepthBuffer::POOL_NO_DEPTH &&
                (!depthBuffer) )
        {
            // Depth is automatically managed and there is no depth buffer attached to this RT
            // or the Current context doesn't match the one this Depth buffer was created with
            setDepthBufferFor( target );
        }
    }
}
