// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#include "OgreGLDepthBufferCommon.h"
#include "OgreGLHardwarePixelBufferCommon.h"
#include "OgreGLRenderSystemCommon.h"
#include "OgreGLRenderTexture.h"

namespace Ogre
{
GLDepthBufferCommon::GLDepthBufferCommon(GLRenderSystemCommon* renderSystem, GLContext* creatorContext,
                                         GLHardwarePixelBufferCommon* depth,
                                         GLHardwarePixelBufferCommon* stencil, const RenderTarget* target,
                                         bool manual)
    : DepthBuffer(target->getDepthBufferPool(), target->getWidth(), target->getHeight(), target->getFSAA(), manual),
      mCreatorContext(creatorContext), mDepthBuffer(depth), mStencilBuffer(stencil),
      mRenderSystem(renderSystem)
{
}

GLDepthBufferCommon::~GLDepthBufferCommon()
{
    if (mStencilBuffer && mStencilBuffer != mDepthBuffer)
    {
        delete mStencilBuffer;
        mStencilBuffer = 0;
    }

    if (mDepthBuffer)
    {
        delete mDepthBuffer;
        mDepthBuffer = 0;
    }
}

bool GLDepthBufferCommon::isCompatible(RenderTarget* renderTarget) const
{
    // Check standard stuff first.
    if (!DepthBuffer::isCompatible(renderTarget))
        return false;

    // Now check this is the appropriate format
    auto fbo = dynamic_cast<GLRenderTarget*>(renderTarget)->getFBO();
    if (!fbo)
        return false;

    PixelFormat internalFormat = fbo->getFormat();
    uint32 depthFormat, stencilFormat;
    mRenderSystem->_getDepthStencilFormatFor(internalFormat, &depthFormat, &stencilFormat);

    bool bSameDepth = false;

    if (mDepthBuffer)
        bSameDepth |= mDepthBuffer->getGLFormat() == depthFormat;

    bool bSameStencil = false;

    if (!mStencilBuffer || mStencilBuffer == mDepthBuffer)
        bSameStencil = stencilFormat == 0; // GL_NONE
    else
    {
        if (mStencilBuffer)
            bSameStencil = stencilFormat == mStencilBuffer->getGLFormat();
    }

    return PixelUtil::isDepth(internalFormat) ? bSameDepth : (bSameDepth && bSameStencil);
}
} // namespace Ogre
