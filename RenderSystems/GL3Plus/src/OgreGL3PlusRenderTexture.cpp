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

#include "OgreGL3PlusRenderTexture.h"
#include "OgreGL3PlusHardwarePixelBuffer.h"

namespace Ogre {

    const String GL3PlusRenderTexture::CustomAttributeString_FBO = "FBO";
    const String GL3PlusRenderTexture::CustomAttributeString_TARGET = "TARGET";
    const String GL3PlusRenderTexture::CustomAttributeString_GLCONTEXT = "GLCONTEXT";

    template<> GL3PlusRTTManager* Singleton<GL3PlusRTTManager>::msSingleton = 0;

    GL3PlusRTTManager::~GL3PlusRTTManager()
    {
    }

    MultiRenderTarget* GL3PlusRTTManager::createMultiRenderTarget(const String & name)
    {
        // TODO: Check rendersystem capabilities before throwing the exception
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "MultiRenderTarget is not supported",
                    "GL3PlusRTTManager::createMultiRenderTarget");
    }

    PixelFormat GL3PlusRTTManager::getSupportedAlternative(PixelFormat format)
    {
        if (checkFormat(format))
        {
            return format;
        }

        /// Find first alternative
        PixelComponentType pct = PixelUtil::getComponentType(format);

        switch (pct)
        {
        case PCT_BYTE:
            format = PF_A8R8G8B8;
            break;
        case PCT_SHORT:
            format = PF_SHORT_RGBA;
            break;
        case PCT_FLOAT16:
            format = PF_FLOAT16_RGBA;
            break;
        case PCT_FLOAT32:
            format = PF_FLOAT32_RGBA;
            break;
        case PCT_COUNT:
        default:
            break;
        }

        if (checkFormat(format))
            return format;

        /// If none at all, return to default
        return PF_A8R8G8B8;
    }

    GL3PlusRenderTexture::GL3PlusRenderTexture(const String &name,
                                               const GL3PlusSurfaceDesc &target,
                                               bool writeGamma,
                                               uint fsaa)
        : RenderTexture(target.buffer, target.zoffset)
    {
        mName = name;
        mHwGamma = writeGamma;
        mFSAA = fsaa;
    }

    GL3PlusRenderTexture::~GL3PlusRenderTexture()
    {
    }


}
